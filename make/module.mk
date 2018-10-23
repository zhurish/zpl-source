# the following is a list of the Platform configurations that have booleans associated with them

_MODULELIST = \
	MODULE_PLATFORM.true \
	MODULE_L2PROTOCOL.false \
	MODULE_SERVICE.true \
	MODULE_STARTUP.true \
	MODULE_PRODUCT.true \
	MODULE_CLI.true \
	MODULE_OSPF.false \
	MODULE_ABSTRACT.true \
	MODULE_COMPONENT.true \
	MODULE_WIFI.true \
	MODULE_MODEM.true \
	MODULE_TOOLS.true \
	MODULE_DHCP.true \
	MODULE_SQLITE.false
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
	DHCPCDIR.dhcpc \
	DHCPDDIR.dhcpd \
	SQLITEDIR.sqlite
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
        
