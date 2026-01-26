include ../common.mk

OBJECTS += ui
ifeq ($(BUILD_UI_X11), y)
	OBJECTS += x11
	CFLAGS += -DBUILD_UI_X11
endif
OBJECT_FILES=$(foreach object,$(OBJECTS),$(object).o)

CC ?= cc
AR ?= ar
CFLAGS += -c -fPIC `pkg-config --cflags cairo`

.PHONY: all clean

all: libui.a

libui.a: $(OBJECT_FILES)
	@echo AR libui.a
	@$(AR) r libui.a $(OBJECT_FILES)

$(OBJECT_FILES): %.o: %.c ui.h style.h
	@echo CC $<
	@$(CC) -o $@ $< $(CFLAGS)

clean:
	@rm libui.a $(OBJECT_FILES)
