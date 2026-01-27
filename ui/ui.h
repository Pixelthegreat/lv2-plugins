#ifndef UI_H
#define UI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <lv2/core/lv2.h>
#include <lv2/ui/ui.h>
#include <cairo.h>
#include "style.h"

/*
 * Common result
 */
typedef enum ui_result {
	UI_RESULT_SUCCESS = 0,
	UI_RESULT_FAILURE,

	UI_RESULT_COUNT,
} ui_result_t;

/*
 * Manage errors
 */
extern void ui_set_error(const char *fmt, ...);

extern const char *ui_get_error(void);

/*
 * Miscellaneous utilities
 */
extern bool ui_endswith(const char *string, const char *suffix);

/*
 * Base UI element
 */
struct ui_element;
struct ui_window;
struct ui_event;

typedef enum ui_align {
	UI_ALIGN_START = 0,
	UI_ALIGN_CENTER,
	UI_ALIGN_END,

	UI_ALIGN_COUNT,
} ui_align_t;

typedef enum ui_orientation {
	UI_ORIENTATION_HORIZONTAL = 0,
	UI_ORIENTATION_VERTICAL,

	UI_ORIENTATION_COUNT,
} ui_orientation_t;

typedef enum ui_unit {
	UI_UNIT_NORMAL = 0,
	UI_UNIT_DB,
	UI_UNIT_FREQUENCY,

	UI_UNIT_COUNT,
} ui_unit_t;

typedef struct ui_element_ops {
	size_t size;
	void (*copy_children)(struct ui_element *element);
	void (*calculate_size)(struct ui_element *element);
	void (*calculate_position)(struct ui_element *element, struct ui_element *parent);
	bool (*process_event)(struct ui_element *element,
			      struct ui_event *event,
			      struct ui_window *window);
	void (*remote_set_value)(struct ui_element *element, float value);
	void (*draw)(struct ui_element *element, struct ui_window *window);
	void (*destroy)(struct ui_element *element);
} ui_element_ops_t;

typedef struct ui_element {
	ui_element_ops_t *ops;
	ui_align_t halign;
	ui_align_t valign;
	int x, y;
	int absx, absy; /* absolute position */
	int width, height;
	int port; /* associated plugin port */
} ui_element_t;

#define UI_ELEMENT(p) ((ui_element_t *)(p))
#define UI_ELEMENT_INIT(p_ops, ...) {\
	.ops = (p_ops),\
	.halign = UI_ALIGN_CENTER,\
	.valign = UI_ALIGN_CENTER,\
	.x = 0, .y = 0,\
	.absx = 0, .absy = 0,\
	.width = 0, .height = 0,\
	.port = -1,\
	__VA_ARGS__\
}
#define UI_ELEMENT_END (ui_element_t []){{.ops = NULL}}

/*
 * Element operations
 */
extern ui_element_t *ui_element_copy(ui_element_t *element);
extern void ui_element_calculate_size(ui_element_t *element);
extern void ui_element_calculate_position(ui_element_t *element, ui_element_t *parent);
extern bool ui_element_process_event(ui_element_t *element,
	       			     struct ui_event *event,
				     struct ui_window *window);
extern void ui_element_remote_set_value(ui_element_t *element, float value);
extern void ui_element_draw(ui_element_t *element, struct ui_window *window);
extern void ui_element_destroy(ui_element_t *element);

extern bool ui_element_contains(ui_element_t *element, int x, int y);

/*
 * Label element
 */
#define UI_LABEL_TEXT_SIZE 256

typedef struct ui_label {
	ui_element_t base;
	char text[UI_LABEL_TEXT_SIZE];
} ui_label_t;

extern ui_element_ops_t ui_element_ops_label;

#define UI_LABEL(p) ((ui_label_t *)(p))
#define UI_LABEL_INIT(p_text, ...) (ui_element_t *)(ui_label_t []){{\
	.base = UI_ELEMENT_INIT(&ui_element_ops_label),\
	.text = (p_text),\
	__VA_ARGS__\
}}

