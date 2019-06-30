#############################################################################
PLM_INCLUDE += -I-$(PLBASE)/$(COMPONENTDIR)/$(UDHCPDIR)
###########################################################################
MODULEDIR = component/udhcp
#OS
#OBJS += llist.o
OBJS += dhcp_arpping.o
#OBJS += dhcp_def.o
OBJS += dhcp_lease.o
OBJS += dhcp_pool.o
OBJS += dhcp_option.o
OBJS += dhcp_rfc1035.o
#OBJS += dumpleases.o
OBJS += dhcp_packet.o
#OBJS += signalpipe.o
OBJS += dhcp_socket.o
OBJS += dhcp_util.o
OBJS += dhcp_main.o
#OBJS += xconnect.o

#OBJS += d6_dhcpc.o
#OBJS += d6_packet.o
#OBJS += d6_socket.o
OBJS += dhcpc.o
OBJS += dhcpd.o
#OBJS += dhcprelay.o

OBJS += dhcp_api.o
#############################################################################
# LIB
###########################################################################
LIBS = libudhcp.a
