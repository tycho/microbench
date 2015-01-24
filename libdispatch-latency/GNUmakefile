include ../common.mk
include ../libtime.mk

CFLAGS  += -fblocks
LIBS    += -ldispatch -lBlocksRuntime

BIN     := dispatchlatency
SOURCES := DispatchLatency/main.c
OBJECTS := $(SOURCES:%.c=%.o)

all: $(BIN)

clean:
	$(RM) $(BIN) $(OBJECTS)

$(BIN): $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
