#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/os
#OS
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
#############################################################################
# LIB
###########################################################################
LIBS = libos.a
