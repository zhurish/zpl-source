
include $(ZPL_MAKE_DIR)/module-dir.mk

include $(ZPL_MAKE_DIR)/multimedia-config.mk
include $(ZPL_MAKE_DIR)/pjsip-config.mk
include $(ZPL_MAKE_DIR)/externsions-config.mk
#
# platform
#
ZPLPRODS += $(PLATFORM_ROOT)/os
ZPLPRODS += $(PLATFORM_ROOT)/lib
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/os
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/lib

ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
ZPLPRODS += $(PLATFORM_ROOT)/shell
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/shell
ZPL_DEFINE	+= -DZPL_SHELL_MODULE
ifeq ($(strip $(ZPL_SHRL_MODULE)),true)
ZPL_DEFINE	+= -DZPL_SHRL_MODULE
endif
endif

ifeq ($(strip $(ZPL_OS_CPPJSON)),true)
JSONCPP_ROOT=$(PLATFORM_ROOT)/jsoncpp
ZPLPRODS += $(JSONCPP_ROOT)
ZPL_INCLUDE += -I$(JSONCPP_ROOT)
endif


ifeq ($(strip $(ZPL_OS_UCI)),true)
ifneq ($(OPENEWRT_BASE),)
OPENWRT_INCLUDE := -I$(OPENEWRT_BASE)/include -I$(OPENEWRT_BASE)/usr/include
OPENWRT_LDFLAGS := -L$(OPENEWRT_BASE)/lib -L$(OPENEWRT_BASE)/usr/lib
ZPLEX_LDLIBS += -luci
ZPL_DEFINE += -DZPL_OPENWRT_UCI -DZPL_OPENWRT_UCI_LIB	
else
LIBUCI_ROOT=$(PLATFORM_ROOT)/libuci
ZPLPRODS += $(LIBUCI_ROOT)
ZPL_INCLUDE += -I$(LIBUCI_ROOT)
ZPL_DEFINE += -DZPL_OPENWRT_UCI -DZPL_OPENWRT_UCI_LIB
endif #			
endif #($(strip $(ZPL_OS_UCI)),true)


ifeq ($(strip $(ZPL_LIBEVENT_MODULE)),true)
LIBEVENT_ROOT=$(PLATFORM_ROOT)/libevent
ZPLPRODS += $(LIBEVENT_ROOT)
ZPL_INCLUDE += -I$(LIBEVENT_ROOT)/include
ZPL_INCLUDE += -I$(LIBEVENT_ROOT)
ifeq ($(strip $(ZPL_LIBEVENT_SIGNAL)),true)
ZPL_DEFINE += -DZPL_LIBEVENT_SIGNAL
endif
endif

ifeq ($(strip $(ZPL_LIBMXML_MODULE)),true)
LIBMXML_ROOT=$(PLATFORM_ROOT)/mxml
ZPLPRODS += $(LIBMXML_ROOT)
ZPL_INCLUDE += -I$(LIBMXML_ROOT)
ZPL_DEFINE += -DZPL_LIBMXML_MODULE
endif

ifeq ($(strip $(ZPL_IPCBC_MODULE)),true)
ZPLPRODS += $(PLATFORM_ROOT)/ipcbc
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/ipcbc
ZPL_DEFINE += -DZPL_IPCBC_MODULE -DZPL_IPCBCBSP_MODULE
endif

ifeq ($(strip $(ZPL_ACTIVE_STANDBY)),true)
ZPLPRODS += $(PLATFORM_ROOT)/ipcstandby
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/ipcstandby
ZPL_DEFINE += -DZPL_ACTIVE_STANDBY
endif


ifeq ($(strip $(ZPL_NSM_MODULE)),true)
ZPLPRODS += $(PLATFORM_ROOT)/nsm
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/nsm
ZPL_DEFINE	+= -DZPL_NSM_MODULE

ifeq ($(strip $(ZPL_NSM_L3MODULE)),true)
ZPL_DEFINE	+= -DZPL_NSM_L3MODULE
endif
ifeq ($(strip $(ZPL_VRF_MODULE)),true)
ZPL_DEFINE	+= -DZPL_VRF_MODULE
endif


ifeq ($(strip $(ZPL_NSM_NEXTHOP)),true)
ZPL_DEFINE	+= -DZPL_NSM_NEXTHOP
endif
ifeq ($(strip $(ZPL_NSM_ROUTEMAP)),true)
ZPL_DEFINE	+= -DZPL_NSM_ROUTEMAP
endif

