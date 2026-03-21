#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>
#include <lv2/ui/ui.h>
#include <ui.h>

#define MIXER_STRIP_URI "http://fanfavoritessofar.com/mixer-strip"

/* ui element structure */
static ui_element_t *root_element = UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {

	UI_LABEL_INIT("Input"),
	UI_BOX_INIT(UI_ORIENTATION_HORIZONTAL, {

		UI_DIAL_INIT(-12.f, 0.25f, 12.f,
			     .color = UI_COLOR_INDEX_ACCENT_RED,
			     .unit = UI_UNIT_DB,
			     .radius = UI_DIAL_RADIUS_NORMAL,
			     .base.port = 4),
		UI_DIAL_INIT(-1.f, 0.1f, 1.f,
			     .color = UI_COLOR_INDEX_ACCENT_RED,
			     .unit = UI_UNIT_PANORAMA,
			     .radius = UI_DIAL_RADIUS_NORMAL,
			     .base.port = 5),
		UI_ELEMENT_END,
	}),
	UI_SEPARATOR_INIT(UI_ORIENTATION_HORIZONTAL, 90),

	/* equalizer and compression */
	UI_LABEL_INIT("Equalizer"),
	UI_BOX_INIT(UI_ORIENTATION_HORIZONTAL, {
		UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {

			UI_DIAL_INIT(-12.f, 0.25f, 12.f,
				     .color = UI_COLOR_INDEX_ACCENT_YELLOW,
				     .unit = UI_UNIT_DB,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 6),
			UI_DIAL_INIT(-12.f, 0.25f, 12.f,
				     .color = UI_COLOR_INDEX_ACCENT_YELLOW,
				     .unit = UI_UNIT_DB,
				     .radius = UI_DIAL_RADIUS_LARGE,
				     .base.port = 8),
			UI_ELEMENT_END,
		}),
		UI_DIAL_INIT(-12.f, 0.25f, 12.f,
			     .color = UI_COLOR_INDEX_ACCENT_YELLOW,
			     .unit = UI_UNIT_DB,
			     .radius = UI_DIAL_RADIUS_LARGE,
			     .base.port = 7),
		UI_ELEMENT_END,
	}),
	UI_SEPARATOR_INIT(UI_ORIENTATION_HORIZONTAL, 90),

	UI_LABEL_INIT("Compression"),
	UI_DIAL_INIT(0, 0.1f, 1.f,
		     .color = UI_COLOR_INDEX_ACCENT_SUMMER_GREEN,
		     .radius = UI_DIAL_RADIUS_LARGE,
		     .base.port = 9),
	UI_SEPARATOR_INIT(UI_ORIENTATION_HORIZONTAL, 90),

	/* output */
	UI_LABEL_INIT("Output"),
	UI_BOX_INIT(UI_ORIENTATION_HORIZONTAL, {

		UI_DIAL_INIT(-12.f, 0.25f, 12.f,
			     .color = UI_COLOR_INDEX_ACCENT_BLUE,
			     .unit = UI_UNIT_DB,
			     .radius = UI_DIAL_RADIUS_NORMAL,
			     .base.port = 10),
		UI_DIAL_INIT(-1.f, 0.1f, 1.f,
			     .color = UI_COLOR_INDEX_ACCENT_BLUE,
			     .unit = UI_UNIT_PANORAMA,
			     .radius = UI_DIAL_RADIUS_NORMAL,
			     .base.port = 11),
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
	UI_DESCRIPTOR(MIXER_STRIP_URI, x11),
};
UI_DESCRIPTOR_FUNCTION;
