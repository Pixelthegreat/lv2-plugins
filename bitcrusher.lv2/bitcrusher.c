#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>

#define BITCRUSHER_URI "http://fanfavoritessofar.com/bitcrusher"

/* ports */
enum {
	PORT_REPEAT = 0,
	PORT_BITRATE,
	PORT_LEFT_INPUT,
	PORT_RIGHT_INPUT,
	PORT_LEFT_OUTPUT,
	PORT_RIGHT_OUTPUT,

	PORT_COUNT,
};
struct port_data {
	union {
		struct {
			const float *repeat; /* repeat samples */
			const float *bitrate; /* subdivide samples */
			const float *left_input, *right_input; /* input */
			float *left_output, *right_output; /* output */
		} __attribute__((packed));
		const float *ports[PORT_COUNT]; /* ports */
	};
};

/* create instance */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double rate, const char *bundle_path, const LV2_Feature *const *features) {

	struct port_data *data = (struct port_data *)malloc(sizeof(struct port_data));
	memset(data, 0, sizeof(struct port_data));

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

/* crush */
static void crush(float *output, const float *input, uint32_t nsamples, uint32_t nrepeat, float bitrate) {

	int ibitrate = 1 << (int)bitrate;
	float sample = 0;

	for (uint32_t i = 0; i < nsamples; i++) {

		if (i % nrepeat == 0) {

			sample = input[i];
			if (ibitrate != 1) {

				float sign = sample < 0? -1.f: 1.f;
				sample = (floor(fabs(sample) * bitrate) * sign) / bitrate;
			}
		}
		output[i] = sample;
	}
}

/* run instance */
static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *data = (struct port_data *)instance;

	uint32_t nrepeat = (uint32_t)*data->repeat;
	float bitrate = floor(*data->bitrate);

	crush(data->left_output, data->left_input, nsamples, nrepeat, bitrate);
	crush(data->right_output, data->right_input, nsamples, nrepeat, bitrate);
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
	BITCRUSHER_URI,
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
