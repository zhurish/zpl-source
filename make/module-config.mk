
include $(MAKE_DIR)/module-dir.mk

include $(MAKE_DIR)/multimedia-config.mk
include $(MAKE_DIR)/pjsip-config.mk

PLPRODS += $(PLATFORM_ROOT)/os
PLPRODS += $(PLATFORM_ROOT)/lib

PLPRODS += $(PLATFORM_ROOT)/shell

PL_INCLUDE += -I$(PLATFORM_ROOT)/os
PL_INCLUDE += -I$(PLATFORM_ROOT)/lib
PL_INCLUDE += -I$(PLATFORM_ROOT)/shell

ifeq ($(strip $(PL_NSM_MODULE)),true)

PLPRODS += $(PLATFORM_ROOT)/nsm
PL_INCLUDE += -I$(PLATFORM_ROOT)/nsm
PL_DEFINE	+= -DPL_NSM_MODULE
PLCLI_DIR += $(CLI_ROOT)/nsm
endif

ifeq ($(strip $(PL_CLI_MODULE)),true)
PLCLI_DIR += $(CLI_ROOT)/system
endif



ifeq ($(strip $(PL_OS_CPPJSON)),true)
JSONCPP_ROOT=$(PLATFORM_ROOT)/jsoncpp
PLPRODS += $(JSONCPP_ROOT)
PL_INCLUDE += -I$(JSONCPP_ROOT)
endif


ifeq ($(strip $(PL_OS_UCI)),true)
ifneq ($(OPENEWRT_BASE),)
OPENWRT_INCLUDE := -I$(OPENEWRT_BASE)/include -I$(OPENEWRT_BASE)/usr/include
OPENWRT_LDFLAGS := -L$(OPENEWRT_BASE)/lib -L$(OPENEWRT_BASE)/usr/lib
PLEX_LDLIBS += -luci
PL_DEFINE += -DPL_OPENWRT_UCI -DPL_OPENWRT_UCI_LIB	
else
LIBUCI_ROOT=$(PLATFORM_ROOT)/libuci
PLPRODS += $(LIBUCI_ROOT)
PL_INCLUDE += -I$(LIBUCI_ROOT)
PL_DEFINE += -DPL_OPENWRT_UCI -DPL_OPENWRT_UCI_LIB
endif #			
endif #($(strip $(PL_OS_UCI)),true)




ifeq ($(strip $(PL_SERVICE_MODULE)),true)
PLPRODS += $(SERVICE_ROOT)/systools
PLCLI_DIR += $(CLI_ROOT)/service
PL_INCLUDE += -I$(SERVICE_ROOT)/systools

ifeq ($(strip $(PL_SERVICE_SNTPC)),true)
PLPRODS += $(SERVICE_ROOT)/sntp
PL_INCLUDE += -I$(SERVICE_ROOT)/sntp
endif

ifeq ($(strip $(PL_SERVICE_SNTPS)),true)
ifeq ($(strip $(PL_SERVICE_SNTPC)),false)
PLPRODS += $(SERVICE_ROOT)/sntp
PL_INCLUDE += -I$(SERVICE_ROOT)/sntp
endif
endif

ifeq ($(strip $(PL_SERVICE_SNTPC)),true)
PL_DEFINE += -DPL_SERVICE_SNTPC
endif

ifeq ($(strip $(PL_SERVICE_SNTPS)),true)
PL_DEFINE += -DPL_SERVICE_SNTPS
endif

ifeq ($(strip $(PL_SERVICE_SYSLOG)),true)
PLPRODS += $(SERVICE_ROOT)/syslog
PL_INCLUDE += -I$(SERVICE_ROOT)/syslog
PL_DEFINE += -DPL_SERVICE_SYSLOG
endif

ifeq ($(strip $(PL_SERVICE_TFTPC)),true)
PL_DEFINE += -DPL_SERVICE_TFTPC
endif
ifeq ($(strip $(PL_SERVICE_TFTPD)),true)
PL_DEFINE += -DPL_SERVICE_TFTPD
endif
ifeq ($(strip $(PL_SERVICE_FTPC)),true)
PL_DEFINE += -DPL_SERVICE_FTPC
endif
ifeq ($(strip $(PL_SERVICE_FTPD)),true)
PL_DEFINE += -DPL_SERVICE_FTPD
endif
ifeq ($(strip $(PL_SERVICE_TELNET)),true)
PL_DEFINE += -DPL_SERVICE_TELNET
endif
ifeq ($(strip $(PL_SERVICE_TELNETD)),true)
PL_DEFINE += -DPL_SERVICE_TELNETD
endif
ifeq ($(strip $(PL_SERVICE_PING)),true)
PL_DEFINE += -DPL_SERVICE_PING
endif
ifeq ($(strip $(PL_SERVICE_TRACEROUTE)),true)
PL_DEFINE += -DPL_SERVICE_TRACEROUTE
endif
ifeq ($(strip $(PL_SERVICE_UBUS_SYNC)),true)
PL_DEFINE += -DPL_SERVICE_UBUS_SYNC
endif

