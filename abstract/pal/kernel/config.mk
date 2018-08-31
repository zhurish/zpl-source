#############################################################################
# DEFINE
###########################################################################
MODULEDIR = abstract/pal/kernel

OBJS	+= ip_util.o
OBJS	+= ioctl.o
OBJS	+= ip_vlan.o
OBJS	+= ip_tunnel.o
OBJS	+= ip_brigde.o
OBJS	+= vrf.o
OBJS	+= ipforward_proc.o
OBJS	+= rt_netlink.o
OBJS	+= pal_interface.o
#############################################################################
# LIB
###########################################################################
LIBS = libpal.a
