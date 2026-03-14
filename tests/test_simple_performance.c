#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <time.h>
#include <getopt.h>

/** Test allocation and free with fixed size */


#define OPS_PER_THREAD 10000
#define ALLOC_SIZE 64

typedef struct 
{
    int thread_id;
} thread_arg_t;

void* worker(void* arg)
{
    void* ptrs[OPS_PER_THREAD];

    for(int i = 0; i < OPS_PER_THREAD; i++) 
    {
        ptrs[i] = malloc(ALLOC_SIZE);
        if(!ptrs[i]) {
            fprintf(stderr, "malloc failed in thread %d\n", ((thread_arg_t*)arg)->thread_id);
            exit(1);
        }
    }

    for(int i = 0; i < OPS_PER_THREAD; i++)
        free(ptrs[i]);

    return NULL;
}

double get_time() 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char* argv[]) 
{
    int num_threads = 1; // default

    // parse options
    int opt;
    while((opt = getopt(argc, argv, "t:")) != -1) 
    {
        switch(opt) 
        {
            case 't':
                num_threads = atoi(optarg);
                if(num_threads <= 0) num_threads = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-t num_threads]\n", argv[0]);
                exit(1);
        }
    }

    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
    thread_arg_t* args = malloc(sizeof(thread_arg_t) * num_threads);

    double start = get_time();

    // spawn threads
    for(int i = 0; i < num_threads; i++) 
    {
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    // join threads
    for(int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    double end = get_time();

    printf("%f\n", end - start);

    free(threads);
    free(args);

    return 0;
}