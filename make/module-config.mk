

ifeq ($(strip $(MODULE_PLATFORM)),true)
PLATFORM_ROOT=$(PLBASE)/$(PLATFORMDIR)

PLPRODS += $(PLATFORM_ROOT)/os
PLPRODS += $(PLATFORM_ROOT)/lib
PLPRODS += $(PLATFORM_ROOT)/shell
PLPRODS += $(PLATFORM_ROOT)/nsm

PLINCLUDE += -I$(PLATFORM_ROOT)/os
PLINCLUDE += -I$(PLATFORM_ROOT)/lib
PLINCLUDE += -I$(PLATFORM_ROOT)/shell
PLINCLUDE += -I$(PLATFORM_ROOT)/nsm
PLDEFINE	+= -DPL_NSM_MODULE
endif

ifeq ($(strip $(MODULE_L2PROTOCOL)),true)

L2PROTOCOL_ROOT=$(PLBASE)/$(L2PROTOCOLDIR)

PLPRODS += $(L2PROTOCOL_ROOT)/ospf
PLPRODS += $(L2PROTOCOL_ROOT)/bgp

PLINCLUDE += -I$(L2PROTOCOL_ROOT)/ospf
PLINCLUDE += -I$(L2PROTOCOL_ROOT)/bgp

#PLDEFINE	+= -DPL_NSM_MODULE
endif

ifeq ($(strip $(MODULE_SERVICE)),true)
SERVICE_ROOT=$(PLBASE)/$(SERVICEDIR)
PLPRODS += $(SERVICE_ROOT)/sntp
PLPRODS += $(SERVICE_ROOT)/syslog

PLINCLUDE += -I$(SERVICE_ROOT)/sntp
PLINCLUDE += -I$(SERVICE_ROOT)/syslog


PLDEFINE += -DPL_SERVICE_MODULE -DSYSLOG_CLIENT
endif

ifeq ($(strip $(MODULE_SYSTOOLS)),true)
SYSTOOLS_ROOT=$(SERVICE_ROOT)/$(SYSTOOLSDIR)
PLPRODS += $(SERVICE_ROOT)/$(SYSTOOLSDIR)
PLINCLUDE += -I$(SERVICE_ROOT)/$(SYSTOOLSDIR)
PLDEFINE += -DPL_SYSTOOLS_MODULE

endif

ifeq ($(strip $(MODULE_STARTUP)),true)
STARTUP_ROOT=$(PLBASE)/$(STARTUPDIR)
PLPRODS += $(STARTUP_ROOT)/src
PLINCLUDE += -I$(STARTUP_ROOT)/src
endif


ifeq ($(strip $(MODULE_PRODUCT)),true)
PRODUCT_ROOT=$(PLBASE)/$(PRODUCTDIR)
PLPRODS += $(PRODUCT_ROOT)/bsp
#PLPRODS += $(PRODUCT_ROOT)/sdk

PLINCLUDE += -I$(PRODUCT_ROOT)/bsp
#PLINCLUDE += -I$(PRODUCT_ROOT)/sdk

PLDEFINE += -DPL_BSP_MODULE
endif

ifeq ($(strip $(MODULE_OSPF)),true)
OSPF_ROOT=$(PLBASE)/l3protocol/$(OSPFDIR)
PLPRODS += $(OSPF_ROOT)
PLINCLUDE += -I$(OSPF_ROOT)
PLDEFINE += -DPL_OSPF_MODULE
endif


ifeq ($(strip $(MODULE_ABSTRACT)),true)
ABSTRACT_ROOT=$(PLBASE)/$(ABSTRACTDIR)

PLPRODS += $(ABSTRACT_ROOT)/pal/kernel
PLPRODS += $(ABSTRACT_ROOT)/hal
PLDEFINE += -DPL_HAL_MODULE


PLINCLUDE += -I$(ABSTRACT_ROOT)/pal/kernel
PLINCLUDE += -I$(ABSTRACT_ROOT)/hal
ifeq ($(strip $(USE_IPROUTE2)),true)
PLPRODS += $(ABSTRACT_ROOT)/pal/kernel/iproute
PLDEFINE += -DPL_IPROUTE2_MODULE
endif
endif



ifeq ($(strip $(MODULE_COMPONENT)),true)
COMPONENT_ROOT=$(PLBASE)/$(COMPONENTDIR)
#PLPRODS += $(COMPONENT_ROOT)
#PLINCLUDE += -I$(COMPONENT_ROOT)
endif


