#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#define UI_STYLE_IMPL
#include "ui.h"
#include "style.h"

/* set and get error string */
#define ERRBUFSZ 256
static char errbuf[ERRBUFSZ];
static bool errset = false;

extern void ui_set_error(const char *fmt, ...) {

	va_list args;
	va_start(args, fmt);
	vsnprintf(errbuf, ERRBUFSZ, fmt, args);
	va_end(args);

	errset = true;
}

extern const char *ui_get_error(void) {

	return errset? errbuf: NULL;
}

/* string ends with other string */
extern bool ui_endswith(const char *string, const char *suffix) {

	size_t l1 = strlen(string);
	size_t l2 = strlen(suffix);

	if (l1 < l2) return false;
	return !strcmp(string+l1-l2, suffix);
}

/* set color from index */
static void set_color(cairo_t *cr, ui_color_index_t index) {

	ui_color_t *color = ui_style.colors+index;
	cairo_set_source_rgb(cr, color->r, color->g, color->b);
}

/* label element */
#define DEFAULT_TEXT_WIDTH 96
#define DEFAULT_TEXT_HEIGHT 24

static void label_calculate_size(ui_element_t *element) {

	element->width = DEFAULT_TEXT_WIDTH;
	element->height = DEFAULT_TEXT_HEIGHT;
}

static void label_draw(ui_element_t *element, ui_window_t *window) {

	ui_label_t *label = UI_LABEL(element);

	cairo_text_extents_t extents;
	cairo_text_extents(window->cr, label->text, &extents);

	int width = (int)extents.width;
	int height = (int)extents.height;

	int x = element->absx + (element->halign * (element->width - width)) / 2;
	int y = element->absy + (element->valign * (element->height - height)) / 2;

	set_color(window->cr, UI_COLOR_INDEX_LIGHT3);
	cairo_move_to(window->cr, (double)x, (double)y + window->font_extents.height);
	cairo_show_text(window->cr, label->text);
}

ui_element_ops_t ui_element_ops_label = {
	.calculate_size = label_calculate_size,
	.draw = label_draw,
};

/* box element */
static void box_calculate_size(ui_element_t *element) {

	ui_box_t *box = UI_BOX(element);

	element->width = 0;
	element->height = 0;

	ui_element_t **elements = box->elements;
	for (; (*elements)->ops; elements++) {

		ui_element_t *child = *elements;
		ui_element_calculate_size(child);

		/* vertical */
		if (box->orientation == UI_ORIENTATION_VERTICAL) {

			if (child->width > element->width)
				element->width = child->width;
			element->height += child->height;

			if ((*(elements+1))->ops) element->height += ui_style.box.spacing;
		}

		/* horizontal */
		else {

			if (child->height > element->height)
				element->height = child->height;
			element->width += child->width;

			if ((*(elements+1))->ops) element->width += ui_style.box.spacing;
		}
	}

	/* position elements */
	int position = 0;
	elements = box->elements;
	for (; (*elements)->ops; elements++) {

		ui_element_t *child = *elements;

		/* vertical */
		if (box->orientation == UI_ORIENTATION_VERTICAL) {

			child->x = (child->halign * (element->width - child->width)) / 2;
			child->y = position;

			position += child->height;
		}

		/* horizontal */
		else {

			child->x = position;
			child->y = (child->valign * (element->height - child->height)) / 2;

			position += child->width;
		}

		position += ui_style.box.spacing;
	}
}

static void box_calculate_position(ui_element_t *element, ui_element_t *parent) {

	ui_box_t *box = UI_BOX(element);

	ui_element_t **elements = box->elements;
	for (; (*elements)->ops; elements++) {

		ui_element_t *child = *elements;

		child->absx = element->absx + child->x;
		child->absy = element->absy + child->y;

		ui_element_calculate_position(child, element);
	}
}