endif #($(strip $(PL_SERVICE_MODULE)),true)


ifeq ($(strip $(PL_PRODUCT_MODULE)),true)

PLPRODS += $(PRODUCT_ROOT)/bsp
PL_INCLUDE += -I$(PRODUCT_ROOT)/bsp
PL_DEFINE += -DPL_BSP_MODULE


ifeq ($(strip $(PL_PRODUCT_SDK_MODULE)),true)
SW_SDK_ROOT=$(PRODUCT_ROOT)/sdk
PLPRODS += $(PRODUCT_ROOT)/sdk
PL_INCLUDE += -I$(PRODUCT_ROOT)/sdk
PL_DEFINE += -DPL_SDK_MODULE
PL_DEFINE += -DPL_SDK_BCM53125
endif #($(strip $(PL_PRODUCT_SDK_MODULE)),true)

endif #($(strip $(PL_PRODUCT_MODULE)),true)


#PL_ABSTRACT_MODULE
ifeq ($(strip $(PL_ABSTRACT_MODULE)),true)

PL_DEFINE += -DPL_PAL_MODULE

ifeq ($(strip $(PL_PAL_KERNEL_STACK)),true)
PLPRODS += $(ABSTRACT_ROOT)/pal/kernel
PL_INCLUDE += -I$(ABSTRACT_ROOT)/pal/kernel
endif
ifeq ($(strip $(PL_PAL_IPCOM_STACK)),true)
PLPRODS += $(ABSTRACT_ROOT)/pal/ipstack
PL_INCLUDE += -I$(ABSTRACT_ROOT)/pal/ipstack
endif

ifeq ($(strip $(PL_HAL_MODULE)),true)
PLPRODS += $(ABSTRACT_ROOT)/hal
PL_INCLUDE += -I$(ABSTRACT_ROOT)/hal
PL_DEFINE += -DPL_HAL_MODULE
endif #($(strip $(PL_HAL_MODULE)),true)

endif #ifeq ($(strip $(PL_PAL_MODULE)),true)


ifeq ($(strip $(PL_COMPONENT_MODULE)),true)

ifeq ($(strip $(PL_MODEM_MODULE)),true)
MODEM_ROOT=$(COMPONENT_ROOT)/modem
PLPRODS += $(MODEM_ROOT)
PL_INCLUDE += -I$(MODEM_ROOT)
PL_DEFINE += -DPL_MODEM_MODULE
PLCLI_DIR += $(CLI_ROOT)/modem
endif

ifeq ($(strip $(PL_UDHCP_MODULE)),true)
UDHCP_ROOT=$(COMPONENT_ROOT)/udhcp
PLPRODS += $(UDHCP_ROOT)
PL_INCLUDE += -I$(UDHCP_ROOT)

PL_DEFINE += -DPL_UDHCP_MODULE

PL_DEFINE += -DPL_DHCP_MODULE
PL_DEFINE += -DPL_DHCPC_MODULE
PL_DEFINE += -DPL_DHCPD_MODULE

PLCLI_DIR += $(CLI_ROOT)/dhcp
endif


ifeq ($(strip $(PL_LIBSSH_MODULE)),true)
LIBSSH_ROOT=$(COMPONENT_ROOT)/ssh

PLPRODS += $(LIBSSH_ROOT)
PL_INCLUDE += -I$(LIBSSH_ROOT)
PL_INCLUDE += -I$(LIBSSH_ROOT)/include

ifeq ($(PL_BUILD_TYPE),X86_64)
PL_INCLUDE += -I$(LIBSSH_ROOT)/include
#PLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(PL_BUILD_TYPE),X86_64)

ifeq ($(PL_BUILD_TYPE),MIPS)
ifneq ($(OPENEWRT_BASE),)
OPENWRT_INCLUDE := -I$(OPENEWRT_BASE)/include -I$(OPENEWRT_BASE)/usr/include
OPENWRT_LDFLAGS := -L$(OPENEWRT_BASE)/lib -L$(OPENEWRT_BASE)/usr/lib
else
PLEX_INCLUDE += -I$(EXTERNSION_ROOT)/openssl/mipsl/include
PLEX_LDFLAGS += -L$(EXTERNSION_ROOT)/openssl/mipsl/lib 
PLEX_INCLUDE += -I$(EXTERNSION_ROOT)/zlib/mipsl/zlib/include
PLEX_LDFLAGS += -L$(EXTERNSION_ROOT)/zlib/mipsl/zlib/lib
endif #($(OPENEWRT_BASE),)
#PLEX_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(PL_BUILD_TYPE),MIPS)

PL_DEFINE += -DPL_LIBSSH_MODULE
#PLEX_LDLIBS += -lutil -lssl -lcrypto -lz

endif #($(strip $(PL_LIBSSH_MODULE)),true)


