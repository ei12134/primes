#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "circularqueue.h"

/* Compile using debug flag [-DDEBUG] for error messages */
#ifdef DEBUG
#define DEBUG_PRINT(x) fprintf(stderr, x)
#else
#define DEBUG_PRINT(x) \
    do                 \
    {                  \
    } while (0)
#endif

QueueElem *primes;                                 /* prime numbers container */
QueueElem hard_upper_limit;                        /* hard upper prime limit */
unsigned int primes_counter;                       /* total number of primes found */
sem_t done;                                        /* completion signalling semaphore */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;    /* conditional variable declaration */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* mutex variable declaration */

void *thr_init(void *arg);
void *thr_filter(void *arg);

int compare_integers(const void *a, const void *b)
{
    return (*(QueueElem *)a - *(QueueElem *)b);
}

void fatal_error(char *msg)
{
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}

QueueElem parse_unsigned_long(char *str, int base)
{
    char *endptr;

    /* Dont convert negative numbers */
    if (str[0] == '-')
    {
        DEBUG_PRINT("Negative number\n");
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
        DEBUG_PRINT("No digits were found\n");
        return ULONG_MAX;
    }

    if (*endptr != '\0')
    {
        DEBUG_PRINT("Non digit char found\n");
        return ULONG_MAX;
    }

    if (number < 2)
    {
        DEBUG_PRINT("Input number was < 2\n");
        return ULONG_MAX;
    }

    return number;
}

int main(int argc, char *argv[])
{
    /* check arguments */
    if (argc != 2)
    {
        char error_message[] = {};
        sprintf(error_message, "Usage: %s <upper prime number limit>.\n", argv[0]);
        fatal_error(error_message);
    }

    /* Safely convert limit string argument to a unsigned integer */
    if ((hard_upper_limit = parse_unsigned_long(argv[1], 10)) == ULONG_MAX)
        fatal_error("Error: Enter a valid upper prime number limit (> 1)\n");

    /* variable initialization */
    unsigned int soft_upper_limit = 0;
    primes_counter = 0;

    if (hard_upper_limit != 2)
        soft_upper_limit = 1 + ((1.2 * (double)hard_upper_limit) - 1) / log((double)hard_upper_limit);

    /* Allocate memory for primes array */
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

    /* Sort primes list */
    qsort(primes, primes_counter, sizeof(QueueElem), compare_integers);

    /* Display primes list */
    unsigned int i;
    fprintf(stdout, "Primes in the range [1-%ld]: ", hard_upper_limit);

    for (i = 0; i < primes_counter; i++)
        printf("%lu ", primes[i]);
    printf("\n");

    /* Exit the primes program main thread */
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    pthread_exit(EXIT_SUCCESS);
}

void *thr_init(void *arg)
{
    primes[0] = 2; /* add first prime to the primes list */
    primes_counter = 1;

    /* Only use a circular queue when the hard upper limit input is > 2 */
    if (hard_upper_limit > 2)
    {
        CircularQueue *queue;
        if (queue_init(&queue, QUEUE_SIZE) != 0)
            fatal_error("Failed creating circular queue\n");

        /* Create filter thread */
        pthread_t tid;
        pthread_create(&tid, NULL, thr_filter, queue);

        /* Put all odd numbers into the queue */
        unsigned int i;
        for (i = 3; i <= hard_upper_limit; i++)
        {
            if (i % 2 != 0)
                queue_put(queue, i);
        }
        queue_put(queue, 0);
    }
    else
        sem_post(&done);

    pthread_exit(EXIT_SUCCESS); /* Exit the thread */
}

void *thr_filter(void *arg)
{
    CircularQueue *input = (CircularQueue *)arg; /* input queue */
    CircularQueue *output;                       /* output queue */

    /* add the first queue number to the primes list */
    unsigned int queue_head, current_prime = queue_get(input);

    pthread_mutex_lock(&mutex);
    primes[primes_counter++] = current_prime;
    pthread_mutex_unlock(&mutex);

    /* Stop checking for multiples of primes if first queue number greater than sqrt(N) */
    if (current_prime > sqrt(hard_upper_limit))
    {
        /* Add all primes in queue to the primes list until 0 appears */
        while ((queue_head = queue_get(input)) != 0)
        {
            pthread_mutex_lock(&mutex);
            primes[primes_counter++] = queue_head;
            pthread_mutex_unlock(&mutex);
        }
        sem_post(&done);
    }
    else
    {
        /* Initialize output thread */
        if (queue_init(&output, QUEUE_SIZE) != 0)
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

    /* Exit the thread */
    queue_destroy(input);
    pthread_exit(EXIT_SUCCESS);
}
