#############################################################################
# DEFINE
###########################################################################
MODULEDIR = abstract/pal/kernel
ifeq ($(strip $(MODULE_PAL_KERNEL)),true)
OBJS	+= kernel_ioctl.o
OBJS	+= kernel_arp.o
OBJS	+= kernel_tunnel.o
OBJS	+= kernel_brigde.o
OBJS	+= kernel_bond.o
OBJS	+= kernel_veth.o
OBJS	+= kernel_vrf.o
OBJS	+= kernel_ipforward.o
OBJS	+= kernel_netlink.o
OBJS	+= kernel_nladdress.o
OBJS	+= kernel_nlroute.o
OBJS	+= kernel_nllisten.o
OBJS	+= kernel_nliface.o
OBJS	+= kernel_nlload.o
OBJS	+= kernel_firewalld.o
OBJS	+= kernel_driver.o
OBJS	+= pal_driver.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libpal.a
