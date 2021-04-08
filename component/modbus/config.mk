#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/modbus
#
PLINCLUDE += -I$(MODBUS_ROOT)/include
#
#
#OS

OBJS += modbus.o
OBJS += modbus-data.o
OBJS += modbus-rtu.o
OBJS += modbus-tcp.o
OBJS += modbus-api.o

BOBJS += modbus-test.o

#############################################################################
# LIB
###########################################################################
LIBS = libmodbus.a