ifeq ($(strip $(ZPL_NSM_SNMP)),true)
ZPL_DEFINE	+= -DZPL_NSM_SNMP -DSNMP_AGENTX
ZPL_LDLIBS += -lnetsnmpmibs -lnetsnmpagent -lnetsnmp
endif

endif

#
# ABSTRACT HAL PAL
#
ifeq ($(strip $(ZPL_HALPAL_MODULE)),true)

ifeq ($(strip $(ZPL_HAL_MODULE)),true)
ZPLPRODS += $(HALPAL_ROOT)/hal
ZPL_INCLUDE += -I$(HALPAL_ROOT)/hal
ZPL_DEFINE += -DZPL_HAL_MODULE
endif #($(strip $(ZPL_HAL_MODULE)),true)

ifeq ($(strip $(ZPL_PAL_MODULE)),true)
ZPLPRODS += $(HALPAL_ROOT)/pal
ZPL_INCLUDE += -I$(HALPAL_ROOT)/pal
ZPL_DEFINE += -DZPL_PAL_MODULE

ifeq ($(strip $(ZPL_KERNEL_MODULE)),true)
ZPLPRODS += $(HALPAL_ROOT)/pal/linux
ZPL_INCLUDE += -I$(HALPAL_ROOT)/pal/linux
ifeq ($(strip $(ZPL_KERNEL_NETLINK)),true)
ZPL_DEFINE += -DZPL_KERNEL_NETLINK
endif
endif

ifeq ($(strip $(ZPL_IPCOM_MODULE)),true)
ZPLPRODS += $(HALPAL_ROOT)/pal/ipnet
ZPL_INCLUDE += -I$(HALPAL_ROOT)/pal/ipnet
endif

endif #($(strip $(ZPL_HAL_MODULE)),true)


endif #ifeq ($(strip $(ZPL_HALPAL_MODULE)),true)


#
# component
#
ifeq ($(strip $(ZPL_COMPONENT_MODULE)),true)

ifeq ($(strip $(ZPL_MODEM_MODULE)),true)
MODEM_ROOT=$(COMPONENT_ROOT)/modem
ZPLPRODS += $(MODEM_ROOT)
ZPL_INCLUDE += -I$(MODEM_ROOT)
ZPL_DEFINE += -DZPL_MODEM_MODULE
endif


ifeq ($(strip $(ZPL_NSM_DHCP)),true)
DHCP_ROOT=$(COMPONENT_ROOT)/udhcp
ZPLPRODS += $(DHCP_ROOT)
ZPL_INCLUDE += -I$(DHCP_ROOT)

ZPL_DEFINE += -DZPL_DHCP_MODULE
ZPL_DEFINE += -DZPL_DHCPC_MODULE
ZPL_DEFINE += -DZPL_DHCPD_MODULE

ifeq ($(strip $(ZPL_BUILD_IPV6)),true)
ZPL_DEFINE += -DZPL_DHCPV6C_MODULE
endif

endif


ifeq ($(strip $(ZPL_SQLITE_MODULE)),true)
SQLITE_ROOT=$(COMPONENT_ROOT)/sqlite
ZPLPRODS += $(SQLITE_ROOT)
ZPL_INCLUDE += -I$(SQLITE_ROOT)
ZPL_DEFINE += -DZPL_SQLITE_MODULE
ZPL_LDLIBS += -lsqlite
endif#($(strip $(ZPL_SQLITE_MODULE)),true)

ifeq ($(strip $(ZPL_WIFI_MODULE)),true)
WIFI_ROOT=$(COMPONENT_ROOT)/wifi
ZPLPRODS += $(WIFI_ROOT)
ZPL_INCLUDE += -I$(WIFI_ROOT)
ZPL_DEFINE += -DZPL_WIFI_MODULE
endif#($(strip $(ZPL_WIFI_MODULE)),true)


ifeq ($(strip $(ZPL_MQTT_MODULE)),true)
MQTT_ROOT=$(COMPONENT_ROOT)/mqtt
ZPLPRODS += $(MQTT_ROOT)
ZPL_INCLUDE += -I$(MQTT_ROOT)
ZPL_INCLUDE += -I$(MQTT_ROOT)/mqttlib
#ZPL_INCLUDE += -I$(MQTT_ROOT)/mqttc
#ZPL_INCLUDE += -I$(MQTT_ROOT)/mqtts

