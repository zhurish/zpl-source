#############################################################################
# DEFINE
###########################################################################
MODULEDIR = abstract/hal

OBJS	+= hal_vlan.o
OBJS	+= hal_port.o
OBJS	+= hal_mac.o
OBJS	+= hal_qinq.o

OBJS	+= hal_8021x.o
OBJS	+= hal_dos.o
OBJS	+= hal_mirror.o
OBJS	+= hal_misc.o
OBJS	+= hal_mstp.o
OBJS	+= hal_trunk.o
#############################################################################
# LIB
###########################################################################
LIBS = libhal.a
