#############################################################################
# DEFINE
###########################################################################
MODULEDIR = abstract/hal
ifeq ($(strip $(PL_HAL_MODULE)),true)

OBJS	+= hal_port.o
OBJS	+= hal_misc.o
OBJS	+= hal_mstp.o

ifeq ($(strip $(PL_NSM_8021X)),true)
OBJS	+= hal_8021x.o		
endif
ifeq ($(strip $(PL_NSM_DOS)),true)
OBJS	+= hal_dos.o			
endif

ifeq ($(strip $(PL_NSM_MAC)),true)
OBJS	+= hal_mac.o		
endif
ifeq ($(strip $(PL_NSM_VLAN)),true)
OBJS	+= hal_vlan.o	
OBJS	+= hal_qinq.o
endif
ifeq ($(strip $(PL_NSM_QOS)),true)
OBJS	+= hal_qos.o		
endif
ifeq ($(strip $(PL_NSM_TRUNK)),true)
OBJS	+= hal_trunk.o		
endif
ifeq ($(strip $(PL_NSM_MIRROR)),true)
OBJS	+= hal_mirror.o	
endif

OBJS	+= hal_driver.o
OBJS	+= hal_test.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libhal.a