ZPL_DEFINE += -DZPL_MQTT_MODULE

export MQTT_SHARED_LIBRARIES = false
ifeq ($(strip $(MQTT_SHARED_LIBRARIES)),true)
ZPL_LDLIBS += -lmosquitto
endif
endif #($(strip $(ZPL_MQTT_MODULE)),true)


ifeq ($(strip $(ZPL_WEBSERVER_MODULE)),true)
WEBGUI_ROOT=$(COMPONENT_ROOT)/webserver
ZPLPRODS += $(WEBGUI_ROOT)
ZPL_INCLUDE += -I$(WEBGUI_ROOT)
ZPL_INCLUDE += -I$(WEBGUI_ROOT)/include
ZPL_DEFINE += -DZPL_WEBGUI_MODULE
endif

ifeq ($(strip $(ZPL_MODBUS_MODULE)),true)
MODBUS_ROOT=$(COMPONENT_ROOT)/modbus
ZPLPRODS += $(MODBUS_ROOT)
ZPL_INCLUDE += -I$(MODBUS_ROOT)
ZPL_INCLUDE += -I$(MODBUS_ROOT)/include
ZPL_DEFINE += -DZPL_MODBUS_MODULE
endif


ifeq ($(strip $(ZPL_ONVIF_MODULE)),true)
ZPLPRODS += $(COMPONENT_ROOT)/onvif
ZPL_INCLUDE += -I$(COMPONENT_ROOT)/onvif -I$(COMPONENT_ROOT)/onvif/gsoap -I$(COMPONENT_ROOT)/onvif/onvifgen
ZPL_DEFINE += -DZPL_ONVIF_MODULE -DSOAP_MEM_DEBUG  -DSOAP_DEBUG -DDEBUG_STAMP -DWITH_DOM -DWITH_NONAMESPACES -D__GLIBC__
ifeq ($(strip $(ZPL_ONVIF_SSL)),true)
ZPL_DEFINE += -DZPL_ONVIF_SSL -DWITH_OPENSSL
endif
endif	



ifeq ($(strip $(ZPL_LIBSSH_MODULE)),true)
LIBSSH_ROOT=$(COMPONENT_ROOT)/ssh

ZPLPRODS += $(LIBSSH_ROOT)
ZPL_INCLUDE += -I$(LIBSSH_ROOT)
ZPL_INCLUDE += -I$(LIBSSH_ROOT)/include

ifeq ($(ZPL_BUILD_ARCH),X86_64)
ZPL_INCLUDE += -I$(LIBSSH_ROOT)/include
#ZPLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(ZPL_BUILD_ARCH),X86_64)

ifeq ($(ZPL_BUILD_ARCH),MIPS)
ifneq ($(OPENEWRT_BASE),)
OPENWRT_INCLUDE := -I$(OPENEWRT_BASE)/include -I$(OPENEWRT_BASE)/usr/include
OPENWRT_LDFLAGS := -L$(OPENEWRT_BASE)/lib -L$(OPENEWRT_BASE)/usr/lib
else
ZPLEX_INCLUDE += -I$(EXTERNSION_ROOT)/openssl/mipsl/include
ZPLEX_LDFLAGS += -L$(EXTERNSION_ROOT)/openssl/mipsl/lib 
ZPLEX_INCLUDE += -I$(EXTERNSION_ROOT)/zlib/mipsl/zlib/include
ZPLEX_LDFLAGS += -L$(EXTERNSION_ROOT)/zlib/mipsl/zlib/lib
endif #($(OPENEWRT_BASE),)
#ZPLEX_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(ZPL_BUILD_ARCH),MIPS)

ZPL_DEFINE += -DZPL_LIBSSH_MODULE
#ZPLEX_LDLIBS += -lutil -lssl -lcrypto -lz

endif #($(strip $(ZPL_LIBSSH_MODULE)),true)

endif#($(strip $(ZPL_COMPONENT_MODULE)),true)




#
# service
#$(findstring true,$(strip $(ZPL_SERVICE_SNTPC)) )
#
ifeq ($(strip $(ZPL_SERVICE_MODULE)),true)

ifeq ($(strip $(ZPL_SERVICE_SNTPC)),true)
ZPLPRODS += $(SERVICE_ROOT)/sntp
ZPL_INCLUDE += -I$(SERVICE_ROOT)/sntp
endif

