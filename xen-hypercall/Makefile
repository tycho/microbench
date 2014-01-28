CC      := clang
RM      := rm -f
LINK    := $(CC)
CFLAGS  := -O2 -Wall -I../time 
LIBS    := -L../time -ltime -lxenctrl -lm -lrt

BIN     := xen-hypercall
SOURCES := test_vmcall.c
OBJECTS := $(SOURCES:%.c=%.o)

all: $(BIN)

clean:
	$(RM) $(BIN) $(OBJECTS)

$(BIN): $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
