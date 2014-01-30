include ../common.mk
include ../time/libtime.mk

LIBS    += -lxenctrl

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
