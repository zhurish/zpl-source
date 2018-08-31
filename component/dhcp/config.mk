#############################################################################
DEFINE=-D_BSD_SOURCE -D_XOPEN_SOURCE=600
###########################################################################
MODULEDIR = component/dhcp
#OS
OBJS += strlcpy.o
OBJS += platform-linux.o
OBJS += if-linux-wireless.o
OBJS += if-linux.o
OBJS += closefrom.o
OBJS += arc4random.o
OBJS += lpf.o

OBJS += signals.o
#OBJS += showlease.o
#OBJS += platform-bsd.o

OBJS += net.o

#OBJS += linkaddr.o
OBJS += ipv6rs.o
OBJS += ipv4ll.o
OBJS += if-pref.o
OBJS += if-options.o

#OBJS += if-bsd.o

#OBJS += ifaddrs.o
#OBJS += getline.o

OBJS += dhcpc_eloop.o
OBJS += duid.o
OBJS += dhcpcd.o
OBJS += dhcp.o
OBJS += control.o

OBJS += configure.o
OBJS += dhcpc_common.o

#OBJS += bpf.o
OBJS += bind.o
OBJS += arp.o

OBJS += dhcpc_api.o
#############################################################################
# LIB
###########################################################################
LIBS = libdhcpc.a
