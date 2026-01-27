#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>
#include <biquad.h>

#define EQ4BP_URI "http://fanfavoritessofar.com/eq4bp"

/* ports */
enum {
	PORT_LEFT_INPUT,
	PORT_RIGHT_INPUT,
	PORT_LEFT_OUTPUT,
	PORT_RIGHT_OUTPUT,

	PORT_FILTER0 = 4,
	PORT_FILTER1 = 7,
	PORT_FILTER2 = 10,
	PORT_FILTER3 = 13,

	PORT_INPUT_GAIN = 16,
	PORT_OUTPUT_GAIN,
	PORT_COUNT,

	PORT_FILTER_COUNT = 4,
};

#define PORT_FREQUENCY(i) (PORT_FILTER0 + (i) * 3)
#define PORT_WIDTH(i) (PORT_FILTER0 + (i) * 3 + 1)
#define PORT_GAIN(i) (PORT_FILTER0 + (i) * 3 + 2)

struct port_data {
	float sample_rate;
	union {
		struct {
			const float *left_input;
			const float *right_input;
			float *left_output;
			float *right_output;
			struct {
				const float *frequency;
				const float *width;
				const float *gain;
			} __attribute__((packed)) filter_ports[PORT_FILTER_COUNT];
			const float *input_gain;
			const float *output_gain;
		} __attribute__((packed));
		const float *ports[PORT_COUNT];
	};
	biquad_filter_t filters[PORT_FILTER_COUNT];
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
	if (port < PORT_COUNT)
		pdata->ports[port] = (const float *)data;
}

/* activate instance */
static void activate(LV2_Handle instance) {

	struct port_data *pdata = (struct port_data *)instance;
}

/* run biquad filters on audio data */
#define DB_CO(g) ((g) > -90.0f? powf(10.0f, (g) * 0.05f): 0.0f)

static void run_filters(struct port_data *pdata, const float *input,
			float *output, int channel, size_t count,
			float input_gain, float output_gain) {

	for (size_t i = 0; i < count; i++) {

		output[i] = input[i] * input_gain;
		for (uint32_t j = 0; j < PORT_FILTER_COUNT; j++) {

			biquad_filter_t *filter = pdata->filters+j;
			output[i] = biquad_filter_process_sample(filter, channel, output[i]);
		}
		output[i] *= output_gain;
	}
}

/* run instance */
static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *pdata = (struct port_data *)instance;

	/* configure filters */
	static biquad_filter_type_t types[PORT_FILTER_COUNT] = {
		BIQUAD_FILTER_TYPE_LOW_SHELF,
		BIQUAD_FILTER_TYPE_BELL,
		BIQUAD_FILTER_TYPE_BELL,
		BIQUAD_FILTER_TYPE_HIGH_SHELF,
	};
	for (uint32_t i = 0; i < PORT_FILTER_COUNT; i++) {

		if (!pdata->initialized ||
		    pdata->filters[i].frequency != *pdata->filter_ports[i].frequency ||
		    pdata->filters[i].width != *pdata->filter_ports[i].width ||
		    pdata->filters[i].gain != *pdata->filter_ports[i].gain) {

			biquad_filter_init(pdata->filters+i,
					   types[i],
					   pdata->sample_rate,
					   *pdata->filter_ports[i].frequency,
					   *pdata->filter_ports[i].width,
					   *pdata->filter_ports[i].gain);
		}
	}
	pdata->initialized = true;

	/* actually run instance */
	float input_gain = DB_CO(*pdata->input_gain);
	float output_gain = DB_CO(*pdata->output_gain);

	run_filters(pdata, pdata->left_input,
		    pdata->left_output, 0, (size_t)nsamples,
		    input_gain, output_gain);
	run_filters(pdata, pdata->right_input,
		    pdata->right_output, 1, (size_t)nsamples,
		    input_gain, output_gain);
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
	EQ4BP_URI,
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
