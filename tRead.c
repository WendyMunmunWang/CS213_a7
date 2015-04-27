#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "queue.h"
#include "disk.h"
#include "uthread.h"

int sum = 0;
queue_t prq;         //wendy
void interruptServiceRoutine () {
  // TODO
  uthread_t this_thread = queue_dequeue(&prq); //dequeue and return the sleeping THREAD
  uthread_unblock (this_thread);  //unblock the thread
  
}

void blockUntilComplete() {
  // TODO
  queue_enqueue(&prq,uthread_self());   
  uthread_block();                       //block the thread while it is reading 
 
  
}

void handleRead (char* buf, int nbytes, int blockno) {
  assert (*((int*)buf) == blockno);
  sum +=  *(((int*) buf) + 1);
}

/**
 * Struct provided as argument to readAndHandleBlock
 */
struct readAndHandleBlockArgs {
  char* buf;
  int   nbytes;
  int   blockno;
};

void* readAndHandleBlock (void* args_voidstar) {
  // TODO
  // Synchronously:
  //   (1) call disk_scheduleRead to request the block
  //   (2) wait read for it to complete
  //   (3) call handleRead
  struct readAndHandleBlockArgs* obj = args_voidstar;
  disk_scheduleRead(obj->buf,obj->nbytes,obj->blockno); //call disk_scheduleRead to request the block 
  blockUntilComplete();
  handleRead(obj->buf,obj->nbytes,obj->blockno); 
  free(obj->buf);
  free(obj);  //how do we get the information after the reading 
  return NULL;
  // read first, because it takes time to read so we block it first. And then 
  // when it finishes reading, we unblock it and put it into a queue. We call interrupt service 
  // routine to handle the data
  
  }

void run (int numBlocks) {
  uthread_t thread [numBlocks];
  for (int blockno = 0; blockno < numBlocks; blockno++) {
    // TODO
    // call readAndHandleBlock in a way that allows this
    // operation to be synchronous without stalling the CPU
	
	struct readAndHandleBlockArgs* temp = malloc(sizeof(readAndHandleBlock)); //create a struct
	temp->buf=malloc(8);
	temp->nbytes=8;
	temp->blockno=numBlocks;
	thread [blockno] = uthread_create(readAndHandleBlock, temp);//create a thread (the thread is made up of the struct)
  }
  for (int i=0; i<numBlocks; i++)
    uthread_join (thread [i], 0);

}

int main (int argc, char** argv) {
  static const char* usage = "usage: tRead numBlocks";
  int numBlocks = 0;
  if (argc == 2)
    numBlocks = strtol (argv [1], NULL, 10);
  if (argc != 2 || (numBlocks == 0 && errno == EINVAL)) {
    printf ("%s\n", usage);
    return EXIT_FAILURE;
  }
  
  uthread_init (1);
  disk_start   (interruptServiceRoutine);
  queue_init   (&prq);  //added
  run (numBlocks);
  printf ("%d\n", sum);
}