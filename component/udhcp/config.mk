#############################################################################
#ZPLM_INCLUDE += -I-$(ZPLBASE)/$(COMPONENTDIR)/$(DHCP_ROOT)
###########################################################################
MODULEDIR = component/udhcp
#OS
#OBJS += llist.o
OBJS += dhcp_arpping.o
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
ifeq ($(ZPL_BUILD_IPV6),true)
OBJS += d6_dhcpc.o
OBJS += d6_packet.o
OBJS += d6_socket.o
endif

OBJS += dhcpc.o
OBJS += dhcpd.o
OBJS += dhcprelay.o

OBJS += dhcp_api.o
#############################################################################
# LIB
###########################################################################
LIBS = libudhcp.a
