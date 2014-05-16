/* Compile using debug flag [-DDEBUG] for error messages  */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#ifndef _CIRCULAR_QUEUE_H_
#include "circularqueue.h"
#endif

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
  long int numberLimit;
  if ((numberLimit = parse_long(argv[1], 10)) == LONG_MAX) {
    printf("Enter a valid numberLimit number\n");
    exit (EXIT_FAILURE);
  }

  exit (EXIT_SUCCESS);
}
