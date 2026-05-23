#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

/* multiply matrix and vector */
extern void matrix_mulsv(
		float *dest,
		float *a,
		float *b,
		size_t width
) {
	size_t k = 0;

	for (size_t i = 0; i < width; i++) {

		dest[i] = 0;
		for (size_t j = 0; j < width; j++)
			dest[i] += a[k++] * b[j];
	}
}

/* perform householder matrix calculation */
extern void matrix_mul_householder(
		float *vector,
		size_t width
) {
	float scale = -2.f / (float)width;

	float sum = 0;
	for (size_t i = 0; i < width; i++)
		sum += vector[i];

	sum *= scale;

	for (size_t i = 0; i < width; i++)
		vector[i] += sum;
}

/* perform hadamard matrix calculation */
extern void matrix_mul_hadamard(
		float *vector,
		size_t width
) {
	if (width <= 1) return;

	size_t half = width / 2;

	matrix_mul_hadamard(vector, half);
	matrix_mul_hadamard(vector+half, half);

	for (size_t i = 0; i < half; i++) {

		float a = vector[i];
		float b = vector[i + half];

		vector[i] = (a + b);
		vector[i + half] = (a - b);
	}
}