ifeq ($(strip $(ZPL_SERVICE_SNTPS)),true)
ifeq ($(strip $(ZPL_SERVICE_SNTPC)),false)
ZPLPRODS += $(SERVICE_ROOT)/sntp
ZPL_INCLUDE += -I$(SERVICE_ROOT)/sntp
endif
endif

ifeq ($(strip $(ZPL_SERVICE_SNTPC)),true)
ZPL_DEFINE += -DZPL_SERVICE_SNTPC
endif

ifeq ($(strip $(ZPL_SERVICE_SNTPS)),true)
ZPL_DEFINE += -DZPL_SERVICE_SNTPS
endif

ifeq ($(strip $(ZPL_SERVICE_SYSLOG)),true)
ZPLPRODS += $(SERVICE_ROOT)/syslog
ZPL_INCLUDE += -I$(SERVICE_ROOT)/syslog
ZPL_DEFINE += -DZPL_SERVICE_SYSLOG
endif


ifeq ($(strip $(ZPL_SERVICE_TFTPC)),true)
ZPL_DEFINE += -DZPL_SERVICE_TFTPC
ZPLPRODS += $(SERVICE_ROOT)/tftp
ZPL_INCLUDE += -I$(SERVICE_ROOT)/tftp
endif
ifeq ($(strip $(ZPL_SERVICE_TFTPD)),true)
ZPL_DEFINE += -DZPL_SERVICE_TFTPD
ifneq ($(strip $(ZPL_SERVICE_TFTPC)),true)
ZPLPRODS += $(SERVICE_ROOT)/tftp
ZPL_INCLUDE += -I$(SERVICE_ROOT)/tftp
endif
endif

ifeq ($(strip $(ZPL_SERVICE_FTPC)),true)
ZPL_DEFINE += -DZPL_SERVICE_FTPC
ZPLPRODS += $(SERVICE_ROOT)/ftp
ZPL_INCLUDE += -I$(SERVICE_ROOT)/ftp
endif
ifeq ($(strip $(ZPL_SERVICE_FTPD)),true)
ZPL_DEFINE += -DZPL_SERVICE_FTPD
ifneq ($(strip $(ZPL_SERVICE_FTPC)),true)
ZPLPRODS += $(SERVICE_ROOT)/ftp
ZPL_INCLUDE += -I$(SERVICE_ROOT)/ftp
endif
endif

ifeq ($(strip $(ZPL_SERVICE_TELNET)),true)
ZPL_DEFINE += -DZPL_SERVICE_TELNET
ZPLPRODS += $(SERVICE_ROOT)/telnet
ZPL_INCLUDE += -I$(SERVICE_ROOT)/telnet
endif
ifeq ($(strip $(ZPL_SERVICE_TELNETD)),true)
ZPL_DEFINE += -DZPL_SERVICE_TELNETD
endif
ifeq ($(strip $(ZPL_SERVICE_PING)),true)
ZPL_DEFINE += -DZPL_SERVICE_PING
ZPLPRODS += $(SERVICE_ROOT)/ping
ZPL_INCLUDE += -I$(SERVICE_ROOT)/ping
endif
ifeq ($(strip $(ZPL_SERVICE_TRACEROUTE)),true)
ZPL_DEFINE += -DZPL_SERVICE_TRACEROUTE
ZPLPRODS += $(SERVICE_ROOT)/traceroute
ZPL_INCLUDE += -I$(SERVICE_ROOT)/traceroute
endif
ifeq ($(strip $(ZPL_SERVICE_UBUS_SYNC)),true)
ZPL_DEFINE += -DZPL_SERVICE_UBUS_SYNC
endif
ZPLPRODS += $(SERVICE_ROOT)/service
ZPL_INCLUDE += -I$(SERVICE_ROOT)/service
endif #($(strip $(ZPL_SERVICE_MODULE)),true)


#
# Externsion openssl
#

#
# PRODUCT
#
ifeq ($(strip $(ZPL_BSP_MODULE)),true)

ZPLPRODS += $(PRODUCT_ROOT)/bsp/hal
ZPL_INCLUDE += -I$(PRODUCT_ROOT)/bsp/hal
ZPL_DEFINE += -DZPL_BSP_MODULE

ZPLPRODS_KERNEL += $(PRODUCT_ROOT)/bsp/kernel

