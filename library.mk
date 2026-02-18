#
# Library template Makefile
#
CC ?= cc
AR ?= ar
CFLAGS += -c -fPIC

.PHONY: all clean

all: lib$(NAME).a

OBJECTS=$(foreach source,$(SOURCES),$(source:%.c=%.o))

lib$(NAME).a: $(OBJECTS) $(DEPENDS)
	@echo AR $@
	@$(AR) r $@ $(OBJECTS)

$(OBJECTS): %.o: %.c $(SOURCE_DEPENDS)
	@echo CC $<
	@$(CC) -o $@ $< $(CFLAGS)

clean:
	@-rm lib$(NAME).a $(OBJECTS)
