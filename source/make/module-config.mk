
include $(ZPL_MAKE_DIR)/module-dir.mk

include $(ZPL_MAKE_DIR)/multimedia-config.mk
include $(ZPL_MAKE_DIR)/pjsip-config.mk
include $(ZPL_MAKE_DIR)/externsions-config.mk
#
# platform
#
OS_LIB_DIR = $(PLATFORM_DIR)/os
OS_ZPLIB_DIR = $(PLATFORM_DIR)/lib

ZPLPRODS += $(ZPLBASE)/$(OS_LIB_DIR)
ZPLPRODS += $(ZPLBASE)/$(OS_ZPLIB_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(OS_LIB_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(OS_ZPLIB_DIR)



ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
SHELL_DIR = $(PLATFORM_DIR)/shell
ZPLPRODS += $(ZPLBASE)/$(SHELL_DIR)

ZPL_INCLUDE += -I$(ZPLBASE)/$(SHELL_DIR)
ZPL_DEFINE	+= -DZPL_SHELL_MODULE
ifeq ($(strip $(ZPL_SHRL_MODULE)),true)
ZPL_DEFINE	+= -DZPL_SHRL_MODULE
endif
endif

ifeq ($(strip $(ZPL_OS_CPPJSON)),true)
JSONCPP_ROOT=$(PLATFORM_DIR)/jsoncpp
ZPLPRODS += $(ZPLBASE)/$(JSONCPP_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(JSONCPP_ROOT)
endif


ifeq ($(strip $(ZPL_OS_UCI)),true)
ifneq ($(OPENEWRT_BASE),)
OPENWRT_INCLUDE := -I$(OPENEWRT_BASE)/include -I$(OPENEWRT_BASE)/usr/include
OPENWRT_LDFLAGS := -L$(OPENEWRT_BASE)/lib -L$(OPENEWRT_BASE)/usr/lib
ZPLEX_LDLIBS += -luci
ZPL_DEFINE += -DZPL_OPENWRT_UCI -DZPL_OPENWRT_UCI_LIB	
else
LIBUCI_ROOT=$(PLATFORM_DIR)/libuci
ZPLPRODS += $(ZPLBASE)/$(LIBUCI_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBUCI_ROOT)
ZPL_DEFINE += -DZPL_OPENWRT_UCI -DZPL_OPENWRT_UCI_LIB
endif #			
endif #($(strip $(ZPL_OS_UCI)),true)


ifeq ($(strip $(ZPL_JTHREAD_MODULE)),true)
LIBJTHREAD_ROOT=$(PLATFORM_DIR)/jthread
ZPLPRODS += $(ZPLBASE)/$(LIBJTHREAD_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBJTHREAD_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBJTHREAD_ROOT)/jthread
ZPL_DEFINE += -DZPL_JTHREAD_MODULE
endif

ifeq ($(strip $(ZPL_LIBEVENT_MODULE)),true)
LIBEVENT_ROOT=$(PLATFORM_DIR)/libevent
ZPLPRODS += $(ZPLBASE)/$(LIBEVENT_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBEVENT_ROOT)/include
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBEVENT_ROOT)
ifeq ($(strip $(ZPL_LIBEVENT_SIGNAL)),true)
ZPL_DEFINE += -DZPL_LIBEVENT_SIGNAL
endif
endif

ifeq ($(strip $(ZPL_LIBMXML_MODULE)),true)
LIBMXML_ROOT=$(PLATFORM_DIR)/mxml
ZPLPRODS += $(ZPLBASE)/$(LIBMXML_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBMXML_ROOT)
ZPL_DEFINE += -DZPL_LIBMXML_MODULE
endif

ifeq ($(strip $(ZPL_IPCBC_MODULE)),true)
IPCBC_ROOT=$(PLATFORM_DIR)/ipcbc
ZPLPRODS += $(ZPLBASE)/$(IPCBC_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(IPCBC_ROOT)
ZPL_DEFINE += -DZPL_IPCBC_MODULE -DZPL_IPCBCBSP_MODULE
endif

ifeq ($(strip $(ZPL_ACTIVE_STANDBY)),true)
STANDBY_DIR=$(PLATFORM_DIR)/ipcstandby
ZPLPRODS += $(ZPLBASE)/$(STANDBY_DIR) 
ZPL_INCLUDE += -I$(ZPLBASE)/$(STANDBY_DIR)
ZPL_DEFINE += -DZPL_ACTIVE_STANDBY
endif


ifeq ($(strip $(ZPL_NSM_MODULE)),true)
NSM_ROOT=$(PLATFORM_DIR)/nsm
ZPLPRODS += $(ZPLBASE)/$(NSM_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(NSM_ROOT)
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
HAL_DIR=$(HALPAL_DIR)/hal
ZPLPRODS += $(ZPLBASE)/$(HAL_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(HAL_DIR)
ZPL_DEFINE += -DZPL_HAL_MODULE
endif #($(strip $(ZPL_HAL_MODULE)),true)

ifeq ($(strip $(ZPL_PAL_MODULE)),true)
PAL_DIR=$(HALPAL_DIR)/pal
ZPLPRODS += $(ZPLBASE)/$(PAL_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(PAL_DIR)
ZPL_DEFINE += -DZPL_PAL_MODULE

ifeq ($(strip $(ZPL_KERNEL_MODULE)),true)
KER_DIR=$(HALPAL_DIR)/pal/linux
ZPLPRODS += $(ZPLBASE)/$(KER_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(KER_DIR)
ifeq ($(strip $(ZPL_KERNEL_NETLINK)),true)
ZPL_DEFINE += -DZPL_KERNEL_NETLINK
endif
endif

ifeq ($(strip $(ZPL_IPCOM_MODULE)),true)
IPNET_DIR=$(HALPAL_DIR)/pal/ipnet
ZPLPRODS += $(ZPLBASE)/$(IPNET_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(IPNET_DIR)

endif

endif #($(strip $(ZPL_HAL_MODULE)),true)


endif #ifeq ($(strip $(ZPL_HALPAL_MODULE)),true)


#
# component
#
ifeq ($(strip $(ZPL_COMPONENT_MODULE)),true)

ifeq ($(strip $(ZPL_MODEM_MODULE)),true)
MODEM_DIR=$(COMPONENT_DIR)/modem
ZPLPRODS += $(ZPLBASE)/$(MODEM_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(MODEM_DIR)
ZPL_DEFINE += -DZPL_MODEM_MODULE
endif


ifeq ($(strip $(ZPL_NSM_DHCP)),true)
DHCP_ROOT=$(COMPONENT_DIR)/udhcp
ZPLPRODS += $(ZPLBASE)/$(DHCP_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(DHCP_ROOT)

ZPL_DEFINE += -DZPL_DHCP_MODULE
ZPL_DEFINE += -DZPL_DHCPC_MODULE
ZPL_DEFINE += -DZPL_DHCPD_MODULE

ifeq ($(strip $(ZPL_BUILD_IPV6)),true)
ZPL_DEFINE += -DZPL_DHCPV6C_MODULE
endif

endif


ifeq ($(strip $(ZPL_SQLITE_MODULE)),true)
SQLITE_ROOT=$(COMPONENT_DIR)/sqlite
ZPLPRODS += $(ZPLBASE)/$(SQLITE_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(SQLITE_ROOT)
ZPL_DEFINE += -DZPL_SQLITE_MODULE
ZPL_LDLIBS += -lsqlite
endif#($(strip $(ZPL_SQLITE_MODULE)),true)

ifeq ($(strip $(ZPL_WIFI_MODULE)),true)
WIFI_ROOT=$(COMPONENT_DIR)/wifi
ZPLPRODS += $(ZPLBASE)/$(WIFI_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(WIFI_ROOT)
ZPL_DEFINE += -DZPL_WIFI_MODULE
endif#($(strip $(ZPL_WIFI_MODULE)),true)


ifeq ($(strip $(ZPL_MQTT_MODULE)),true)
MQTT_ROOT=$(COMPONENT_DIR)/mqtt
ZPLPRODS += $(ZPLBASE)/$(MQTT_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(MQTT_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(MQTT_ROOT)/mqttlib
#ZPL_INCLUDE += -I$(MQTT_ROOT)/mqttc
#ZPL_INCLUDE += -I$(MQTT_ROOT)/mqtts

ZPL_DEFINE += -DZPL_MQTT_MODULE

export MQTT_SHARED_LIBRARIES = false
ifeq ($(strip $(MQTT_SHARED_LIBRARIES)),true)
ZPL_LDLIBS += -lmosquitto
endif
endif #($(strip $(ZPL_MQTT_MODULE)),true)


ifeq ($(strip $(ZPL_WEBSERVER_MODULE)),true)
WEBGUI_ROOT=$(COMPONENT_DIR)/webserver
ZPLPRODS += $(ZPLBASE)/$(WEBGUI_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(WEBGUI_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(WEBGUI_ROOT)/include
ZPL_DEFINE += -DZPL_WEBGUI_MODULE
endif

ifeq ($(strip $(ZPL_MODBUS_MODULE)),true)
MODBUS_ROOT=$(COMPONENT_DIR)/modbus
ZPLPRODS += $(ZPLBASE)/$(MODBUS_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(MODBUS_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(MODBUS_ROOT)/include
ZPL_DEFINE += -DZPL_MODBUS_MODULE
endif


ifeq ($(strip $(ZPL_ONVIF_MODULE)),true)
ZPLPRODS += $(COMPONENT_BASE_ROOT)/onvif
ZPL_INCLUDE += -I$(COMPONENT_BASE_ROOT)/onvif -I$(COMPONENT_BASE_ROOT)/onvif/gsoap -I$(COMPONENT_BASE_ROOT)/onvif/onvifgen
ZPL_DEFINE += -DZPL_ONVIF_MODULE -DSOAP_MEM_DEBUG  -DSOAP_DEBUG -DDEBUG_STAMP -DWITH_DOM -DWITH_NONAMESPACES -D__GLIBC__
ifeq ($(strip $(ZPL_ONVIF_SSL)),true)
ZPL_DEFINE += -DZPL_ONVIF_SSL -DWITH_OPENSSL
endif
endif	



ifeq ($(strip $(ZPL_LIBSSH_MODULE)),true)
LIBSSH_ROOT=$(COMPONENT_DIR)/ssh
ZPLPRODS += $(ZPLBASE)/$(LIBSSH_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBSSH_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBSSH_ROOT)/include

ifeq ($(ZPL_BUILD_ARCH),X86_64)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBSSH_ROOT)/include
#ZPLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(ZPL_BUILD_ARCH),X86_64)

ifeq ($(ZPL_BUILD_ARCH),MIPS)
ifneq ($(OPENEWRT_BASE),)
OPENWRT_INCLUDE := -I$(OPENEWRT_BASE)/include -I$(OPENEWRT_BASE)/usr/include
OPENWRT_LDFLAGS := -L$(OPENEWRT_BASE)/lib -L$(OPENEWRT_BASE)/usr/lib
else
ZPLEX_INCLUDE += -I$(EXTERNSION_BASE_ROOT)/openssl/mipsl/include
ZPLEX_LDFLAGS += -L$(EXTERNSION_BASE_ROOT)/openssl/mipsl/lib 
ZPLEX_INCLUDE += -I$(EXTERNSION_BASE_ROOT)/zlib/mipsl/zlib/include
ZPLEX_LDFLAGS += -L$(EXTERNSION_BASE_ROOT)/zlib/mipsl/zlib/lib
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
SNTPC_DIR=$(SERVICE_DIR)/sntp
ZPLPRODS += $(ZPLBASE)/$(SNTPC_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(SNTPC_DIR)
endif

ifeq ($(strip $(ZPL_SERVICE_SNTPS)),true)
ifeq ($(strip $(ZPL_SERVICE_SNTPC)),false)
SNTPC_DIR=$(SERVICE_DIR)/sntp
ZPLPRODS += $(ZPLBASE)/$(SNTPC_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(SNTPC_DIR)
endif
endif

ifeq ($(strip $(ZPL_SERVICE_SNTPC)),true)
ZPL_DEFINE += -DZPL_SERVICE_SNTPC
endif

ifeq ($(strip $(ZPL_SERVICE_SNTPS)),true)
ZPL_DEFINE += -DZPL_SERVICE_SNTPS
endif

ifeq ($(strip $(ZPL_SERVICE_SYSLOG)),true)
SYSLOG_DIR = $(SERVICE_DIR)/syslog
ZPLPRODS += $(ZPLBASE)/$(SYSLOG_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(SYSLOG_DIR)
ZPL_DEFINE += -DZPL_SERVICE_SYSLOG
endif


ifeq ($(strip $(ZPL_SERVICE_TFTPC)),true)
ZPL_DEFINE += -DZPL_SERVICE_TFTPC
TFTP_DIR = $(SERVICE_DIR)/tftp
ZPLPRODS += $(ZPLBASE)/$(TFTP_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(TFTP_DIR)
endif
ifeq ($(strip $(ZPL_SERVICE_TFTPD)),true)
ZPL_DEFINE += -DZPL_SERVICE_TFTPD
ifneq ($(strip $(ZPL_SERVICE_TFTPC)),true)
TFTP_DIR = $(SERVICE_DIR)/tftp
ZPLPRODS += $(ZPLBASE)/$(TFTP_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(TFTP_DIR)
endif
endif

ifeq ($(strip $(ZPL_SERVICE_FTPC)),true)
ZPL_DEFINE += -DZPL_SERVICE_FTPC
FTP_DIR = $(SERVICE_DIR)/ftp
ZPLPRODS += $(ZPLBASE)/$(FTP_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(FTP_DIR)
endif
ifeq ($(strip $(ZPL_SERVICE_FTPD)),true)
ZPL_DEFINE += -DZPL_SERVICE_FTPD
ifneq ($(strip $(ZPL_SERVICE_FTPC)),true)
FTP_DIR = $(SERVICE_DIR)/ftp
ZPLPRODS += $(ZPLBASE)/$(FTP_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(FTP_DIR)
endif
endif

ifeq ($(strip $(ZPL_SERVICE_TELNET)),true)
ZPL_DEFINE += -DZPL_SERVICE_TELNET
TELNET_DIR = $(SERVICE_DIR)/telnet
ZPLPRODS += $(ZPLBASE)/$(TELNET_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(TELNET_DIR)

endif
ifeq ($(strip $(ZPL_SERVICE_TELNETD)),true)
ZPL_DEFINE += -DZPL_SERVICE_TELNETD
endif
ifeq ($(strip $(ZPL_SERVICE_PING)),true)
ZPL_DEFINE += -DZPL_SERVICE_PING
PING_DIR = $(SERVICE_DIR)/ping
ZPLPRODS += $(ZPLBASE)/$(PING_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(PING_DIR)
endif
ifeq ($(strip $(ZPL_SERVICE_TRACEROUTE)),true)
ZPL_DEFINE += -DZPL_SERVICE_TRACEROUTE
TRACEROUTE_DIR = $(SERVICE_DIR)/traceroute
ZPLPRODS += $(ZPLBASE)/$(TRACEROUTE_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(TRACEROUTE_DIR)
endif
ifeq ($(strip $(ZPL_SERVICE_UBUS_SYNC)),true)
ZPL_DEFINE += -DZPL_SERVICE_UBUS_SYNC
endif
MYSERVICE_DIR = $(SERVICE_DIR)/service
ZPLPRODS += $(ZPLBASE)/$(MYSERVICE_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(MYSERVICE_DIR)
endif #($(strip $(ZPL_SERVICE_MODULE)),true)


#
# Externsion openssl
#

#
# PRODUCT
#
ifeq ($(strip $(ZPL_BSP_MODULE)),true)

BSP_HAL_DIR = $(PRODUCT_DIR)/bsp/hal
ZPLPRODS += $(ZPLBASE)/$(BSP_HAL_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(BSP_HAL_DIR)

ZPL_DEFINE += -DZPL_BSP_MODULE

BSP_KER_DIR = $(PRODUCT_DIR)/bsp/kernel
ZPLPRODS_KERNEL += $(ZPLBASE)/$(BSP_KER_DIR)
#ZPL_INCLUDE += -I$(ZPLBASE)/$(BSP_KER_DIR)


ifeq ($(strip $(ZPL_SDK_MODULE)),true)
ZPL_DEFINE += -DZPL_SDK_MODULE
ifeq ($(strip $(ZPL_SDK_NONE)),true) 
ZPL_DEFINE += -DZPL_SDK_NONE
endif
ifeq ($(strip $(ZPL_SDK_USER)),true) 
ZPL_DEFINE += -DZPL_SDK_USER
ifeq ($(strip $(ZPL_SDK_BCM53125)),true)

SDK_USER_DIR = $(PRODUCT_DIR)/sdk/user
ZPLPRODS += $(ZPLBASE)/$(SDK_USER_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(SDK_USER_DIR)
ZPL_DEFINE += -DZPL_SDK_BCM53125
endif #($(strip $(ZPL_SDK_BCM53125)),true)
endif #($(strip $(ZPL_SDK_USER)),true)

ifeq ($(strip $(ZPL_SDK_KERNEL)),true)
ZPL_DEFINE += -DZPL_SDK_KERNEL
ifeq ($(strip $(ZPL_SDK_BCM53125)),true)
SDK_KER_DIR = $(PRODUCT_DIR)/sdk/kernel
ZPLPRODS_KERNEL += $(ZPLBASE)/$(SDK_KER_DIR)
ZPL_DEFINE += -DZPL_SDK_BCM53125
endif #($(strip $(ZPL_SDK_BCM53125)),true)
endif #($(strip $(ZPL_SDK_KERNEL)),true)


endif #($(strip $(ZPL_SDK_MODULE)),true)


endif #($(strip $(ZPL_BSP_MODULE)),true)



#
#application
#
ifeq ($(strip $(ZPL_APPLICATION_MODULE)),true)
ZPLPRODS += $(APP_BASE_ROOT)
ZPL_INCLUDE += -I$(APP_BASE_ROOT)
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

VTYSH_DIR = $(TOOLS_DIR)/vtysh
ZPLPRODS += $(ZPLBASE)/$(VTYSH_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(VTYSH_DIR)

ZPL_DEFINE += -DZPL_VTYSH_MODULE
endif

ifeq ($(strip $(ZPL_WATCHDOG_MODULE)),true)
WATCHDOG_DIR = $(TOOLS_DIR)/watchdog
ZPLPRODS += $(ZPLBASE)/$(WATCHDOG_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(WATCHDOG_DIR)

ZPL_DEFINE += -DZPL_WATCHDOG_MODULE
endif

ifeq ($(strip $(ZPL_TOOLS_PROCESS)),true)

PROCESS_DIR = $(TOOLS_DIR)/process
ZPLPRODS += $(ZPLBASE)/$(PROCESS_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(PROCESS_DIR)

ZPL_DEFINE += -DZPL_TOOLS_PROCESS
endif

ifeq ($(strip $(ZPL_TOOLS_QUECTEL_CM)),true)
QUECTEL_DIR = $(TOOLS_DIR)/quectel-CM
ZPLPRODS += $(ZPLBASE)/$(QUECTEL_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(QUECTEL_DIR)
endif

ifeq ($(strip $(ZPL_TOOLS_SYSTEM)),true)
SYSTEM_DIR = $(TOOLS_DIR)/system
ZPLPRODS += $(ZPLBASE)/$(SYSTEM_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(SYSTEM_DIR)
endif

ifeq ($(strip $(ZPL_SWCONFIG_MODULE)),true)
swconfig_DIR = $(TOOLS_DIR)/swconfig
ZPLPRODS += $(ZPLBASE)/$(swconfig_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(swconfig_DIR)

ZPL_INCLUDE += -I$(ZPLBASE)/$(swconfig_DIR)/include
ZPL_INCLUDE += -I$(ZPLBASE)/$(swconfig_DIR)/include/libnl-tiny
endif

endif


ZPLPRODS += $(ZPLPRODS_LAST)
#ZPLPRODS += $(ZPLPRODS_KERNEL)
#
# 下面两个模块保持在最后
# 


#
STARTUP_SRC_DIR = $(STARTUP_DIR)/src

ZPLPRODS += $(ZPLBASE)/$(STARTUP_SRC_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(STARTUP_SRC_DIR)
ZPLPRODS += $(ZPLBASE)/$(STARTUP_DIR)/etc
