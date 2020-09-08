#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "barrier.h"

void barrier_init(struct barrier_t *barrier, int nthreads)
{
    barrier->numThreads = nthreads;
    barrier->waitCount = 0;
    lock_init(&(barrier->lockNum));
    lock_init(&(barrier->lockSecond));
    cond_init(&(barrier->conditionThreadWait));
    return;
}

void barrier_wait(struct barrier_t *barrier)
{
    int isMaster = 1;

    // REFER: https://www.geeksforgeeks.org/condition-wait-signal-multi-threading/
    lock_acquire(&(barrier->lockNum));
    if(barrier->waitCount == 0) {
        barrier->waitCount = 1;

        lock_acquire(&(barrier->lockSecond));
        // printf("\nMASTER: thread ID = %d ---> %d, %d\n", pthread_self(), barrier->numThreads, barrier->waitCount);  // DEBUG
        while(1 <= barrier->waitCount && barrier->waitCount < barrier->numThreads) {
            // printf("(%d, %d)\n", barrier->waitCount, barrier->numThreads); fflush(stdout);  // DEBUG
            cond_wait(&(barrier->conditionThreadWait), &(barrier->lockNum));
        }
        // printf("\nDONE\n"); fflush(stdout);  // DEBUG
        lock_release(&(barrier->lockSecond));
    } else {
        isMaster = 0;
        ++(barrier->waitCount);
        cond_signal(&(barrier->conditionThreadWait), &(barrier->lockNum));
        // printf("* "); fflush(stdout);  // DEBUG
    }
    lock_release(&(barrier->lockNum));

    if(!isMaster) {
        // * ASSUMING: the below lock and unlock take negligible time
        // * AVAILABLE time to lock and unlock "barrier->lockSecond"
        //   by all threads except master = (2.5 - 1) = 1.5 seconds

        while(1 <= barrier->waitCount && barrier->waitCount <= barrier->numThreads) sleep(1);
        // printf("a"); fflush(stdout);  // DEBUG
        lock_acquire(&(barrier->lockSecond));
        lock_release(&(barrier->lockSecond));
        // printf("b\n"); fflush(stdout);  // DEBUG
        sleep(1.5);
    } else {
        barrier->waitCount = 0;
        sleep(2.5);
    }

    return;
}
