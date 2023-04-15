#############################################################################
#ZPLM_INCLUDE += -I-$(ZPLBASE)/$(COMPONENTDIR)/$(DHCP_ROOT)
###########################################################################

#OS
#vpath %.c udhcp

OBJS += dhcp_arpping.o
OBJS += dhcp_lease.o
OBJS += dhcp_pool.o
OBJS += dhcp_option.o
OBJS += dhcp_rfc1035.o

OBJS += dhcp_packet.o

OBJS += dhcp_socket.o
OBJS += dhcp_util.o
OBJS += dhcp_main.o


OBJS += dhcpc.o
OBJS += dhcpd.o
OBJS += dhcprelay.o

ifeq ($(strip $(ZPL_BUILD_IPV6)),true)
#OBJS += dhcpv6c.o
#OBJS += dhcpv6c_state.o
#OBJS += dhcpv6c_option.o
#OBJS += dhcpv6c_api.o
#OBJS += rtadv.o
#OBJS += script.o
#OBJS += odhcp6c.o
endif


OBJS += dhcp_api.o
#############################################################################
# LIB
###########################################################################
LIBS = libudhcp.a
