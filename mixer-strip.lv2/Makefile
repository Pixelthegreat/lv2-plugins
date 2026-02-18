include ../common.mk

NAME=mixer-strip

PLUGIN_CFLAGS += $(BUILD_BIQUAD)
PLUGIN_LDFLAGS += $(LINK_BIQUAD)
PLUGIN_DEPENDS += ../biquad/libbiquad.a

SOURCES += mixer-strip.c
UI_SOURCES += mixer-strip-ui.c

include ../plugin.mk
