#include <semaphore.h>
//------------------------------------------------------------------------------------------
// Type of the circular queue elements
typedef unsigned long  QueueElem;
//------------------------------------------------------------------------------------------
// Struct for representing a "circular queue"
// Space for the queue elements will be allocated dinamically by queue_init()
typedef struct
{
QueueElem *v; // pointer to the queue buffer
unsigned int capacity; // queue capacity
unsigned int first; // head of the queue
unsigned int last; // tail of the queue
sem_t empty; // semaphores and mutex for implementing the
sem_t full; // producer-consumer paradigm
pthread_mutex_t mutex;
}  CircularQueue;
//------------------------------------------------------------------------------------------
// Allocates space for circular queue 'q' having 'capacity' number of elements
// Initializes semaphores & mutex needed to implement the producer-consumer paradigm
// Initializes indexes of the head and tail of the queue
// TO DO BY STUDENTS: ADD ERROR TESTS TO THE CALLS & RETURN a value INDICATING (UN)SUCESS
int queue_init( CircularQueue **q, unsigned int capacity);  // TO DO: change return value
//------------------------------------------------------------------------------------------
// Inserts 'value' at the tail of queue 'q'
void queue_put( CircularQueue *q, QueueElem value);
//------------------------------------------------------------------------------------------
// Removes element at the head of queue 'q' and returns its 'value'
QueueElem queue_get( CircularQueue *q);
//------------------------------------------------------------------------------------------
// Frees space allocated for the queue elements and auxiliary management data
// Must be called when the queue is no more needed
void queue_destroy( CircularQueue *q);
//==========================================================================================
// EXAMPLE: Creation of a circular queue using queue_init()
#define QUEUE_SIZE 10 // TO DO: test your program using different queue sizes
// ...
// CircularQueue *q;
// queue_init(&q,QUEUE_SIZE);
// ...