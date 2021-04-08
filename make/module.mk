# the following is a list of the Platform configurations that have booleans associated with them
ifeq ($(MENUCONFIG_PL_BUILD),true)
include $(MENUCONFIG_PL_MODULE)
else

#
# Global Settings
#
_MODULELIST_GLOBAL = \
	PL_RUNNING_BASE_PATH="/tmp/app" \
	PL_REAL_SYSCONFIG_PATH="/dm/zhurish/work/busybox-1.28.4/SWPlatform/debug/etc" \
	PL_INSTALL_PATH="./_install" 

PL_RUNNING_BASE_PATH="/tmp/app"
PL_REAL_SYSCONFIG_PATH="/dm/zhurish/work/busybox-1.28.4/SWPlatform/debug/etc"

#
# Platform Arch Config
#
_MODULELIST_PLATFORM = \
	PL_SYSTEM_MODULE.true	 \
	PL_SYSTEM_LINUX.true	 \
	PL_SYSTEM_OPENWRT.false	 \
	PL_ARCH_MODULE.true	 \
	PL_ARCH_ARM.false	 \
	PL_ARCH_ARM64.false	 \
	PL_ARCH_X86.false	 \
	PL_ARCH_X86_64.true	

#
# Toolchain Config
#
_MODULELIST_TOOLCHAIN = \
	PL_HOST_TOOLCHAIN_MODULE.true	 \
	PL_EXTERNAL_TOOLCHAIN_MODULE.false	 \
	PL_EXTERNAL_TOOLCHAIN_GLIBC.false	 \
	PL_EXTERNAL_TOOLCHAIN_UCLIBC.false	 

#
# Product Module Config
#
_MODULELIST_PRODUCT = \
	PL_PRODUCT_MODULE.true	 \
	PL_PRODUCT_L2SWITCH.true	 \
	PL_PRODUCT_L3ROUTE.true	 \
	PL_VPN_MODULE.true	 \
	PL_IPCOM_STACK_MODULE.false	 \
	PL_KERNEL_STACK_MODULE.true	 \
	PL_PRODUCT_BSP_MODULE.true	 \
	PL_PRODUCT_SDK_MODULE.true	

#
# Platform configuration
#

#
# Platform OS Abstract Layer Module
#
_MODULELIST_OS = \
	PL_OS_MODULE.true	 \
	PL_OS_JSON.true	 \
	PL_OS_UCI.true	 \
	PL_OS_TLV.true	 \
	PL_OS_RNG.true	 \
	PL_OS_QUEUE.true	 \
	PL_OS_NVRAM.true	 \
	PL_OS_AVL.true	 \
	PL_OS_TTYCOM.true	 \
	PL_OS_XYZ_MODEM.true	 \
	PL_OS_CPPJSON.true	

#
# Platform Misc Lib Layer Module
#
_MODULELIST_MISC = \
	PL_MISC_MODULE.true	 \
	PL_MISC_BITMAP.true	\
	PL_MISC_IFHOOK.false	 \
	PL_MISC_SHOW_HOOK.false	 

#
# Platform NSM Layer Module
#
_MODULELIST_NSM = \
	PL_NSM_MODULE.true	 \
	PL_NSM_8021X.true	 \
	PL_NSM_ARP.true	 \
	PL_NSM_BRIDGE.true	 \
	PL_NSM_DHCP.true	 \
	PL_NSM_DNS.true	 \
	PL_NSM_DOS.true	 \
	PL_NSM_FIREWALLD.true	 \
	PL_NSM_MAC.true	 \
	PL_NSM_MIRROR.true	 \
	PL_NSM_PPP.true	 \
	PL_NSM_QOS.true	 \
	PL_NSM_SERIAL.true	 \
	PL_NSM_TRUNK.true	 \
	PL_NSM_TUNNEL.true	 \
	PL_NSM_VLAN.true	 \
	PL_NSM_VETH.true	 \
	PL_NSM_SECURITY.true	 

