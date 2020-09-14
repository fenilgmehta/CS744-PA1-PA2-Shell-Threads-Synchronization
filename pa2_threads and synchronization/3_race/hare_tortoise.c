#include "cs_thread.h"

// --------------------------------------------------------------------

#define printflush(a, ...) { fprintf(a, __VA_ARGS__); fflush(a); }

struct my_barrier_t {
    long int numThreads, waitCount;
    struct lock lockMaster, lockOne, lockSecondAndLater;
    struct condition conditionThreadWait;
};

void my_barrier_init(struct my_barrier_t *barrier, int nthreads) {
    barrier->numThreads = nthreads;
    barrier->waitCount = 0;
    lock_init(&(barrier->lockMaster));
    lock_init(&(barrier->lockOne));
    lock_init(&(barrier->lockSecondAndLater));
    cond_init(&(barrier->conditionThreadWait));
    return;
}

void my_barrier_wait(struct my_barrier_t *barrier) {
    // REFER: https://www.geeksforgeeks.org/condition-wait-signal-multi-threading/
    if(barrier->numThreads <= 1) return;

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
}

// --------------------------------------------------------------------

struct Repositioning {
	char player; 		// T for turtle and H for hare
	int time; 			// At what time god interrupt's
	int distance;		// How much does god move any of the player. 
							// distance can be negetive or positive.
							// using this distance if any of players position is less than zero then bring him to start line.
							// If more than finish_distance then make him win.
							// If time is after completion of game than you can ignore it as we will already have winner.
};

struct race {
	//	Don't change these variables.
	//	speeds are unit distance per unit time.
	int printing_delay;
	int tortoise_speed;					// speed of Turtle
	int hare_speed;						// speed of hare
	int hare_sleep_time; 				// how much time does hare sleep (in case he decides to sleep)
	int hare_turtle_distance_for_sleep; // minimum lead which hare has on turtle after which hare decides to move
										// Any lead greater than this distance and hare will ignore turtle and go to sleep
	int finish_distance;				// Distance between start and finish line

	// NOTE: It is given that the reposition array will be sorted in ascending order.
	//       Time will be 0 if the randomizer has to intervene before the race starts.
	//       The lowest value in Repositioning->time is zero (0)
	//       Can assume that reposition is sorted in strictly increasing order
	struct Repositioning* reposition;	// pointer to array containing Randomizer's decision
	int repositioning_count;			// number of elements in array of repositioning structure
	
	// Add your custom variables here.
	// NOTE: time unit starts from 0
	int mCurrentTime, mTurtlePosition, mHarePosition;
	char mCalculatedWinner;
	struct my_barrier_t mBarrierSync;
};

char check_winner(struct race *race) {
	// NOTE from PDF: ​If hare and turtle cross the finish line at the same
	// time unit then we’ll let the turtle win,because slow and steady wins
	// the race, in stories and in assignments!
	if(race->mTurtlePosition >= race->finish_distance) {
		race->mCalculatedWinner = 'T';
	} else if (race->mHarePosition >= race->finish_distance) {
		race->mCalculatedWinner = 'H';
	}
	return race->mCalculatedWinner;
}

// --------------------------------------------------------------------

void* Turtle(void *race);
void* Hare(void *race);
void* Randomizer(void *race);
void* Report(void *race);

char init(struct race *race) {
	race->mCurrentTime = race->mTurtlePosition = race->mHarePosition = 0;
	race->mCalculatedWinner = '\0';
	my_barrier_init(&(race->mBarrierSync), 5);
	// NOTE: that the barrier sync thread count is 5 due to: init, tortoise, hare, randomizer, reporter

	pthread_t thread_turtle, thread_hare, thread_randomizer, thread_report;

	if(pthread_create(&thread_turtle, NULL, Turtle, (void*) race) != 0) {
		printflush(stderr, "ERROR: unable to create the thread for Turtle\n");
	}
	if(pthread_create(&thread_hare, NULL, Hare, (void*) race) != 0) {
		printflush(stderr, "ERROR: unable to create the thread for Hare\n");
	}
	if(pthread_create(&thread_randomizer, NULL, Randomizer, (void*) race) != 0) {
		printflush(stderr, "ERROR: unable to create the thread for Randomizer\n");
	}
	if(pthread_create(&thread_report, NULL, Report, (void*) race) != 0) {
		printflush(stderr, "ERROR: unable to create the thread for Report\n");
	}

	check_winner(race);
	my_barrier_wait(&(race->mBarrierSync));  // barrier 1
	while(!(race->mCalculatedWinner)) {
		// synchronize all four threads

		my_barrier_wait(&(race->mBarrierSync));  // barrier 3
		// Randomizer works
		my_barrier_wait(&(race->mBarrierSync));  // barrier 4
		// Hare moves
		my_barrier_wait(&(race->mBarrierSync));  // barrier 5a
		// Turtle moves
		my_barrier_wait(&(race->mBarrierSync));  // barrier 5b
		// Increment race->mCurrentTime
		// Report the positions
		my_barrier_wait(&(race->mBarrierSync));  // barrier 6
		check_winner(race); // check winner
		// printflush(stderr, "WINNER val = %d\n", race->mCalculatedWinner);
		my_barrier_wait(&(race->mBarrierSync));  // barrier 7
	}
	
	// printflush(stderr, "WAITING for threads to exit\n");
	pthread_join(thread_turtle, NULL);
	pthread_join(thread_hare, NULL);
	pthread_join(thread_randomizer, NULL);
	pthread_join(thread_report, NULL);

	// return '-'; 
	return race->mCalculatedWinner;
}

