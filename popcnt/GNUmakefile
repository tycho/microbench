include ../common.mk
include ../time/libtime.mk

BIN     := popcnt
SOURCES := popcnt.c
OBJECTS := $(SOURCES:%.c=%.o)

CC      := gcc

CFLAGS  += -O3 -march=native

all: $(BIN)

clean:
	$(RM) $(BIN) $(OBJECTS)

$(BIN): $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
