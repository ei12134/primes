#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "circularqueue.h"

/* Compile using debug flag [-DDEBUG] for error messages */
#ifdef DEBUG
#define DEBUG_PRINT(x, y) fprintf(x, "Debug: %s", y)
#else
#define DEBUG_PRINT(x, y) \
    do                    \
    {                     \
    } while (false)
#endif

QueueElem *primes;                                 /* prime numbers container */
QueueElem hard_upper_limit;                        /* hard upper prime limit */
QueueElem primes_index;                            /* total number of primes found */
sem_t done;                                        /* completion signalling semaphore */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;    /* conditional variable declaration */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* mutex variable declaration */

void *thr_init(void *arg);
void *thr_filter(void *arg);

void fatal_error(char *msg)
{
    fprintf(stderr, "Error: %s", msg);
    exit(EXIT_FAILURE);
}

QueueElem parse_unsigned_long(char *str, int base)
{
    char *endptr;

    /* Dont convert negative numbers */
    if (str[0] == '-')
    {
        DEBUG_PRINT(stderr, "Negative number\n");
        return ULONG_MAX;
    }

    /* Convert the string to an unsigned long integer */
    QueueElem number = strtoul(str, &endptr, base);

    if ((errno == ERANGE && number == ULONG_MAX) || (errno != 0 && number == 0))
    {
        perror("Failure in strtoul()");
        return ULONG_MAX;
    }

    if (endptr == str)
    {
        DEBUG_PRINT(stderr, "No digits were found\n");
        return ULONG_MAX;
    }

    if (*endptr != '\0')
    {
        DEBUG_PRINT(stderr, "Non digit was found\n");
        return ULONG_MAX;
    }

    if (number < 2)
    {
        DEBUG_PRINT(stderr, "Input number was < 2\n");
        return ULONG_MAX;
    }
    
    return number;
}

int main(int argc, char *argv[])
{
    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <upper prime number limit>.\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Safely convert string argument to an unsigned long integer */
    if ((hard_upper_limit = parse_unsigned_long(argv[1], 10)) == ULONG_MAX)
        fatal_error("Enter a valid upper prime number limit (> 1)\n");

    /* Variable initialization */
    QueueElem soft_upper_limit = 0;
    primes_index = 0;

    /* Predict the number of primes in the given range */
    if (hard_upper_limit != 2)
        soft_upper_limit = 1 + ((1.2 * (double)hard_upper_limit) - 1) / log((double)hard_upper_limit);

    /* Allocate memory for the primes array */
    if ((primes = (QueueElem *)malloc(sizeof(QueueElem) * soft_upper_limit)) == NULL)
    {
        perror("Failure in malloc()");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&done, SHARED, 0) == -1)
    {
        perror("Failure in sem_init()");
        exit(EXIT_FAILURE);
    }

    /* Create main thread */
    pthread_t tid;
    pthread_create(&tid, NULL, thr_init, NULL);

    /* Wait for the completion of all primes threads */
    sem_wait(&done);

    /* Free allocated memory */
    free(primes);

    /* Exit the primes program main thread */
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    pthread_exit(EXIT_SUCCESS);
}

void *thr_init(void *arg)
{
    primes[primes_index++] = 2; /* add first prime to the primes list */
    fprintf(stdout, "Primes in the range [1-%ld]: %lu", hard_upper_limit, primes[0]);

    /* Only use a circular queue when the hard upper limit input is > 2 */
    if (hard_upper_limit > 2)
    {
        CircularQueue *queue;
        if (!queue_init(&queue, QUEUE_SIZE))
            fatal_error("Failed creating circular queue\n");

        pthread_t tid;
        pthread_create(&tid, NULL, thr_filter, queue); /* create filter thread */

        /* Put all odd numbers into the queue */
        QueueElem i;
        for (i = 3; i <= hard_upper_limit; i++)
        {
            if (i % 2 != 0)
                queue_put(queue, i);
        }
        queue_put(queue, 0);
    }
    else
        sem_post(&done);

    pthread_exit(EXIT_SUCCESS); /* successfully exit the thread */
}

void *thr_filter(void *arg)
{
    CircularQueue *input = (CircularQueue *)arg; /* input queue */
    CircularQueue *output;                       /* output queue */

    /* Add the first queue number to the primes list */
    QueueElem queue_head, current_prime = queue_get(input);

    pthread_mutex_lock(&mutex);
    primes[primes_index++] = current_prime;
    fprintf(stdout, " %lu", current_prime);
    pthread_mutex_unlock(&mutex);

    /* Stop checking for multiples of primes if first queue number greater than sqrt(N) */
    if (current_prime > sqrt(hard_upper_limit))
    {
        /* Add all primes in queue to the primes list until 0 appears */
        while ((queue_head = queue_get(input)) != 0)
        {
            pthread_mutex_lock(&mutex);
            primes[primes_index++] = queue_head;
            fprintf(stdout, " %lu", queue_head);
            pthread_mutex_unlock(&mutex);
        }
        fprintf(stdout, "\n");
        sem_post(&done);
    }
    else
    {
        /* Initialize output thread */
        if (!queue_init(&output, QUEUE_SIZE))
            fatal_error("Failed creating output circular queue\n");

        /* Create the next thread */
        pthread_t tid;
        pthread_create(&tid, NULL, thr_filter, output);

        /* Add non multiple numbers to the new queue list until 0 appears */
        while ((queue_head = queue_get(input)) != 0)
        {
            if (queue_head % current_prime != 0)
                queue_put(output, queue_head);
        }
        queue_put(output, 0);
    }

    queue_destroy(input);
    pthread_exit(EXIT_SUCCESS); /* successfully exit the thread */
}
