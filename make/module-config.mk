

ifeq ($(strip $(MODULE_PLATFORM)),true)
PLATFORM_ROOT=$(PLBASE)/$(PLATFORMDIR)

PLPRODS += $(PLATFORM_ROOT)/os
PLPRODS += $(PLATFORM_ROOT)/lib
PLPRODS += $(PLATFORM_ROOT)/shell
PLPRODS += $(PLATFORM_ROOT)/nsm

PL_INCLUDE += -I$(PLATFORM_ROOT)/os
PL_INCLUDE += -I$(PLATFORM_ROOT)/lib
PL_INCLUDE += -I$(PLATFORM_ROOT)/shell
PL_INCLUDE += -I$(PLATFORM_ROOT)/nsm
PL_DEFINE	+= -DPL_NSM_MODULE
PL_DEFINE	+= -DVTY_STDIO_MODULE
PL_DEFINE	+= -DVTY_CONSOLE_MODULE
endif #($(strip $(MODULE_PLATFORM)),true)

ifeq ($(strip $(MODULE_L2PROTOCOL)),true)

L2PROTOCOL_ROOT=$(PLBASE)/$(L2PROTOCOLDIR)

PLPRODS += $(L2PROTOCOL_ROOT)/ospf
PLPRODS += $(L2PROTOCOL_ROOT)/bgp

PL_INCLUDE += -I$(L2PROTOCOL_ROOT)/ospf
PL_INCLUDE += -I$(L2PROTOCOL_ROOT)/bgp
#PL_DEFINE	+= -DPL_NSM_MODULE
endif #($(strip $(MODULE_L2PROTOCOL)),true)

ifeq ($(strip $(MODULE_SERVICE)),true)
SERVICE_ROOT=$(PLBASE)/$(SERVICEDIR)
PLPRODS += $(SERVICE_ROOT)/sntp
PLPRODS += $(SERVICE_ROOT)/syslog

PL_INCLUDE += -I$(SERVICE_ROOT)/sntp
PL_INCLUDE += -I$(SERVICE_ROOT)/syslog

PL_DEFINE += -DPL_SERVICE_MODULE -DSYSLOG_CLIENT
endif #($(strip $(MODULE_SERVICE)),true)

ifeq ($(strip $(MODULE_SYSTOOLS)),true)
SYSTOOLS_ROOT=$(SERVICE_ROOT)/$(SYSTOOLSDIR)
PLPRODS += $(SERVICE_ROOT)/$(SYSTOOLSDIR)
PL_INCLUDE += -I$(SERVICE_ROOT)/$(SYSTOOLSDIR)
PL_DEFINE += -DPL_SYSTOOLS_MODULE
endif #($(strip $(MODULE_SYSTOOLS)),true)

ifeq ($(strip $(MODULE_STARTUP)),true)
STARTUP_ROOT=$(PLBASE)/$(STARTUPDIR)
PLPRODS += $(STARTUP_ROOT)/src
PL_INCLUDE += -I$(STARTUP_ROOT)/src
PLPRODS += $(STARTUP_ROOT)/etc
endif #($(strip $(MODULE_STARTUP)),true)


ifeq ($(strip $(MODULE_PRODUCT)),true)
PRODUCT_ROOT=$(PLBASE)/$(PRODUCTDIR)
PLPRODS += $(PRODUCT_ROOT)/bsp
PL_INCLUDE += -I$(PRODUCT_ROOT)/bsp
PL_DEFINE += -DPL_BSP_MODULE

ifeq ($(strip $(MODULE_BCM53125)),true)
PLPRODS += $(PRODUCT_ROOT)/sdk
PL_INCLUDE += -I$(PRODUCT_ROOT)/sdk
PL_DEFINE += -DPL_SDK_MODULE
endif #($(strip $(MODULE_BCM53125)),true)
endif #($(strip $(MODULE_PRODUCT)),true)

ifeq ($(strip $(MODULE_OSPF)),true)
OSPF_ROOT=$(PLBASE)/l3protocol/$(OSPFDIR)
PLPRODS += $(OSPF_ROOT)
PL_INCLUDE += -I$(OSPF_ROOT)
PL_DEFINE += -DPL_OSPF_MODULE
endif


ifeq ($(strip $(MODULE_ABSTRACT)),true)
ABSTRACT_ROOT=$(PLBASE)/$(ABSTRACTDIR)

PLPRODS += $(ABSTRACT_ROOT)/pal/kernel
PL_INCLUDE += -I$(ABSTRACT_ROOT)/pal/kernel
PL_DEFINE += -DPL_PAL_MODULE
ifeq ($(strip $(USE_IPROUTE2)),true)
PLPRODS += $(ABSTRACT_ROOT)/pal/kernel/iproute
PL_DEFINE += -DPL_IPROUTE2_MODULE
endif #($(strip $(USE_IPROUTE2)),true)

