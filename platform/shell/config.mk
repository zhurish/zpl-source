#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/shell
#shell
#OBJS	+= zplgetopt.o
#OBJS	+= zplgetopt1.o
OBJS	+= vty_user.o
OBJS	+= vty.o
OBJS	+= vty_stdio.o
ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS	+= cmd_vty.o
OBJS	+= cmd_user_vty.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libshell.a
