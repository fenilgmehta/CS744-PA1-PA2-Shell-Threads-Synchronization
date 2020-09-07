#include "cs_thread.h"

struct Repositioning {
	char player; 		// T for turtle and H for hare
	int time; 		// At what time god interrupt's
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
	struct Repositioning* reposition;	// pointer to array containing Randomizer's decision
	int repositioning_count;			// number of elements in array of repositioning structure
	
	//	Add your custom variables here.

	
	
};


void* Turtle(void *race);
void* Hare(void *race);
void* Randomizer(void *race);
void* Report(void *race);

char init(struct race *race)
{
	
	return '-'; 
}

void* Turtle(void *arg)
{
	
	return NULL;
  
}

void* Hare(void *arg)
{
	
	return NULL;
}


void* Randomizer(void *arg)
{
	
	return NULL;
}

void* Report(void *arg)
{
	
	return NULL;
}

