/* Compile using debug flag [-DDEBUG] for error messages  */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include "circularqueue.h"
#include "quicksort.h"

CircularQueue *q;

long int parse_long(char *str, int base) {

  char *endptr;
  long int number = strtol(str, &endptr, base);

  /* Check for various possible errors */
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

  if (number <= 1) {
#ifdef DEBUG
    fprintf(stderr, "Non positive number\n");
#endif
    return LONG_MAX;
  }
#ifdef DEBUG
  printf("strtol() successfuly returned %ld\n", number);
#endif

  /* Successful conversion */
  return number;
}

int main(int argc, char *argv[]) {

  /* Verify arguments validity */
  if (argc < 2) {
    printf("Usage: %s <upper number limit>.\n",
	   argv[0]);
    exit (EXIT_FAILURE);
  }
       
  /* Safely convert limit string argument to a long integer */
  unsigned int size;
  if ((size = parse_long(argv[1], 10)) == LONG_MAX) {
    printf("Enter a valid number list size number\n");
    exit (EXIT_FAILURE);
  }
  
  /* Create circular queue */
  
  if ( queue_init( &q, size) == -1 ){
	printf("Failed creating circular queue\n");
    exit (EXIT_FAILURE);
  }
  
  unsigned int i;
  for (i = 3; i < (size + 3); i++){
	queue_put(q, i);
  }
  
  for (i = 0; i < size; i++){
	printf("%d ",(unsigned int) queue_get(q));
  }
	printf("\n");
  
  exit (EXIT_SUCCESS);
}
