
PLPROD ?= ospf

#############################################################################
# DEFINE
###########################################################################
#PLDEFINE	+= -DPL_CLI_DEBUG

#############################################################################
# INCLUDE
###########################################################################

PLINCLUDE += -I$(OSPF_ROOT)
#PLINCLUDE += -I$(PRODUCT_ROOT)



#############################################################################
# OBJECTS
###########################################################################
OSPFOBJ	+= ospf_memory.o
OSPFOBJ	+= ospf_ack.o
OSPFOBJ	+= ospf_area.o
OSPFOBJ	+= ospf_dbd.o
OSPFOBJ	+= ospf_trap.o
OSPFOBJ	+= ospf_update.o
OSPFOBJ	+= ospf_hello.o
OSPFOBJ	+= ospf_lsa.o
OSPFOBJ	+= ospf_nbr.o
OSPFOBJ	+= ospf_packet.o
OSPFOBJ	+= ospf_request.o
OSPFOBJ	+= ospf_route.o
OSPFOBJ	+= ospf_spf.o
OSPFOBJ	+= ospf_syn.o
OSPFOBJ	+= ospf_export.o
OSPFOBJ	+= ospf_frr.o
OSPFOBJ	+= ospf_policy.o
OSPFOBJ	+= ospf_redistribute.o
OSPFOBJ	+= ospf_relation.o
OSPFOBJ	+= ospf_restart.o

OSPFOBJ	+= ospf_interface.o
OSPFOBJ	+= ospf_te_interface.o

OSPFOBJ	+= ospf_dcn.o
OSPFOBJ	+= ospf_dcn_api.o

OSPFOBJ	+= ospf_util.o
OSPFOBJ	+= ospf_api.o
OSPFOBJ	+= ospf_os.o
OSPFOBJ	+= ospf_nm.o

OSPFOBJ	+= ospf_pal.o
OSPFOBJ	+= ospf_main.o
OSPFOBJ	+= ospf_zebra.o
#############################################################################
# LIB
###########################################################################

OSPFLIBS = libospf.a
