#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>
#include "delay.h"

#define DELAY_URI "http://fanfavoritessofar.com/delay"

/* ports */
enum {
	PORT_LEFT_INPUT = 0,
	PORT_RIGHT_INPUT,
	PORT_LEFT_OUTPUT,
	PORT_RIGHT_OUTPUT,
	PORT_TIME,
	PORT_FEEDBACK,
	PORT_INPUT_GAIN,
	PORT_OUTPUT_GAIN,

	PORT_COUNT,
};

struct port_data {
	union {
		struct {
			const float *left_input, *right_input;
			float *left_output, *right_output;
			const float *time;
			const float *feedback;
			const float *input_gain;
			const float *output_gain;
		} __attribute__((packed));
		const float *ports[PORT_COUNT];
	};
	float sample_rate;
	float current_time;
	delay_t delays[2];
};

/* create instance */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double rate,
			      const char *bundle_path, const LV2_Feature *const *features) {

	struct port_data *data = (struct port_data *)malloc(sizeof(struct port_data));
	memset(data, 0, sizeof(struct port_data));

	data->sample_rate = (float)rate;

	delay_init(
		&data->delays[0],
		data->sample_rate,
		0.01f,
		0.1f,
		1.f
	);
	delay_init(
		&data->delays[1],
		data->sample_rate,
		0.01f,
		0.1f,
		1.f
	);
	data->current_time = 10.f;

	return (LV2_Handle)data;
}

/* connect port */
static void connect_port(LV2_Handle instance, uint32_t port, void *data) {

	struct port_data *pdata = (struct port_data *)instance;
	if (port < PORT_COUNT)
		pdata->ports[port] = (const float *)data;
}

/* activate instance */
static void activate(LV2_Handle instance) {
}

/*
 * This is the primary delay logic:
 *  - Pull a sample from farther ahead in the buffer,
 *    multiply it by the feedback amount and add it to
 *    the input sample
 *  - Write the current sample to the buffer
 */
#define DB_CO(g) ((g) > -90.0f? powf(10.0f, (g) * 0.05f): 0.0f)

static void delay(struct port_data *pdata, float *output,
		  const float *input, size_t count, int channel,
		  float input_gain, float output_gain) {

	for (uint32_t i = 0; i < count; i++)
		output[i] = delay_process_sample(
				&pdata->delays[channel],
				input[i] * input_gain
			) * output_gain;
}

/* run instance */
static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *pdata = (struct port_data *)instance;

	if (pdata->current_time != *pdata->time) {

		pdata->current_time = *pdata->time;

		delay_set_time(&pdata->delays[0], *pdata->time / 1000.f);
		delay_set_time(&pdata->delays[1], *pdata->time / 1000.f);
	}

	float input_gain = DB_CO(*pdata->input_gain); /* convert from dB to scalar */
	float output_gain = DB_CO(*pdata->output_gain);

	delay_set_feedback(&pdata->delays[0], *pdata->feedback);
	delay_set_feedback(&pdata->delays[1], *pdata->feedback);

	delay(pdata, pdata->left_output, pdata->left_input,
	      (size_t)nsamples, 0, input_gain, output_gain);
	delay(pdata, pdata->right_output, pdata->right_input,
	      (size_t)nsamples, 1, input_gain, output_gain);
}

/* deactivate instance */
static void deactivate(LV2_Handle instance) {
}

/* free instance */
static void cleanup(LV2_Handle instance) {

	free((void *)instance);
}

/* extension info */
static const void *extension_data(const char *uri) {

	return NULL;
}

/* plugin description */
static const LV2_Descriptor descriptor = {
	DELAY_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data,
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor(uint32_t index) {

	return index == 0? &descriptor: NULL;
}
