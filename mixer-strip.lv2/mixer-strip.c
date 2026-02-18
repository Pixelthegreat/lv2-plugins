#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>

#define MIXER_STRIP_URI "http://fanfavoritessofar.com/mixer-strip"

/* ports */
enum {
	PORT_INPUT_GAIN = 0,
	PORT_INPUT_PAN = 1,
	PORT_LOW_EQ = 2,
	PORT_MID_EQ = 3,
	PORT_HIGH_EQ = 4,
	PORT_OUTPUT_GAIN = 5,
	PORT_OUTPUT_PAN = 6,
	PORT_LEFT_INPUT = 7,
	PORT_RIGHT_INPUT = 8,
	PORT_LEFT_OUTPUT = 9,
	PORT_RIGHT_OUTPUT = 10,

	PORT_COUNT,
};
struct port_data {
	union {
		const float *ports[PORT_COUNT]; /* ports */
		struct {
			const float *input_gain; /* input gain */
			const float *input_pan; /* input pan */
			const float *low_eq; /* low equalization */
			const float *mid_eq; /* mid equalization */
			const float *high_eq; /* high equalization */
			const float *output_gain; /* output gain */
			const float *output_pan; /* output pan */
			const float *left_input, *right_input; /* input data */
			float *left_output, *right_output; /* output data */
		} __attribute__((packed));
	};
};

/* create instance */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double rate, const char *bundle_path, const LV2_Feature *const *features) {

	struct port_data *data = (struct port_data *)malloc(sizeof(struct port_data));
	memset(data, 0, sizeof(struct port_data));

	while (*features) {

		const LV2_Feature *feature = *features++;
		printf("mixer-strip:info: Feature %s\n", feature->URI);
	}

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

/* run instance */
static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *data = (struct port_data *)instance;

	/* input processing */
	adjust_gain(data->left_output, data->left_input, *data->input_gain, nsamples);
	adjust_gain(data->right_output, data->right_input, *data->input_gain, nsamples);

	adjust_pan(data, *data->input_pan, nsamples);

	/* equalization */

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