static bool box_process_event(ui_element_t *element,
			      ui_event_t *event,
			      ui_window_t *window) {

	ui_box_t *box = UI_BOX(element);

	ui_element_t **elements = box->elements;
	for (; (*elements)->ops; elements++) {

		if (!ui_element_process_event(*elements, event, window))
			return false;
	}
	return true;
}

static void box_draw(ui_element_t *element, ui_window_t *window) {

	ui_box_t *box = UI_BOX(element);

	ui_element_t **elements = box->elements;
	for (; (*elements)->ops; elements++)
		ui_element_draw(*elements, window);
}

ui_element_ops_t ui_element_ops_box = {
	.calculate_size = box_calculate_size,
	.calculate_position = box_calculate_position,
	.process_event = box_process_event,
	.draw = box_draw,
};

/* separator element */
static void separator_calculate_size(ui_element_t *element) {

	element->width = 1;
	element->height = 1;
}

static void separator_draw(ui_element_t *element, ui_window_t *window) {
}

ui_element_ops_t ui_element_ops_separator = {
	.calculate_size = separator_calculate_size,
	.draw = separator_draw,
};

/* button element */
static void button_calculate_size(ui_element_t *element) {

	element->width = DEFAULT_TEXT_WIDTH;
	element->height = DEFAULT_TEXT_HEIGHT;
}

static void button_draw(ui_element_t *element, ui_window_t *window) {
}

ui_element_ops_t ui_element_ops_button = {
	.calculate_size = button_calculate_size,
	.draw = button_draw,
};

/* slider element */
#define SLIDER_WIDTH 256
#define SLIDER_HEIGHT 32
#define SLIDER_CONTROL_WIDTH 6
#define SLIDER_SLOT_WIDTH 6
#define SLIDER_LABEL_AREA 12
#define SLIDER_LABEL_WIDTH 24
#define SLIDER_VALUE_AREA 56
#define SLIDER_VALUE_SPACING 8

static const char *get_slider_value_format(float *position, float step, ui_unit_t unit) {

	bool ignore_decimal = (step == 1.f);

	switch (unit) {

		/* decibels (dB) */
		case UI_UNIT_DB:
			return "%.1f dB";

		/* frequency (Hz, kHz) */
		case UI_UNIT_FREQUENCY:
			if (*position < 1000.f)
				return "%.0f Hz";
			*position /= 1000.f;
			return "%.1f kHz";
	}
	return ignore_decimal? "%.0f": "%.3f";
}

static void slider_calculate_size(ui_element_t *element) {

	element->width = SLIDER_WIDTH;
	element->height = SLIDER_HEIGHT;
}

static void slider_set_position(ui_element_t *element, int x, int y) {

	if (!ui_element_contains(element, x, element->absy+1))
		return;

	ui_slider_t *slider = UI_SLIDER(element);

	slider->position = (float)(x - element->absx) /
			   (float)(SLIDER_WIDTH - SLIDER_VALUE_AREA - SLIDER_CONTROL_WIDTH) *
			   (slider->end - slider->start) + slider->start;

	if (slider->position < slider->start) slider->position = slider->start;
	if (slider->position > slider->end) slider->position = slider->end;
	slider->position = roundf(slider->position / slider->step) * slider->step;
}

static bool slider_process_event(ui_element_t *element,
				 ui_event_t *event,
				 ui_window_t *window) {

	bool prop = false;

	/* set focused */
	if (!window->focused &&
	    event->type == UI_EVENT_TYPE_BUTTON && event->button.pressed &&
	    ui_element_contains(element, event->button.x, event->button.y)) {

		window->focused = element;
		window->draw_window = true;
		slider_set_position(element, event->button.x, event->button.y);
	}

	/* remove focus */
	else if (window->focused == element &&
		 event->type == UI_EVENT_TYPE_BUTTON &&
		 !event->button.pressed) {

		window->focused = NULL;
		window->draw_window = true;
	}

	/* move slider */
	else if (window->focused == element &&
		 event->type == UI_EVENT_TYPE_MOTION) {

		slider_set_position(element, event->motion.x, event->motion.y);
		window->draw_window = true;
	}

	else prop = true;
	return prop;
}

