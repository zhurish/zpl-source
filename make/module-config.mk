
include $(ZPL_MAKE_DIR)/module-dir.mk

include $(ZPL_MAKE_DIR)/multimedia-config.mk
include $(ZPL_MAKE_DIR)/pjsip-config.mk

#
# platform
#
ZPLPRODS += $(PLATFORM_ROOT)/os
ZPLPRODS += $(PLATFORM_ROOT)/lib
ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
ZPLPRODS += $(PLATFORM_ROOT)/shell
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/shell
ZPL_DEFINE	+= -DZPL_SHELL_MODULE
endif
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/os
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/lib


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


ifeq ($(strip $(ZPL_IPCBUS_MODULE)),true)
ZPLPRODS += $(PLATFORM_ROOT)/ipcbus
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/ipcbus
ZPL_DEFINE += -DZPL_IPCBUS_MODULE
endif



ifeq ($(strip $(ZPL_NSM_MODULE)),true)

ZPLPRODS += $(PLATFORM_ROOT)/nsm
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/nsm
ZPL_DEFINE	+= -DZPL_NSM_MODULE
endif

ifeq ($(strip $(ZPL_RTPL_MODULE)),true)
ZPLPRODS += $(PLATFORM_ROOT)/rtpl
ZPL_INCLUDE += -I$(PLATFORM_ROOT)/rtpl
ZPL_DEFINE	+= -DZPL_NSM_RTPL
endif


#
# service
#
ifeq ($(strip $(ZPL_SERVICE_MODULE)),true)
ZPLPRODS += $(SERVICE_ROOT)/systools
ZPL_INCLUDE += -I$(SERVICE_ROOT)/systools

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
endif
ifeq ($(strip $(ZPL_SERVICE_TFTPD)),true)
ZPL_DEFINE += -DZPL_SERVICE_TFTPD
endif
ifeq ($(strip $(ZPL_SERVICE_FTPC)),true)
ZPL_DEFINE += -DZPL_SERVICE_FTPC
endif
ifeq ($(strip $(ZPL_SERVICE_FTPD)),true)
ZPL_DEFINE += -DZPL_SERVICE_FTPD
endif
ifeq ($(strip $(ZPL_SERVICE_TELNET)),true)
ZPL_DEFINE += -DZPL_SERVICE_TELNET
endif
ifeq ($(strip $(ZPL_SERVICE_TELNETD)),true)
ZPL_DEFINE += -DZPL_SERVICE_TELNETD
endif
ifeq ($(strip $(ZPL_SERVICE_PING)),true)
ZPL_DEFINE += -DZPL_SERVICE_PING
endif
ifeq ($(strip $(ZPL_SERVICE_TRACEROUTE)),true)
ZPL_DEFINE += -DZPL_SERVICE_TRACEROUTE
endif
ifeq ($(strip $(ZPL_SERVICE_UBUS_SYNC)),true)
ZPL_DEFINE += -DZPL_SERVICE_UBUS_SYNC
endif

endif #($(strip $(ZPL_SERVICE_MODULE)),true)


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
# Externsion openssl
#
ifeq ($(strip $(ZPL_OPENSSL_MODULE)),true)
ifneq ($(ZPL_BUILD_ARCH),X86_64)
ZPLEX_DIR += $(EXTERNSION_ROOT)/openssl/openssl-1.1.1/
export PLATFORM=linux-armv4
ZPLEX_INCLUDE += -I$(ZPL_INSTALL_ROOTFS_DIR)/include
ZPLEX_LDFLAGS += -L$(ZPL_INSTALL_ROOTFS_DIR)/lib
ZPLEX_LDLIBS += -lutil -lssl -lcrypto
ZPLEX_DIR += $(EXTERNSION_ROOT)/zlib/zlib-1.2.11/
ZPL_INCLUDE += -I$(ZPL_INSTALL_ROOTFS_DIR)/include
ZPLEX_LDFLAGS += -L$(ZPL_INSTALL_ROOTFS_DIR)/lib
ZPLEX_LDLIBS += -lz
else 
ZPLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(ZPL_BUILD_ARCH),X86_64)
ZPL_DEFINE += -DZPL_OPENSSL_MODULE
endif #($(strip $(ZPL_OPENSSL_MODULE)),true)

#
# ABSTRACT HAL PAL
#
ifeq ($(strip $(ZPL_ABSTRACT_MODULE)),true)

ifeq ($(strip $(ZPL_HAL_MODULE)),true)
ZPLPRODS += $(ABSTRACT_ROOT)/hal
ZPL_INCLUDE += -I$(ABSTRACT_ROOT)/hal
ZPL_DEFINE += -DZPL_HAL_MODULE
endif #($(strip $(ZPL_HAL_MODULE)),true)

ZPL_DEFINE += -DZPL_PAL_MODULE

ifeq ($(strip $(ZPL_KERNEL_STACK_MODULE)),true)
ZPLPRODS += $(ABSTRACT_ROOT)/pal/kernel
ZPL_INCLUDE += -I$(ABSTRACT_ROOT)/pal/kernel
ifeq ($(strip $(ZPL_KERNEL_STACK_NETLINK)),true)
ZPL_DEFINE += -DZPL_KERNEL_STACK_NETLINK
endif
ifeq ($(strip $(ZPL_KERNEL_SORF_FORWARDING)),true)
ZPL_DEFINE += -DZPL_KERNEL_SORF_FORWARDING
endif
endif

ifeq ($(strip $(ZPL_IPCOM_STACK_MODULE)),true)
ZPLPRODS += $(ABSTRACT_ROOT)/pal/ipstack
ZPL_INCLUDE += -I$(ABSTRACT_ROOT)/pal/ipstack
endif

endif #ifeq ($(strip $(ZPL_ABSTRACT_MODULE)),true)

#
# PRODUCT
#
ifeq ($(strip $(ZPL_PRODUCT_MODULE)),true)

ZPLPRODS += $(PRODUCT_ROOT)/bsp
ZPL_INCLUDE += -I$(PRODUCT_ROOT)/bsp
ZPL_DEFINE += -DZPL_BSP_MODULE

ifeq ($(strip $(ZPL_PRODUCT_SDK_MODULE)),true)
SW_SDK_ROOT=$(PRODUCT_ROOT)/sdk
ZPLPRODS += $(PRODUCT_ROOT)/sdk
ZPL_INCLUDE += -I$(PRODUCT_ROOT)/sdk
ZPL_DEFINE += -DZPL_SDK_MODULE
ZPL_DEFINE += -DZPL_SDK_BCM53125
endif #($(strip $(ZPL_PRODUCT_SDK_MODULE)),true)

endif #($(strip $(ZPL_PRODUCT_MODULE)),true)




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

endif


ZPLPRODS += $(ZPLPRODS_LAST)

#
# 下面两个模块保持在最后
# 


#
ZPLPRODS += $(STARTUP_ROOT)/src
ZPL_INCLUDE += -I$(STARTUP_ROOT)/src
ZPLPRODS += $(STARTUP_ROOT)/etc
