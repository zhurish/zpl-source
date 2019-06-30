#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/os
#OS
OSOBJ	+= avl.o
OSOBJ	+= os_job.o
OSOBJ	+= os_list.o
OSOBJ	+= os_rng.o
OSOBJ	+= os_sem.o
OSOBJ	+= os_task.o
OSOBJ	+= os_time.o
OSOBJ	+= os_ansync.o
OSOBJ	+= os_util.o
OSOBJ	+= os_tlv.o
OSOBJ	+= os_socket.o
OSOBJ	+= cJSON.o
OSOBJ	+= os_queue.o
OSOBJ	+= os_nvram.o
ifeq ($(strip $(MODULE_UCI)),true)
OSOBJ	+= os_uci.o				
endif
#############################################################################
# LIB
###########################################################################
LIBS = libos.a
