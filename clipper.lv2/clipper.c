#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>

#define CLIPPER_URI "http://fanfavoritessofar.com/clipper"

/* port data */
enum {
	PORT_LEFT_INPUT = 0,
	PORT_RIGHT_INPUT,
	PORT_LEFT_OUTPUT,
	PORT_RIGHT_OUTPUT,

	PORT_INPUT_GAIN,
	PORT_CEILING,
	PORT_OUTPUT_GAIN,

	PORT_COUNT,
};
struct port_data {
	union {
		const float *ports[PORT_COUNT];
		struct {
			const float *left_input, *right_input;
			float *left_output, *right_output;
			const float *input_gain;
			const float *ceiling;
			const float *output_gain;
		} __attribute__((packed));
	};
};

/* create instance */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double sample_rate,
			      const char *bundle_path, const LV2_Feature *const *features) {

	struct port_data *data = (struct port_data *)malloc(sizeof(struct port_data));
	memset(data, 0, sizeof(struct port_data));

	return (LV2_Handle)data;
}

/* connect port */
static void connect_port(LV2_Handle instance, uint32_t port, void *data) {

	struct port_data *pdata = (struct port_data *)instance;
	if (port < PORT_COUNT) pdata->ports[port] = (const float *)data;
}

/* activate instance */
static void activate(LV2_Handle instance) {}

/* clip audio */
static void clip(struct port_data *pdata,
		 float *output,
		 const float *input,
		 float output_gain,
		 float input_gain,
		 size_t count) {

	if (*pdata->ceiling < 0.001f) {

		memcpy(output, input, sizeof(float) * count);
		return;
	}
	float inv_ceil = 1.f - *pdata->ceiling;

	for (size_t i = 0; i < count; i++) {

		float abs_y = fabs(input[i] * input_gain);
		float sign_y = input[i] < 0.001f? -1.f: 1.f;

		float w = abs_y * (1.f + *pdata->ceiling);
		float y = w;

		if (w > inv_ceil)
			y = (0.5f * (w - inv_ceil)) + inv_ceil;

		if (y > 1.f) y = 1.f;
		output[i] = y * sign_y * output_gain;
	}
}

/* run instance */
#define DB_CO(g) ((g) > -90.0f? powf(10.0f, (g) * 0.05f): 0.0f)

static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *pdata = (struct port_data *)instance;

	float input_gain = DB_CO(*pdata->input_gain);
	float output_gain = DB_CO(*pdata->output_gain);

	clip(
		pdata,
		pdata->left_output,
		pdata->left_input,
		output_gain,
		input_gain,
		(size_t)nsamples
	);
	clip(
		pdata,
		pdata->right_output,
		pdata->right_input,
		output_gain,
		input_gain,
		(size_t)nsamples
	);
}

/* deactivate instance */
static void deactivate(LV2_Handle instance) {}

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
	.URI = CLIPPER_URI,
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
