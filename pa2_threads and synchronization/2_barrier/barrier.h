#include "cs_thread.h"

struct barrier_t
{
    // Barrier related variables
    int numThreads, waitCount;
    struct lock lockMaster, lockOne, lockSecondAndLater;
    struct condition conditionThreadWait;
};

void barrier_init(struct barrier_t *barrier, int nthreads);
void barrier_wait(struct barrier_t *barrier);
