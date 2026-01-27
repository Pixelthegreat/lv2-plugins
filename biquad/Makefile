include ../common.mk

CC ?= cc
AR ?= ar
CFLAGS += -c -fPIC

.PHONY: all clean

all: libbiquad.a

libbiquad.a: biquad.o
	@echo AR libbiquad.a
	@$(AR) r libbiquad.a biquad.o

biquad.o: biquad.c biquad.h
	@echo CC biquad.c
	@$(CC) $(CFLAGS) biquad.c

clean:
	@rm biquad.o libbiquad.a
