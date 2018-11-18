#############################################################################
#DEFINE=-DHAVE_CONFIG_H -DNDEBUG -DTHERE_IS_NO_FORK -D_GNU_SOURCE \
	-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE \
	-DINET -DARP -DARPING -DIPV4LL -DINET6 -DDHCP6 -DAUTH -DNO_SIGNALS \

DEFINE=-D__USE_GNU -D_GNU_SOURCE -DNDEBUG -DTHERE_IS_NO_FORK -DINET \
		-DARP -DARPING -DIPV4LL -DAUTH -DNO_SIGNALS
###########################################################################
MODULEDIR = component/dhcpcd
#OS
OBJS += common.o 
OBJS += control.o 
OBJS += dhcpcd.o 
OBJS += duid.o 
OBJS += dhcp-eloop.o 
OBJS += logerr.o
OBJS += dhcpc-if.o 
OBJS += if-options.o 
OBJS += sa.o 
OBJS += route.o
OBJS += dhcp-common.o 
OBJS += script.o
OBJS += auth.o
OBJS += if-linux.o
OBJS += dhcp.o 
OBJS += ipv4.o 
OBJS += bpf.o
OBJS += arp.o
OBJS += ipv4ll.o
ifeq ($(strip $(BUILD_IPV6)),true)
OBJS += ipv6.o 
OBJS += ipv6nd.o
OBJS += dhcp6.o
DEFINE +=-DINET6 -DDHCP6
endif

OBJS += dhcpcd-embedded.o

OBJS += strtoi.o
OBJS += strtou.o
OBJS += arc4random.o
OBJS += arc4random_uniform.o

OBJS += hmac.o
#OBJS += md5.o
OBJS += sha256.o

OBJS += dhcp-util.o 
OBJS += dhcpcd_api.o 
#############################################################################
# LIB
###########################################################################
LIBS = libdhcpcd.a
