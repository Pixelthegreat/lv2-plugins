include ../common.mk

CC=cc
CFLAGS=-c -fPIC
LDFLAGS=-shared -lm

all: bitcrusher.so bitcrusher-ui.so

bitcrusher.so: bitcrusher.o
	@echo LD bitcrusher.so
	@$(CC) -o bitcrusher.so bitcrusher.o $(LDFLAGS)

bitcrusher.o: bitcrusher.c
	@echo CC bitcrusher.c
	@$(CC) $(CFLAGS) bitcrusher.c

bitcrusher-ui.so: bitcrusher-ui.o ../ui/libui.a ../ui/ui.h
	@echo LD bitcrusher-ui.so
	@$(CC) -o bitcrusher-ui.so bitcrusher-ui.o $(LINK_UI) $(LDFLAGS)

bitcrusher-ui.o: bitcrusher-ui.c
	@echo CC bitcrusher-ui.c
	@$(CC) $(CFLAGS) $(BUILD_UI) bitcrusher-ui.c

clean:
	rm bitcrusher.o bitcrusher-ui.o bitcrusher.so bitcrusher-ui.so
