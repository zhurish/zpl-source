#############################################################################
# DEFINE
###########################################################################
MODULEDIR = cli/nsm
#OS
OBJS	+= cmd_interface.o
OBJS	+= cmd_routerid.o
OBJS	+= cmd_vrf.o
OBJS	+= cmd_mac.o
OBJS	+= cmd_arp.o
OBJS	+= cmd_vlan.o
OBJS	+= cmd_port.o
OBJS	+= cmd_route.o
OBJS	+= cmd_qos.o
OBJS	+= cmd_trunk.o
OBJS	+= cmd_dos.o
OBJS	+= cmd_dot1x.o
OBJS	+= cmd_mirror.o
#############################################################################
# LIB
###########################################################################
LIBS = libclinsm.a
