CFLAGS  += -I../time
LIBS    += -L../time -ltime -lm

ifeq ($(OSNAME),Linux)
LIBS    += -lrt
endif

ifeq ($(OSNAME),Cygwin)
LIBS    += -lwinmm
endif
