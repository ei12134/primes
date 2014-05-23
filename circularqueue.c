#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "circularqueue.h"
#define SHARED 0
//------------------------------------------------------------------------------------------
// Allocates space for circular queue 'q' having 'capacity' number of elements
// Initializes semaphores & mutex needed to implement the producer-consumer paradigm
// Initializes indexes of the head and tail of the queue
// TO DO BY STUDENTS: ADD ERROR TESTS TO THE CALLS & RETURN a value INDICATING (UN)SUCESS
int queue_init(CircularQueue **q, unsigned int capacity) // TO DO: change return value
{
  if ((*q = (CircularQueue *) malloc(sizeof(CircularQueue))) == NULL) {
    fprintf(stderr, "Memory exhausted");
    return 1;
  }
  if (sem_init(&((*q)->empty), SHARED, capacity) == -1) {
    //..sets errno -> include errno.h?
    return 1;
  }
  if (sem_init(&((*q)->full), SHARED, 0) == -1) {
    //..
    return 1;
  }
  if ( (pthread_mutex_init(&((*q)->mutex), NULL)) != 0) {
    // ..
    return 1;
  };
  
  if (((*q)->v = (QueueElem *) malloc(capacity * sizeof(QueueElem))) == NULL){
    fprintf(stderr,
	    "Memory exhausted (malloc of %d bytes) - Try lower upper limit\n",
	    capacity);
    return 1;
  }
  (*q)->capacity = capacity;
  (*q)->first = 0;
  (*q)->last = 0;
  
  return 0;
}
//------------------------------------------------------------------------------------------
// Inserts 'value' at the tail of queue 'q'
void queue_put(CircularQueue *q, QueueElem value) {
  printf("1\n");
  sem_wait(&q->empty);
  pthread_mutex_lock(&q->mutex);
  if (q->last == q->capacity - 1) {
    printf("2\n");
    q->v[q->last] = 0;
    q->last = 0;
  } else {
    q->v[q->last] = value;
    q->last++;  
    printf("3\n");
  } 
  pthread_mutex_unlock(&q->mutex);
  sem_post(&q->full);
}
//------------------------------------------------------------------------------------------
// Removes element at the head of queue 'q' and returns its 'value'
QueueElem queue_get(CircularQueue *q) {
  printf("4\n");
  sem_wait(&q->full);
  printf("5\n");
  pthread_mutex_lock(&q->mutex);
  QueueElem value = q->v[q->first++];
  pthread_mutex_unlock(&q->mutex);
  printf("6\n");
  if (q->first == q->capacity){
    pthread_mutex_lock(&q->mutex);
    q->first = 0;
    pthread_mutex_unlock(&q->mutex);
    printf("7\n");
  }
  printf("8\n");
  sem_post(&q->empty);
  printf("9\n");
  
  return value;
}
//------------------------------------------------------------------------------------------
// Frees space allocated for the queue elements and auxiliary management data
// Must be called when the queue is no more needed
void queue_destroy(CircularQueue *q) {
  free(q->v);
  sem_destroy(&q->empty);
  sem_destroy(&q->full);
  free(q);
}