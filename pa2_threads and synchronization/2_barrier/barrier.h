#include <pthread.h>

struct barrier_t
{
    /*
        TODO
        Barrier related variables
    */
    int numThreads, waitCount;
    pthread_mutex_t numMutex;
    
};

void barrier_init(struct barrier_t *barrier, int nthreads);
void barrier_wait(struct barrier_t *barrier);
