#############################################################################
# DEFINE
###########################################################################
MODULEDIR = startup/src
#OS
OBJS	+= module_tbl.o
OBJS	+= os_module.o
OBJS	+= os_start.o
OBJS	+= os_test.o
#############################################################################
# LIB
###########################################################################
LIBS = libstartup.a
