#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>
#include <lv2/core/lv2_util.h>
#include <lv2/log/log.h>
#include <lv2/log/logger.h>
#include <lv2/midi/midi.h>
#include <lv2/urid/urid.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include "biquad.h"

#define CHIPTUNE_URI "http://fanfavoritessofar.com/chiptune"

#define TAU (M_PI * 2.f)

enum {
	WAVE_SINE = 0,
	WAVE_SQUARE,
	WAVE_TRIANGLE,
	WAVE_SAWTOOTH,

	WAVE_COUNT,
};
static float duty_cycles[3] = {
	0.125f, 0.25f, 0.5f,
};

/* port data */
enum {
	PORT_INPUT = 0,
	PORT_OUTPUT,

	PORT_MIDI_CONTROL,

	PORT_WAVE_TYPE,
	PORT_DUTY_CYCLE,

	PORT_ADSR_ENABLE,
	PORT_ATTACK,
	PORT_DECAY,
	PORT_SUSTAIN,
	PORT_RELEASE,

	PORT_COUNT,
};

#define BASE_NOTE 33
#define BASE_NOTE_HZ 55.f
#define NOTES 95

#define OVERSAMPLE 8

#define HEAD 8
#define TAIL 8

#define SUSTAIN_LEVEL 0.5f

struct parameter {
	float value, next;
	size_t time;
};

struct port_data {
	union {
		struct {
			const float *input;
			float *output;
			const LV2_Atom_Sequence *midi_control;
			const float *f_wave_type;
			const float *f_duty_cycle;
			const float *adsr_enable;
			const float *attack, *decay, *sustain, *release;
		} __attribute__((packed));
		const void *ports[PORT_COUNT];
	};
	float sample_rate;
	float sample_time;
	float sample_rate_oversample;

	LV2_URID_Map *map;
	LV2_Log_Logger logger;

	struct {
		LV2_URID midi_MidiEvent;
	} uris;

	struct {
		bool active;
		size_t head;
		size_t tail;
		float time;
		float total_time;
	} notes[NOTES];

	biquad_filter_t filter;

	int wave_type;
	float duty_cycle;

	struct parameter pitch_bend;
};

/* update parameter */
static void set_parameter(struct parameter *parameter) {

	if (!parameter->time) return;

	parameter->time--;
	if (!parameter->time) parameter->value = parameter->next;
}

/* create instance */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double sample_rate,
			      const char *bundle_path, const LV2_Feature *const *features) {

	struct port_data *data = (struct port_data *)malloc(sizeof(struct port_data));
	memset(data, 0, sizeof(struct port_data));

	data->sample_rate = (float)sample_rate;
	data->sample_rate_oversample = data->sample_rate * (float)OVERSAMPLE;
	data->sample_time = 1.f / data->sample_rate_oversample;

	/* check for features */
	const char *missing = lv2_features_query(
			features,
			LV2_LOG__log, &data->logger.log, false,
			LV2_URID__map, &data->map, true,
			NULL
	);

	lv2_log_logger_set_map(&data->logger, data->map);
	if (missing) {

		lv2_log_error(&data->logger, "Missing feature <%s>\n", missing);
		free(data);
		return NULL;
	}

	data->uris.midi_MidiEvent =
		data->map->map(data->map->handle, LV2_MIDI__MidiEvent);

	biquad_filter_init(
			&data->filter,
			BIQUAD_FILTER_TYPE_LOW_PASS,
			data->sample_rate_oversample,
			data->sample_rate / 2.f, 0.707f, 0
	);

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

/* activate note */
static void set_note(struct port_data *pdata, uint8_t note, size_t add_head) {

	if (note < BASE_NOTE || note >= BASE_NOTE+NOTES)
		return;

	note -= BASE_NOTE;
	if (pdata->notes[note].active)
		return;

	pdata->notes[note].active = true;
	pdata->notes[note].head = HEAD + add_head;
	pdata->notes[note].tail = TAIL;
	pdata->notes[note].time = 0;
	pdata->notes[note].total_time = 0;
}

/* deactivate note */
static void unset_note(struct port_data *pdata, uint8_t note, size_t add_tail) {

	if (note < BASE_NOTE || note >= BASE_NOTE+NOTES)
		return;
	note -= BASE_NOTE;

	pdata->notes[note].active = false;
	pdata->notes[note].tail += add_tail;
}

/* deactivate all notes */
static void reset_notes(struct port_data *pdata) {

	memset(&pdata->notes, 0, sizeof(pdata->notes));
}

/* generate square wave */
static float square_wave(float time, float duty_cycle) {

	return fmodf(time, TAU) > TAU * duty_cycle? -1.f: 1.f;
}

/* get wave value */
static float get_wave(struct port_data *pdata, float time) {

	switch (pdata->wave_type) {
		case WAVE_SINE:
			return sinf(time);
		case WAVE_SQUARE:
			return square_wave(time, pdata->duty_cycle);
	}
	return 0;
}

/* process note */
#define NOTE_TO_HZ(pdata, note) (BASE_NOTE_HZ * powf(2.f, ((float)(note) +\
				(pdata)->pitch_bend.value) / 12.f))

