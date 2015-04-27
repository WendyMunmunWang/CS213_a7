#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "spinlock.h"
#include "uthread.h"
#define MAX_ITEMS 10

int items = 0;   //shared sourced pool 

const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
spinlock_t lock_items;  
spinlock_t lock_producer_wait_count;
spinlock_t lock_consumer_wait_count;  
void produce() {
  // TODO ensure proper synchronization
  while(1){
    while(items==MAX_ITEMS){
		spinlock_lock(lock_producer_wait_count);
		producer_wait_count++;
		spinlock_unlock(lock_producer_wait_count);
		
	}
	assert(items<MAX_ITEMS);
	spinlock_lock(lock_items);
	items++;
	spinlock_unlock(lock_items);
  }
}
   

void consume() {
  // TODO ensure proper synchronization
  while(1){
	  while(items==0){
		  spinlock_lock(lock_consumer_wait_count);
		  consumer_wait_count++;
		  spinlock_unlock(lock_consumer_wait_count);
	  }
	  assert(items>0);
	  spinlock_lock(lock_items);
	  items--;
	  spinlock_unlock(lock_items);
	 
  }
}

void* producer(void* p) { //producer thread should loop
  
  for (int i=0; i < NUM_ITERATIONS; i++){
     produce();
  }
  return NULL;
}

void* consumer(void* c) { //consumer thread should loop

  for (int i=0; i< NUM_ITERATIONS; i++){
	  consume();
	}
	return NULL;
  }


int main (int argc, char** argv) {
	uthread_t thread_con[2];
	uthread_t thread_pro[2];
    // TODO create threads to run the producers and consumers
    spinlock_create(&lock_items); //create a lock for items 
	spinlock_create(&lock_consumer_wait_count); //create a lock for consumers
	spinlock_create(&lock_producer_wait_count); //create a lock for producers
	uthread_init(4);        //initialize 4 threads
	for (int processors_con = 0; processors_con < 2; processors_con++){
		thread_con[processors_con] = uthread_create(consumer,NULL);
	}
	
	for (int processors_pro=0;processors_pro<2;processors_pro++){
		thread_pro[processors_pro]= uthread_create(producer,NULL);
	}
	uthread_join (thread_con[0], (void**)argv);
	uthread_join (thread_con[1], (void**)argv);
	uthread_join (thread_pro[0], (void**)argv);
	uthread_join (thread_pro[1], (void**)argv);
    printf("Producer wait: %d\nConsumer wait: %d\n",
         producer_wait_count, consumer_wait_count);
    for(int i=0;i<MAX_ITEMS+1;i++){
    printf("items %d count %d\n", i, histogram[i]);
}
}