include ../common.mk

CC ?= CC
CFLAGS += -c -fPIC
LDFLAGS += -shared -lm

all: eq4bp.so eq4bp-ui.so

eq4bp.so: eq4bp.o
	@echo LD eq4bp.so
	@$(CC) -o eq4bp.so eq4bp.o $(LINK_BIQUAD) $(LDFLAGS)

eq4bp.o: eq4bp.c
	@echo CC eq4bp.c
	@$(CC) $(CFLAGS) $(BUILD_BIQUAD) eq4bp.c

eq4bp-ui.so: eq4bp-ui.o ../ui/libui.a ../biquad/libbiquad.a
	@echo LD eq4bp-ui.so
	@$(CC) -o eq4bp-ui.so eq4bp-ui.o $(LINK_UI) $(LDFLAGS)

eq4bp-ui.o: eq4bp-ui.c ../ui/ui.h ../biquad/biquad.h
	@echo CC eq4bp-ui.c
	@$(CC) $(CFLAGS) $(BUILD_UI) eq4bp-ui.c

clean:
	rm eq4bp.o eq4bp-ui.o eq4bp.so eq4bp-ui.so
