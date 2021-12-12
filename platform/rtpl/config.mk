#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/rtpl
#nsm
ifeq ($(strip $(ZPL_NSM_MODULE)),true)
OBJS	+= nsm_fpm.o
#OBJS	+= nsm_client.o
#OBJS	+= nsm_hook.o
OBJS	+= nsm_rib.o
OBJS	+= router-id.o
OBJS	+= nsm_redistribute.o
OBJS	+= nsm_zebra_routemap.o
OBJS	+= nsm_rnh.o
OBJS	+= nsm_zserv.o
#OBJS	+= nsm_vty.o
#############################################################################
# LIB
###########################################################################
LIBS = librtpl.a
endif