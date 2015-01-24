include ../common.mk
include ../libtime.mk

BIN     := popcnt
SOURCES := popcnt.c
OBJECTS := $(SOURCES:%.c=%.o)

CFLAGS  += -O3 -march=native

all: $(BIN)

clean:
	$(RM) $(BIN) $(OBJECTS)

$(BIN): $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
