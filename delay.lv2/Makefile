CC ?= CC
CFLAGS += -c -fPIC
LDFLAGS += -shared -lm

all: delay.so

delay.so: delay.o
	@echo LD delay.so
	@$(CC) -o delay.so delay.o $(LDFLAGS)

delay.o: delay.c
	@echo CC delay.c
	@$(CC) $(CFLAGS) delay.c

clean:
	rm delay.o delay.so
