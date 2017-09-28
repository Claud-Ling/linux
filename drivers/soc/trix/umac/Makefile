ifneq ($(CONFIG_SIGMA_SOC_UNION),)
  soc=union1
endif

ifneq ($(CONFIG_SIGMA_SOC_UNION2),)
  soc=union2
endif

$(eval header-dir=$(obj)/$(soc))
ccflags-y += -I$(header-dir) -I$(obj)

obj-y	+= umac.o
obj-y	+= umac_proc.o
obj-y	+= compensation_stub.o
obj-$(CONFIG_TRIX_LPDDR4_PHY_DOLPHIN)	+= dolphin_drift_compensation.o
