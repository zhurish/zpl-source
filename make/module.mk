# the following is a list of the Platform configurations that have booleans associated with them

_MODULELIST = \
	MODULE_PLATFORM.true \
	MODULE_L2PROTOCOL.false \
	MODULE_SERVICE.true \
	MODULE_STARTUP.true \
	MODULE_PRODUCT.true \
	MODULE_BCM53125.false \
	MODULE_CLI.true \
	MODULE_OSPF.false \
	MODULE_ABSTRACT.true \
	MODULE_COMPONENT.true \
	MODULE_WIFI.true \
	MODULE_MODEM.true \
	MODULE_TOOLS.true \
	MODULE_DHCP.false \
	MODULE_UDHCP.true \
	MODULE_SQLITE.false \
	MODULE_SYSTOOLS.true \
	MODULE_SSH.true \
	MODULE_PJSIP.true \
	MODULE_OSIP.false \
	MODULE_APP.true \
	MODULE_UCI.true \
	MODULE_WEB.false 	
#
#
#
#
#
define _MODULE_DEF
#
#
export $(subst .,:=, $1)
#endif
#endif
BUILD_MODULE := $(BUILD_MODULE) $(subst .,=, $1)
endef
#
#
$(foreach IModule,$(_MODULELIST), $(eval $(call _MODULE_DEF,$(IModule))))
#
#
#
#
ifeq ($(strip $(MODULE_PJSIP)),true)
MODULE_OSIP=false
endif
#
ifeq ($(strip $(MODULE_OSIP)),true)
MODULE_PJSIP=false
endif
#
#
ifeq ($(strip $(MODULE_UDHCP)),true)
MODULE_DHCP=false
endif
#
ifeq ($(strip $(MODULE_DHCP)),true)
MODULE_UDHCP=false
endif
#
#
#
#
#
#
PlatformModule = \
	PLATFORMDIR.platform \
	L2PROTOCOLDIR.l2protocol \
	SERVICEDIR.service \
	STARTUPDIR.startup \
	PRODUCTDIR.product \
	CLIDIR.cli \
	OSPFDIR.ospf \
	ABSTRACTDIR.abstract \
	COMPONENTDIR.component \
	WIFIDIR.wifi \
	MODEMDIR.modem \
	TOOLSDIR.tools \
	DHCPCDDIR.dhcpcd \
	DHCPDDIR.dhcpd \
	UDHCPDIR.udhcp \
	SQLITEDIR.sqlite \
	SYSTOOLSDIR.systools \
	LIBSSHDIR.ssh \
	OSIPDIR.osip \
	PJSIPDIR.pjsip \
	APPDIR.application \
	WEBDIR.webgui
###
# By default we choose the lexically last Platform component version in hopes
# that it is the most recent. The directory macros (e.g. IPCOMDIR) may of
# course be overriden on the command line.
#
define _DIR_DEF
#
#
$(subst .,=, $1)
#endif
#endif
PLATFORM_COMPS := $(PLATFORM_COMPS) $(subst .,=, $1)
#
#
endef
#
#
$(foreach PlatformModule,$(PlatformModule), $(eval $(call _DIR_DEF,$(PlatformModule))))
#
#
        
