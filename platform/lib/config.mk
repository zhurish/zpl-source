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
OBJS	+= thread.o
OBJS	+= vector.o
OBJS	+= workqueue.o
OBJS	+= eloop.o
OBJS	+= host.o
OBJS	+= moduletypes.o
OBJS	+= module.o
OBJS	+= bitmap.o

OBJS	+= network.o
OBJS	+= sockopt.o
OBJS	+= sockunion.o
OBJS	+= prefix.o

OBJS	+= memory.o
OBJS	+= memtypes.o
OBJS	+= daemon.o
OBJS	+= pid_output.o
OBJS	+= sigevent.o

OBJS	+= nsm_filter.o
OBJS	+= nsm_keychain.o
OBJS	+= nsm_plist.o
OBJS	+= nsm_connected.o
OBJS	+= nsm_vrf.o
OBJS	+= nsm_pqueue.o

OBJS	+= if_name.o
OBJS	+= if.o
#############################################################################
# LIB
###########################################################################
LIBS = libbase.a