ifeq ($(strip $(MODULE_MODEM)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
MODEM_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(MODEMDIR)
PLPRODS += $(MODEM_ROOT)
PLINCLUDE += -I$(MODEM_ROOT)
PLDEFINE += -DPL_MODEM_MODULE
endif
endif

ifeq ($(strip $(MODULE_DHCP)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)

DHCPCD_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(DHCPCDDIR)
PLPRODS += $(DHCPCD_ROOT)
#PLINCLUDE += -I$(DHCPCD_ROOT)

DHCPD_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(DHCPDDIR)
PLPRODS += $(DHCPD_ROOT)
PLINCLUDE += -I$(DHCPD_ROOT)

PLDEFINE += -DPL_DHCPC_MODULE
PLDEFINE += -DPL_DHCPD_MODULE
PLDEFINE += -DPL_DHCP_MODULE 

endif
endif

ifeq ($(strip $(MODULE_SSH)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
LIBSSH_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(LIBSSHDIR)
PLPRODS += $(LIBSSH_ROOT)
PLINCLUDE += -I$(LIBSSH_ROOT)
PLINCLUDE += -I$(LIBSSH_ROOT)/include
ifeq ($(BUILD_TYPE),X86)
PLINCLUDE += -I$(LIBSSH_ROOT)/include
PLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif
ifeq ($(BUILD_TYPE),MIPS)
PLINCLUDE += -I$(PLBASE)/externsions/openssl/mipsl/include
PL_CFLAGS += -L$(PLBASE)/externsions/openssl/mipsl/lib 
PL_CFLAGS += -I$(PLBASE)/externsions/zlib/mipsl/zlib/include
PL_CFLAGS += -L$(PLBASE)/externsions/zlib/mipsl/zlib/lib
PLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif
PLDEFINE += -DPL_SSH_MODULE
endif
endif


ifeq ($(strip $(MODULE_SQLITE)),true)
#ifeq ($(strip $(MODULE_COMPONENT)),true)
SQLITE_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(SQLITEDIR)
PLPRODS += $(SQLITE_ROOT)
PLINCLUDE += -I$(SQLITE_ROOT)
PLDEFINE += -DPL_SQLITE_MODULE
#endif
endif

ifeq ($(strip $(MODULE_WIFI)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
WIFI_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(WIFIDIR)
PLPRODS += $(WIFI_ROOT)
PLINCLUDE += -I$(WIFI_ROOT)

PLDEFINE += -DPL_WIFI_MODULE

endif
endif


ifeq ($(strip $(MODULE_TOOLS)),true)
TOOLS_ROOT=$(PLBASE)/$(TOOLSDIR)
PLPRODS += $(TOOLS_ROOT)/system
PLINCLUDE += -I$(TOOLS_ROOT)/system

#PLPRODS += $(TOOLS_ROOT)/quectel-CM
#PLINCLUDE += -I$(TOOLS_ROOT)/quectel-CM

PLPRODS += $(TOOLS_ROOT)/process
PLINCLUDE += -I$(TOOLS_ROOT)/process

PLDEFINE += -DDOUBLE_PROCESS

endif


ifeq ($(strip $(MODULE_UCI)),true)
ifeq ($(BUILD_OPENWRT),true)
PLINCLUDE += -I$(PLBASE)/externsions/uci/mips/include
PL_CFLAGS += -L$(PLBASE)/externsions/uci/mips/lib 
PL_LDLIBS += -luci			
PLDEFINE += -DPL_OPENWRT_UCI
endif				
endif


ifeq ($(strip $(MODULE_VOIP)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
VOIP_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(VOIPDIR)
PLPRODS += $(VOIP_ROOT)
PLINCLUDE += -I$(VOIP_ROOT)

PLDEFINE += -DPL_VOIP_MODULE
EXTRA_DEFINE += -DVOIP_CARDS_DEBUG
ifeq ($(BUILD_TYPE),MIPS)

PLINCLUDE += -I$(PLBASE)/externsions/mediastream/include
PL_CFLAGS += -L$(PLBASE)/externsions/mediastream/lib 
PL_LDLIBS += -lortp -lbctoolbox -lbcunit -lmbedtls -lmbedx509 -lmbedcrypto -lmediastreamer_base \
				-lmediastreamer_voip -lstdc++ -lspeex -lspeexdsp

PLDEFINE += -DPL_VOIP_MEDIASTREAM
#PLINCLUDE += -I$(PLBASE)/externsions/ortp/mips/include
#PL_CFLAGS += -L$(PLBASE)/externsions/ortp/mips/lib 
#PL_LDLIBS += -lortp

#PLINCLUDE += -I$(PLBASE)/externsions/opencore-amr/mips/include
#PL_CFLAGS += -L$(PLBASE)/externsions/opencore-amr/mips/lib 
#PL_LDLIBS += -lopencore-amrwb -lopencore-amrnb

PLOS_LDLIBS += -lasound 
else
#
# ortp ,opencore-amr shuld be install on X86 system
#
#PLINCLUDE += -I$(PLBASE)/externsions/ortp/x86/include
#PL_CFLAGS += -L$(PLBASE)/externsions/ortp/x86/lib 
#PL_LDLIBS += -lortp

#PLINCLUDE += -I$(PLBASE)/externsions/opencore-amr/x86/include
#PL_CFLAGS += -L$(PLBASE)/externsions/opencore-amr/x86/lib 
#PL_LDLIBS += -lopencore-amrwb -lopencore-amrnb

#PL_LDLIBS += -lasound -lportaudio
endif

endif
endif


ifeq ($(strip $(MODULE_OSIP)),true)
OSIP_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(OSIPDIR)
PLPRODS += $(OSIP_ROOT)/libosip/src/osip2
PLPRODS += $(OSIP_ROOT)/libosip/src/osipparser2
PLPRODS += $(OSIP_ROOT)/libexosip/src

PLINCLUDE += -I$(OSIP_ROOT)/libosip/include
PLINCLUDE += -I$(OSIP_ROOT)/libexosip/include

PLINCLUDE += -I$(OSIP_ROOT)
PLDEFINE += -DPL_OSIP_MODULE -DOSIP_MONOTHREAD -D__linux
PLOS_LDLIBS += -lresolv
endif

ifeq ($(strip $(MODULE_EXSIP)),true)
PLDEFINE += -DPL_EXSIP_MODULE
endif

ifeq ($(strip $(MODULE_PJSIP)),true)
ifeq ($(strip $(MODULE_COMPONENT)),true)
PJSIP_ROOT=$(PLBASE)/$(COMPONENTDIR)/$(PJSIPDIR)

#PLPRODS += $(PLBASE)/externsions/pjproject-2.8
#PLINCLUDE += -I$(PLBASE)/externsions/pjproject-2.8


PLPRODS += $(PJSIP_ROOT)
PLINCLUDE += -I$(PJSIP_ROOT)

PLDEFINE += -DPL_PJSIP_MODULE

PLINCLUDE += -I$(PLBASE)/externsions/pjproject-2.8/_install/include
PL_CFLAGS += -L$(PLBASE)/externsions/pjproject-2.8/_install/lib 
PL_LDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
			 -lpjsua -lsrtp -lgsmcodec -lspeex -lilbccodec -lg7221codec -lyuv
endif
endif

ifeq ($(strip $(MODULE_APP)),true)
APP_ROOT=$(PLBASE)/$(APPDIR)
PLPRODS += $(APP_ROOT)

PLINCLUDE += -I$(APP_ROOT)

PLDEFINE += -DPL_APP_MODULE
EXTRA_DEFINE += -DAPP_X5BA_MODULE -DX5_B_A_DEBUG

endif


ifeq ($(strip $(MODULE_CLI)),true)
CLI_ROOT=$(PLBASE)/$(CLIDIR)
PLPRODS += $(CLI_ROOT)/nsm
PLPRODS += $(CLI_ROOT)/service
PLPRODS += $(CLI_ROOT)/system

PLINCLUDE += -I$(CLI_ROOT)/nsm
PLINCLUDE += -I$(CLI_ROOT)/service
PLINCLUDE += -I$(CLI_ROOT)/system

ifeq ($(strip $(MODULE_MODEM)),true)
PLPRODS += $(CLI_ROOT)/modem
PLINCLUDE += -I$(CLI_ROOT)/modem
endif
ifeq ($(strip $(MODULE_DHCP)),true)
PLPRODS += $(CLI_ROOT)/dhcp
PLINCLUDE += -I$(CLI_ROOT)/dhcp
endif


ifeq ($(strip $(MODULE_VOIP)),true)
PLPRODS += $(CLI_ROOT)/voip
PLINCLUDE += -I$(CLI_ROOT)/voip
endif

ifeq ($(strip $(MODULE_APP)),true)
PLPRODS += $(CLI_ROOT)/app
PLINCLUDE += -I$(CLI_ROOT)/app
endif
endif
