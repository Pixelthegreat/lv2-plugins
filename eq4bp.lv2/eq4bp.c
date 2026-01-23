#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>

#define EQ4BP_URI "http://fanfavoritessofar.com/eq4bp"

/* biquadratic filter */
enum filter_type {
	FILTER_TYPE_LOW_SHELF = 0,
	FILTER_TYPE_BELL,
	FILTER_TYPE_HIGH_SHELF,

	FILTER_TYPE_COUNT,
};

struct biquad_filter {
	enum filter_type type;
	float a0, a1, a2, b1, b2;
	float frequency, width, gain;
	float z1[2], z2[2];
};

static void biquad_filter_set_parameters(struct biquad_filter *filter,
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
		case FILTER_TYPE_LOW_SHELF:
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
		case FILTER_TYPE_BELL:
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
		case FILTER_TYPE_HIGH_SHELF:
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

static void biquad_filter_init(struct biquad_filter *filter,
			       enum filter_type type,
			       float sample_rate,
			       float frequency,
			       float width,
			       float gain) {

	memset(filter, 0, sizeof(struct biquad_filter));
	filter->type = type;

	biquad_filter_set_parameters(filter, sample_rate, frequency, width, gain);
}

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
	struct biquad_filter filters[PORT_FILTER_COUNT];
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

			struct biquad_filter *filter = pdata->filters+j;
			float in = output[i];

			float out = in * filter->a0 + filter->z1[channel];
			filter->z1[channel] = in * filter->a1 +
					      filter->z2[channel] -
					      filter->b1 * out;
			filter->z2[channel] = in * filter->a2 -
					      filter->b2 * out;
			output[i] = out;
		}
		output[i] *= output_gain;
	}
}

/* run instance */
static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *pdata = (struct port_data *)instance;

	/* configure filters */
	static enum filter_type types[PORT_FILTER_COUNT] = {
		FILTER_TYPE_LOW_SHELF,
		FILTER_TYPE_BELL,
		FILTER_TYPE_BELL,
		FILTER_TYPE_HIGH_SHELF,
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
