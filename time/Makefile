CC      := clang
RM      := rm -f
AR      := ar rcu
CFLAGS  := -O2 -Wall -I../include

LIB     := libtime.a
SOURCES := libtime.c
OBJECTS := $(SOURCES:%.c=%.o)

all: $(LIB)

clean:
	$(RM) $(LIB) $(OBJECTS)

$(LIB): $(OBJECTS)
	$(AR) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
