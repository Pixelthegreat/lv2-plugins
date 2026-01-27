include ../common.mk

CC ?= CC
CFLAGS += -c -fPIC
LDFLAGS += -shared -lm

all: delay.so delay-ui.so

delay.so: delay.o
	@echo LD delay.so
	@$(CC) -o delay.so delay.o $(LDFLAGS)

delay.o: delay.c
	@echo CC delay.c
	@$(CC) $(CFLAGS) delay.c

delay-ui.so: delay-ui.o ../ui/libui.a
	@echo LD delay-ui.so
	@$(CC) -o delay-ui.so delay-ui.o $(LINK_UI) $(LDFLAGS)

delay-ui.o: delay-ui.c ../ui/ui.h
	@echo CC delay-ui.c
	@$(CC) $(CFLAGS) $(BUILD_UI) delay-ui.c

clean:
	rm delay.o delay-ui.o delay.so delay-ui.so
