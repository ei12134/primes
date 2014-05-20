/* Compile using debug flag [-DDEBUG] for error messages  */

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circularqueue.h"
#include "utils.h"

unsigned int *primes, currentPrimes, maxPrimes, size;

void *thr_func(void *arg);

long int parse_long(char *str, int base) {

	char *endptr;
	long int number = strtol(str, &endptr, base);

	// Check for various possible errors
	if ((errno == ERANGE && (number == LONG_MAX || number == LONG_MIN))
			|| (errno != 0 && number == 0)) {
		perror("strtol");
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
	if (argc < 2) {
		printf("Usage: %s <upper number limit>.\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	// Safely convert limit string argument to a unsigned integer
	if ((size = parse_long(argv[1], 10)) == LONG_MAX) {
		printf("Enter a valid number list size number\n");
		exit (EXIT_FAILURE);
	}

	// Allocate memory for primes array
	if (size == 2)
		maxPrimes = 1;
	else
		maxPrimes = 1 + ((1.2 * (double) size) - 1) / log((double) size);

	primes = malloc(sizeof(unsigned int) * maxPrimes);
	if (primes == NULL) {
		fprintf(stderr, "Memory exhausted");
		exit (EXIT_FAILURE);
	}

	// Add first prime to the primes list
	primes[0] = 2;
	currentPrimes = 1;

	// Create circular queue if size > 2
	if (size > 2) {
		CircularQueue *q;
		if (queue_init(&q, size) == -1) {
			printf("Failed creating circular queue\n");
			exit (EXIT_FAILURE);
		}
		unsigned int i;
		for (i = 3; i <= size; i++) {
			if (i % 2 != 0) {
				queue_put(q, i);
			}
		}
		pthread_t tid;
		pthread_create(&tid, NULL, thr_func, q);
		pthread_join(tid, NULL);

	}
	// Signal the end of primes determination else{}

	// Sort primes list
	// quick_sort()...
	
	// Display primes list
	unsigned int i;
	printf("Primes in the range [1-%d]: ",size);
	for (i = 0; i < maxPrimes; i++)
		if(primes[i] != 0)
			printf("%d ", primes[i]);
	printf("\n");

	exit (EXIT_SUCCESS);
}

void *thr_func(void *arg) {

	CircularQueue *p = arg, *c;

	if (queue_init(&c, size) == -1) {
		printf("Failed creating circular queue\n");
		exit (EXIT_FAILURE);
	}

	// Add first queue number to the primes list as it is necessarily a prime
	unsigned int prime = queue_get(p);
	primes[currentPrimes++] = prime;

	// Stop checking for multiples of primes if first queue number greater than sqrt(N)
	if (prime > sqrt(size)) {
		unsigned int first;
		// Add all primes in queue to the primes list until 0 appears
		while ((first = queue_get(p)) != 0)
			primes[currentPrimes++] = first;
		queue_destroy(c);
	}

	// Create exit queue
	else {
		unsigned int first;
		// Add non multiple numbers to the new queue list until 0 appears
		while ((first = queue_get(p)) != 0) {
			if (first % prime != 0)
				queue_put(c, first);
		}
		pthread_t tid;
		pthread_create(&tid, NULL, thr_func, c);
		pthread_join(tid, NULL);
	}

	// Exit the thread
	queue_destroy(p);
	pthread_exit(0);
}
