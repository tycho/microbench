all:

SHELL      := /bin/bash
MAKEFLAGS  += -Rr
.SUFFIXES:

OSNAME     := $(shell uname -s || echo "not")
ifneq ($(findstring CYGWIN,$(OSNAME)),)
OSNAME     := Cygwin
endif

ifeq ($(shell type -P clang),)
CC         := gcc
else
CC         := clang
endif

LINK       := $(CC)
AR         := ar rcu
RM         := rm -f
CFLAGS     := -O3 -std=gnu99 -fno-strict-aliasing -fno-strict-overflow -fno-stack-protector -Wall