/*
 * Box element
 */
typedef struct ui_box {
	ui_element_t base;
	ui_element_t **elements;
	ui_orientation_t orientation;
} ui_box_t;

extern ui_element_ops_t ui_element_ops_box;

#define UI_BOX(p) ((ui_box_t *)(p))
#define UI_BOX_INIT(p_orientation, ...) (ui_element_t *)(ui_box_t []){{\
	.base = UI_ELEMENT_INIT(&ui_element_ops_box),\
	.orientation = (p_orientation),\
	.elements = (ui_element_t *[])__VA_ARGS__,\
}}

/*
 * Separator element
 */
typedef struct ui_separator {
	ui_element_t base;
	ui_orientation_t orientation;
	int length;
} ui_separator_t;

extern ui_element_ops_t ui_element_ops_separator;

#define UI_SEPARATOR(p) ((ui_separator_t *)(p))
#define UI_SEPARATOR_INIT(p_orientation, p_length, ...) (ui_element_t *)(ui_separator_t []){{\
	.base = UI_ELEMENT_INIT(&ui_element_ops_separator),\
	.orientation = (p_orientation),\
	.length = (p_length),\
	__VA_ARGS__\
}}

/*
 * Button element
 */
#define UI_BUTTON_TEXT_SIZE 256

typedef struct ui_button {
	ui_element_t base;
	char text[UI_BUTTON_TEXT_SIZE];
} ui_button_t;

extern ui_element_ops_t ui_element_ops_button;

#define UI_BUTTON(p) ((ui_button_t *)(p))
#define UI_BUTTON_INIT(p_text, ...) (ui_element_t *)(ui_button_t []){{\
	.base = UI_ELEMENT_INIT(&ui_element_ops_button),\
	.text = (p_text),\
	__VA_ARGS__\
}}

/*
 * Slider element
 */
typedef struct ui_slider {
	ui_element_t base;
	float start, step, end;
	ui_unit_t unit;
	ui_color_index_t color;
	float position;
	size_t stops;
} ui_slider_t;

extern ui_element_ops_t ui_element_ops_slider;

#define UI_DEFAULT_STOPS 5

#define UI_SLIDER(p) ((ui_slider_t *)(p))
#define UI_SLIDER_INIT(p_start, p_step, p_end, ...) (ui_element_t *)(ui_slider_t []){{\
	.base = UI_ELEMENT_INIT(&ui_element_ops_slider),\
	.start = (p_start), .step = (p_step), .end = (p_end),\
	.unit = UI_UNIT_NORMAL,\
	.color = UI_COLOR_INDEX_LIGHT2,\
	.position = (p_start),\
	.stops = UI_DEFAULT_STOPS,\
	__VA_ARGS__\
}}

/*
 * Dial element
 */
typedef struct ui_dial {
	ui_element_t base;
	float start, step, end;
	ui_unit_t unit;
	ui_color_index_t color;
	int radius;
	float position;
} ui_dial_t;

extern ui_element_ops_t ui_element_ops_dial;

#define UI_DIAL_RADIUS_SMALL 12
#define UI_DIAL_RADIUS_NORMAL 16
#define UI_DIAL_RADIUS_LARGE 20

#define UI_DIAL(p) ((ui_dial_t *)(p))
#define UI_DIAL_INIT(p_start, p_step, p_end, ...) (ui_element_t *)(ui_dial_t []){{\
	.base = UI_ELEMENT_INIT(&ui_element_ops_dial),\
	.start = (p_start), .step = (p_step), .end = (p_end),\
	.unit = UI_UNIT_NORMAL,\
	.color = UI_COLOR_INDEX_LIGHT2,\
	.radius = UI_DIAL_RADIUS_NORMAL,\
	.position = (p_start),\
	__VA_ARGS__\
}}

/*
 * Events
 */
