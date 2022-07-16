#############################################################################
# DEFINE
###########################################################################
MODULEDIR = product/bsp/pal/linux
ifeq ($(strip $(ZPL_KERNEL_MODULE)),true)

OBJS	+= linux_ioctl.o


#ifeq ($(strip $(ZPL_KERNEL_FORWARDING)),true)
ifeq ($(strip $(ZPL_NSM_TUNNEL)),true)
OBJS	+= linux_tunnel.o		
endif
ifeq ($(strip $(ZPL_NSM_BRIDGE)),true)
OBJS	+= linux_brigde.o		
endif

ifeq ($(strip $(ZPL_NSM_TRUNK)),true)
OBJS	+= linux_bond.o		
endif

ifeq ($(strip $(ZPL_NSM_VLANETH)),true)
OBJS	+= linux_vlaneth.o	
endif

ifeq ($(strip $(ZPL_NSM_ARP)),true)
OBJS	+= linux_arp.o	
endif

#endif

OBJS	+= linux_vrf.o
OBJS	+= linux_netlink.o
OBJS	+= linux_address.o
OBJS	+= linux_route.o
OBJS	+= linux_iface.o


ifeq ($(strip $(ZPL_NSM_FIREWALLD)),true)
OBJS	+= linux_firewalld.o		
endif
OBJS	+= linux_driver.o

endif
#############################################################################
# LIB
###########################################################################
LIBS = libkernel.a