static void slider_draw(ui_element_t *element, ui_window_t *window) {

	ui_slider_t *slider = UI_SLIDER(element);

	char value[64];
	float f_position = slider->position;
	const char *value_fmt = get_slider_value_format(&f_position, slider->step, slider->unit);

	snprintf(value, sizeof(value), value_fmt, f_position);

	double position = (double)((slider->position - slider->start) /
				   (slider->end - slider->start));
	position *= (double)(SLIDER_WIDTH - SLIDER_VALUE_AREA - SLIDER_CONTROL_WIDTH);

	/* draw controls */
	set_color(window->cr, window->focused == element? slider->color+1: slider->color);
	cairo_rectangle(window->cr, (double)element->absx,
			(double)element->absy + (SLIDER_HEIGHT -
			 SLIDER_LABEL_AREA - SLIDER_SLOT_WIDTH) / 2,
			SLIDER_WIDTH - SLIDER_VALUE_AREA,
			SLIDER_SLOT_WIDTH);
	cairo_fill(window->cr);

	set_color(window->cr, window->focused == element? slider->color: slider->color+1);
	cairo_rectangle(window->cr, (double)element->absx + floor(position),
			(double)element->absy, SLIDER_CONTROL_WIDTH,
			SLIDER_HEIGHT - SLIDER_LABEL_AREA);
	cairo_fill(window->cr);

	/* draw text */
	set_color(window->cr, UI_COLOR_INDEX_LIGHT3);
	cairo_move_to(window->cr,
		      (double)element->absx + SLIDER_WIDTH -
		      SLIDER_VALUE_AREA + SLIDER_VALUE_SPACING,
		      (double)element->absy + window->font_extents.height);
	cairo_show_text(window->cr, value);
}

ui_element_ops_t ui_element_ops_slider = {
	.calculate_size = slider_calculate_size,
	.process_event = slider_process_event,
	.draw = slider_draw,
};

/* dial element */
#define DIAL_END_WIDTH 6
#define DIAL_MAX_ANGLE 0.083333f
#define DIAL_MIN_ANGLE 0.416666f
#define DIAL_RANGE 0.666666f
#define DIAL_WIDTH 0.20

#ifndef TAU
#define TAU (M_PI * 2)
#endif
#ifndef HALF_PI
#define HALF_PI (M_PI / 2)
#endif

static void dial_calculate_size(ui_element_t *element) {

	ui_dial_t *dial = UI_DIAL(element);

	element->width = dial->radius * 2;
	element->height = dial->radius * 2 + DEFAULT_TEXT_HEIGHT;

	if (DEFAULT_TEXT_WIDTH > element->width)
		element->width = DEFAULT_TEXT_WIDTH;
}

static bool dial_process_event(ui_element_t *element,
			       ui_event_t *event,
			       ui_window_t *window) {

	bool prop = false;
	ui_dial_t *dial = UI_DIAL(element);

	/* set focused */
	if (!window->focused &&
	    event->type == UI_EVENT_TYPE_BUTTON && event->button.pressed &&
	    ui_element_contains(element, event->button.x, event->button.y)) {

		window->focused = element;
		window->draw_window = true;
	}

	/* remove focus */
	else if (window->focused == element &&
		 event->type == UI_EVENT_TYPE_BUTTON &&
		 !event->button.pressed) {

		window->focused = NULL;
		window->draw_window = true;
	}

	/* move dial */
	else if (window->focused == element &&
		 event->type == UI_EVENT_TYPE_MOTION) {

		dial->position += (float)-event->motion.yrel * dial->step;

		if (dial->position < dial->start) dial->position = dial->start;
		if (dial->position > dial->end) dial->position = dial->end;
		window->draw_window = true;
	}

	else prop = true;
	return prop;
}

