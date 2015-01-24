include ../common.mk
include ../libtime.mk

BIN     := hz
SOURCES := hz.c
OBJECTS := $(SOURCES:%.c=%.o)

all: $(BIN)

clean:
	$(RM) $(BIN) $(OBJECTS)

$(BIN): $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
