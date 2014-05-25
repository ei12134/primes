/* Compile using debug flag [-DDEBUG] for error messages */
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "circularqueue.h"

#define QUEUE_SIZE 5
#define SHARED 0

unsigned int *primes, primesCounter;
long int threadCounter, size;
sem_t done;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thr_init(void *arg);
void *thr_filter(void *arg);

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

long int parse_long(char *str, int base) {
  
  char *endptr;
  long int number = strtol(str, &endptr, base);
  
  // Check for various possible errors
  if ((errno == ERANGE && (number == LONG_MAX || number == LONG_MIN)) || (errno != 0 && number == 0)) {
    perror("Failure in strtol()");
    return LONG_MAX;
  }
  
  if (endptr == str) {
    #ifdef DEBUG
    fprintf(stderr, "No digits were found\n");
    #endif
    return LONG_MAX;
  }
  
  if (*endptr != '\0') {
    #ifdef DEBUG
    fprintf(stderr, "Non digit char found\n");
    #endif
    return LONG_MAX;
  }
  
  if (number < 2) {
    #ifdef DEBUG
    fprintf(stderr, "Non positive number\n");
    #endif
    return LONG_MAX;
  }
  #ifdef DEBUG
  printf("strtol() successfuly returned %ld\n", number);
  #endif
  
  // Successful conversion
  return number;
}

int main(int argc, char *argv[]) {
  
  // Verify arguments validity
  if (argc != 2) {
    printf("Usage: %s <upper number limit>.\n", argv[0]);
    exit (EXIT_FAILURE);
  }
  
  // Safely convert limit string argument to a unsigned integer
  if ((size = parse_long(argv[1], 10)) == LONG_MAX) {
    printf("Enter a valid number list size number\n");
    exit (EXIT_FAILURE);
  }
  
  // Allocate memory for primes array 
  unsigned int maxPrimes;
  if (size == 2)
    maxPrimes = 1;
  else
    maxPrimes = 1 + ((1.2 * (double) size) - 1) / log((double) size);
  
  if ( (primes = malloc(sizeof(unsigned int) * maxPrimes)) == NULL ){
    perror("Failure in malloc()");
    exit(EXIT_FAILURE);
  }
  
  if (sem_init(&done, SHARED, 0) == -1) {
    perror("Failure in sem_init()");
    exit(EXIT_FAILURE);
  }
  
  // Create main thread
  pthread_t tid;
  pthread_create(&tid, NULL, thr_init, NULL);
  
  // Wait for signal at the end of primes determination
  sem_wait(&done);
  
  // Uses a conditional variable to wait for all threads to finish
  pthread_mutex_lock(&mutex);
  while (threadCounter > 0)
    pthread_cond_wait(&cond, &mutex);
  pthread_mutex_unlock(&mutex);
  
  // Sort primes list
  qsort(primes, primesCounter, sizeof(unsigned int), compare);
  
  // Display primes list
  unsigned int i;
  printf("Primes in the range [1-%ld]: ", size);
  for (i = 0; i < primesCounter; i++) {
    printf("%d ", primes[i]);
  }
  printf("\n");
  
  // Exit the primes program
  pthread_cond_destroy(&cond);
  pthread_mutex_destroy(&mutex);
  exit(EXIT_SUCCESS);
}

void *thr_init(void *arg){
  
  // Add first prime to the primes list
  primes[0] = 2;
  primesCounter = 1;
  
  // Create circular queue if size > 2
  if (size > 2) {
    CircularQueue *q;
    if (queue_init(&q, QUEUE_SIZE) == -1) {
      printf("Failed creating circular queue\n");
      exit (EXIT_FAILURE);
    }
    // Create filter thread
    pthread_t tid;
    pthread_create(&tid, NULL, thr_filter, q);
    
    pthread_mutex_lock(&mutex);
    threadCounter = 2;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    
    // Put all odd numbers into the queue
    unsigned int i;
    for (i = 3; i <= size; i++){
      if (i % 2 != 0){	
	queue_put(q, i);
      }
    }
    queue_put(q, 0);
  }
  else
    sem_post(&done);
  
  pthread_mutex_lock(&mutex);
  threadCounter--;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  
  // Exit the thread
  pthread_exit(EXIT_SUCCESS);
}

void *thr_filter(void *arg) {
  
  CircularQueue *input = arg, *output;
  // Add first queue number to the primes list as it is necessarily a prime
  unsigned int first, prime = queue_get(input);
  pthread_mutex_lock(&mutex);
  primes[primesCounter++] = prime;
  pthread_mutex_unlock(&mutex);
  
  // Stop checking for multiples of primes if first queue number greater than sqrt(N)
  if (prime > sqrt(size)) {
    // Add all primes in queue to the primes list until 0 appears
    while ((first = queue_get(input)) != 0){
      pthread_mutex_lock(&mutex);
      primes[primesCounter++] = first; 
      pthread_mutex_unlock(&mutex);
    }
    sem_post(&done);
  }
  else {
    // Initialize output thread
    if (queue_init(&output, QUEUE_SIZE) == -1) {
      printf("Failed creating output circular queue\n");
      exit (EXIT_FAILURE);
    }
    // Create next thread
    pthread_t tid;
    pthread_create(&tid, NULL, thr_filter, output);
    
    pthread_mutex_lock(&mutex);
    threadCounter++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    
    // Add non multiple numbers to the new queue list until 0 appears
    while ((first = queue_get(input)) != 0){   
      if (first % prime != 0){
	queue_put(output, first);
      }
    } 
    queue_put(output,0);	
  }
  
  // Exit the thread
  queue_destroy(input);
  pthread_mutex_lock(&mutex);
  threadCounter--;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  
  pthread_exit(EXIT_SUCCESS);
}