#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/lib
#lib
OBJS	+= checksum.o
OBJS	+= hash.o
OBJS	+= jhash.o
OBJS	+= log.o
OBJS	+= md5.o
OBJS	+= str.o
OBJS	+= thread.o
OBJS	+= eloop.o

OBJS	+= module.o

OBJS	+= network.o
OBJS	+= sockopt.o
OBJS	+= sockunion.o
OBJS	+= prefix.o

OBJS	+= memory.o
OBJS	+= memtypes.o
OBJS	+= daemon.o
OBJS	+= pid_output.o
OBJS	+= algorithm.o

OBJS	+= host.o
OBJS	+= libgl.o

OBJS	+= linklist.o
OBJS	+= pqueue.o
OBJS	+= stream.o

OBJS	+= lib_event.o
OBJS	+= lib_pqueue.o


ifeq ($(strip $(ZPL_IP_FILTER)),true)
OBJS	+= filter.o		
endif
ifeq ($(strip $(ZPL_IP_PLIST)),true)
OBJS	+= plist.o		
endif		

ifeq ($(strip $(ZPL_WORKQUEUE)),true)
OBJS	+= workqueue.o	
endif
ifeq ($(strip $(ZPL_KEYCHAIN)),true)
OBJS	+= keychain.o		
endif
ifeq ($(strip $(ZPL_DISTRIBUTE)),true)
OBJS	+= distribute.o		
endif


ifeq ($(strip $(ZPL_NSM_MODULE)),true)
OBJS	+= if.o
OBJS	+= if_name.o
ifeq ($(strip $(ZPL_VRF_MODULE)),true)
OBJS	+= vrf.o	
endif
OBJS	+= nexthop.o
OBJS	+= table.o
OBJS	+= connected.o
OBJS	+= if_rmap.o
OBJS	+= routemap.o
OBJS	+= router-id.o



endif

ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS	+= buffer.o
OBJS	+= vector.o
OBJS	+= command.o
OBJS	+= template.o
OBJS	+= cli_node.o

OBJS	+= cmd_log.o
OBJS	+= cmd_host.o
OBJS	+= cmd_memory.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libbase.a
