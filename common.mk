BUILD_UI_X11 ?= y

BUILD_UI=-I../ui `pkg-config --cflags cairo`
LINK_UI=-L../ui -lui `pkg-config --libs cairo`

ifeq ($(BUILD_UI_X11), y)
	BUILD_UI += -DBUILD_UI_X11
	LINK_UI += -lX11
endif

DEBUG ?= y
ifeq ($(DEBUG), y)
	CFLAGS += -g
	LDFLAGS += -g
else
	CFLAGS += -O2
endif