ifeq ($(strip $(ZPL_SDK_MODULE)),true)
ZPL_DEFINE += -DZPL_SDK_MODULE
ifeq ($(strip $(ZPL_SDK_NONE)),true) 
ZPL_DEFINE += -DZPL_SDK_NONE
endif
ifeq ($(strip $(ZPL_SDK_USER)),true) 
ZPL_DEFINE += -DZPL_SDK_USER
ifeq ($(strip $(ZPL_SDK_BCM53125)),true)
ZPLPRODS += $(PRODUCT_ROOT)/sdk/user

ZPL_INCLUDE += -I$(PRODUCT_ROOT)/sdk/user
ZPL_DEFINE += -DZPL_SDK_BCM53125
endif #($(strip $(ZPL_SDK_BCM53125)),true)
endif #($(strip $(ZPL_SDK_USER)),true)

ifeq ($(strip $(ZPL_SDK_KERNEL)),true)
ZPL_DEFINE += -DZPL_SDK_KERNEL
ifeq ($(strip $(ZPL_SDK_BCM53125)),true)
ZPLPRODS_KERNEL += $(PRODUCT_ROOT)/bsp/kernel
ZPL_DEFINE += -DZPL_SDK_BCM53125
endif #($(strip $(ZPL_SDK_BCM53125)),true)
endif #($(strip $(ZPL_SDK_KERNEL)),true)


endif #($(strip $(ZPL_SDK_MODULE)),true)


endif #($(strip $(ZPL_BSP_MODULE)),true)



#
#application
#
ifeq ($(strip $(ZPL_APPLICATION_MODULE)),true)
ZPLPRODS += $(APP_ROOT)
ZPL_INCLUDE += -I$(APP_ROOT)
ZPL_DEFINE += -DZPL_APP_MODULE

ifeq ($(strip $(ZPL_APP_X5_MODULE)),true)
ZPLM_DEFINE += -DAPP_X5BA_MODULE
endif
ifeq ($(strip $(ZPL_APP_V9_MODULE)),true)
ZPLM_DEFINE += -DAPP_V9_MODULE
ZPLM_DEFINE += -DZPL_VIDEO_MODULE
#ZPL_LDLIBS += -loal_privateProtocol
endif
endif


#
#TOOLS
#
ifeq ($(strip $(ZPL_TOOLS_MODULE)),true)

ifeq ($(strip $(ZPL_VTYSH_MODULE)),true)
ZPLPRODS += $(TOOLS_ROOT)/vtysh
ZPL_INCLUDE += -I$(TOOLS_ROOT)/vtysh
ZPL_DEFINE += -DZPL_VTYSH_MODULE
endif

ifeq ($(strip $(ZPL_WATCHDOG_MODULE)),true)
ZPLPRODS += $(TOOLS_ROOT)/watchdog
ZPL_INCLUDE += -I$(TOOLS_ROOT)/watchdog
ZPL_DEFINE += -DZPL_WATCHDOG_MODULE
endif

ifeq ($(strip $(ZPL_TOOLS_PROCESS)),true)
ZPLPRODS += $(TOOLS_ROOT)/process
ZPL_INCLUDE += -I$(TOOLS_ROOT)/process
ZPL_DEFINE += -DZPL_TOOLS_PROCESS
endif

ifeq ($(strip $(ZPL_TOOLS_QUECTEL_CM)),true)
ZPLPRODS += $(TOOLS_ROOT)/quectel-CM
ZPL_INCLUDE += -I$(TOOLS_ROOT)/quectel-CM
endif

ifeq ($(strip $(ZPL_TOOLS_SYSTEM)),true)
ZPLPRODS += $(TOOLS_ROOT)/system
ZPL_INCLUDE += -I$(TOOLS_ROOT)/system
endif

ifeq ($(strip $(ZPL_SWCONFIG_MODULE)),true)
ZPLPRODS += $(TOOLS_ROOT)/swconfig
ZPL_INCLUDE += -I$(TOOLS_ROOT)/swconfig
ZPL_INCLUDE += -I$(TOOLS_ROOT)/swconfig/include
ZPL_INCLUDE += -I$(TOOLS_ROOT)/swconfig/include/libnl-tiny
endif

endif


ZPLPRODS += $(ZPLPRODS_LAST)
ZPLPRODS += $(ZPLPRODS_KERNEL)
#
# 下面两个模块保持在最后
# 


#
ZPLPRODS += $(STARTUP_ROOT)/src
ZPL_INCLUDE += -I$(STARTUP_ROOT)/src
ZPLPRODS += $(STARTUP_ROOT)/etc
