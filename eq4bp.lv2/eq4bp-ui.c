#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>
#include <lv2/ui/ui.h>
#include <ui.h>

#define EQ4BP_URI "http://fanfavoritessofar.com/eq4bp"

/* ui element structure */
static ui_element_t *root_element = UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {

	UI_BOX_INIT(UI_ORIENTATION_HORIZONTAL, {

		/* filter 0 (low-pass) */
		UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {
			UI_LABEL_INIT("Frequency"),
			UI_DIAL_INIT(1.f, 20.f, 20000.f,
				     .color = UI_COLOR_INDEX_ACCENT_RED,
				     .unit = UI_UNIT_FREQUENCY,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 4),
			UI_LABEL_INIT("Gain"),
			UI_DIAL_INIT(-12.f, 0.25f, 12.f,
				     .color = UI_COLOR_INDEX_ACCENT_RED,
				     .unit = UI_UNIT_DB,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 6),

			UI_ELEMENT_END,
		}),
		UI_SEPARATOR_INIT(UI_ORIENTATION_VERTICAL, 280),

		/* filter 1 (bell) */
		UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {
			UI_LABEL_INIT("Frequency"),
			UI_DIAL_INIT(1.f, 20.f, 20000.f,
				     .color = UI_COLOR_INDEX_ACCENT_YELLOW,
				     .unit = UI_UNIT_FREQUENCY,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 7),
			UI_LABEL_INIT("Width / Q"),
			UI_DIAL_INIT(0.1f, 0.025f, 2.f,
				     .color = UI_COLOR_INDEX_ACCENT_YELLOW,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 8),
			UI_LABEL_INIT("Gain"),
			UI_DIAL_INIT(-12.f, 0.25f, 12.f,
				     .color = UI_COLOR_INDEX_ACCENT_YELLOW,
				     .unit = UI_UNIT_DB,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 9),

			UI_ELEMENT_END,
		}),
		UI_SEPARATOR_INIT(UI_ORIENTATION_VERTICAL, 280),

		/* filter 2 (bell) */
		UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {
			UI_LABEL_INIT("Frequency"),
			UI_DIAL_INIT(1.f, 20.f, 20000.f,
				     .color = UI_COLOR_INDEX_ACCENT_SUMMER_GREEN,
				     .unit = UI_UNIT_FREQUENCY,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 10),
			UI_LABEL_INIT("Width / Q"),
			UI_DIAL_INIT(0.1f, 0.025f, 2.f,
				     .color = UI_COLOR_INDEX_ACCENT_SUMMER_GREEN,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 11),
			UI_LABEL_INIT("Gain"),
			UI_DIAL_INIT(-12.f, 0.25f, 12.f,
				     .color = UI_COLOR_INDEX_ACCENT_SUMMER_GREEN,
				     .unit = UI_UNIT_DB,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 12),

			UI_ELEMENT_END,
		}),
		UI_SEPARATOR_INIT(UI_ORIENTATION_VERTICAL, 280),

		/* filter 3 (high-pass) */
		UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {
			UI_LABEL_INIT("Frequency"),
			UI_DIAL_INIT(1.f, 20.f, 20000.f,
				     .color = UI_COLOR_INDEX_ACCENT_BLUE,
				     .unit = UI_UNIT_FREQUENCY,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 13),
			UI_LABEL_INIT("Gain"),
			UI_DIAL_INIT(-12.f, 0.25f, 12.f,
				     .color = UI_COLOR_INDEX_ACCENT_BLUE,
				     .unit = UI_UNIT_DB,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 15),

			UI_ELEMENT_END,
		}),
		UI_SEPARATOR_INIT(UI_ORIENTATION_VERTICAL, 280),

		/* input and output */
		UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {
			UI_LABEL_INIT("Input"),
			UI_DIAL_INIT(-12.f, 0.25f, 12.f,
				     .unit = UI_UNIT_DB,
				     .base.port = 16),
			UI_LABEL_INIT("Output"),
			UI_DIAL_INIT(-12.f, 0.26f, 12.f,
				     .unit = UI_UNIT_DB,
				     .base.port = 17),

			UI_ELEMENT_END,
		}),

		UI_ELEMENT_END,
	}),
	UI_ELEMENT_END,
});

/* gui data */
struct gui_data {
	ui_window_t *window;
};

/* instantiate */
static LV2UI_Handle instantiate(const struct LV2UI_Descriptor *descriptor,
				const char *uri,
				const char *bundle_path,
				LV2UI_Write_Function write_function,
				LV2UI_Controller controller,
				LV2UI_Widget *widget,
				const LV2_Feature *const *features) {


	ui_window_t *window = ui_window_new_elements(descriptor->URI,
						     controller,
						     write_function,
						     features,
						     root_element);
	if (!window) {

		fprintf(stderr, "ui: %s\n", ui_get_error());
		return NULL;
	}

	struct gui_data *gdata = (struct gui_data *)malloc(sizeof(struct gui_data));
	memset(gdata, 0, sizeof(struct gui_data));

	gdata->window = window;
	*widget = (LV2UI_Widget)ui_window_get_native_handle(window);

	return gdata;
}

/* clean up */
static void cleanup(LV2UI_Handle instance) {

	struct gui_data *gdata = (struct gui_data *)instance;

	ui_window_destroy(gdata->window);

	free(gdata);
}

/* port event */
static void port_event(LV2UI_Handle instance,
		       uint32_t port,
		       uint32_t buffer_size,
		       uint32_t format,
		       const void *buffer) {

	struct gui_data *gdata = (struct gui_data *)instance;
	ui_port_event(gdata->window, port, buffer_size, format, buffer);
}

/* idle callback */
extern int ui_idle(LV2UI_Handle instance) {

	struct gui_data *gdata = (struct gui_data *)instance;

	ui_window_update(gdata->window);
	return 0;
}

/* descriptors */
UI_DESCRIPTOR_LIST = {
	UI_DESCRIPTOR(EQ4BP_URI, x11),
};
UI_DESCRIPTOR_FUNCTION;
