#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <lv2/core/lv2.h>
#include "biquad.h"
#include "delay.h"
#include "matrix.h"

#define HERB_URI "http://fanfavoritessofar.com/herb"

/* ports */
enum {
	PORT_LEFT_INPUT = 0,
	PORT_RIGHT_INPUT,
	PORT_LEFT_OUTPUT,
	PORT_RIGHT_OUTPUT,
	PORT_FEEDBACK,
	PORT_DRY,
	PORT_WET,
	PORT_INPUT_GAIN,
	PORT_OUTPUT_GAIN,
	PORT_LOW_CUT,
	PORT_HIGH_CUT,

	PORT_COUNT,
};

#define CHANNELS 2
#define DELAYS 8
#define STEPS 4

struct channel {
	size_t index;
	delay_t delays[DELAYS * STEPS];
	delay_t mix_delays[DELAYS];
};

struct port_data {
	union {
		struct {
			const float *left_input, *right_input;
			float *left_output, *right_output;
			const float *feedback;
			const float *dry, *wet;
			const float *input_gain, *output_gain;
			const float *low_cut, *high_cut;
		} __attribute__((packed));
		const float *ports[PORT_COUNT];
	};
	float sample_rate;
	struct channel channels[CHANNELS];

	size_t shuffles[DELAYS];
	bool inverts[DELAYS];
	float hadamard_scale;

	biquad_filter_t low_cut_filters[DELAYS];
	biquad_filter_t high_cut_filters[DELAYS];
};

/* get floating point random value */
static float rand_float01(void) {

	return (float)rand() / (float)RAND_MAX;
}

static float rand_float(float low, float high) {

	return low + rand_float01() * (high - low);
}

