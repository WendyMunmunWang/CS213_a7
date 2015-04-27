#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "uthread.h"
#include "queue.h"
#include "disk.h"

int sum = 0;

/**
 * Description of 1 pending read.  
 * Each pending read has one of these stored on the prq queue.
 */
struct PendingRead {
  // TODO
  char*               aBuf;
  int                 aSiz;
  int                 ablockno;
  void (*handler) (char*, int, int);
} pendingRead;

/**
 * Queue of pending reads.
 */
queue_t prq;

/**
 *when one reading is completed, the CPU will be interrupted and call handler to 
 *handle the information 
 */
void interruptServiceRoutine () {
  //called when disk fires an interrupt TODO 
  struct PendingRead* obj = queue_dequeue(&prq);
  obj->handler(obj->aBuf, obj->aSiz, obj->ablockno);
}

/**
 * schedule a read
 */
void asyncRead (char* buf, int nbytes, int blockno, void (*handler) (char*, int, int)) {
  // call disk_scheduleRead to schedule a read TODO
  struct PendingRead* temp = malloc (sizeof (struct PendingRead));
  temp->aBuf = buf;
  temp->aSiz = nbytes;
  temp->ablockno = blockno;
  temp->handler = handler; //save information about this pending request 
  queue_enqueue(&prq,temp);  //add the empty buf in the queue
  disk_scheduleRead(buf, nbytes, blockno); //actual reading 
}

/**
 * This is the read completion routine.  This is where you 
 * would place the code that processed the data you got back
 * from the disk.  In this case, we'll just verify that the
 * data is there. (complete a read, and check whether the data is correct)
 */
void handleRead (char* buf, int nbytes, int blockno) {
  assert (*((int*)buf) == blockno); //assert data in the disk is the same as blockno
  sum += *(((int*)buf)+1);
}

/**
 * Read numBlocks blocks from disk sequentially starting at block 0. 
 * read the block one by one 
 */
void run (int numBlocks) {
  for (int blockno = 0; blockno < numBlocks; blockno++) {
    // call asyncRead to schedule read TODO
	char buf [1000];
	asyncRead(buf,sizeof (buf),blockno,handleRead); //waiting queue
  }
  
  disk_waitForReads();
}

int main (int argc, char** argv) {
  static const char* usage = "usage: aRead numBlocks";
  int numBlocks = 0;
  
  if (argc == 2)
    numBlocks = strtol (argv [1], NULL, 10);
  if (argc != 2 || (numBlocks == 0 && errno == EINVAL)) {
    printf ("%s\n", usage);
    return EXIT_FAILURE;
  }
  
  uthread_init (1);
  disk_start   (interruptServiceRoutine);
  
  run (numBlocks);
  
  printf ("%d\n", sum);
}