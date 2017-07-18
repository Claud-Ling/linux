ifneq ($(CONFIG_SIGMA_SOC_UNION),)
  soc=union1
endif


$(eval header-dir=$(obj)/$(soc))
ccflags-y += -I$(header-dir) -I$(obj)

obj-y	+= umac.o
obj-y	+= umac_proc.o
