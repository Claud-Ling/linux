#
# Makefile for the linux kernel.
#

# Common support
obj-y := board-sx6.o io.o irq.o local_timer.o serial.o soc-timer.o clock.o plat.o device.o mcuctrl.o chipver.o
obj-y += memtest.o

# SMP support ONLY available for OMAP4
obj-$(CONFIG_SMP)			+= smp.o
obj-$(CONFIG_SMP)			+= sigma-headsmp.o
obj-$(CONFIG_HOTPLUG_CPU)		+= hotplug.o
obj-$(CONFIG_SIGMA_SX6_CACHE_L2X0)	+= l2c.o 
obj-$(CONFIG_SIGMA_SX6_SMC)		+= sigma-smc.o smc.o
obj-$(CONFIG_SIGMA_TRIX_ENET)		+= ethernet.o

# Drivers

# Power Management
obj-$(CONFIG_CPU_FREQ)          += cpufreq.o

obj-$(CONFIG_PM)                += pm.o sleep.o
obj-$(CONFIG_SX6_PM_CHECK)  	+= pm-check.o 
ifeq ($(CONFIG_PM_DEBUG),y)
CFLAGS_pm.o += -DDEBUG
endif

plus_sec := $(call as-instr,.arch_extension sec,+sec)
AFLAGS_sigma-headsmp.o			:=-Wa,-march=armv7-a$(plus_sec)
AFLAGS_sigma-smc.o			:=-Wa,-march=armv7-a$(plus_sec)
AFLAGS_sleep.o	                        :=-Wa,-march=armv7-a$(plus_sec)

