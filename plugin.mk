#
# Plugin template Makefile
#
CC ?= cc
CFLAGS += -c -fPIC
LDFLAGS += -shared -lm
UI_CFLAGS += $(BUILD_UI)
UI_LDFLAGS += $(LINK_UI)
UI_DEPENDS += ../ui/libui.a

.PHONY: all clean

ALL += $(NAME).so
ifneq ($(UI_SOURCES),)
ALL += $(NAME)-ui.so
endif

all: $(ALL)

# plugin #
OBJECTS=$(foreach source,$(SOURCES),$(source:%.c=%.o))

$(NAME).so: $(OBJECTS) $(PLUGIN_DEPENDS) $(DEPENDS)
	@echo LD $@
	@$(CC) -o $@ $(OBJECTS) $(PLUGIN_LDFLAGS) $(LDFLAGS)

$(OBJECTS): %.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) $(PLUGIN_CFLAGS) -o $@ $<

# plugin ui #
ifneq ($(UI_SOURCES),)

UI_OBJECTS=$(foreach source,$(UI_SOURCES),$(source:%.c=%.o))

$(NAME)-ui.so: $(UI_OBJECTS) $(UI_DEPENDS) $(DEPENDS)
	@echo LD $@
	@$(CC) -o $@ $(UI_OBJECTS) $(UI_LDFLAGS) $(LDFLAGS)

$(UI_OBJECTS): %.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) $(UI_CFLAGS) -o $@ $<

endif

# clean #
clean:
	@-rm $(OBJECTS) $(UI_OBJECTS) $(ALL)
