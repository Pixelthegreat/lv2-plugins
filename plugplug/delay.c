#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"

/* initialize delay buffer */
extern void delay_init(
		delay_t *delay,
		float sample_rate,
		float time,
		float feedback,
		float dry
) {
	memset(delay, 0, sizeof(delay_t));

	delay->sample_rate = sample_rate;

	delay_set_time(delay, time);
	delay_set_feedback(delay, feedback);
	delay_set_dry(delay, dry);
}

/* set delay amount */
extern void delay_set_time(
		delay_t *delay,
		float time
) {
	size_t sample_time = (size_t)(delay->sample_rate * time);

	if (sample_time >= DELAY_SAMPLES-1)
		return;
	sample_time = DELAY_SAMPLES-1-sample_time;

	delay->time = time;
	delay->sample_time = sample_time;

	delay->start = 0;
	delay->end = sample_time;
}

/* set feedback */
extern void delay_set_feedback(
		delay_t *delay,
		float feedback
) {
	delay->feedback = feedback;
}

/* set dry mix */
extern void delay_set_dry(
		delay_t *delay,
		float dry
) {
	delay->dry = dry;
}

/* process sample */
extern float delay_process_sample(
		delay_t *delay,
		float in
) {
	float add = delay->data[delay->end++] * delay->feedback;

	delay->data[delay->start++] = in + add * delay->dry;

	delay->start %= DELAY_SAMPLES;
	delay->end %= DELAY_SAMPLES;

	return in * delay->dry + add;
}

/* process two samples (one for feedback) */
extern float delay_process_sample2(
		delay_t *delay,
		float in_a, float in_b
) {
	float add = delay->data[delay->end++] * delay->feedback;

	delay->data[delay->start++] = in_b + add * delay->dry;

	delay->start %= DELAY_SAMPLES;
	delay->end %= DELAY_SAMPLES;

	return in_a * delay->dry + add;
}
