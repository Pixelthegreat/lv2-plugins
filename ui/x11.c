#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include "ui.h"

static Display *display = NULL;
static size_t window_count = 0;

#define EVENT_COUNT 32
static XEvent events[EVENT_COUNT]; /* unhandled events */
static size_t event_start, event_end;

/* create window */
static void *create_window(uintptr_t parent, size_t width, size_t height) {

	if (!display) {

		display = XOpenDisplay(NULL);
		if (!display) {

			ui_set_error("Can't connect to X11 display");
			return NULL;
		}
	}

	Window wparent = (Window)parent;
	if (!wparent) wparent = XDefaultRootWindow(display);

	unsigned long black = XBlackPixel(display, XDefaultScreen(display));
	Window window = XCreateSimpleWindow(display,
					    wparent,
					    0, 0,
					    width, height,
					    0, black, black);
	if (!window) {

		ui_set_error("Can't create X11 window");
		return NULL;
	}
	window_count++;

	XSelectInput(display, window, ExposureMask |
				      PointerMotionMask | ButtonPressMask |
				      ButtonReleaseMask | StructureNotifyMask |
				      SubstructureNotifyMask);
	XMapRaised(display, window);
	XFlush(display);

	return (void *)window;
}

/* create cairo surface */
static cairo_surface_t *create_surface(void *data, size_t width, size_t height) {

	Window window = (Window)data;

	cairo_surface_t *surface = cairo_xlib_surface_create(display, window,
		XDefaultVisual(display, XDefaultScreen(display)), width, height);
	if (!surface) {

		ui_set_error("Can't create X11 Cairo surface");
		return NULL;
	}
	XFlush(display);

	return surface;
}

/* get native window handle */
static uintptr_t get_native_handle(void *data) {

	return (uintptr_t)data;
}

/* convert events */
static bool convert_event(ui_event_t *event, XEvent *xevent) {

	switch (xevent->type) {

		/* map notify */
		case MapNotify:
			event->type = UI_EVENT_TYPE_MAP;
			return true;

		/* expose */
		case Expose:
			event->type = UI_EVENT_TYPE_EXPOSE;
			return true;

		/* button press or release */
		case ButtonPress:
		case ButtonRelease:
			event->type = UI_EVENT_TYPE_BUTTON;
			event->button.pressed = (xevent->type == ButtonPress);
			event->button.code = UI_BUTTON_LEFT;
			event->button.x = xevent->xbutton.x;
			event->button.y = xevent->xbutton.y;

			switch (xevent->xbutton.button) {
				case Button1: event->button.code = UI_BUTTON_LEFT; break;
				case Button2: event->button.code = UI_BUTTON_MIDDLE; break;
				case Button3: event->button.code = UI_BUTTON_RIGHT; break;
			}
			return true;

		/* mouse motion */
		case MotionNotify:
			event->type = UI_EVENT_TYPE_MOTION;
			event->motion.x = xevent->xmotion.x;
			event->motion.y = xevent->xmotion.y;
			return true;
	}
	return false;
}

/* get next window event */
static bool get_next_event(void *data, ui_event_t *event) {

	Window window = (Window)data;

	while (XPending(display)) {

		XNextEvent(display, events + event_end++);
		event_end %= EVENT_COUNT;
	}

	for (size_t i = event_start; i != event_end; i = (i + 1) % EVENT_COUNT) {

		XEvent *xevent = events+i;
		if (xevent->xany.window == window &&
		    convert_event(event, xevent)) {

			event_start = (event_start + 1) % EVENT_COUNT;
			return true;
		}
	}
	return false;
}

/* destroy window */
static void destroy_window(void *data) {

	Window window = (Window)data;

	XDestroyWindow(display, window);
	window_count--;

	if (!window_count) {

		XCloseDisplay(display);
		display = NULL;
	}
}

/* backend */
ui_backend_t ui_backend_x11 = {
	.create_window = create_window,
	.create_surface = create_surface,
	.get_native_handle = get_native_handle,
	.get_next_event = get_next_event,
	.destroy_window = destroy_window,
};
