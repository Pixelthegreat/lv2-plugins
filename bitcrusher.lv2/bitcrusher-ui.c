#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <lv2/core/lv2.h>
#include <lv2/ui/ui.h>
#include <ui.h>

#define BITCRUSHER_URI "http://fanfavoritessofar.com/bitcrusher"

/* ui element structure */
static ui_element_t *root_element = UI_BOX_INIT(UI_ORIENTATION_VERTICAL, {
	UI_LABEL_INIT("Repeat Count", .base.halign = UI_ALIGN_START),
	UI_SLIDER_INIT(1.f, 1.f, 5.f, .color = UI_COLOR_INDEX_ACCENT_RED, .base.port = 0),
	UI_LABEL_INIT("Bit Rate", .base.halign = UI_ALIGN_START),
	UI_SLIDER_INIT(0, 1.f, 16.f, .color = UI_COLOR_INDEX_ACCENT_YELLOW, .base.port = 1),

	UI_ELEMENT_END,
});

/* gui data */
struct gui_data {
	ui_window_t *window;
};

#define WIDTH 512
#define HEIGHT 384

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
	UI_DESCRIPTOR(BITCRUSHER_URI, x11),
};
UI_DESCRIPTOR_FUNCTION;
