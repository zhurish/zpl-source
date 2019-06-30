#############################################################################
# DEFINE
###########################################################################
MODULEDIR = product/sdk
#OS
OBJS	+= bcm53125_sdk.o
OBJS	+= b53_mdio.o
OBJS	+= b53_global.o
OBJS	+= b53_dos.o
OBJS	+= b53_mac_tbl.o
OBJS	+= b53_mirror.o
OBJS	+= b53_port.o
OBJS	+= b53_stp.o
OBJS	+= b53_trunk.o
OBJS	+= b53_vlan.o
OBJS	+= b53_driver.o
#############################################################################
# LIB
###########################################################################
LIBS = libsdk.a
