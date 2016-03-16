#
# Makefile.Inc to build platform specific test program
#
# Compiler definition
CROSS_COMPILE ?= /opt/codesourcery/arm/bin/arm-linux-

LD	:=$(CCACHE)$(CROSS_COMPILE)gcc
CC	:=$(CCACHE)$(CROSS_COMPILE)gcc
CXX	:=$(CCACHE)$(CROSS_COMPILE)g++
CPP	:=$(CC) -E
AR	:=$(CROSS_COMPILE)ar
NM	:=$(CROSS_COMPILE)nm
STRIP	:=$(CROSS_COMPILE)strip
OBJCOPY	:=$(CROSS_COMPILE)objcopy
OBJDUMP	:=$(CROSS_COMPILE)objdump
RANLIB	:=$(CROSS_COMPILE)ranlib

#Directory definition
PATH_TAG := arch
TOP_DIR := $(shell pwd -P | sed "s@/$(PATH_TAG)/.*@@g")
CUR_DIR := .

INCLUDES = 	-I$(TOP_DIR)/arm/mach-trix
