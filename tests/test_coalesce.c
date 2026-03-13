#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define OPS 100000          // total allocations
#define MIN_ALLOC 32
#define MAX_ALLOC 512

int main() {
    void* ptrs[OPS];
    srand(time(NULL));

    double start = 0.0, end = 0.0;
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    start = ts.tv_sec + ts.tv_nsec * 1e-9;

    // Phase 1: allocate blocks
    for(int i = 0; i < OPS; i++) {
        size_t size = MIN_ALLOC + rand() % (MAX_ALLOC - MIN_ALLOC + 1);
        ptrs[i] = malloc(size);
        if(!ptrs[i]) exit(1);
        // optional write to memory
        ((char*)ptrs[i])[0] = 'A';
        ((char*)ptrs[i])[size-1] = 'Z';
    }

    // Phase 2: free every other block to create fragmentation
    for(int i = 0; i < OPS; i += 2)
        free(ptrs[i]);

    // Phase 3: allocate new blocks in the freed holes
    for(int i = 0; i < OPS; i += 2) {
        size_t size = MIN_ALLOC + rand() % (MAX_ALLOC - MIN_ALLOC + 1);
        ptrs[i] = malloc(size);
        if(!ptrs[i]) exit(1);
    }

    // Phase 4: free all blocks
    for(int i = 0; i < OPS; i++)
        free(ptrs[i]);

    clock_gettime(CLOCK_MONOTONIC, &ts);
    end = ts.tv_sec + ts.tv_nsec * 1e-9;

    // print only total elapsed time
    printf("%f\n", end - start);

    return 0;
}