static void dial_draw(ui_element_t *element, ui_window_t *window) {

	ui_dial_t *dial = UI_DIAL(element);

	double centerx = (double)(element->absx + DEFAULT_TEXT_WIDTH / 2);
	double centery = (double)(element->absy + dial->radius);

	/* draw base */
	set_color(window->cr, window->focused == element? dial->color+1: dial->color);
	cairo_arc(window->cr,
		  centerx, centery,
		  (double)dial->radius,
		  0, TAU);
	cairo_fill(window->cr);

	set_color(window->cr, window->focused == element? dial->color: dial->color+1);
	cairo_arc(window->cr,
		  centerx, centery,
		  (double)(dial->radius - DIAL_END_WIDTH),
		  0, TAU);
	cairo_fill(window->cr);

	/* draw indicator bit */
	float position = ((dial->position - dial->start) /
			  (dial->end - dial->start)) *
			  DIAL_RANGE + DIAL_MIN_ANGLE;
	double start_angle = (double)position * TAU + DIAL_WIDTH;
	double end_angle = (double)position * TAU - DIAL_WIDTH;

	cairo_arc(window->cr,
		  centerx, centery,
		  (double)(dial->radius - DIAL_END_WIDTH),
		  start_angle, end_angle);
	cairo_arc(window->cr,
		  centerx, centery,
		  (double)dial->radius,
		  end_angle, start_angle);
	cairo_close_path(window->cr);
	cairo_fill(window->cr);

	/* draw label */
	char value[64];
	float f_position = dial->position;
	const char *value_fmt = get_slider_value_format(&f_position, dial->step, dial->unit);

	snprintf(value, sizeof(value), value_fmt, f_position);

	cairo_text_extents_t extents;
	cairo_text_extents(window->cr, value, &extents);

	double textx = (double)element->absx + (DEFAULT_TEXT_WIDTH - extents.width) / 2;
	double texty = (double)(element->absy + dial->radius * 2) +
			(DEFAULT_TEXT_HEIGHT - extents.height) / 2;

	set_color(window->cr, UI_COLOR_INDEX_LIGHT3);
	cairo_move_to(window->cr, textx, texty + extents.height);
	cairo_show_text(window->cr, value);
}

ui_element_ops_t ui_element_ops_dial = {
	.calculate_size = dial_calculate_size,
	.process_event = dial_process_event,
	.draw = dial_draw,
};

/* calculate element size */
extern void ui_element_calculate_size(ui_element_t *element) {

	if (element->ops->calculate_size)
		element->ops->calculate_size(element);
}

/* calculate element position */
extern void ui_element_calculate_position(ui_element_t *element, ui_element_t *parent) {

	if (element->ops->calculate_position)
		element->ops->calculate_position(element, parent);
}

/* process event */
extern bool ui_element_process_event(ui_element_t *element,
	       			     ui_event_t *event,
				     ui_window_t *window) {

	if (element->ops->process_event)
		return element->ops->process_event(element, event, window);
	return true;
}

/* draw element */
extern void ui_element_draw(ui_element_t *element, ui_window_t *window) {

	if (element->ops->draw)
		element->ops->draw(element, window);
}

/* check if element contains point */
extern bool ui_element_contains(ui_element_t *element, int x, int y) {

	return x > element->absx && x < (element->absx + element->width) &&
	       y > element->absy && y < (element->absy + element->height);
}

/* create window */
static ui_backend_t *selected_backend = NULL;

