#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>
#include <biquad.h>

#define MIXER_STRIP_URI "http://fanfavoritessofar.com/mixer-strip"

/* port data */
enum {
	PORT_LEFT_INPUT = 0,
	PORT_RIGHT_INPUT,
	PORT_LEFT_OUTPUT,
	PORT_RIGHT_OUTPUT,
	PORT_INPUT_GAIN,
	PORT_INPUT_PAN,
	PORT_LOW_EQ,
	PORT_MID_EQ,
	PORT_HIGH_EQ,
	PORT_COMPRESSION,
	PORT_OUTPUT_GAIN,
	PORT_OUTPUT_PAN,

	PORT_COUNT,

	PORT_EQ_FIRST = PORT_LOW_EQ,
	PORT_EQ_LAST = PORT_HIGH_EQ,
	PORT_EQ_COUNT = PORT_EQ_LAST - PORT_EQ_FIRST + 1,
};
struct port_data {
	float sample_rate;
	union {
		const float *ports[PORT_COUNT];
		struct {
			const float *left_input, *right_input;
			float *left_output, *right_output;
			const float *input_gain;
			const float *input_pan;
			const float *low_eq;
			const float *mid_eq;
			const float *high_eq;
			const float *compression;
			const float *output_gain;
			const float *output_pan;
		} __attribute__((packed));
	};
	biquad_filter_t filters[PORT_EQ_COUNT];
	bool initialized;
};

/* create instance */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double sample_rate,
			      const char *bundle_path, const LV2_Feature *const *features) {

	struct port_data *data = (struct port_data *)malloc(sizeof(struct port_data));
	memset(data, 0, sizeof(struct port_data));

	data->sample_rate = (float)sample_rate;

	return (LV2_Handle)data;
}

/* connect port */
static void connect_port(LV2_Handle instance, uint32_t port, void *data) {

	struct port_data *pdata = (struct port_data *)instance;
	if (port < PORT_COUNT) pdata->ports[port] = (const float *)data;
}

/* activate instance */
static void activate(LV2_Handle instance) {
}

/* adjust gain */
#define DB_CO(g) ((g) > -90.0f? powf(10.0f, (g) * 0.05f): 0.0f)

static void adjust_gain(float *output, const float *input, float gain, uint32_t nsamples) {

	float coef = DB_CO(gain);
	for (uint32_t i = 0; i < nsamples; i++)
		output[i] = input[i] * coef;
}

/* adjust panning */
static void adjust_pan(struct port_data *data, float pan, uint32_t nsamples) {

	float abspan = fabs(pan);
	if (abspan <= 0.001f) return;

	float gain = abspan * 3.0f;
	float *channel = pan > 0.f? data->left_output: data->right_output;

	for (uint32_t i = 0; i < nsamples; i++)
		channel[i] *= 1.f - abspan;

	adjust_gain(data->left_output, data->left_output, gain, nsamples);
	adjust_gain(data->right_output, data->right_output, gain, nsamples);
}

/* run biquad filters */
static void run_filters(struct port_data *data, const float *input,
			float *output, int channel, size_t count) {

	for (size_t i = 0; i < count; i++) {

		output[i] = input[i];
		for (uint32_t j = 0; j < PORT_EQ_COUNT; j++) {

			biquad_filter_t *filter = data->filters+j;
			output[i] = biquad_filter_process_sample(filter, channel, output[i]);
		}
	}
}

/* run instance */
static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *data = (struct port_data *)instance;

	/* input processing */
	adjust_gain(data->left_output, data->left_input, *data->input_gain, nsamples);
	adjust_gain(data->right_output, data->right_input, *data->input_gain, nsamples);

	adjust_pan(data, *data->input_pan, nsamples);

	/* equalization */
	static biquad_filter_type_t types[PORT_EQ_COUNT] = {
		BIQUAD_FILTER_TYPE_LOW_SHELF,
		BIQUAD_FILTER_TYPE_BELL,
		BIQUAD_FILTER_TYPE_HIGH_SHELF,
	};
	static float frequencies[PORT_EQ_COUNT] = {
		300.f, 750.f, 2000.f,
	};
	static float widths[PORT_EQ_COUNT] = {
		0.707f, 0.707f, 0.707f,
	};

	for (uint32_t i = 0; i < PORT_EQ_COUNT; i++) {

		if (!data->initialized ||
		    data->filters[i].gain != *data->ports[PORT_EQ_FIRST+i]) {

			biquad_filter_init(data->filters+i,
					   types[i],
					   data->sample_rate,
					   frequencies[i],
					   widths[i],
					   *data->ports[PORT_EQ_FIRST+i]);
		}
	}
	data->initialized = true;

	run_filters(data, data->left_input, data->left_output, 0, (size_t)nsamples);
	run_filters(data, data->right_input, data->right_output, 0, (size_t)nsamples);

	/* output processing */
	adjust_gain(data->left_output, data->left_output, *data->output_gain, nsamples);
	adjust_gain(data->right_output, data->right_output, *data->output_gain, nsamples);

	adjust_pan(data, *data->output_pan, nsamples);
}

/* deactivate instance */
static void deactivate(LV2_Handle instance) {
}

/* free instance */
static void cleanup(LV2_Handle instance) {

	free((struct port_data *)instance);
}

/* extension info */
static const void *extension_data(const char *uri) {

	return NULL;
}

/* plugin description */
static const LV2_Descriptor descriptor = {
	.URI = MIXER_STRIP_URI,
	.instantiate = instantiate,
	.connect_port = connect_port,
	.activate = activate,
	.run = run,
	.deactivate = deactivate,
	.cleanup = cleanup,
	.extension_data = extension_data,
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor(uint32_t index) {

	return !index? &descriptor: NULL;
}
