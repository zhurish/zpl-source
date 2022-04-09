#############################################################################
# DEFINE
###########################################################################
MODULEDIR = product/bsp/pal/linux
ifeq ($(strip $(ZPL_KERNEL_MODULE)),true)

OBJS	+= linux_ioctl.o

ifeq ($(strip $(ZPL_NSM_ARP)),true)
OBJS	+= linux_arp.o			
endif

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

#endif

OBJS	+= linux_vrf.o
OBJS	+= linux_netlink.o
OBJS	+= linux_nladdress.o
OBJS	+= linux_nlroute.o
OBJS	+= linux_nliface.o


ifeq ($(strip $(ZPL_NSM_FIREWALLD)),true)
OBJS	+= linux_firewalld.o		
endif
OBJS	+= linux_driver.o

# iproute
ifeq ($(strip $(ZPL_NSM_IPROUTE)),true)
OBJS	+= libnetlink.o
OBJS	+= inet_proto.o
OBJS	+= mpls_ntop.o
OBJS	+= mpls_pton.o
OBJS	+= names.o
OBJS	+= namespace.o
OBJS	+= rt_names.o
OBJS	+= utils.o
OBJS	+= ll_addr.o
OBJS	+= ll_map.o
OBJS	+= ll_proto.o
OBJS	+= ll_types.o
OBJS	+= rtm_map.o
#OBJS	+= tunnel.o
OBJS	+= ip.o
OBJS	+= iplink.o
#OBJS	+= ip6tunnel.o

#OBJS	+= ipaddress.o
#OBJS	+= ipaddrlabel.o
#OBJS	+= ipl2tp.o

#OBJS	+= iplink_bond.o
#OBJS	+= iplink_bond_slave.o
#OBJS	+= iplink_bridge.o
#OBJS	+= iplink_bridge_slave.o
#OBJS	+= iplink_ipvlan.o
#OBJS	+= iplink_macvlan.o

#OBJS	+= iplink_vlan.o
#OBJS	+= iplink_vrf.o
#OBJS	+= iplink_vxlan.o
#OBJS	+= iplink_gre.o
#OBJS	+= iplink_gre6.o
#OBJS	+= iplink_ip6tnl.o
#OBJS	+= iplink_iptnl.o
#OBJS	+= iplink_veth.o
#OBJS	+= iplink_vti.o
#OBJS	+= iplink_vti6.o

#OBJS	+= ipmaddr.o

#OBJS	+= ipmroute.o
#OBJS	+= ipneigh.o
#OBJS	+= ipnetns.o
#OBJS	+= iproute.o
#OBJS	+= iproute_lwtunnel.o

#OBJS	+= iprule.o

#OBJS	+= iptunnel.o
#OBJS	+= iptuntap.o
#OBJS	+= ipvrf.o
endif
endif
#############################################################################
# LIB
###########################################################################
LIBS = libkernel.a
