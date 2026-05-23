#ifndef DELAY_H
#define DELAY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Delay buffer
 */
#define DELAY_SAMPLES 9600

typedef struct delay {
	float sample_rate;
	float time;
	float feedback;
	float dry;
	size_t sample_time;
	float data[DELAY_SAMPLES];
	size_t start, end;
} delay_t;

/*
 * Initialize delay buffer
 */
extern void delay_init(
		delay_t *delay,
		float sample_rate,
		float time,
		float feedback,
		float dry
);

/*
 * Set delay amount
 */
extern void delay_set_time(
		delay_t *delay,
		float time
);

/*
 * Set feedback
 */
extern void delay_set_feedback(
		delay_t *delay,
		float feedback
);

/*
 * Set dry mix
 */
extern void delay_set_dry(
		delay_t *delay,
		float dry
);

/*
 * Process sample
 */
extern float delay_process_sample(
		delay_t *delay,
		float in
);

extern float delay_process_sample2(
		delay_t *delay,
		float in_a, float in_b
);

#endif /* DELAY_H */
