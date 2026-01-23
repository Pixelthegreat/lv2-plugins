CC=cc
CFLAGS=-c -fPIC
LDFLAGS=-shared -lm

all: bitcrusher.so

bitcrusher.so: bitcrusher.o
	@echo LD bitcrusher.so
	@$(CC) -o bitcrusher.so bitcrusher.o $(LDFLAGS)

bitcrusher.o: bitcrusher.c
	@echo CC bitcrusher.c
	@$(CC) $(CFLAGS) bitcrusher.c

clean:
	rm bitcrusher.o bitcrusher.so