static void process_note(
		struct port_data *pdata,
		float *output,
		uint8_t note,
		size_t count
) {
	if (note < BASE_NOTE || note >= BASE_NOTE+NOTES)
		return;

	bool adsr_enable = *pdata->adsr_enable > 0.5f;

	note -= BASE_NOTE;
	if (!pdata->notes[note].tail)
		return;

	float hz = NOTE_TO_HZ(pdata, note);
	float period = 1.f / hz;

	const float env_ad = *pdata->attack + *pdata->decay;
	const float env_ads = env_ad + *pdata->sustain;

	for (size_t i = 0; i < count && pdata->notes[note].tail; i++) {

		/* adjust to pitch bend */
		float old_pitch_bend = pdata->pitch_bend.value;
		set_parameter(&pdata->pitch_bend);

		if (old_pitch_bend != pdata->pitch_bend.value)
			hz = NOTE_TO_HZ(pdata, note);

		/* oversample and filter with a low-pass to remove aliasing */
		if (pdata->notes[note].head > HEAD)
			goto after_sampling;

		float total = 0;
		for (size_t j = 0; j < OVERSAMPLE; j++) {

			float value = get_wave(
					pdata,
					pdata->notes[note].time * 2.f * M_PI * hz
			);
			total += biquad_filter_process_sample(
					&pdata->filter,
					0, value
			);
			pdata->notes[note].time += pdata->sample_time;
			pdata->notes[note].total_time += pdata->sample_time;
			pdata->notes[note].time = fmod(pdata->notes[note].time, period);
		}
after_sampling:
		/* account for adsr envelope */
		float adsr = 0;

		if (!adsr_enable) {

			adsr = 1.f;
			goto after_adsr;
		}

		if (pdata->notes[note].total_time > env_ads) {

			float time = pdata->notes[note].total_time - env_ads;
			if (time < *pdata->release)
				adsr = (1.f - (time / *pdata->release)) * SUSTAIN_LEVEL;
		}
		else if (pdata->notes[note].total_time > env_ad)
			adsr = SUSTAIN_LEVEL;

		else if (pdata->notes[note].total_time > *pdata->attack) {

			float time = pdata->notes[note].total_time - *pdata->attack;
			adsr = (1.f - (time / *pdata->decay)) *
			       (1.f - SUSTAIN_LEVEL) + SUSTAIN_LEVEL;
		}
		else if (*pdata->attack > 0.001f)
			adsr = pdata->notes[note].total_time / *pdata->attack;
after_adsr:

		/* account for fade-in and fade-out to prevent popping */
		float head = 0, tail = 1.f;
		if (pdata->notes[note].head < HEAD)
			head = (float)(HEAD - pdata->notes[note].head) / (float)HEAD;
		if (pdata->notes[note].tail < TAIL)
			tail = (float)pdata->notes[note].tail / (float)TAIL;

		pdata->notes[note].tail--;

		output[i] += total / (float)OVERSAMPLE * head * tail * adsr;

		if (pdata->notes[note].active)
			pdata->notes[note].tail++;
		if (pdata->notes[note].head)
			pdata->notes[note].head--;
	}
}

/* read lsb word */
static uint16_t read_lsb_word(const uint8_t *buffer) {

	return ((uint16_t)buffer[0] & 0x7f) |
	       (((uint16_t)buffer[1] & 0x7f) << 7);
}

/* run instance */
static void run(LV2_Handle instance, uint32_t nsamples) {

	struct port_data *pdata = (struct port_data *)instance;

	/* set parameters */
	pdata->wave_type = (int)*pdata->f_wave_type;
	pdata->duty_cycle = duty_cycles[(int)*pdata->f_duty_cycle];

	/* process midi events */
	LV2_ATOM_SEQUENCE_FOREACH(pdata->midi_control, event) {

		if (event->body.type != pdata->uris.midi_MidiEvent)
			continue;

		size_t add = 0;
		uint16_t pitch_bend;

		const uint8_t *message = (const uint8_t *)(event + 1);
		switch (lv2_midi_message_type(message)) {

			/* turn note on */
			case LV2_MIDI_MSG_NOTE_ON:
				if (event->time.frames > 0)
					add = (size_t)event->time.frames;

				set_note(pdata, message[1], add);
				break;

			/* turn note off */
			case LV2_MIDI_MSG_NOTE_OFF:
				if (event->time.frames > 0)
					add = (size_t)event->time.frames;

				unset_note(pdata, message[1], add);
				break;

			/* turn all notes off */
			case LV2_MIDI_MSG_CONTROLLER:
				if (message[1] == LV2_MIDI_CTL_ALL_NOTES_OFF)
					reset_notes(pdata);
				break;

			/* pitch bender */
			case LV2_MIDI_MSG_BENDER:
				pdata->pitch_bend.next =
					((float)read_lsb_word(message+1) -
						8192.f) / 4096.f;

				if (event->time.frames > 0)
					pdata->pitch_bend.time =
						(size_t)event->time.frames;
				else pdata->pitch_bend.time = 1;
				break;
		}
	}

	/* generate audio */
	memset(pdata->output, 0, sizeof(float) * (size_t)nsamples);

	for (uint8_t i = BASE_NOTE; i < BASE_NOTE+NOTES; i++)
		process_note(pdata, pdata->output, i, (size_t)nsamples);
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
	CHIPTUNE_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data,
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor(uint32_t index) {

	return !index? &descriptor: NULL;
}
