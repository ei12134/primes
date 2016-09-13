#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "circularqueue.h"

/*
* Allocates space for circular queue 'q' having 'capacity' number of elements.
* Initializes semaphores & mutex needed to implement the producer-consumer paradigm.
* Initializes indexes of the head and tail of the queue.
*/
int queue_init(CircularQueue **q, unsigned int capacity)
{
    if ((*q = (CircularQueue *)malloc(sizeof(CircularQueue))) == NULL)
    {
        perror("Failure in malloc()");
        return -1;
    }
    if (sem_init(&((*q)->empty), SHARED, capacity) == -1)
    {
        perror("Failure in sem_init()");
        return -1;
    }
    if (sem_init(&((*q)->full), SHARED, 0) == -1)
    {
        perror("Failure in sem_init()");
        return -1;
    }
    if ((pthread_mutex_init(&((*q)->mutex), NULL)) != 0)
    {
        perror("Failure in pthread_mutex_init()");
        return -1;
    };

    if (((*q)->v = (QueueElem *)malloc(capacity * sizeof(QueueElem))) == NULL)
    {
        perror("Failure in malloc()");
        return -1;
    }
    
    /* Initialize queue variables */
    (*q)->capacity = capacity;
    (*q)->first = 0;
    (*q)->last = 0;

    return 0;
}

/* Inserts 'value' at the tail of queue 'q' */
void queue_put(CircularQueue *q, QueueElem value)
{
    sem_wait(&q->empty);

    pthread_mutex_lock(&q->mutex);
    q->v[q->last++] = value;
    q->last %= q->capacity;
    pthread_mutex_unlock(&q->mutex);

    sem_post(&q->full);
}

/* Removes element at the head of queue 'q' and returns its 'value' */
QueueElem queue_get(CircularQueue *q)
{
    sem_wait(&q->full);
    pthread_mutex_lock(&q->mutex);
    QueueElem value = q->v[q->first++];

    if (q->first == q->capacity)
        q->first = 0;

    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->empty);

    return value;
}

/*
* Frees space allocated for the queue elements and auxiliary management data
* Must be called when the queue is no more needed
*/
void queue_destroy(CircularQueue *q)
{
    free(q->v);
    pthread_mutex_destroy(&q->mutex);
    sem_destroy(&q->empty);
    sem_destroy(&q->full);
    free(q);
}
