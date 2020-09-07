#include <pthread.h>

struct barrier_t
{
    /*
        TODO
        Barrier related variables
    */
    
};

void barrier_init(struct barrier_t *b, int i);
void barrier_wait(struct barrier_t *b);