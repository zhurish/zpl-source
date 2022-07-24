#############################################################################
# DEFINE
###########################################################################
MODULEDIR = halpal/pal/linux

ifeq ($(strip $(ZPL_KERNEL_MODULE)),true)

OBJS	+= linux_ioctl.o


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
OBJS	+= linux_vlan.o	
OBJS	+= linux_eth.o	
endif

ifeq ($(strip $(ZPL_NSM_ARP)),true)
OBJS	+= linux_arp.o	
endif


OBJS	+= linux_vrf.o
OBJS	+= librtnl_netlink.o
OBJS	+= linux_address.o
OBJS	+= linux_route.o
OBJS	+= linux_iface.o
OBJS	+= linux_vxlan.o

ifeq ($(strip $(ZPL_NSM_FIREWALLD)),true)
OBJS	+= linux_firewalld.o		
endif
OBJS	+= linux_driver.o

LIB_IPROUTE_ENABLE=false
ifeq ($(strip $(LIB_IPROUTE_ENABLE)),true)
IPOBJS	+= ll_types.o
IPOBJS	+= ll_proto.o
IPOBJS	+= ll_map.o
IPOBJS	+= ll_addr.o
IPOBJS	+= libnetlink.o
IPOBJS	+= utils.o
IPOBJS	+= rt_names.o
IPOBJS	+= iplink.o
IPOBJS	+= iplink_vrf.o
IPOBJS	+= ip.o
IPOBJS	+= inet_proto.o		
endif


endif
#############################################################################
# LIB
###########################################################################
LIBS = libpallinux.a
