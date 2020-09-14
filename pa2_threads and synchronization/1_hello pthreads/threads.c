#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define printflush(a, ...) { fprintf(a, __VA_ARGS__); fflush(a); }

// TODO: comment this before submitting the code
// #define PRINT_DEBUG_STATEMENTS

// ---------------------------------------------------------------------------------------------------------------------

/* 
 * This is the counter value which is to
 * be incremented by all the threads created 
 * by your program
 **/
// REFER: https://www.tutorialspoint.com/volatile-qualifier-in-c
// REFER: https://www.geeksforgeeks.org/understanding-volatile-qualifier-in-c/#:~:text=The%20volatile%20keyword%20is%20intended,current%20code%20at%20any%20time.
// REFER: https://stackoverflow.com/questions/2484980/why-is-volatile-not-considered-useful-in-multithreaded-c-or-c-programming
volatile int counter = 0;

pthread_mutex_t counter_mutex;

void* increment_counter(void *ptr) {
    if(pthread_mutex_lock(&counter_mutex) == (-1)) {
        printflush(stderr, "ERROR: unable to get ```counter_mutex``` lock\n");
        return NULL;
    }
    ++counter;
    if(pthread_mutex_unlock(&counter_mutex) == (-1)) {
        printflush(stderr, "ERROR: unable to unlock ```counter_mutex```\n");
    }
    return NULL;
}

int main(int argc, char const *argv[]) {
    /* NOTE: How to COMPILE: ``` gcc threads.c -lpthread ``` */
    
    /* Write you code to create n threads here */
    /* Each thread must increment the counter once and exit */
    char* charNull;
    int64_t n = strtol(argv[1], &charNull, 10);

#ifdef PRINT_DEBUG_STATEMENTS
    // REFER: https://www.tutorialspoint.com/c_standard_library/c_function_strtol.htm
    printflush(stdout, "n = %d\n", n);
#endif

    // REFER: BEST: https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
    if(pthread_mutex_init(&counter_mutex, NULL) == (-1)) {
        // returns: 0 on success, -1 on failure
        printflush(stderr, "ERROR: unable to initialize the mutex :(\n");
        printflush(stderr, "EXITING\n");
        exit(1);
    }

    pthread_t *threadArr = malloc(sizeof(pthread_t) * n);

    // REFER: BEST: https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html
    // REFER: BEST: https://man7.org/linux/man-pages/man7/pthreads.7.html
    // REFER: https://man7.org/linux/man-pages/man3/pthread_create.3.html
    for(int i = 0; i < n; ++i) {
        if(pthread_create(&threadArr[i], NULL, increment_counter, NULL) != 0) {
            printflush(stderr, "ERROR: unable to create the thread i=%d\n", i);
        }
    }

    // REFER: https://man7.org/linux/man-pages/man3/pthread_join.3.html
    for(int i = 0; i < n; ++i) {
        pthread_join(threadArr[i], NULL);
    }

    free(threadArr);

    if(pthread_mutex_destroy(&counter_mutex) == (-1)) {
        printflush(stderr, "ERROR: unable to destroy the mutex :(\n");
    }

    /* Make sure you reap all threads created by
     * your program before printing the counter */
    printf("%d\n", counter);
    fflush(stdout);

    // NOTE: do NOT remove this, this is used to evaluate the program
    //       and see if the threads spawned have been reaped
    sleep(10000);
    return 0;
}