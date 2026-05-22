#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Multiply vector and square matrix
 */
extern void matrix_mulsv(
		float *dest, /* vector */
		float *a, /* matrix */
		float *b, /* vector */
		size_t width
);

/*
 * Perform Hadamard matrix calculation
 */
extern void matrix_mul_hadamard(
		float *vector,
		size_t width
);

#endif /* MATRIX_H */
