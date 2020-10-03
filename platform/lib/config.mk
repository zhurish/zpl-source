#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/lib
#lib
#OBJS	+= avl.o
OBJS	+= list.o
OBJS	+= rbtree.o
OBJS	+= buffer.o
OBJS	+= checksum.o
OBJS	+= command.o
OBJS	+= daemon.o
OBJS	+= hash.o

OBJS	+= jhash.o
OBJS	+= linklist.o
OBJS	+= log.o
OBJS	+= md5.o
OBJS	+= memory.o
OBJS	+= memtypes.o

OBJS	+= pid_output.o
OBJS	+= pqueue.o
OBJS	+= template.o
OBJS	+= sigevent.o
OBJS	+= str.o
OBJS	+= stream.o
OBJS	+= table.o
OBJS	+= thread.o
OBJS	+= vector.o
OBJS	+= workqueue.o

OBJS	+= eloop.o

OBJS	+= host.o

OBJS	+= module.o


#############################################################################
# LIB
###########################################################################
LIBS = libbase.a
