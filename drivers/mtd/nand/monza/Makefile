obj-$(CONFIG_MTD_NAND_MONZA) += hidtv_nand.o
hidtv_nand-objs += monza_nand.o monza_ecc.o

ifeq ($(CONFIG_SIGMA_SOC_SX6),y)
hidtv_nand-objs += platform_sx6_nand.o
else
 ifeq ($(CONFIG_SIGMA_SOC_SX7),y)
   hidtv_nand-objs += platform_sx6_nand.o  #TODO
 endif
endif
