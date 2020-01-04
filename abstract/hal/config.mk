#############################################################################
# DEFINE
###########################################################################
MODULEDIR = abstract/hal
ifeq ($(strip $(MODULE_HAL)),true)
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
OBJS	+= hal_qos.o

OBJS	+= hal_driver.o
OBJS	+= hal_test.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libhal.a
