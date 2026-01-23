CC ?= CC
CFLAGS += -c -fPIC
LDFLAGS += -shared -lm

all: eq4bp.so

eq4bp.so: eq4bp.o
	@echo LD eq4bp.so
	@$(CC) -o eq4bp.so eq4bp.o $(LDFLAGS)

eq4bp.o: eq4bp.c
	@echo CC eq4bp.c
	@$(CC) $(CFLAGS) eq4bp.c

clean:
	rm eq4bp.o eq4bp.so
