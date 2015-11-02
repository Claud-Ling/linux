#
# Makefile to build platform specific test program
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
SCRIPT_DIR := $(CUR_DIR)/script
TEMP_BUILD_DIR := .obj

INC_INSTALL_DIR := $(INC_ROOT)/Utility/TridUtil

TARGET	:= read_fuse

INCLUDES = 	-I$(TOP_DIR)/arm/mach-trix \
		-I$(CUR_DIR)/inc 

CFLAGS += -Werror $(INCLUDES)

FUSEMAP_INC := $(TOP_DIR)/arch/arm/mach-trix/fuse_data_map.inc
AUTOFUSE_H := inc/autofuse.h
GENFUSE := $(SCRIPT_DIR)/genfuseheader.awk
SRCS	:= proc_fuse.c otp_test.c

VPATH	:= .:src
OBJ_DIR := $(TEMP_BUILD_DIR)/
OBJS	:= $(patsubst %.c, $(OBJ_DIR)%.o, $(filter %.c, $(SRCS)))

all: create_obj_dir $(TARGET)

$(TARGET) : $(AUTOFUSE_H) $(OBJS)
	$(CC) $(OBJS) -o $@

create_obj_dir:
	@mkdir -p $(OBJ_DIR)

$(AUTOFUSE_H) : $(FUSEMAP_INC) $(GENFUSE)
	$(GENFUSE) $< > $@

$(patsubst %.c, $(OBJ_DIR)%.o, $(SRCS)): $(OBJ_DIR)%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ -g -Wp,-MD,$(OBJ_DIR).$(<F).dep,-MT,$@

clean cleanall:
	rm  -fr *.o  $(OBJ_DIR)

.SUFFIXES:

IGNORE=${wildcard $(OBJ_DIR).*.c.dep}
-include ${IGNORE}

PHONY += all clean cleanall $(TARGET) $(AUTOFUSE_H)