#
# Platform Shell Module
#
_MODULELIST_SHELL = \
	PL_SHELL_MODULE.true	

#
# Component Module Config
#
_MODULELIST_COMPONENT = \
	PL_COMPONENT_MODULE.true	 \
	PL_MODEM_MODULE.true	 \
	PL_MQTT_MODULE.true	 \
	PL_MQTT_SSL.false	 \
	PL_MQTT_SRV.true	 \
	PL_PJSIP_MODULE.true	 \
	PL_PJSIP_PJSUA2.true	 \
	PL_SQLITE_MODULE.true	 \
	PL_SQLITE_EXE_MODULE.false	 \
	PL_LIBSSH_MODULE.true	 \
	PL_LIBSSH_NACL.false	 \
	PL_LIBSSH_ZLIB.true	 \
	PL_LIBSSH_SFTP.true	 \
	PL_LIBSSH_MBEDTLS.false	 \
	PL_LIBSSH_GCRYPT.false	 \
	PL_LIBSSH_CRYPTO.true	 \
	PL_LIBSSH_OPENSSL_ED25519.false	 \
	PL_LIBSSH_BLOWFISH.false	 \
	PL_LIBSSH_SERVER.true	 \
	PL_LIBSSH_PTHREAD.true	 \
	PL_LIBSSH_GSSAPI.false	 \
	PL_LIBSSH_GEX.false	 	\
	PL_LIBSSH_PCAP.false	 \
	PL_UDHCP_MODULE.true	 \
	PL_UDHCPS_MODULE.true	 \
	PL_UDHCPC_MODULE.true	 \
	PL_UDHCPR_MODULE.true	 \
	PL_WEBSERVER_MODULE.true	 \
	PL_WIFI_MODULE.true	 

#
# Externsions Module Config
#
_MODULELIST_EXTERNSIONS = \
	PL_EXTERNSIONS_MODULE.true	 \
	PL_PJPROJECT_MODULE.true	 \
	PL_PJPROJECT_OPTIONS.true	 \
	PL_PJ_RESAMPLE_ENABLE.true	 \
	PL_PJ_SIMPLE_ENABLE.true	 \
	PL_PJ_SRTP_ENABLE.true	 \
	PL_PJ_VIDEO_ENABLE.true	 \
	PL_PJ_VIDEO_YUV_ENABLE.true	 \
	PL_PJ_VIDEO_H264_ENABLE.true	 \
	PL_PJ_CODEC_GSM_ENABLE.true	 \
	PL_PJ_CODEC_SPEEX_ENABLE.true	 \
	PL_PJ_CODEC_ILBC_ENABLE.true	 \
	PL_PJ_CODEC_G722_ENABLE.true	 \
	PL_PJ_CODEC_WEBRTC_ENABLE.true	 \
	PL_PJ_AUDIO_ALSA.true	 \
	PL_PJ_AUDIO_PORTAUDIO.false	 \
	PL_PJ_CODEC_VPX_ENABLE.false	 \
	PL_PJ_FFMPEG_ENABLE.false	 \
	PL_PJ_SDL_ENABLE.false	 \
	PL_OPENSSL_MODULE.false

PL_PJ_CODEC_H264_LIB_PATH=/usr/local
#
# Abstract Module Config
#
_MODULELIST_ABSTRACT = \
	PL_ABSTRACT_MODULE.true	 \
	PL_HAL_MODULE.true	 \
	PL_PAL_MODULE.true	 \
	PL_PAL_IPCOM_STACK.false	 \
	PL_PAL_KERNEL_STACK.true	 

#
# Service Module Config
#
_MODULELIST_SERVICE = \
	PL_SERVICE_MODULE.true	 \
	PL_SERVICE_SNTPC.true	 \
	PL_SERVICE_SNTPS.true	 \
	PL_SERVICE_SYSLOG.true	 \
	PL_SERVICE_FTPC.true	 \
	PL_SERVICE_FTPD.true	 \
	PL_SERVICE_TFTPC.true	 \
	PL_SERVICE_TFTPD.true	 \
	PL_SERVICE_PING.true	 \
	PL_SERVICE_TELNET.true	 \
	PL_SERVICE_TELENTD.false	 \
	PL_SERVICE_TRACEROUTE.true	 \
	PL_SERVICE_UBUS_SYNC.true	 

