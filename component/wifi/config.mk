#############################################################################
# DEFINE
###########################################################################
MODULEDIR = component/wifi
#OS
OBJS += nsm_wifi.o

MODULE_WIFI_SRC=true

ifeq ($(strip $(MODULE_WIFI_SRC)),true)
OBJS += iw.o genl.o event.o info.o phy.o \
	interface.o ibss.o station.o survey.o util.o ocb.o \
	mesh.o mpath.o mpp.o scan.o reg.o version.o \
	reason.o status.o connect.o link.o offch.o ps.o cqm.o \
	bitrate.o wowlan.o coalesce.o roc.o p2p.o vendor.o mgmt.o \
	ap.o version.o
OBJS += sections.o

OBJS-$(HWSIM) += hwsim.o

OBJS += $(OBJS-y) $(OBJS-Y)
endif
#############################################################################
# LIB
###########################################################################
LIBS = libwifi.a
