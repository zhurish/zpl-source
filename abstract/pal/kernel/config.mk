#############################################################################
# DEFINE
###########################################################################
MODULEDIR = abstract/pal/kernel
ifeq ($(strip $(PL_PAL_KERNEL_STACK)),true)

OBJS	+= kernel_ioctl.o
ifeq ($(strip $(PL_NSM_ARP)),true)
OBJS	+= kernel_arp.o			
endif
ifeq ($(strip $(PL_NSM_TUNNEL)),true)
OBJS	+= kernel_tunnel.o		
endif
ifeq ($(strip $(PL_NSM_BRIDGE)),true)
OBJS	+= kernel_brigde.o		
endif

ifeq ($(strip $(PL_NSM_TRUNK)),true)
OBJS	+= kernel_bond.o		
endif

ifeq ($(strip $(PL_NSM_VETH)),true)
OBJS	+= kernel_veth.o	
endif

OBJS	+= kernel_vrf.o
OBJS	+= kernel_ipforward.o
OBJS	+= kernel_netlink.o
OBJS	+= kernel_nladdress.o
OBJS	+= kernel_nlroute.o
OBJS	+= kernel_nllisten.o
OBJS	+= kernel_nliface.o
OBJS	+= kernel_nlload.o

ifeq ($(strip $(PL_NSM_FIREWALLD)),true)
OBJS	+= kernel_firewalld.o		
endif
OBJS	+= kernel_driver.o
OBJS	+= pal_driver.o

endif
#############################################################################
# LIB
###########################################################################
LIBS = libpal.a
