include ../common.mk

NAME=ui
CFLAGS += `pkg-config --cflags cairo`
SOURCES += ui.c
SOURCE_DEPENDS += ui.h style.h

ifeq ($(BUILD_UI_X11), y)
	SOURCES += x11.c
	CFLAGS += -DBUILD_UI_X11
endif

include ../library.mk
