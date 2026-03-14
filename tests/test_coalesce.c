#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define OPS_PER_THREAD 10000      
#define ALLOC_SIZE 64

typedef struct 
{
    int thread_id;
    int ops;
} thread_arg_t;

// Worker function: allocates, frees, and reallocates blocks
void* worker(void* arg) 
{
    thread_arg_t* t = (thread_arg_t*)arg;
    void* ptrs[t->ops];

    // Phase 1: allocate blocks
    for(int i = 0; i < t->ops; i++) 
    {
        size_t size = ALLOC_SIZE;
        ptrs[i] = malloc(size);
        if(!ptrs[i]) exit(1);
    }

    // Phase 2: free every other block to create fragmentation
    for(int i = 0; i < t->ops; i += 2)
        free(ptrs[i]);


    // Phase 3: allocate new blocks to test coalescing/reuse
    for(int i = 0; i < t->ops; i += 2) 
    {
        size_t size = ALLOC_SIZE;
        ptrs[i] = malloc(size);
        if(!ptrs[i]) exit(1);
    }

    // Phase 4: free all remaining blocks
    for(int i = 0; i < t->ops; i++)
        free(ptrs[i]);

    return NULL;
}

// Get current time in seconds
double get_time() 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char* argv[]) 
{
    int num_threads = 1;  // default
    int ops = OPS_PER_THREAD;

    // Parse command line options: -t threads, -n ops per thread
    int opt;
    while((opt = getopt(argc, argv, "t:n:")) != -1) 
    {
        switch(opt) 
        {
            case 't': num_threads = atoi(optarg); break;
            case 'n': ops = atoi(optarg); break;
            default:
                fprintf(stderr, "Usage: %s [-t num_threads] [-n ops_per_thread]\n", argv[0]);
                exit(1);
        }
    }

    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
    thread_arg_t* args = malloc(sizeof(thread_arg_t) * num_threads);

    srand(time(NULL));

    double start = get_time();

    for(int i = 0; i < num_threads; i++) 
    {
        args[i].thread_id = i;
        args[i].ops = ops;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    for(int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    double end = get_time();

    printf("%f\n", end - start);  // total elapsed time

    free(threads);
    free(args);
    return 0;
}