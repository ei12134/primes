#ifndef _CIRCULARQUEUE_H_
#define _CIRCULARQUEUE_H_

#include <semaphore.h>
#include <stdbool.h>

#define SHARED 0
#define QUEUE_SIZE 300

typedef unsigned long QueueElem; /* type of the circular queue elements */

/* 
* Struct for representing a "circular queue".
* Space for the queue elements will be allocated dinamically by queue_init().
*/
typedef struct
{
    QueueElem *v;          /* pointer to the queue buffer */
    unsigned int capacity; /* queue capacity */
    unsigned int first;    /* head of the queue */
    unsigned int last;     /* tail of the queue */
    sem_t empty;           /* empty queue semaphore */
    sem_t full;            /* full queue semaphore */
    pthread_mutex_t mutex; /* critical section mutex */
} CircularQueue;

/* 
* Allocates space for circular queue 'q' having 'capacity' number of elements.
* Initializes semaphores & mutex needed to implement the producer-consumer paradigm.
* Initializes indexes of the head and tail of the queue.
*/
bool queue_init(CircularQueue **q, unsigned int capacity);

/* Inserts 'value' at the tail of queue 'q'. */
void queue_put(CircularQueue *q, QueueElem value);

/* Removes element at the head of queue 'q' and returns its 'value'. */
QueueElem queue_get(CircularQueue *q);

/* 
* Frees space allocated for the queue elements and auxiliary management data.
* Must be called when the queue is no longer needed.
*/
void queue_destroy(CircularQueue *q);

#endif