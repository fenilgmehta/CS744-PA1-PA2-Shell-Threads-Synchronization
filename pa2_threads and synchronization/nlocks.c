#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<stdint.h>

#define printflush(a, ...) { fprintf(a, __VA_ARGS__); fflush(a); }
const int32_t THOUSAND = 1000;

/* 
 * This is the data array which is to
 * be incremented by all the threads created 
 * by your program as specified in the problem state
*/
volatile int data[10];

pthread_mutex_t data_lock[10];

void* increment_data_1000(void *ptr) {
    // REFER: https://stackoverflow.com/questions/26805461/why-do-i-get-cast-from-pointer-to-integer-of-different-size-error
    // REFER: https://stackoverflow.com/questions/1845482/what-is-uintptr-t-data-type
    uintptr_t thread_num = (uintptr_t) ptr;
    // printflush(stderr, "THREAD num = %d\n", thread_num);  // TODO: DEBUG: remove this

    for(int i = 0; i < THOUSAND; ++i) {
        pthread_mutex_lock(&data_lock[thread_num]);
        ++data[thread_num];
        pthread_mutex_unlock(&data_lock[thread_num]);
    }

    return NULL;
}

int main(int argc, char const *argv[]) {
    /* NOTE: How to COMPILE: ``` gcc threads.c -lpthread ``` */

    /* Write you code to create 10 threads here*/
    /* Increment the data array as specified in the problem statement*/

    for(int i = 0; i < 10; ++i) {
        if(pthread_mutex_init(&data_lock[i], NULL) == (-1)) {
            // returns: 0 on success, -1 on failure
            printflush(stderr, "ERROR: unable to initialize the mutex i=%d\n", i);
            printflush(stderr, "EXITING\n");
            exit(1);
        }
    }

    pthread_t threadArr[10];
    for(int i = 0; i < 10; ++i) {
        if(pthread_create(&threadArr[i], NULL, increment_data_1000, (void *)((uintptr_t)i)) != 0) {
            printflush(stderr, "ERROR: unable to create the thread i=%d\n", i);
        }
    }

    for(int i = 0; i < THOUSAND; ++i) {
        for(int j = 0; j < 10; ++j) {
            pthread_mutex_lock(&data_lock[j]);
            ++data[j];
            pthread_mutex_unlock(&data_lock[j]);
        }
    }

    for(int i = 0; i < 10; ++i) {
        pthread_join(threadArr[i], NULL);
    }

    for(int i = 0; i < 10; ++i) {
        if(pthread_mutex_destroy(&data_lock[i]) == (-1)) {
            printflush(stderr, "ERROR: unable to destroy the mutex i=%d\n", i);
        }
    }

    /* Make sure you reap all threads created by your program
     before printing the counter*/
    for (int i = 0; i < 10; ++i) {
        printf("%d\n", data[i]);
    }
    fflush(stdout);
    
    sleep(10000);  // TODO: do we remove this
    return 0;
}