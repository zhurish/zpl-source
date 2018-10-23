#############################################################################
DEFINE=-D_BSD_SOURCE -D_XOPEN_SOURCE=600
###########################################################################
MODULEDIR = component/dhcpc
#OS
#OBJS += strlcpy.o
OBJS += if-linux.o
OBJS += arc4random.o
OBJS += lpf.o

OBJS += signals.o
#OBJS += showlease.o


OBJS += net.o

#OBJS += linkaddr.o
OBJS += ipv6rs.o
OBJS += ipv4ll.o
OBJS += if-options.o

#OBJS += if-bsd.o

#OBJS += ifaddrs.o
#OBJS += getline.o

OBJS += dhcpc_eloop.o
OBJS += dhcpcd.o
OBJS += dhcp.o
OBJS += control.o

OBJS += configure.o
OBJS += dhcpc_common.o

OBJS += dhcpc_util.o
OBJS += bind.o
OBJS += arp.o

OBJS += dhcpc_main.o
OBJS += dhcpc_api.o
#############################################################################
# LIB
###########################################################################
LIBS = libdhcpc.a
