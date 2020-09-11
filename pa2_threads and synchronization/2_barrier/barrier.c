#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "barrier.h"

void barrier_init(struct barrier_t *barrier, int nthreads)
{
    barrier->numThreads = nthreads;
    barrier->waitCount = 0;
    lock_init(&(barrier->lockMaster));
    lock_init(&(barrier->lockOne));
    lock_init(&(barrier->lockSecondAndLater));
    cond_init(&(barrier->conditionThreadWait));
    return;
}

void barrier_wait(struct barrier_t *barrier)
{
    // REFER: https://www.geeksforgeeks.org/condition-wait-signal-multi-threading/
    lock_acquire(&(barrier->lockMaster));
    ++(barrier->waitCount);
    if(barrier->waitCount == barrier->numThreads) {
        lock_acquire(&(barrier->lockOne));
        cond_signal(&(barrier->conditionThreadWait), &(barrier->lockOne));
        lock_release(&(barrier->lockOne));
        // NOTE: this loop ensures that NO thread is able to start the work for
        //       next call to barrier_wait(...) unless every thread except the last
        //       thread leaves this function.
        while(barrier->waitCount != 2) sched_yield();
        barrier->waitCount = 0;
        lock_release(&(barrier->lockMaster));
    } else if (barrier->waitCount == 1) {
        lock_acquire(&(barrier->lockSecondAndLater));
        lock_acquire(&(barrier->lockOne));

        lock_release(&(barrier->lockMaster));
        cond_wait(&(barrier->conditionThreadWait), &(barrier->lockOne));

        lock_release(&(barrier->lockOne));
        lock_release(&(barrier->lockSecondAndLater));
    } else {
        lock_release(&(barrier->lockMaster));
        lock_acquire(&(barrier->lockSecondAndLater));
        --(barrier->waitCount);  // NOTE: this statement will only decrement the waitCount "N-2" times
        lock_release(&(barrier->lockSecondAndLater));
    }
    return;
}