#
# CLI Module Config
#
_MODULELIST_CLI = \
	PL_CLI_MODULE.true	 

#
# Multimedia Module Config
#
_MODULELIST_MULTIMEDIA = \
	PL_MULTIMEDIA_MODULE.true \
	PL_LIVE555_MODULE.true \
	PL_LIBX264_MODULE.true \
	PL_OPENH264_MODULE.true \
	PL_LIBVPX_MODULE.true \
	PL_FFMPEG_MODULE.true
#
# Application Module Config
#
_MODULELIST_APPLICATION = \
	PL_APPLICATION_MODULE.true	 \
	PL_APP_X5_MODULE.true	 \
	PL_APP_V9_MODULE.false	 

#
# Tools Module Config
#
_MODULELIST_TOOLS = \
	PL_TOOLS_MODULE.true	 \
	PL_TOOLS_PROCESS.true	 \
	PL_TOOLS_QUECTEL_CM.true	 \
	PL_TOOLS_SYSTEM.true	 

_MODULELIST = $(_MODULELIST_PLATFORM) $(_MODULELIST_PRODUCT) $(_MODULELIST_OS) $(_MODULELIST_MISC) \
				$(_MODULELIST_NSM) $(_MODULELIST_SHELL) $(_MODULELIST_COMPONENT) \
				$(_MODULELIST_EXTERNSIONS) $(_MODULELIST_ABSTRACT) $(_MODULELIST_SERVICE) \
				$(_MODULELIST_CLI) $(_MODULELIST_MULTIMEDIA) $(_MODULELIST_APPLICATION) $(_MODULELIST_TOOLS)


_MODULELIST_OLD = \
	PL_SERVICE_MODULE.true \
	PL_SERVICE_SNTPC.true \
	PL_SERVICE_SNTPS.true \
	PL_SERVICE_SYSLOG.true \
	PL_STARTUP_MODULE.true \
	PL_PRODUCT_MODULE.true \
	PL_PRODUCT_SDK_MODULE.false \
	PL_CLI_MODULE.true \
	PL_ABSTRACT_MODULE.true \
	PL_HAL_MODULE.false \
	PL_PAL_KERNEL_STACK.true \
	PL_PAL_IPCOM_STACK.false \
	PL_COMPONENT_MODULE.true \
	PL_OPENSSL_MODULE.true \
	PL_WIFI_MODULE.false \
	PL_MODEM_MODULE.false \
	PL_TOOLS_MODULE.true \
	PL_TOOLS_PROCESS.false \
	PL_TOOLS_QUECTEL_CM.false \
	PL_UDHCP_MODULE.false \
	PL_SQLITE_MODULE.true \
	PL_EXTERNSIONS_MODULE.true \
	PL_SERVICE_FTPD.true \
	PL_SERVICE_FTPC.true \
	PL_SERVICE_TFTPD.true \
	PL_SERVICE_TFTPC.true \
	PL_SERVICE_TELNET.true \
	PL_SERVICE_TELNETD.false \
	PL_SERVICE_PING.true \
	PL_SERVICE_TRACEROUTE.true \
	PL_SERVICE_UBUS_SYNC.false \
	PL_LIBSSH_MODULE.false \
	PL_PJSIP_MODULE.true \
	PL_APPLICATION_MODULE.true \
	PL_OS_UCI.true \
	PL_WEBSERVER_MODULE.false \
	PL_MQTT_MODULE.true
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
PL_BUILD_MODULE := $(PL_BUILD_MODULE) $(subst .,=, $1)
endef
#
#
$(foreach IModule,$(_MODULELIST), $(eval $(call _MODULE_DEF,$(IModule))))
#
endif
#

        
