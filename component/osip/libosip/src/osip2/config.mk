#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/osip/libosip/src/osip2
#
PLINCLUDE += -I$(OSIP_ROOT)/libosip/include
#
#
#OS
OBJS += fsm_misc.o
OBJS += ict.o
OBJS += ict_fsm.o
OBJS += ist.o
OBJS += ist_fsm.o
OBJS += nict.o
OBJS += nict_fsm.o
OBJS += nist.o
OBJS += nist_fsm.o
OBJS += osip.o
OBJS += osip_dialog.o
OBJS += osip_event.o
OBJS += osip_time.o
OBJS += osip_transaction.o
OBJS += port_condv.o
OBJS += port_fifo.o
OBJS += port_sema.o
OBJS += port_thread.o

#############################################################################
# LIB
###########################################################################
LIBS = libosip2.a
