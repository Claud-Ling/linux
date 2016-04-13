#
# Makefile for the linux kernel.
#

# Common support
obj-y := board-generic.o io.o irq.o local_timer.o serial.o soc-timer.o clock.o device.o mcuctrl.o chipver.o otp.o lowpower.o
obj-y += memtest.o thermal.o sist-common.o ringo.o sist-soc.o sist-a9.o

# Board specific support
obj-$(CONFIG_SIGMA_SOC_SX6)		+= board-sx6.o
obj-$(CONFIG_SIGMA_SOC_SX7)		+= board-sx7.o
obj-$(CONFIG_SIGMA_SOC_UXLB)		+= board-uxl.o
obj-$(CONFIG_SIGMA_SOC_SX8)		+= board-sx8.o

# SMP support
obj-$(CONFIG_SMP)			+= smp.o
obj-$(CONFIG_SMP)			+= sigma-headsmp.o
obj-$(CONFIG_HOTPLUG_CPU)		+= hotplug.o

obj-$(CONFIG_TRIX_CACHE_L2X0)		+= cache-l2x0.o
obj-$(CONFIG_TRIX_SMC)			+= sigma-smc.o smc.o

# Drivers
obj-$(CONFIG_SIGMA_TRIX_ENET)		+= ethernet.o

# Power Management
obj-$(CONFIG_CPU_FREQ)          += cpufreq.o
obj-$(CONFIG_PM)                += pm.o sleep.o
obj-$(CONFIG_PM_CRC)  		+= pm-check.o 
ifeq ($(CONFIG_PM_DEBUG),y)
CFLAGS_pm.o += -DDEBUG
endif

# DCSN Driver
obj-$(CONFIG_TRIX_DCSN)	+= dcsn.o
# regtest utility
obj-$(CONFIG_TRIX_UTIL_REGTEST)	+= regtest.o
# bootversion
obj-$(CONFIG_TRIX_PROC_BOOT_VERSION)	+= bootversion.o

plus_sec := $(call as-instr,.arch_extension sec,+sec)
AFLAGS_sigma-headsmp.o			:=-Wa,-march=armv7-a$(plus_sec)
AFLAGS_sigma-smc.o			:=-Wa,-march=armv7-a$(plus_sec)
AFLAGS_sleep.o	                        :=-Wa,-march=armv7-a$(plus_sec)

