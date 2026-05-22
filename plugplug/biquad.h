#ifndef BIQUAD_H
#define BIQUAD_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Biquadratic filter
 */
typedef enum biquad_filter_type {
	BIQUAD_FILTER_TYPE_LOW_SHELF = 0,
	BIQUAD_FILTER_TYPE_BELL,
	BIQUAD_FILTER_TYPE_HIGH_SHELF,

	BIQUAD_FILTER_TYPE_COUNT,
} biquad_filter_type_t;

#define BIQUAD_FILTER_CHANNEL_COUNT 2

typedef struct biquad_filter {
	biquad_filter_type_t type;
	float frequency, width, gain;
	float a0, a1, a2, b1, b2; /* filter coefficients */
	float z1[BIQUAD_FILTER_CHANNEL_COUNT],
	      z2[BIQUAD_FILTER_CHANNEL_COUNT]; /* delay buffers */
} biquad_filter_t;

/*
 * Initialize filter
 *
 * This just calls biquad_filter_set_parameters, in
 * addition to setting the filter type.
 */
extern void biquad_filter_init(biquad_filter_t *filter,
			       biquad_filter_type_t type,
			       float sample_rate,
			       float frequency,
			       float width,
			       float gain);

/*
 * Set filter parameters
 */
extern void biquad_filter_set_parameters(biquad_filter_t *filter,
					 float sample_rate,
					 float frequency,
					 float width,
					 float gain);

/*
 * Process one sample
 */
extern float biquad_filter_process_sample(biquad_filter_t *filter, int channel, float in);

#endif /* BIQUAD_H */
