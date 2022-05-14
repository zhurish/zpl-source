#############################################################################
# DEFINE
###########################################################################
MODULEDIR = halpal/pal

OBJS	+= pal_interface.o
OBJS	+= pal_router.o		

OBJS	+= pal_global.o
ifeq ($(strip $(ZPL_NSM_ARP)),true)
OBJS	+= pal_arp.o
OBJS	+= linux_arp.o			
endif
#############################################################################
# LIB
###########################################################################
LIBS = libpal.a
