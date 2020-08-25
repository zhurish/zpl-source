# the following is a list of the Platform configurations that have booleans associated with them

_MODULELIST = \
	MODULE_PLATFORM.true \
	MODULE_L2PROTOCOL.false \
	MODULE_SERVICE.true \
	MODULE_SNTPC.true \
	MODULE_SNTPS.true \
	MODULE_SYSLOG.true \
	MODULE_STARTUP.true \
	MODULE_PRODUCT.true \
	MODULE_SWITCH_SDK.false \
	MODULE_CLI.true \
	MODULE_OSPF.false \
	MODULE_ABSTRACT.true \
	MODULE_HAL.false \
	MODULE_PAL_KERNEL.true \
	MODULE_PAL_IPCOM.false \
	MODULE_COMPONENT.true \
	MODULE_OPENSSL.true \
	MODULE_WIFI.false \
	MODULE_MODEM.false \
	MODULE_TOOLS.true \
	MODULE_PROCESS.false \
	MODULE_QUECTEL_CM.false \
	MODULE_UDHCP.false \
	MODULE_SQLITE.true \
	MODULE_SYSTOOLS.true \
	MODULE_FTPD.true \
	MODULE_FTPC.true \
	MODULE_TFTPD.true \
	MODULE_TFTPC.true \
	MODULE_TELNET.true \
	MODULE_TELNETD.false \
	MODULE_PING.true \
	MODULE_TRACEROUTE.true \
	MODULE_UBUS.false \
	MODULE_SSH.false \
	MODULE_PJSIP.true \
	MODULE_APP.true \
	MODULE_UCI.true \
	MODULE_WEB.false \
	MODULE_MQTT.true
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
ifeq ($(strip $(MODULE_PAL_KERNEL)),true)
MODULE_PAL_IPCOM=false
MODULE_ABSTRACT=true
endif
#
ifeq ($(strip $(MODULE_PAL_IPCOM)),true)
MODULE_PAL_KERNEL=false
MODULE_ABSTRACT=true
endif
#
#
#
ifeq ($(strip $(MODULE_WIFI)),true)
MODULE_PROCESS=true
endif
#
ifeq ($(strip $(MODULE_MODEM)),true)
MODULE_QUECTEL_CM=true
MODULE_PROCESS=true
endif
#
ifeq ($(strip $(MODULE_SWITCH_SDK)),true)
MODULE_HAL=true
endif
#
ifeq ($(strip $(MODULE_APP)),true)
export EN_APP_X5BA = true 
export EN_APP_V9 = false
endif


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
	OPENSSLDIR.openssl-1.1.1 \
	WIFIDIR.wifi \
	MODEMDIR.modem \
	TOOLSDIR.tools \
	UDHCPDIR.udhcp \
	SQLITEDIR.sqlite \
	SYSTOOLSDIR.systools \
	LIBSSHDIR.ssh \
	PJSIPDIR.pjsip \
	APPDIR.application \
	WEBDIR.webserver \
	MQTTDIR.mqtt
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
        
