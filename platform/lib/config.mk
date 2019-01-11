#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/lib
#lib
OBJS	+= avl.o
OBJS	+= list.o
OBJS	+= rbtree.o
OBJS	+= buffer.o
OBJS	+= checksum.o
OBJS	+= command.o
OBJS	+= daemon.o
OBJS	+= hash.o
#OBJS	+= if_hook.o
OBJS	+= if_name.o
OBJS	+= if.o
#OBJS	+= ip_vrf.o
OBJS	+= jhash.o
OBJS	+= linklist.o
OBJS	+= log.o
OBJS	+= md5.o
OBJS	+= memory.o
OBJS	+= memtypes.o
OBJS	+= network.o
OBJS	+= nexthop.o
OBJS	+= pid_output.o
OBJS	+= pqueue.o
OBJS	+= prefix.o
#OBJS	+= privs.o
OBJS	+= template.o
OBJS	+= sigevent.o
OBJS	+= sockopt.o
OBJS	+= sockunion.o
OBJS	+= str.o
OBJS	+= stream.o
OBJS	+= table.o
OBJS	+= thread.o
OBJS	+= vector.o
OBJS	+= workqueue.o

OBJS	+= eloop.o
OBJS	+= zclient.o
#OBJS	+= distribute.o
#OBJS	+= filter.o
#OBJS	+= if_rmap.o
#OBJS	+= keychain.o
#OBJS	+= plist.o
#OBJS	+= regex.o
#OBJS	+= routemap.o
OBJS	+= host.o
#OBJS	+= router-id.o
OBJS	+= module.o
OBJS	+= bitmap.o
OBJS	+= tty_com.o
#############################################################################
# LIB
###########################################################################
LIBS = libbase.a
