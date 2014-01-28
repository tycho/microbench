CC      := clang
RM      := rm -f
LINK    := $(CC)
CFLAGS  := -O2 -Wall -fblocks -I../time 
LIBS    := -L../time -ltime -ldispatch -lBlocksRuntime -lm -lrt

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
