#############################################################################
# DEFINE
###########################################################################
MODULEDIR = product/sdk
#OS

OBJS	+= b53_mdio.o
OBJS	+= b53_global.o
OBJS	+= b53_dos.o
OBJS	+= b53_mac_tbl.o
OBJS	+= b53_mirror.o
OBJS	+= b53_port.o
OBJS	+= b53_stp.o
OBJS	+= b53_trunk.o
OBJS	+= b53_vlan.o
OBJS	+= b53_qos.o
OBJS	+= b53_rate.o
OBJS	+= b53_eap.o
OBJS	+= b53_phy.o
OBJS	+= b53_cpu.o
OBJS	+= b53_test.o
OBJS	+= b53_driver.o

OBJS	+= sdk_driver.o
OBJS	+= sdk_netpkt.o

#############################################################################
# LIB
###########################################################################
LIBS = libsdk.a
