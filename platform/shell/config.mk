#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/shell
#shell
OBJS	+= getopt.o
OBJS	+= getopt1.o
OBJS	+= vty_user.o
OBJS	+= vty.o
#############################################################################
# LIB
###########################################################################
LIBS = libshell.a
