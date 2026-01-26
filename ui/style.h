#ifndef UI_STYLE_H
#define UI_STYLE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Generic style color
 */
typedef struct ui_color {
	double r, g, b;
} ui_color_t;

#define UI_COLOR_INIT(p_r, p_g, p_b) {.r = (p_r), .g = (p_g), .b = (p_b)}
#define UI_COLOR_INIT_INT(v) {.r = (double)(((v) >> 16) & 0xff) / 255.0, .g = (double)(((v) >> 8) & 0xff) / 255.0, .b = (double)((v) & 0xff) / 255.0}

/*
 * Common color indices
 */
typedef enum ui_color_index {
	UI_COLOR_INDEX_ACCENT_BLUE = 0,
	UI_COLOR_INDEX_ACCENT_LIGHT_BLUE,

	UI_COLOR_INDEX_ACCENT_RED,
	UI_COLOR_INDEX_ACCENT_LIGHT_RED,

	UI_COLOR_INDEX_ACCENT_SUMMER_GREEN,
	UI_COLOR_INDEX_ACCENT_LIGHT_SUMMER_GREEN,

	UI_COLOR_INDEX_ACCENT_YELLOW,
	UI_COLOR_INDEX_ACCENT_LIGHT_YELLOW,

	UI_COLOR_INDEX_ACCENT_WINTER_GREEN,
	UI_COLOR_INDEX_ACCENT_LIGHT_WINTER_GREEN,

	UI_COLOR_INDEX_ACCENT_VIOLET,
	UI_COLOR_INDEX_ACCENT_LIGHT_VIOLET,

	UI_COLOR_INDEX_DARK0,
	UI_COLOR_INDEX_DARK1,
	UI_COLOR_INDEX_DARK2,
	UI_COLOR_INDEX_DARK3,

	UI_COLOR_INDEX_LIGHT0,
	UI_COLOR_INDEX_LIGHT1,
	UI_COLOR_INDEX_LIGHT2,
	UI_COLOR_INDEX_LIGHT3,

	UI_COLOR_INDEX_COUNT,
} ui_color_index_t;

/*
 * Style
 */
typedef struct ui_style {
	const char *font_face;
	double font_size;
	ui_color_t colors[UI_COLOR_INDEX_COUNT];
	/*
	 * Box style
	 */
	struct {
		int spacing;
	} box;
} ui_style_t;

#ifdef UI_STYLE_IMPL
ui_style_t ui_style = {
	.font_face = "@cairo:sans-serif",
	.font_size = 13,

	.colors = {
		[UI_COLOR_INDEX_ACCENT_BLUE] = UI_COLOR_INIT_INT(0x608fbd),
		[UI_COLOR_INDEX_ACCENT_LIGHT_BLUE] = UI_COLOR_INIT_INT(0xaaccee),

		[UI_COLOR_INDEX_ACCENT_RED] = UI_COLOR_INIT_INT(0xbd608f),
		[UI_COLOR_INDEX_ACCENT_LIGHT_RED] = UI_COLOR_INIT_INT(0xeeaacc),

		[UI_COLOR_INDEX_ACCENT_SUMMER_GREEN] = UI_COLOR_INIT_INT(0x8fbd60),
		[UI_COLOR_INDEX_ACCENT_LIGHT_SUMMER_GREEN] = UI_COLOR_INIT_INT(0xcceeaa),

		[UI_COLOR_INDEX_ACCENT_YELLOW] = UI_COLOR_INIT_INT(0xbd8f60),
		[UI_COLOR_INDEX_ACCENT_LIGHT_YELLOW] = UI_COLOR_INIT_INT(0xeeccaa),

		[UI_COLOR_INDEX_ACCENT_WINTER_GREEN] = UI_COLOR_INIT_INT(0x60bd8f),
		[UI_COLOR_INDEX_ACCENT_LIGHT_WINTER_GREEN] = UI_COLOR_INIT_INT(0xaaeecc),

		[UI_COLOR_INDEX_ACCENT_VIOLET] = UI_COLOR_INIT_INT(0x8f60bd),
		[UI_COLOR_INDEX_ACCENT_LIGHT_VIOLET] = UI_COLOR_INIT_INT(0xccaaee),

		[UI_COLOR_INDEX_DARK0] = UI_COLOR_INIT_INT(0x202126),
		[UI_COLOR_INDEX_DARK1] = UI_COLOR_INIT_INT(0x28292f),
		[UI_COLOR_INDEX_DARK2] = UI_COLOR_INIT_INT(0x31333a),
		[UI_COLOR_INDEX_DARK3] = UI_COLOR_INIT_INT(0x383a42),

		[UI_COLOR_INDEX_LIGHT0] = UI_COLOR_INIT_INT(0xaeb5cd),
		[UI_COLOR_INDEX_LIGHT1] = UI_COLOR_INIT_INT(0xc3cbe7),
		[UI_COLOR_INDEX_LIGHT2] = UI_COLOR_INIT_INT(0xc8d0ec),
		[UI_COLOR_INDEX_LIGHT3] = UI_COLOR_INIT_INT(0xd7e0fe),
	},

	.box = {
		.spacing = 8,
	},
};
#else
extern ui_style_t ui_style;
#endif

#endif /* UI_STYLE_H */
