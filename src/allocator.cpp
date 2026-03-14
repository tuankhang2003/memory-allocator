#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <atomic>
#include <stdlib.h>
/*----------------------------*/

/* >= 128KB -> use mmap */
#define MAX_REQUEST_IN_MEMORY (128 * 1024)
#define ALIGNMENT 8
#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define MIN_BLOCK_SIZE 32
#define NUM_BINS 16
#define NUM_ARENA 7
#define PAGE_SIZE 4096

struct Block
{
    /* size including header; LSB = free flag */
    size_t size_and_flags;
    Block *prev;
    Block *next;
};

struct List
{
    pthread_mutex_t lock;
    Block *head;
    Block *end;
    pthread_t threadID;
};

/** Global Arena List */
static List *arenas;

/** local arena for thread */
__thread List *local_arena = NULL;
/** local index of global arenas for thread */
__thread int local_idx = 0;

static std::atomic<int> arena_counter{0};
static pthread_mutex_t global_brk_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t once = PTHREAD_ONCE_INIT;

/** get size of 1 block */
static inline size_t block_get_size(Block *b)
{
    return b->size_and_flags & ~(size_t)1;
}

/** return if block is free  */
static inline int block_is_free(Block *b, List *arena)
{
    if (!b)
        return 0;
    return (b->size_and_flags & (size_t)1) != 0;
}

/** set size, flag for block */
static inline void block_set_size_and_free(Block *b, size_t size, bool is_free)
{
    b->size_and_flags = size | (is_free ? 1 : 0);
}

/** mark allocated flag for block */
static inline void mark_allocated(Block *block)
{
    block_set_size_and_free(block, block_get_size(block), false);
}

/** check if block can merge with next block */
static bool can_merge_with_next(Block *block, List *a)
{
    Block *next = block->next;
    if (next == nullptr)
        return false;
    return block_is_free(next, a);
}

/** Get Block from payload */
static Block *get_block(void *payload)
{
    return (Block *)((char *)payload - sizeof(Block));
}

/** create global arenas list, only created once */
static void init_arena_once()
{
    arenas = (List *)mmap(nullptr,
                          sizeof(List) * NUM_ARENA,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS,
                          -1, 0);
    if (arenas == MAP_FAILED)
    {
        return;
    }
    memset(arenas, 0, sizeof(List));
    // Initialize each List
    for (int i = 0; i < NUM_ARENA; i++)
    {
        pthread_mutex_init(&arenas[i].lock, nullptr);
        arenas[i].head = nullptr;
        arenas[i].end = nullptr;
    }
}

/** assign local arena for thread */
static inline void assign_arena_for_thread()
{
    if (local_arena)
    {
        return;
    }
    // Compute index for arena
    int idx = arena_counter.fetch_add(1, std::memory_order_relaxed) % NUM_ARENA;
    local_arena = &arenas[idx];
    local_arena->threadID = pthread_self();
    local_idx = idx;
    // fprintf(stderr, "Thread %lu has index %d\n", (unsigned long)pthread_self(), local_idx);

    // fprintf(stderr, "Thread ID: %lu with index %d\n", (unsigned long)pthread_self(), idx);
}

/** Comnpute size of block + payload with alignment */
static inline size_t total_block_size_of_payload(size_t payload_size)
{
    size_t header_size = sizeof(Block);
    size_t total = header_size + ALIGN_UP(payload_size, ALIGNMENT);
    if (total < MIN_BLOCK_SIZE)
        total = MIN_BLOCK_SIZE;
    size_t result = ALIGN_UP(total, ALIGNMENT);
    return result;
}

/**
 * IMPLEMENTATION
 */

/** malloc using mmap for large malloc size request */
static void *mmap_alloc(size_t size)
{
    size_t total_size = ALIGN_UP(size + sizeof(Block), PAGE_SIZE);
    void *p = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED)
        return NULL;
    Block *b = (Block *)p;
    b->prev = nullptr;
    b->next = nullptr;
    /*marked allocated */
    block_set_size_and_free(b, total_size, false);
    /**
     * return payload pointer after header
     * sizeof(block_t) = size of metadata (size_and_flags + prev_free + next_free) */
    return (void *)((char *)b + sizeof(Block));
}
/** free of mmap malloc */
static void mmap_free(void *payload)
{
    if (!payload)
        return;
    // access metadata
    Block *b = (Block *)((char *)payload - sizeof(Block));
    size_t total_size = block_get_size(b);
    munmap((void *)b, total_size);
}

static Block *request_more_space(size_t size, List *a)
{
    size_t req = ALIGN_UP(size, PAGE_SIZE);
    // fprintf(stderr, "[Thread %lu] locked in request_more_space\n", (unsigned long)pthread_self());
    // fprintf(stderr, "request more space: %zu, req: %zu\n", size, req);
    //  requires more memory
    pthread_mutex_lock(&global_brk_lock);
    void *p = sbrk(req);
    if (p == (void *)-1)
    {
        pthread_mutex_unlock(&global_brk_lock);
        return NULL;
    }
    pthread_mutex_unlock(&global_brk_lock);

    /** Set newBlock and update the arena list */
    Block *newBlock = (Block *)p;
    block_set_size_and_free(newBlock, req, 1);
    newBlock->next = nullptr;
    newBlock->prev = nullptr;

    // 1. No blocks existed before → initialize start and end
    if (a->head == NULL)
    {
        a->head = newBlock;
        a->end = newBlock;
    }
    else
    {
        // Append to the end of the heap list
        newBlock->prev = a->end;
        a->end->next = newBlock;
        a->end = newBlock;
    }
    // fprintf(stderr, "[Thread %lu] unlocked in request_more_space\n", (unsigned long)pthread_self());

    return newBlock;
}

