CC      := clang
RM      := rm -f
LINK    := $(CC)
CFLAGS  := -O2 -Wall -I../time
LIBS    := -L../time -ltime -lm -lrt

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
