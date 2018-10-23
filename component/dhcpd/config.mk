#############################################################################
#DEFINE=-D_BSD_SOURCE -D_XOPEN_SOURCE=600
_DHCPD_CONF=false
ifeq ($(_DHCPD_CONF),true)
DEFINE +=-DDHCPD_CONF_DEBUG
endif

#PL_CFLAGS += -I-component/dhcpc
PL_CFLAGS += -I-$(PLBASE)/$(COMPONENTDIR)/$(DHCPCDIR)
###########################################################################
MODULEDIR = component/dhcpd
#OS
OBJS += bootp.o
OBJS += db.o
OBJS += dhcp.o
OBJS += dhcpd.o
#OBJS += bpf.o
OBJS += packet.o
OBJS += errwarn.o
OBJS += dispatch.o
OBJS += print.o
OBJS += memory.o
OBJS += options.o
OBJS += inet.o
OBJS += alloc.o
OBJS += tables.o
OBJS += tree.o
OBJS += hash.o
OBJS += convert.o
OBJS += icmp.o
#OBJS += sync.o

OBJS += dhcp_decl.o
#OBJS += duid.o
#OBJS += dhcpcd.o
#OBJS += dhcp.o
#OBJS += control.o

OBJS += udpsock.o
#OBJS += pfutils.o
OBJS += dhcp_default.o

OBJS += dhcpd_main.o
OBJS += dhcpd_api.o

ifeq ($(_DHCPD_CONF),true)
OBJS += conflex.o
OBJS += confpars.o
OBJS += parse.o
endif

#############################################################################
# LIB
###########################################################################
LIBS = libdhcps.a
