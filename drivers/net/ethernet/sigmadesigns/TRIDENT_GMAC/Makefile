#
# Makefile for drivers/net/ethernet/sigmadesigns/TRIDENT_GMAC
#
# All object files to be linked


obj-$(CONFIG_SIGMA_TRIX_ENET) += TRIDENT_GMAC.o

PHY_OBJS-y +=  	comps/PhySMSC8710/src/PhySMSC8710.o

TRIDENT_GMAC-objs = src/gmac_drv.o src/remap.o src/PhyAccess.o \
		    comps/hwTRIDENTGMACEth/src/hwTRIDENTGMACEth.o \
		    comps/hwTRIDENTGMACEth/cfg/hwTRIDENTGMACEth_Cfg.o $(PHY_OBJS-y)