ifeq ($(strip $(PL_OPENSSL_MODULE)),true)
ifneq ($(PL_BUILD_TYPE),X86_64)
PLEX_DIR += $(EXTERNSION_ROOT)/openssl/openssl-1.1.1/
export PLATFORM=linux-armv4
PLEX_INCLUDE += -I$(DSTROOTFSDIR)/include
PLEX_LDFLAGS += -L$(DSTROOTFSDIR)/lib
PLEX_LDLIBS += -lutil -lssl -lcrypto
PLEX_DIR += $(EXTERNSION_ROOT)/zlib/zlib-1.2.11/
PL_INCLUDE += -I$(DSTROOTFSDIR)/include
PLEX_LDFLAGS += -L$(DSTROOTFSDIR)/lib
PLEX_LDLIBS += -lz
else 
PLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(PL_BUILD_TYPE),X86_64)
PL_DEFINE += -DPL_OPENSSL_MODULE
endif #($(strip $(PL_OPENSSL_MODULE)),true)


ifeq ($(strip $(PL_SQLITE_MODULE)),true)
SQLITE_ROOT=$(COMPONENT_ROOT)/sqlite
PLPRODS += $(SQLITE_ROOT)
PL_INCLUDE += -I$(SQLITE_ROOT)
PL_DEFINE += -DPL_SQLITE_MODULE
PL_LDLIBS += -lsqlite
endif#($(strip $(PL_SQLITE_MODULE)),true)

ifeq ($(strip $(PL_WIFI_MODULE)),true)
WIFI_ROOT=$(COMPONENT_ROOT)/wifi
PLPRODS += $(WIFI_ROOT)
PL_INCLUDE += -I$(WIFI_ROOT)
PL_DEFINE += -DPL_WIFI_MODULE
endif#($(strip $(PL_WIFI_MODULE)),true)


ifeq ($(strip $(PL_MQTT_MODULE)),true)
MQTT_ROOT=$(COMPONENT_ROOT)/mqtt
PLPRODS += $(MQTT_ROOT)
PL_INCLUDE += -I$(MQTT_ROOT)
PL_INCLUDE += -I$(MQTT_ROOT)/mqttlib
#PL_INCLUDE += -I$(MQTT_ROOT)/mqttc
#PL_INCLUDE += -I$(MQTT_ROOT)/mqtts

PL_DEFINE += -DPL_MQTT_MODULE

export MQTT_SHARED_LIBRARIES = false
ifeq ($(strip $(MQTT_SHARED_LIBRARIES)),true)
PL_LDLIBS += -lmosquitto
endif

PLCLI_DIR += $(CLI_ROOT)/mqtt
endif #($(strip $(PL_MQTT_MODULE)),true)


ifeq ($(strip $(PL_WEBSERVER_MODULE)),true)
WEBGUI_ROOT=$(COMPONENT_ROOT)/webserver
PLPRODS += $(WEBGUI_ROOT)
PL_INCLUDE += -I$(WEBGUI_ROOT)
PL_INCLUDE += -I$(WEBGUI_ROOT)/include
PL_DEFINE += -DPL_WEBGUI_MODULE
endif

endif#($(strip $(PL_COMPONENT_MODULE)),true)


ifeq ($(strip $(PL_APPLICATION_MODULE)),true)
PLPRODS += $(APP_ROOT)

PL_INCLUDE += -I$(APP_ROOT)

PL_DEFINE += -DPL_APP_MODULE

ifeq ($(strip $(PL_APP_X5_MODULE)),true)
PLM_DEFINE += -DAPP_X5BA_MODULE
endif
ifeq ($(strip $(PL_APP_V9_MODULE)),true)
PLM_DEFINE += -DAPP_V9_MODULE
PLM_DEFINE += -DPL_VIDEO_MODULE
#PL_LDLIBS += -loal_privateProtocol
endif
PLCLI_DIR += $(CLI_ROOT)/app
endif


ifeq ($(strip $(PL_TOOLS_MODULE)),true)

ifeq ($(strip $(PL_TOOLS_PROCESS)),true)
PLPRODS += $(TOOLS_ROOT)/process
PL_INCLUDE += -I$(TOOLS_ROOT)/process
PL_DEFINE += -DDOUBLE_PROCESS
endif

ifeq ($(strip $(PL_TOOLS_QUECTEL_CM)),true)
PLPRODS += $(TOOLS_ROOT)/quectel-CM
PL_INCLUDE += -I$(TOOLS_ROOT)/quectel-CM
endif

ifeq ($(strip $(PL_TOOLS_SYSTEM)),true)
PLPRODS += $(TOOLS_ROOT)/system
PL_INCLUDE += -I$(TOOLS_ROOT)/system
endif

endif



#
# 下面两个模块保持在最后
# 
ifeq ($(strip $(PL_CLI_MODULE)),true)
PLPRODS += $(PLCLI_DIR)
endif


#
PLPRODS += $(STARTUP_ROOT)/src
PL_INCLUDE += -I$(STARTUP_ROOT)/src
PLPRODS += $(STARTUP_ROOT)/etc
