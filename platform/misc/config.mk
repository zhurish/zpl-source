#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/misc
#lib

OBJS	+= if_name.o
OBJS	+= if.o
#OBJS	+= ip_vrf.o

OBJS	+= network.o
OBJS	+= nexthop.o
OBJS	+= prefix.o

OBJS	+= sockopt.o
OBJS	+= sockunion.o
ifeq ($(strip $(PL_NSM_MODULE)),true)
OBJS	+= zclient.o
endif
ifeq ($(strip $(PL_MISC_IFHOOK)),true)
OBJS	+= if_hook.o			
endif
ifeq ($(strip $(PL_MISC_SHOW_HOOK)),true)
OBJS	+= show_hook.o			
endif
#############################################################################
# LIB
###########################################################################
LIBS = libmisc.a
