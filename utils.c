#include <stdio.h>
#include "utils.h"
#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))

unsigned int odds_count(unsigned int n) {

	unsigned int counter = 0;

	if (n < 2)
		counter = 1;
	else
		counter = (((n - 2) + 1) / 2) + 1;

	return counter;
}

void quick_sort(int arr[], int min, int max) {
	int pivot, j, tmp, i;
	if (min < max) {
		pivot = min;
		i = min;
		j = max;

		while (i < j) {
			while ((arr[i] <= arr[pivot]) && (i < max))
				i++;

			while (arr[j] > arr[pivot])
				j--;

			if (i < j) {
				tmp = arr[i];
				arr[i] = arr[j];
				arr[j] = tmp;
			}
		}

		tmp = arr[pivot];
		arr[pivot] = arr[j];
		arr[j] = tmp;
		quick_sort(arr, min, j - 1);
		quick_sort(arr, j + 1, max);
	}
}
