#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/os
#OS

OSOBJ	+= os_job.o
OSOBJ	+= os_list.o
OSOBJ	+= os_sem.o
OSOBJ	+= os_task.o
OSOBJ	+= os_time.o
OSOBJ	+= os_ansync.o
OSOBJ	+= os_util.o
OSOBJ	+= os_socket.o

ifeq ($(strip $(PL_OS_QUEUE)),true)
OSOBJ	+= os_queue.o			
endif
ifeq ($(strip $(PL_OS_AVL)),true)
OSOBJ	+= avl.o			
endif
ifeq ($(strip $(PL_OS_NVRAM)),true)
OSOBJ	+= os_nvram.o		
endif

ifeq ($(strip $(PL_OS_JSON)),true)
OSOBJ	+= cJSON.o			
endif
ifeq ($(strip $(PL_OS_TLV)),true)
OSOBJ	+= os_tlv.o				
endif
ifeq ($(strip $(PL_OS_RNG)),true)
OSOBJ	+= os_rng.o			
endif

ifeq ($(strip $(PL_OS_XYZ_MODEM)),true)
OSOBJ	+= xyz_modem.o				
endif
ifeq ($(strip $(PL_OS_TTYCOM)),true)
OSOBJ	+= tty_com.o				
endif

ifeq ($(strip $(PL_OS_UCI)),true)
OSOBJ	+= os_uci.o				
endif
#############################################################################
# LIB
###########################################################################
LIBS = libos.a
