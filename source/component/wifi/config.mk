#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/wifi
#
ZPLINCLUDE += -I$(WIFI_ROOT)/include
#
#
#OS

OBJS += iw_interface.o
OBJS += iw_config.o
OBJS += iw_client.o
OBJS += iw_ap.o
OBJS += iw_ap_script.o

OBJS += iwlist.o
OBJS += iwconfig.o
OBJS += iwioctl.o
OBJS += iwlib.o

#LIBNL_OBJ = nl.o handlers.o msg.o attr.o cache.o cache_mngt.o object.o socket.o error.o
#GENL_OBJ = genl_lib.o genl_family.o genl_ctrl.o genl_mngt.o unl.o

#OBJS += $(LIBNL_OBJ)
#OBJS += $(GENL_OBJ)

#OBJS += iw.o genl.o event.o info.o phy.o \
	interface.o ibss.o station.o survey.o util.o ocb.o \
	mesh.o mpath.o mpp.o scan.o reg.o version.o \
	reason.o status.o connect.o link.o offch.o ps.o cqm.o \
	bitrate.o wowlan.o coalesce.o roc.o p2p.o vendor.o mgmt.o \
	ap.o version.o sections.o
#############################################################################
# LIB
###########################################################################
LIBS = libwifi.a
