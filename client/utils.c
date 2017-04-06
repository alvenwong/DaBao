#include "utils.h"


/*
 * shuffer the array in random order
 */
void shuffer(unsigned int *array, const int size)
{
	if (size > 1) {
		int i, j, tmp;
		for (i=0; i<size; i++) {
			j = i + rand() / (RAND_MAX / (size - i) + 1);
			tmp = array[j];
			array[j] = array[i];
			array[i] = tmp;
		}
	}
}


void showMap(unsigned int *array, const int size)
{
	int i;
	for (i=0; i<size; i++) {
		if (array[i] != 0) {
			printf("%d, %d\t", i, array[i]);
		}
	}
}
