CFLAGS  += -I../libtime/include
LIBS    += -L../libtime -ltime -lm

ifeq ($(OSNAME),Linux)
LIBS    += -lrt
endif

ifeq ($(OSNAME),Cygwin)
LIBS    += -lwinmm
endif
