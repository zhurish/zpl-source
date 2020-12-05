#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/libuci
#shell
OBJS	+= blob.o
OBJS	+= delta.o
OBJS	+= file.o
#OBJS	+= list.o
OBJS	+= parse.o
OBJS	+= ucimap.o
OBJS	+= util.o
OBJS	+= libuci.o
ifeq ($(strip $(PL_OS_UCI)),true)
OBJS	+= os_uci.o				
endif
#############################################################################
# LIB
###########################################################################
LIBS = libuci.a