/** Find block has enough size with first fit */
static Block *find_fit(size_t size, List *arena)
{
    for (Block *current = arena->head; current != nullptr; current = current->next)
    {
        size_t current_size = block_get_size(current);
        if (block_is_free(current, arena) && current_size >= size)
        {
            return current;
        }
    }
    return nullptr;
}

/**
 * Split a large free block into a block of the requested size
 * and a remaining free block. Updates links in the arena's block list.
 */
static void split_block(Block *block, size_t needed, List *a)
{
    size_t size = block_get_size(block);
    /*Cannot split (min size of next block) */
    if (size < needed + sizeof(Block) + MIN_BLOCK_SIZE)
    {
        // write(2, "cannot split block\n", 20);
        return;
    }
    /* create new block at offset needed */
    /*|<------ needed ---->|<------ remainder ------>|
                           ^
                           newb */
    size_t left_size = size - needed;
    char *split_pos = (char *)block + needed;
    Block *newb = (Block *)split_pos;
    /* adjust current block */
    block_set_size_and_free(block, needed, true);
    block_set_size_and_free(newb, left_size, true);
    newb->next = block->next;
    block->next = newb;
    newb->prev = block;
    if (a->end == block)
    {
        a->end = newb;
    }
}

static void merge_block_with_next(Block *block, List *a)
{
    // write(2, "merge with next\n", 17);
    Block *next = block->next;
    if (next == nullptr || !block_is_free(next, a))
    {
        return;
    }
    size_t total_size = block_get_size(block) + block_get_size(next);
    block_set_size_and_free(block, total_size, true);

    // safely update links
    block->next = next->next;
    if (next->next)
        next->next->prev = block;

    if (next == a->end)
        a->end = block;
}

/**
 * Allocates a memory block from the current thread's arena.
 *
 * This function is called internally by malloc() after the thread
 * has been assigned an arena. It acquires the arena lock, calculates
 * the total block size required for the requested payload (including
 * metadata), and attempts to allocate memory from the arena.
 *
 * Thread safety:
 *  - The arena mutex must be held while manipulating arena structures.
 *
 * @param size Requested payload size in bytes.
 * @return Pointer to the allocated memory block, or nullptr if allocation fails.
 */

void *malloc_from_arena(size_t size)
{
    pthread_mutex_lock(&local_arena->lock);
    size_t total_request = total_block_size_of_payload(size);
    Block *block = find_fit(total_request, local_arena);
    // require more space
    if (block == nullptr)
    {
        /** Trying to merge with end block (if free) to reduce fragmentation */
        if (local_arena->end != nullptr && block_is_free(local_arena->end, local_arena))
        {
            Block *old_end_block = local_arena->end;
            size_t needed = total_request - block_get_size(local_arena->end);
            block = request_more_space(needed > 0 ? needed : MIN_BLOCK_SIZE, local_arena);
            if (block == nullptr)
            {
                pthread_mutex_unlock(&local_arena->lock);
                return nullptr;
            }

            merge_block_with_next(old_end_block, local_arena);
            local_arena->end = block = old_end_block;
        }
        else
        {
            block = request_more_space(total_request > 0 ? total_request : MIN_BLOCK_SIZE, local_arena);
        }
    }
    split_block(block, total_request, local_arena);
    mark_allocated(block);
    pthread_mutex_unlock(&local_arena->lock);
    return (void *)((char *)block + sizeof(Block));
}

extern "C"
{
    void *malloc(size_t size)
    {
        if (size == 0)
            size = 1;
        if (size > MAX_REQUEST_IN_MEMORY)
            return mmap_alloc(size);
        pthread_once(&once, init_arena_once);
        if (local_arena == nullptr)
        {
            assign_arena_for_thread();
        }
        return malloc_from_arena(size);
    }

    void free(void *ptr)
    {
        if (ptr == nullptr || local_arena == nullptr)
        {
            return;
        }
        pthread_mutex_lock(&local_arena->lock);

        Block *block = get_block(ptr);
        size_t block_size = block_get_size(block);

        if (block_size >= MAX_REQUEST_IN_MEMORY)
        {
            pthread_mutex_unlock(&local_arena->lock);
            mmap_free(ptr);
            return;
        }

        block_set_size_and_free(block, block_size, true);

        while (block && can_merge_with_next(block, local_arena))
        {
            merge_block_with_next(block, local_arena);
        }
        if (!block->next)
        {
            local_arena->end = block;
        }

        Block *prev = block->prev;
        while (prev && block_is_free(prev, local_arena) && can_merge_with_next(prev, local_arena))
        {
            merge_block_with_next(prev, local_arena);
            block = block->prev;
        }
        if (!block->prev)
        {
            local_arena->head = block;
        }

        pthread_mutex_unlock(&local_arena->lock);
    }
}