void* Turtle(void *arg) {
	struct race *r = (struct race*) arg;
	my_barrier_wait(&(r->mBarrierSync));  // barrier 1

	while(!(r->mCalculatedWinner)) {
		// move at the specified speed every unit time

		my_barrier_wait(&(r->mBarrierSync));  // barrier 3
		my_barrier_wait(&(r->mBarrierSync));  // barrier 4
		my_barrier_wait(&(r->mBarrierSync));  // barrier 5a
		r->mTurtlePosition += r->tortoise_speed;
		my_barrier_wait(&(r->mBarrierSync));  // barrier 5b
		my_barrier_wait(&(r->mBarrierSync));  // barrier 6
		my_barrier_wait(&(r->mBarrierSync));  // barrier 7
	}

	// printflush(stderr, "----------Turtle exiting\n");
	return NULL;
}

void* Hare(void *arg) {
	// NOTE from above: Any lead greater than "hare_turtle_distance_for_sleep"
	//                  distance and hare will ignore turtle and go to sleep

	struct race *r = (struct race*) arg;
	my_barrier_wait(&(r->mBarrierSync));  // barrier 1

	int iSleep = r->hare_sleep_time;
	while(!(r->mCalculatedWinner)) {
		// if turtle is far off, sleep
		// else, move at specified speed once, then sleep

		my_barrier_wait(&(r->mBarrierSync));  // barrier 3
		my_barrier_wait(&(r->mBarrierSync));  // barrier 4
		if(iSleep == r->hare_sleep_time) {
			// printflush(stderr, "DEBUG: %d, %d, %d\n", iSleep, r->hare_sleep_time, r->mCurrentTime);
			if((r->mHarePosition - r->mTurtlePosition) <= r->hare_turtle_distance_for_sleep) {
				r->mHarePosition += r->hare_speed;
				// printflush(stderr, "*** DEBUG: %d, %d\n", r->mHarePosition, r->mCurrentTime);
			}
			iSleep = 0;
		} else {
			iSleep += 1;
			// printflush(stderr, "DEBUG 2: %d, %d\n", iSleep, r->mCurrentTime);
		}
		my_barrier_wait(&(r->mBarrierSync));  // barrier 5a
		my_barrier_wait(&(r->mBarrierSync));  // barrier 5b
		my_barrier_wait(&(r->mBarrierSync));  // barrier 6
		my_barrier_wait(&(r->mBarrierSync));  // barrier 7
	}
	// printflush(stderr, "----------Hare exiting\n");
	return NULL;
}


void* Randomizer(void *arg) {
	// NOTE from PDF: Randomizer must intervene and reposition a player at ​the BEGINNING of a
	//                particular time unit (i.e. when it's the randomizer’s turn to reposition)​

	struct race *r = (struct race*) arg;
	my_barrier_wait(&(r->mBarrierSync));  // barrier 1

	int indexRandomizer = 0;
	while(!(r->mCalculatedWinner)) {
		// if time to reposition, select hare or turtle and reposition

		my_barrier_wait(&(r->mBarrierSync));  // barrier 3
		while(indexRandomizer < r->repositioning_count && r->mCurrentTime == r->reposition[indexRandomizer].time) {
			if(r->reposition[indexRandomizer].player == 'T') {
				r->mTurtlePosition += r->reposition[indexRandomizer].distance;
				if(r->mTurtlePosition < 0) r->mTurtlePosition = 0;
			} else if (r->reposition[indexRandomizer].player == 'H') {
				r->mHarePosition += r->reposition[indexRandomizer].distance;
				if(r->mHarePosition < 0) r->mHarePosition = 0;
			} else {
				printflush(stderr, "ERROR: ignoring wrong repositioning request: %c\n", r->reposition[indexRandomizer].player);
			}
			indexRandomizer += 1;
		}
		my_barrier_wait(&(r->mBarrierSync));  // barrier 4
		my_barrier_wait(&(r->mBarrierSync));  // barrier 5a
		my_barrier_wait(&(r->mBarrierSync));  // barrier 5b
		my_barrier_wait(&(r->mBarrierSync));  // barrier 6
		my_barrier_wait(&(r->mBarrierSync));  // barrier 7
	}
	// printflush(stderr, "----------Randomizer exiting\n");
	return NULL;
}

void* Report(void *arg) {
	// NOTE from PDF: The current outcome of the race should be displayed
	//                AFTER each specified interval of Ntime units

	struct race *r = (struct race*) arg;
	my_barrier_wait(&(r->mBarrierSync));  // barrier 1

	while(!(r->mCalculatedWinner)) {
		// report the positions of hare and turtle in distance from
		// the start in specific time unit intervals given in input

		my_barrier_wait(&(r->mBarrierSync));  // barrier 3
		my_barrier_wait(&(r->mBarrierSync));  // barrier 4
		my_barrier_wait(&(r->mBarrierSync));  // barrier 5a
		my_barrier_wait(&(r->mBarrierSync));  // barrier 5b

		// increment the time
		r->mCurrentTime += 1;

		if(r->mCurrentTime % r->printing_delay == 0)
			printf("Time = %9d , Hare = %9d , Turtle = %9d\n", r->mCurrentTime, r->mHarePosition, r->mTurtlePosition);

		my_barrier_wait(&(r->mBarrierSync));  // barrier 6
		my_barrier_wait(&(r->mBarrierSync));  // barrier 7
	}

	// printflush(stderr, "----------Reporter exiting\n");
	return NULL;
}

#undef printflush
