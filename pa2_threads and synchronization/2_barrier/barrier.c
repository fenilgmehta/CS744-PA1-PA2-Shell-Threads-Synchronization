#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "barrier.h"

void barrier_init(struct barrier_t *barrier, int nthreads)
{
    barrier->numThreads = barrier->waitCount = nthreads;
    pthread_mutex_init(&(barrier->numMutex), NULL);
    return;
}

void barrier_wait(struct barrier_t *barrier)
{
    pthread_mutex_lock(&(barrier->numMutex));
    ++(barrier->waitCount);
    pthread_mutex_unlock(&(barrier->numMutex));    

    // REFER: https://www.man7.org/linux/man-pages/man2/sched_yield.2.html
    while(barrier->waitCount < (2 * barrier->numThreads)) sched_yield();

    pthread_mutex_lock(&(barrier->numMutex));
    ++(barrier->waitCount);
    pthread_mutex_unlock(&(barrier->numMutex));

    // when all the threads are FREE from the above while loop and increment the waitCount,
    // this condition will become false and reset waitCount to numThreads (which too make 
    // the below while loop condition false)
    while((2 * barrier->numThreads) <= barrier->waitCount && barrier->waitCount < (3 * barrier->numThreads)) sched_yield();

    // pthread_mutex_lock(&(barrier->numMutex));
    // NOTE: lock not required here because all the threads will reset the value of waitCount to numThreads
    barrier->waitCount = barrier->numThreads;
    // pthread_mutex_unlock(&(barrier->numMutex));

    return;
}
