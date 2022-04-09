#############################################################################
# DEFINE
###########################################################################
MODULEDIR = product/bsp/hal
#OS
OBJS	+= hal_client.o
OBJS	+= bsp_global.o

ifeq ($(strip $(ZPL_HAL_MODULE)),true)
OBJS	+= bsp_port.o
OBJS	+= bsp_misc.o

OBJS	+= bsp_netpkt.o

ifeq ($(strip $(ZPL_NSM_MSTP)),true)
OBJS	+= bsp_mstp.o	
endif


ifeq ($(strip $(ZPL_NSM_8021X)),true)
OBJS	+= bsp_8021x.o		
endif
ifeq ($(strip $(ZPL_NSM_DOS)),true)
OBJS	+= bsp_dos.o			
endif

ifeq ($(strip $(ZPL_NSM_MAC)),true)
OBJS	+= bsp_mac.o		
endif
ifeq ($(strip $(ZPL_NSM_VLAN)),true)
OBJS	+= bsp_vlan.o	
OBJS	+= bsp_qinq.o
endif
ifeq ($(strip $(ZPL_NSM_QOS)),true)
OBJS	+= bsp_qos.o		
endif
ifeq ($(strip $(ZPL_NSM_TRUNK)),true)
OBJS	+= bsp_trunk.o		
endif
ifeq ($(strip $(ZPL_NSM_MIRROR)),true)
OBJS	+= bsp_mirror.o	
endif

OBJS	+= bsp_cpu.o
OBJS	+= bsp_driver.o

endif
#############################################################################
# LIB
###########################################################################
LIBS = libproduct.a
