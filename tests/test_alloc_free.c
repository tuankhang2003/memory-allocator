#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ALLOCS 6000
#define MIN_SIZE 2
#define MAX_SIZE 4096
#define ALIGNMENT 8


/** Test with simple, small alocation and free */

int is_aligned(void* p) 
{
    return ((size_t)p % ALIGNMENT) == 0;
}

int main() 
{
    void* blocks[NUM_ALLOCS];
    size_t sizes[NUM_ALLOCS];

    srand(time(NULL));

    /* allocate memory blocks */
    for (int i = 0; i < NUM_ALLOCS; i++) 
    {
        // allocate random size
        sizes[i] = MIN_SIZE + rand() % MAX_SIZE;
        blocks[i] = malloc(sizes[i]);

        if (!blocks[i]) 
        {
            fprintf(stderr, "Allocation failed for %zu bytes\n", sizes[i]);
            return 1;
        }

        if (!is_aligned(blocks[i])) 
        {
            fprintf(stderr, "Returned pointer is not aligned\n");
            return 1;
        }

        /* fill memory with a pattern */
        memset(blocks[i], (i % 255), sizes[i]);

        /* mark boundaries */
        ((char*)blocks[i])[0] = 'A';
        ((char*)blocks[i])[sizes[i] - 1] = 'Z';
    }

    /* verify memory content */
    for (int i = 0; i < NUM_ALLOCS; i++) 
    {
        char* ptr = (char*)blocks[i];
        if (ptr[0] != 'A' || ptr[sizes[i] - 1] != 'Z') 
        {
            fprintf(stderr, "Memory corruption detected\n");
            return 1;
        }
    }

    /* free memory */
    for (int i = 0; i < NUM_ALLOCS; i++) 
    {
        free(blocks[i]);
    }
    
    return 0;
}