CFLAGS  += -I../time
LIBS    += -L../time -ltime -lm -lrt

ifeq ($(OSNAME),Cygwin)
LIBS    += -lwinmm
endif
