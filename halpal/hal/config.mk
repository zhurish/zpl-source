#############################################################################
# DEFINE
###########################################################################
MODULEDIR = halpal/hal
ifeq ($(strip $(ZPL_HAL_MODULE)),true)
OBJS	+= hal_ipcsrv.o
OBJS	+= hal_ipccmd.o
OBJS	+= hal_ipcmsg.o
OBJS	+= hal_global.o
OBJS	+= hal_netpkt.o
OBJS	+= hal_misc.o

OBJS	+= hal_port.o		

ifeq ($(strip $(ZPL_NSM_MSTP)),true)
OBJS	+= hal_mstp.o		
endif
ifeq ($(strip $(ZPL_NSM_IGMP)),true)
OBJS	+= hal_igmp.o	
endif
ifeq ($(strip $(ZPL_NSM_8021X)),true)
OBJS	+= hal_8021x.o		
endif
ifeq ($(strip $(ZPL_NSM_DOS)),true)
OBJS	+= hal_dos.o			
endif

ifeq ($(strip $(ZPL_NSM_MAC)),true)
OBJS	+= hal_mac.o
#OBJS	+= hal_l2mc.o		
endif
ifeq ($(strip $(ZPL_NSM_VLAN)),true)
OBJS	+= hal_vlan.o	
OBJS	+= hal_qinq.o
endif
ifeq ($(strip $(ZPL_NSM_QOS)),true)
OBJS	+= hal_qos.o		
endif
ifeq ($(strip $(ZPL_NSM_TRUNK)),true)
OBJS	+= hal_trunk.o		
endif
ifeq ($(strip $(ZPL_NSM_MIRROR)),true)
OBJS	+= hal_mirror.o	
endif

OBJS	+= hal_driver.o
OBJS	+= hal_test.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libhal.a
