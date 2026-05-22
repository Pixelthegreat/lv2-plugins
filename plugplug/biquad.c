#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "biquad.h"

/* initialize filter */
extern void biquad_filter_init(biquad_filter_t *filter,
			       biquad_filter_type_t type,
			       float sample_rate,
			       float frequency,
			       float width,
			       float gain) {

	memset(filter, 0, sizeof(biquad_filter_t));
	filter->type = type;

	biquad_filter_set_parameters(filter, sample_rate, frequency, width, gain);
}

/*
 * Set filter parameters
 *
 * Some of the code for this function was derived from:
 * https://www.earlevel.com/main/2012/11/26/biquad-c-source-code
 */
extern void biquad_filter_set_parameters(biquad_filter_t *filter,
					 float sample_rate,
					 float frequency,
					 float width,
					 float gain) {

	filter->frequency = frequency;
	filter->width = width;
	filter->gain = gain;

	frequency /= sample_rate;
	if (frequency > 0.4999f)
		frequency = 0.4999f;
	else if (frequency < 0.0001f)
		frequency = 0.0001f;

	float norm;
	float V = powf(10.f, fabs(gain) / 20.f);
	float K = tanf(3.141592653589793f * frequency);

	float a0 = 1.f, a1 = 0, a2 = 0, b1 = 0, b2 = 0;

	switch (filter->type) {

		/* low shelf */
		case BIQUAD_FILTER_TYPE_LOW_SHELF:
			if (gain >= 0.001f) {

				norm = 1.f / (1.f + sqrtf(2.f) * K + K * K);

				a0 = (1.f + sqrtf(2.f * V) * K + V * K * K) * norm;
				a1 = 2.f * (V * K * K - 1.f) * norm;
				a2 = (1.f - sqrtf(2.f * V) * K + V * K * K) * norm;
				b1 = 2.f * (K * K - 1.f) * norm;
				b2 = (1.f - sqrtf(2.f) * K + K * K) * norm;
			}
			else {
				norm = 1.f / (1.f + sqrtf(2.f * V) * K + V * K * K);

				a0 = (1.f + sqrtf(2.f) * K + K * K) * norm;
				a1 = 2.f * (K * K - 1.f) * norm;
				a2 = (1.f - sqrtf(2.f) * K + K * K) * norm;
				b1 = 2.f * (V * K * K - 1.f) * norm;
				b2 = (1.f - sqrtf(2.f * V) * K + V * K * K) * norm;
			}
			break;

		/* bell / peak filter */
		case BIQUAD_FILTER_TYPE_BELL:
			if (gain >= 0.001f) {

				norm = 1.f / (1.f + 1.f / width * K + K * K);

				a0 = (1.f + V / width * K + K * K) * norm;
				a1 = 2.f * (K * K - 1.f) * norm;
				a2 = (1.f - V / width * K + K * K) * norm;
				b1 = a1;
				b2 = (1.f - 1.f / width * K + K * K) * norm;
			}
			else {
				norm = 1.f / (1.f + V / width * K + K * K);

				a0 = (1.f + 1.f / width * K + K * K) * norm;
				a1 = 2.f * (K * K - 1.f) * norm;
				a2 = (1.f - 1.f / width * K + K * K) * norm;
				b1 = a1;
				b2 = (1.f - V / width * K + K * K) * norm;
			}
			break;

		/* high shelf */
		case BIQUAD_FILTER_TYPE_HIGH_SHELF:
			if (gain >= 0.001f) {

				norm = 1.f / (1.f + sqrtf(2.f) * K + K * K);

				a0 = (V + sqrtf(2.f * V) * K + K * K) * norm;
				a1 = 2.f * (K * K - V) * norm;
				a2 = (V - sqrtf(2.f * V) * K + K * K) * norm;
				b1 = 2.f * (K * K - 1.f) * norm;
				b2 = (1.f - sqrtf(2.f) * K + K * K) * norm;
			}
			else {
				norm = 1.f / (V + sqrtf(2.f * V) * K + V * K * K);

				a0 = (1.f + sqrtf(2.f) * K + K * K) * norm;
				a1 = 2.f * (K * K - 1) * norm;
				a2 = (1.f - sqrtf(2.f) * K + K * K) * norm;
				b1 = 2.f * (K * K - V) * norm;
				b2 = (V - sqrtf(2.f * V) * K + K * K) * norm;
			}
			break;
	}

	filter->a0 = a0;
	filter->a1 = a1;
	filter->a2 = a2;
	filter->b1 = b1;
	filter->b2 = b2;
}

/* process one sample */
extern float biquad_filter_process_sample(biquad_filter_t *filter, int channel, float in) {

	float out = in * filter->a0 + filter->z1[channel];

	filter->z1[channel] = in * filter->a1 + filter->z2[channel] - filter->b1 * out;
	filter->z2[channel] = in * filter->a2 - filter->b2 * out;

	return out;
}
