#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>

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

#define DELAY_SAMPLES 9600

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
	float current_delay;
	size_t current_sample_delay;
	struct {
		float data[DELAY_SAMPLES];
		size_t start, end;
	} buffers[2]; /* one buffer per channel */
};

/* create instance */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double rate,
			      const char *bundle_path, const LV2_Feature *const *features) {

	struct port_data *data = (struct port_data *)malloc(sizeof(struct port_data));
	memset(data, 0, sizeof(struct port_data));

	data->sample_rate = (float)rate;

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

	for (uint32_t i = 0; i < count; i++) {

		output[i] = input[i] * input_gain;

		output[i] += pdata->buffers[channel].data[pdata->buffers[channel].end++] *
			     (*pdata->feedback);
		pdata->buffers[channel].data[pdata->buffers[channel].start++] = output[i];

		output[i] *= output_gain;

		pdata->buffers[channel].start %= DELAY_SAMPLES;
		pdata->buffers[channel].end %= DELAY_SAMPLES;
	}
}

/* run instance */
static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *pdata = (struct port_data *)instance;

	if (pdata->current_delay != *pdata->time) {

		pdata->current_delay = *pdata->time;

		size_t sample_delay = (size_t)(pdata->sample_rate * (*pdata->time)) / 1000;
		if (sample_delay && sample_delay < DELAY_SAMPLES-1) {

			pdata->current_sample_delay = sample_delay;
			for (size_t i = 0; i < 2; i++) {

				pdata->buffers[i].start = 0;
				pdata->buffers[i].end = sample_delay;
			}
		}
	}

	float input_gain = DB_CO(*pdata->input_gain); /* convert from dB to scalar */
	float output_gain = DB_CO(*pdata->output_gain);

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
