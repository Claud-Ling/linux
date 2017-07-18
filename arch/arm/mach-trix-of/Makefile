#
# Makefile for the linux kernel.
#

ccflags-y := -D__LINUX__
asflags-y := -D__LINUX__

# Common support
obj-y := board-generic.o
obj-$(CONFIG_SMP) += platsmp.o sigma-headsmp.o

plus_sec := $(call as-instr,.arch_extension sec,+sec)
AFLAGS_sigma-headsmp.o		:=-Wa,-march=armv7-a$(plus_sec)
