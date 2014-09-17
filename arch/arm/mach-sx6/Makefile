#
# Makefile for the linux kernel.
#

# Common support
obj-y := board-generic.o io.o irq.o local_timer.o serial.o soc-timer.o clock.o plat.o device.o mcuctrl.o chipver.o
obj-y += memtest.o

# Board specific support
obj-$(CONFIG_SIGMA_SOC_SX6)		+= board-sx6.o
obj-$(CONFIG_SIGMA_SOC_SX7)		+= board-sx7.o

# SMP support
obj-$(CONFIG_SMP)			+= smp.o
obj-$(CONFIG_SMP)			+= sigma-headsmp.o
obj-$(CONFIG_HOTPLUG_CPU)		+= hotplug.o

obj-$(CONFIG_SIGMA_CACHE_L2X0)		+= l2c.o 
obj-$(CONFIG_SIGMA_SMC)			+= sigma-smc.o smc.o

# Drivers
obj-$(CONFIG_SIGMA_TRIX_ENET)		+= ethernet.o

# Power Management
obj-$(CONFIG_CPU_FREQ)          += cpufreq.o
obj-$(CONFIG_PM)                += pm.o sleep.o
obj-$(CONFIG_PM_CRC)  		+= pm-check.o 
ifeq ($(CONFIG_PM_DEBUG),y)
CFLAGS_pm.o += -DDEBUG
endif

plus_sec := $(call as-instr,.arch_extension sec,+sec)
AFLAGS_sigma-headsmp.o			:=-Wa,-march=armv7-a$(plus_sec)
AFLAGS_sigma-smc.o			:=-Wa,-march=armv7-a$(plus_sec)
AFLAGS_sleep.o	                        :=-Wa,-march=armv7-a$(plus_sec)