typedef enum ui_button_code {
	UI_BUTTON_LEFT = 0,
	UI_BUTTON_MIDDLE,
	UI_BUTTON_RIGHT,

	UI_BUTTON_COUNT,
} ui_button_code_t;

typedef enum ui_event_type {
	UI_EVENT_TYPE_MAP = 0,
	UI_EVENT_TYPE_EXPOSE,
	UI_EVENT_TYPE_BUTTON,
	UI_EVENT_TYPE_MOTION,

	UI_EVENT_TYPE_COUNT,
} ui_event_type_t;

typedef struct ui_event {
	ui_event_type_t type;
	/*
	 * Button press or release
	 */
	struct {
		bool pressed;
		ui_button_code_t code;
		int x, y;
	} button;
	/*
	 * Mouse motion
	 */
	struct {
		int x, y;
		int xrel, yrel;
	} motion;
} ui_event_t;

/*
 * Window backend
 */
typedef struct ui_backend {
	void *(*create_window)(uintptr_t parent, size_t width, size_t height);
	cairo_surface_t *(*create_surface)(void *data, size_t width, size_t height);
	uintptr_t (*get_native_handle)(void *data);
	bool (*get_next_event)(void *data, ui_event_t *event);
	void (*destroy_window)(void *data);
} ui_backend_t;

#ifdef BUILD_UI_X11
extern ui_backend_t ui_backend_x11;
#endif

/*
 * UI window
 */
#define UI_WINDOW_MAX_PORTS 64

typedef struct ui_window {
	size_t width, height;
	void *backend_data;
	cairo_surface_t *surface;
	cairo_t *cr;
	cairo_font_extents_t font_extents;
	ui_element_t *root_element;
	bool draw_window;
	ui_element_t *focused;
	int x, y;
	LV2UI_Write_Function write_function;
	LV2UI_Controller controller;
	ui_element_t *ports[UI_WINDOW_MAX_PORTS];
} ui_window_t;

/*
 * Create window
 */
extern ui_window_t *ui_window_new_direct(ui_backend_t *backend, uintptr_t parent,
					 size_t width, size_t height);

extern ui_window_t *ui_window_new_features(const char *uri,
					   LV2UI_Controller controller,
					   LV2UI_Write_Function write_function,
					   const LV2_Feature *const *features,
					   size_t width, size_t height);

extern ui_window_t *ui_window_new_elements(const char *uri,
					   LV2UI_Controller controller,
					   LV2UI_Write_Function write_function,
					   const LV2_Feature *const *features,
					   ui_element_t *root_element);

/*
 * Get native window handle
 */
extern uintptr_t ui_window_get_native_handle(ui_window_t *window);

/*
 * Get next event
 */
extern bool ui_window_get_next_event(ui_window_t *window, ui_event_t *event);

/*
 * Update window
 */
extern void ui_window_update(ui_window_t *window);

/*
 * Draw window elements
 */
extern void ui_window_draw(ui_window_t *window);

/*
 * Destroy window
 */
extern void ui_window_destroy(ui_window_t *window);

/*
 * LV2 ui helper functions
 */
extern const void *ui_extension_data(const char *uri);

extern void ui_port_event(ui_window_t *window,
			  uint32_t port,
			  uint32_t buffer_size,
			  uint32_t format,
			  const void *buffer);

extern void ui_send_port_event(ui_window_t *window,
			       uint32_t port,
			       uint32_t buffer_size,
			       uint32_t port_protocol,
			       const void *buffer);

extern int ui_idle(LV2UI_Handle instance);

/*
 * Define LV2 UI descriptors
 */
#define UI_DESCRIPTOR_LIST static LV2UI_Descriptor descriptors[]

#define UI_DESCRIPTOR(uri, backend) {\
	uri "#ui-" #backend,\
	instantiate,\
	cleanup,\
	port_event,\
	ui_extension_data,\
}

#define UI_DESCRIPTOR_FUNCTION extern const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index) {\
	return index < sizeof(descriptors) / sizeof(descriptors[0])? descriptors + index: NULL;\
}

#endif /* UI_H */
