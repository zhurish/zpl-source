#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/lib
#lib
OBJS	+= list.o
OBJS	+= rbtree.o
OBJS	+= buffer.o
OBJS	+= checksum.o
OBJS	+= command.o
OBJS	+= hash.o
OBJS	+= jhash.o
OBJS	+= linklist.o
OBJS	+= log.o
OBJS	+= md5.o
OBJS	+= pqueue.o
OBJS	+= template.o
OBJS	+= str.o
OBJS	+= stream.o
OBJS	+= table.o
OBJS	+= thread.o
OBJS	+= vector.o
OBJS	+= workqueue.o
OBJS	+= eloop.o
OBJS	+= host.o
OBJS	+= module.o
OBJS	+= bitmap.o

OBJS	+= memory.o
OBJS	+= memtypes.o
OBJS	+= daemon.o
OBJS	+= pid_output.o
OBJS	+= sigevent.o

#############################################################################
# LIB
###########################################################################
LIBS = libbase.a
