#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/os
#OS
OSOBJ	+= os_ipstack.o
OSOBJ	+= os_job.o
OSOBJ	+= os_list.o
OSOBJ	+= os_sem.o
OSOBJ	+= os_task.o
OSOBJ	+= os_time.o
OSOBJ	+= os_ansync.o
OSOBJ	+= os_log.o
OSOBJ	+= os_util.o
OSOBJ	+= os_socket.o
OSOBJ	+= os_process.o
OSOBJ	+= os_file.o
OSOBJ	+= os_url.o
OSOBJ	+= os_signal.o
OSOBJ	+= os_bitmap.o
OSOBJ	+= os_backtrace.o
OSOBJ	+= zpl_skbuffer.o
OSOBJ	+= zpl_errno.o
OSOBJ	+= os_sem_errchk.o
ifeq ($(strip $(ZPL_OS_QUEUE)),true)
OSOBJ	+= os_queue.o			
endif
ifeq ($(strip $(ZPL_OS_AVL)),true)
OSOBJ	+= avl.o			
endif
ifeq ($(strip $(ZPL_OS_NVRAM)),true)
OSOBJ	+= os_nvram.o		
endif

ifeq ($(strip $(ZPL_OS_JSON)),true)
OSOBJ	+= cJSON.o			
endif
ifeq ($(strip $(ZPL_OS_TLV)),true)
OSOBJ	+= os_tlv.o				
endif
ifeq ($(strip $(ZPL_OS_RNG)),true)
OSOBJ	+= os_rng.o			
endif

ifeq ($(strip $(ZPL_OS_XYZ_MODEM)),true)
OSOBJ	+= xyz_modem.o				
endif
ifeq ($(strip $(ZPL_OS_TTYCOM)),true)
OSOBJ	+= tty_com.o				
endif

ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OSOBJ	+= cmd_os.o
OSOBJ	+= cmd_nvram_env.o
endif

OSOBJ	+= os_message.o

OSOBJ	+= libnetpro.o
OSOBJ	+= osker_list.o
OSOBJ	+= rbtree.o
#############################################################################
# LIB
###########################################################################
LIBS = libos.a