ifeq ($(strip $(MODULE_BCM53125)),true)
PLPRODS += $(ABSTRACT_ROOT)/hal
PL_INCLUDE += -I$(ABSTRACT_ROOT)/hal
PL_DEFINE += -DPL_HAL_MODULE
endif #($(strip $(MODULE_BCM53125)),true)
endif #ifeq ($(strip $(MODULE_ABSTRACT)),true)



ifeq ($(strip $(MODULE_COMPONENT)),true)
COMPONENT_ROOT=$(PLBASE)/$(COMPONENTDIR)
#PLPRODS += $(COMPONENT_ROOT)
#PL_INCLUDE += -I$(COMPONENT_ROOT)
endif


ifeq ($(strip $(MODULE_MODEM)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
MODEM_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(MODEMDIR)
PLPRODS += $(MODEM_ROOT)
PL_INCLUDE += -I$(MODEM_ROOT)
PL_DEFINE += -DPL_MODEM_MODULE
endif
endif

ifeq ($(strip $(MODULE_DHCP)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)

DHCPCD_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(DHCPCDDIR)
PLPRODS += $(DHCPCD_ROOT)
#PL_INCLUDE += -I$(DHCPCD_ROOT)

DHCPD_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(DHCPDDIR)
PLPRODS += $(DHCPD_ROOT)
PL_INCLUDE += -I$(DHCPD_ROOT)

PL_DEFINE += -DPL_DHCP_MODULE 
PL_DEFINE += -DPL_DHCPC_MODULE
PL_DEFINE += -DPL_DHCPD_MODULE

endif
endif

ifeq ($(strip $(MODULE_UDHCP)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
UDHCP_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(UDHCPDIR)
PLPRODS += $(UDHCP_ROOT)
PL_INCLUDE += -I$(UDHCP_ROOT)

PL_DEFINE += -DPL_UDHCP_MODULE 

PL_DEFINE += -DPL_DHCP_MODULE
PL_DEFINE += -DPL_DHCPC_MODULE
PL_DEFINE += -DPL_DHCPD_MODULE
endif
endif

ifeq ($(strip $(MODULE_SSH)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
LIBSSH_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(LIBSSHDIR)
PLPRODS += $(LIBSSH_ROOT)
PL_INCLUDE += -I$(LIBSSH_ROOT)
PL_INCLUDE += -I$(LIBSSH_ROOT)/include
ifeq ($(BUILD_TYPE),X86)
PL_INCLUDE += -I$(LIBSSH_ROOT)/include
PLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(BUILD_TYPE),X86)
ifeq ($(BUILD_TYPE),MIPS)
ifneq ($(OPENEWRT_BASE),)
OPENWRT_INCLUDE := -I$(OPENEWRT_BASE)/include -I$(OPENEWRT_BASE)/usr/include
OPENWRT_LDFLAGS := -L$(OPENEWRT_BASE)/lib -L$(OPENEWRT_BASE)/usr/lib
else #($(OPENEWRT_BASE),)
PLEX_INCLUDE += -I$(PLBASE)/externsions/openssl/mipsl/include
PLEX_LDFLAGS += -L$(PLBASE)/externsions/openssl/mipsl/lib 
PLEX_INCLUDE += -I$(PLBASE)/externsions/zlib/mipsl/zlib/include
PLEX_LDFLAGS += -L$(PLBASE)/externsions/zlib/mipsl/zlib/lib
endif #($(OPENEWRT_BASE),)
PLEX_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(BUILD_TYPE),MIPS)
PL_DEFINE += -DPL_SSH_MODULE
endif #($(strip $(MODULE_COMPONENT)),true)
endif #($(strip $(MODULE_SSH)),true)


ifeq ($(strip $(MODULE_SQLITE)),true)
#ifeq ($(strip $(MODULE_COMPONENT)),true)
SQLITE_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(SQLITEDIR)
PLPRODS += $(SQLITE_ROOT)
PL_INCLUDE += -I$(SQLITE_ROOT)
PL_DEFINE += -DPL_SQLITE_MODULE
#endif
endif

ifeq ($(strip $(MODULE_WIFI)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
WIFI_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(WIFIDIR)
PLPRODS += $(WIFI_ROOT)
PL_INCLUDE += -I$(WIFI_ROOT)

PL_DEFINE += -DPL_WIFI_MODULE

endif
endif

ifeq ($(strip $(MODULE_WEB)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
WEBGUI_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(WEBDIR)
PLPRODS += $(WEBGUI_ROOT)
PL_INCLUDE += -I$(WEBGUI_ROOT)
PL_INCLUDE += -I$(WEBGUI_ROOT)/include
PL_DEFINE += -DPL_WEBGUI_MODULE
endif
endif

ifeq ($(strip $(MODULE_TOOLS)),true)
TOOLS_ROOT=$(PLBASE)/$(TOOLSDIR)
PLPRODS += $(TOOLS_ROOT)/system
PL_INCLUDE += -I$(TOOLS_ROOT)/system

#PLPRODS += $(TOOLS_ROOT)/quectel-CM
#PL_INCLUDE += -I$(TOOLS_ROOT)/quectel-CM

PLPRODS += $(TOOLS_ROOT)/process
PL_INCLUDE += -I$(TOOLS_ROOT)/process

PL_DEFINE += -DDOUBLE_PROCESS

endif


ifeq ($(strip $(MODULE_UCI)),true)
ifneq ($(OPENEWRT_BASE),)
OPENWRT_INCLUDE := -I$(OPENEWRT_BASE)/include -I$(OPENEWRT_BASE)/usr/include
OPENWRT_LDFLAGS := -L$(OPENEWRT_BASE)/lib -L$(OPENEWRT_BASE)/usr/lib
PLEX_LDLIBS += -luci
PL_DEFINE += -DPL_OPENWRT_UCI -DPL_OPENWRT_UCI_LIB	
else
LIBUCI_ROOT=$(PLATFORM_ROOT)/os/libuci
PLPRODS += $(LIBUCI_ROOT)
PL_INCLUDE += -I$(LIBUCI_ROOT)
PL_DEFINE += -DPL_OPENWRT_UCI -DPL_OPENWRT_UCI_LIB
endif			
#PL_DEFINE += -DPL_OPENWRT_UCI -DPL_OPENWRT_UCI_LIB	
endif



ifeq ($(strip $(MODULE_OSIP)),true)
OSIP_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(OSIPDIR)
PLPRODS += $(OSIP_ROOT)/libosip/src/osip2
PLPRODS += $(OSIP_ROOT)/libosip/src/osipparser2
PLPRODS += $(OSIP_ROOT)/libexosip/src
PLPRODS += $(OSIP_ROOT)/voip

PL_INCLUDE += -I$(OSIP_ROOT)/libosip/include
PL_INCLUDE += -I$(OSIP_ROOT)/libexosip/include
PL_INCLUDE += -I$(OSIP_ROOT)/voip

#PL_INCLUDE += -I$(OSIP_ROOT)

PL_DEFINE += -DPL_OSIP_MODULE -DOSIP_MONOTHREAD -D__linux
PL_DEFINE += -DPL_VOIP_MODULE  -DPL_VOIP_MEDIASTREAM
EXTRA_DEFINE += -DVOIP_CARDS_DEBUG

PLOS_LDLIBS += -lresolv -lasound -lutil -lssl -lcrypto -lz

endif #($(strip $(MODULE_OSIP)),true)



ifeq ($(strip $(MODULE_PJSIP)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
PJSIP_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(PJSIPDIR)
#PLPRODS += $(PLBASE)/externsions/pjproject-2.8
#PL_INCLUDE += -I$(PLBASE)/externsions/pjproject-2.8
PLPRODS += $(PJSIP_ROOT)
PL_INCLUDE += -I$(PJSIP_ROOT)
PL_DEFINE += -DPL_PJSIP_MODULE
PL_DEFINE += -DPL_VOIP_MODULE
PL_INCLUDE += -I$(PLBASE)/externsions/pjproject-2.8/_install/include

export PJSHARE_ENABLE = true
export PJMEDIA_ENABLE = true
export PJMEDIA_AUDIODEV_ENABLE = true
export PJMEDIA_CODEC_ENABLE = true
export PJMEDIA_VIDEODEV_ENABLE = false
export PJMEDIA_NATH_ENABLE = true
export PJMEDIA_SIMPLE_ENABLE = true
export PJMEDIA_RESAMPLE_ENABLE = false
export PJMEDIA_UA_ENABLE = true

export PJMEDIA_SRTP_ENABLE = true

export PJMEDIA_YUV_ENABLE = false
export PJMEDIA_GSM_ENABLE = true
export PJMEDIA_SPEEX_ENABLE = true
export PJMEDIA_ILBC_ENABLE = true
export PJMEDIA_G722_ENABLE = true

PL_LDLIBS += -lpj -lpjlib-util -lpjsip 
			 
PL_LDLIBS += -luuid -lasound
	
ifeq ($(PJSHARE_ENABLE),true)			 
ifeq ($(PJMEDIA_ENABLE),true)
PL_LDLIBS += -lpjmedia
endif
ifeq ($(PJMEDIA_AUDIODEV_ENABLE),true)
PL_LDLIBS += -lpjmedia-audiodev
endif
ifeq ($(PJMEDIA_CODEC_ENABLE),true)
PL_LDLIBS += -lpjmedia-codec
endif
ifeq ($(PJMEDIA_VIDEODEV_ENABLE),true)
PL_LDLIBS += -lpjmedia-videodev
endif
ifeq ($(PJMEDIA_NATH_ENABLE),true)
PL_LDLIBS += -lpjnath
endif
ifeq ($(PJMEDIA_SIMPLE_ENABLE),true)
PL_LDLIBS += -lpjsip-simple
endif
ifeq ($(PJMEDIA_RESAMPLE_ENABLE),true)
PL_LDLIBS += -lpjsip-resample
endif
ifeq ($(PJMEDIA_UA_ENABLE),true)
PL_LDLIBS += -lpjsua -lpjsip-ua
endif
ifeq ($(PJMEDIA_SRTP_ENABLE),true)
PL_LDLIBS += -lsrtp
endif
ifeq ($(PJMEDIA_YUV_ENABLE),true)
PL_LDLIBS += -lyuv
endif
ifeq ($(PJMEDIA_GSM_ENABLE),true)
PL_LDLIBS += -lgsmcodec
endif
ifeq ($(PJMEDIA_SPEEX_ENABLE),true)
PL_LDLIBS += -lspeex
endif
ifeq ($(PJMEDIA_ILBC_ENABLE),true)
PL_LDLIBS += -lilbccodec
endif
ifeq ($(PJMEDIA_G722_ENABLE),true)
PL_LDLIBS += -lg7221codec
endif
endif
#PL_LDFLAGS += -L$(PLBASE)/externsions/pjproject-2.8/_install/lib 
#PL_LDFLAGS += -L$(PLBASE)/externsions/pjproject-2.8/third_party/lib 
#PL_LDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
			 -lpjsua -lsrtp -lgsmcodec -lspeex -lilbccodec -lg7221codec 
			 
#PL_LDLIBS += -luuid -lasound
endif #($(strip $(MODULE_COMPONENT)),true)
endif #($(strip $(MODULE_PJSIP)),true)


ifeq ($(strip $(MODULE_APP)),true)
APP_ROOT=$(PLBASE)/$(APPDIR)
PLPRODS += $(APP_ROOT)

PL_INCLUDE += -I$(APP_ROOT)

PL_DEFINE += -DPL_APP_MODULE
PLM_DEFINE += -DAPP_X5BA_MODULE 
PL_DEBUG = -DX5_B_A_DEBUG

endif


ifeq ($(strip $(MODULE_CLI)),true)
CLI_ROOT=$(PLBASE)/$(CLIDIR)
PLPRODS += $(CLI_ROOT)/nsm
PLPRODS += $(CLI_ROOT)/service
PLPRODS += $(CLI_ROOT)/system

PL_INCLUDE += -I$(CLI_ROOT)/nsm
PL_INCLUDE += -I$(CLI_ROOT)/service
PL_INCLUDE += -I$(CLI_ROOT)/system

ifeq ($(strip $(MODULE_MODEM)),true)
PLPRODS += $(CLI_ROOT)/modem
PL_INCLUDE += -I$(CLI_ROOT)/modem
endif
ifeq ($(strip $(MODULE_DHCP)),true)
PLPRODS += $(CLI_ROOT)/dhcp
PL_INCLUDE += -I$(CLI_ROOT)/dhcp
endif

ifeq ($(strip $(MODULE_UDHCP)),true)
PLPRODS += $(CLI_ROOT)/dhcp
PL_INCLUDE += -I$(CLI_ROOT)/dhcp
endif

ifeq ($(strip $(MODULE_PJSIP)),true)
PLPRODS += $(CLI_ROOT)/voip
PL_INCLUDE += -I$(CLI_ROOT)/voip
endif

ifeq ($(strip $(MODULE_OSIP)),true)
ifneq ($(strip $(MODULE_PJSIP)),true)
PLPRODS += $(CLI_ROOT)/voip
PL_INCLUDE += -I$(CLI_ROOT)/voip
endif
endif

ifeq ($(strip $(MODULE_APP)),true)
PLPRODS += $(CLI_ROOT)/app
PL_INCLUDE += -I$(CLI_ROOT)/app
endif
endif
