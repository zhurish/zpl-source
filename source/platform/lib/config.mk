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

OBJS	+= zmemory.o
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
OBJS	+= lib_netlink.o

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
OBJS	+= ipvrf.o	
endif
OBJS	+= if_utsp.o
OBJS	+= nexthop.o
OBJS	+= table.o
OBJS	+= if_rmap.o
OBJS	+= routemap.o
OBJS	+= zclient.o


endif

ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS	+= buffer.o
OBJS	+= vector.o
OBJS	+= command.o
OBJS	+= template.o
OBJS	+= cli_node.o
ifeq ($(strip $(ZPL_VRF_MODULE)),true)
OBJS	+= cmd_ipvrf.o
endif
OBJS	+= cmd_log.o
OBJS	+= cmd_host.o
OBJS	+= cmd_memory.o
endif

ifeq ($(strip $(ZPL_NSM_SNMP)),true)
OBJS	+= snmp.o
OBJS	+= agentx.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libbase.a