extern ui_window_t *ui_window_new_direct(ui_backend_t *backend, uintptr_t parent,
					 size_t width, size_t height) {

	if (!backend || (selected_backend && selected_backend != backend)) {

		ui_set_error("Invalid backend");
		return NULL;
	}
	selected_backend = backend;

	void *data = backend->create_window(parent, width, height);
	if (!data) return NULL;

	cairo_surface_t *surface = backend->create_surface(data, width, height);
	if (!surface) {

		backend->destroy_window(data);
		return NULL;
	}

	ui_window_t *window = (ui_window_t *)malloc(sizeof(ui_window_t));
	memset(window, 0, sizeof(ui_window_t));

	window->width = width;
	window->height = height;
	window->backend_data = data;
	window->surface = surface;
	window->cr = cairo_create(surface);

	if (!window->cr) {

		ui_set_error("Can't create Cairo context");
		ui_window_destroy(window);
		return NULL;
	}
	cairo_select_font_face(window->cr,
			       ui_style.font_face,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(window->cr, ui_style.font_size);
	cairo_font_extents(window->cr, &window->font_extents);

	return window;
}

/* create window with extra info */
extern ui_window_t *ui_window_new_features(const char *uri,
					   const LV2_Feature *const *features,
					   size_t width, size_t height) {

	ui_backend_t *backend;

	if (ui_endswith(uri, "#ui-x11"))
		backend = &ui_backend_x11;
	else {

		ui_set_error("Can't select appropriate backend");
		return NULL;
	}

	/* get parent and resize handle */
	uintptr_t parent = 0;
	LV2UI_Resize *resize = NULL;

	for (int i = 0; features && features[i]; i++) {
		if (!strcmp(features[i]->URI, LV2_UI__parent))
			parent = (uintptr_t)features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize))
			resize = (LV2UI_Resize *)features[i]->data;
	}

	ui_window_t *window = ui_window_new_direct(backend, parent, width, height);
	if (!window) return NULL;

	if (resize) resize->ui_resize(resize->handle, width, height);
	return window;
}

/* create window with extra info and elements */
extern ui_window_t *ui_window_new_elements(const char *uri,
					   const LV2_Feature *const *features,
					   ui_element_t *root_element) {

	ui_element_calculate_size(root_element);

	root_element->absx = ui_style.box.spacing;
	root_element->absy = ui_style.box.spacing;

	ui_element_calculate_position(root_element, NULL);

	ui_window_t *window = ui_window_new_features(uri, features,
		(size_t)root_element->width + (size_t)ui_style.box.spacing * 2,
		(size_t)root_element->height + (size_t)ui_style.box.spacing * 2);
	if (!window) return NULL;

	window->root_element = root_element;
	return window;
}

/* get native window handle */
extern uintptr_t ui_window_get_native_handle(ui_window_t *window) {

	return selected_backend->get_native_handle(window->backend_data);
}

/* get next window event */
extern bool ui_window_get_next_event(ui_window_t *window, ui_event_t *event) {

	return selected_backend->get_next_event(window->backend_data, event);
}

/* update window */
extern void ui_window_update(ui_window_t *window) {

	ui_event_t event;
	while (ui_window_get_next_event(window, &event)) {

		switch (event.type) {

			/* expose or map */
			case UI_EVENT_TYPE_MAP:
			case UI_EVENT_TYPE_EXPOSE:
				window->draw_window = true;
				break;

			/* motion (fall-through intentional) */
			case UI_EVENT_TYPE_MOTION:
				event.motion.xrel = event.motion.x - window->x;
				event.motion.yrel = event.motion.y - window->y;
				window->x = event.motion.x;
				window->y = event.motion.y;

			/* element event */
			default:
				ui_element_process_event(
					window->root_element,
					&event, window);
				break;
		}
	}

	/* draw window contents */
	if (window->draw_window) ui_window_draw(window);
	window->draw_window = false;
}

/* draw window elements */
extern void ui_window_draw(ui_window_t *window) {

	cairo_save(window->cr);

	set_color(window->cr, UI_COLOR_INDEX_DARK0);
	cairo_paint(window->cr);

	ui_element_draw(window->root_element, window);

	cairo_restore(window->cr);
	cairo_surface_flush(window->surface);
}

/* destroy window */
extern void ui_window_destroy(ui_window_t *window) {

	if (window->cr) cairo_destroy(window->cr);
	cairo_surface_destroy(window->surface);

	selected_backend->destroy_window(window->backend_data);
}

/* lv2 extension data helper function */
extern const void *ui_extension_data(const char *uri) {

	static const LV2UI_Idle_Interface idle_iface = {ui_idle};
	if (!strcmp(uri, LV2_UI__idleInterface))
		return &idle_iface;
	return NULL;
}