/* create instance */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double rate,
			      const char *bundle_path, const LV2_Feature *const *features) {

	struct port_data *data = (struct port_data *)malloc(sizeof(struct port_data));
	memset(data, 0, sizeof(struct port_data));

	data->sample_rate = (float)rate;

	data->hadamard_scale = sqrt(1.f / (size_t)DELAYS);

	/* get delay times and other random values */
	srand((unsigned int)time(NULL));

	float times[DELAYS];
	for (size_t i = 0; i < DELAYS; i++) {

		float low = (float)i * 1.f / (float)DELAYS;
		float high = low + 1.f / (float)DELAYS;

		times[i] = rand_float(low, high);

		data->inverts[i] = (bool)(rand() % 2);
		data->shuffles[i] = i;
	}

	for (size_t i = 0; i < DELAYS; i++) {

		size_t j = (size_t)rand() % DELAYS;
		size_t k = (size_t)rand() % DELAYS;

		data->shuffles[j] ^= data->shuffles[k]; /* swap values */
		data->shuffles[k] ^= data->shuffles[j];
		data->shuffles[j] ^= data->shuffles[k];
	}

	/* initialize delays */
	for (size_t i = 0; i < CHANNELS; i++) {

		struct channel *channel = data->channels+i;
		channel->index = i;

		for (size_t j = 0; j < DELAYS; j++) {

			for (size_t k = 0; k < STEPS; k++) {

				/* each diffusion step has longer and longer delays */
				float low = 0.01f;
				float high = 0.04f * (float)(k + 1);

				delay_init(
					channel->delays + j * STEPS + k,
					data->sample_rate,
					low + times[j] * (high - low),
					1.f,
					0.f
				);
			}

			delay_init(
				channel->mix_delays+i,
				data->sample_rate,
				0.1f,
				0.1f,
				1.f
			);
		}
	}

	/* initialize filters */
	for (size_t i = 0; i < DELAYS; i++) {

		biquad_filter_init(
				data->low_cut_filters+i,
				BIQUAD_FILTER_TYPE_LOW_SHELF,
				data->sample_rate,
				1.f, 0.707f, -36.f
		);
		biquad_filter_init(
				data->high_cut_filters+i,
				BIQUAD_FILTER_TYPE_HIGH_SHELF,
				data->sample_rate,
				20000.f, 0.707f, -36.f
		);
	}
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

/* diffuse values in channel */
static void diffuse(
		struct port_data *pdata,
		struct channel *channel,
		float values[DELAYS],
		size_t step
) {
	for (size_t i = 0; i < DELAYS; i++) {

		values[i] = delay_process_sample(
				channel->delays + i * STEPS + step,
				values[i]
		);
		if (pdata->inverts[i])
			values[i] *= -1.f;
	}

	/* shuffle values */
	float new_values[DELAYS];

	for (size_t i = 0; i < DELAYS; i++)
		new_values[pdata->shuffles[i]] = values[i];

	memcpy(values, new_values, sizeof(new_values));

	/* hadamard matrix */
	matrix_mul_hadamard(values, DELAYS);

	for (size_t i = 0; i < DELAYS; i++)
		values[i] *= pdata->hadamard_scale;
}

/* process single channel */
static void process_channel(
		struct port_data *pdata,
		struct channel *channel,
		const float *input,
		float *output,
		float dry, float wet,
		float input_gain,
		float output_gain,
		size_t count
) {
	bool do_low_cut = *pdata->low_cut > 2.f;
	bool do_high_cut = *pdata->high_cut < 19999.f;

	for (size_t i = 0; i < count; i++) {

		float in = input[i];

		/* split and diffuse values */
		float values[DELAYS];
		for (size_t j = 0; j < DELAYS; j++)
			values[j] = in * input_gain;

		for (size_t j = 0; j < STEPS; j++)
			diffuse(pdata, channel, values, j);

		/* mix channels using the householder matrix */
		float hh_values[DELAYS];
		memcpy(hh_values, values, sizeof(hh_values));

		matrix_mul_householder(
				hh_values,
				DELAYS
		);

		/* perform filter operations and final delays */
		for (size_t j = 0; j < DELAYS; j++) {

			float feed = hh_values[j];

			if (do_low_cut)
				feed = biquad_filter_process_sample(
						pdata->low_cut_filters+j,
						channel->index,
						feed
				);
			if (do_high_cut)
				feed = biquad_filter_process_sample(
						pdata->high_cut_filters+j,
						channel->index,
						feed
				);

			values[j] = delay_process_sample2(
					channel->mix_delays+j,
					values[j],
					feed
			);
		}

		/* sum values */
		output[i] = in * dry +
			    (values[0] + values[1]) / 2.f *
			    output_gain * wet;
	}
}

/* run instance */
#define DB_CO(g) ((g) > -90.0f? powf(10.0f, (g) * 0.05f): 0.0f)

static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *pdata = (struct port_data *)instance;

	for (size_t i = 0; i < DELAYS; i++) {

		delay_set_feedback(
			pdata->channels[0].mix_delays+i,
			*pdata->feedback
		);
		delay_set_feedback(
			pdata->channels[1].mix_delays+i,
			*pdata->feedback
		);
	}

	float dry = DB_CO(*pdata->dry);
	float wet = DB_CO(*pdata->wet);
	float input_gain = DB_CO(*pdata->input_gain);
	float output_gain = DB_CO(*pdata->output_gain);

	/* set filter parameters */
	if (pdata->low_cut_filters[0].frequency != *pdata->low_cut) {

		for (size_t i = 0; i < DELAYS; i++)
			biquad_filter_init(
					pdata->low_cut_filters+i,
					BIQUAD_FILTER_TYPE_LOW_SHELF,
					pdata->sample_rate,
					*pdata->low_cut, 0.707f, -36.f
			);
	}

	if (pdata->high_cut_filters[0].frequency != *pdata->high_cut) {

		for (size_t i = 0; i < DELAYS; i++)
			biquad_filter_init(
					pdata->high_cut_filters+i,
					BIQUAD_FILTER_TYPE_HIGH_SHELF,
					pdata->sample_rate,
					*pdata->high_cut, 0.707f, -36.f
			);
	}

	/* process audio channels */
	process_channel(
			pdata,
			&pdata->channels[0],
			pdata->left_input,
			pdata->left_output,
			dry, wet,
			input_gain,
			output_gain,
			(size_t)nsamples
	);
	process_channel(
			pdata,
			&pdata->channels[1],
			pdata->right_input,
			pdata->right_output,
			dry, wet,
			input_gain,
			output_gain,
			(size_t)nsamples
	);
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
	HERB_URI,
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
