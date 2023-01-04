#
# Automatically generated make config: don't edit
# Busybox version: 
# Wed Jan  4 22:50:41 2023
#
CONFIG_HAVE_DOT_CONFIG=true

#
# Global Settings
#
ZPL_RUNNING_BASE_PATH="/tmp/app"
ZPL_REAL_SYSCONFIG_PATH="/home/app"
ZPL_INSTALL_PATH="./_install"

#
# Arch Config
#
ZPL_SYSTEM_MODULE=true
ZPL_SYSTEM_LINUX=true
# CONFIG_ZPL_SYSTEM_OPENWRT is not set
# CONFIG_ZPL_SYSTEM_MINGW32 is not set
# CONFIG_ZPL_SYSTEM_MINGW64 is not set
# CONFIG_ZPL_SYSTEM_MSVC32 is not set
# CONFIG_ZPL_SYSTEM_MSVC64 is not set
ZPL_ARCH_MODULE=true
# CONFIG_ZPL_ARCH_ARM is not set
# CONFIG_ZPL_ARCH_ARM64 is not set
# CONFIG_ZPL_ARCH_AARCH64 is not set
# CONFIG_ZPL_ARCH_X86 is not set
ZPL_ARCH_X86_64=true
# CONFIG_ZPL_ARM_A5 is not set
# CONFIG_ZPL_ARM_A6 is not set
# CONFIG_ZPL_ARM_A7 is not set
# CONFIG_ZPL_ARM_A8 is not set
# CONFIG_ZPL_ARM_A9 is not set

#
# Toolchain Config
#
ZPL_HOST_TOOLCHAIN_MODULE=true
# CONFIG_ZPL_EXTERNAL_TOOLCHAIN_MODULE is not set
ZPL_TOOLCHAIN_PREFIX=""
# CONFIG_ZPL_TOOLCHAIN_GLIBC is not set
# CONFIG_ZPL_TOOLCHAIN_UCLIBC is not set
ZPL_TOOLCHAIN_PATH=""
ZPL_TOOLCHAIN_INC_PATH=""
ZPL_TOOLCHAIN_LIB_PATH=""
ZPL_COMPILE_OPTIONS="-g"
# CONFIG_ZPL_COREDUMP_ENABLE is not set
ZPL_FPU_HARD=true
# CONFIG_ZPL_FPU_SOFT is not set
# CONFIG_ZPL_FPU_SOFTFP is not set
# CONFIG_ZPL_FPU_NEON is not set
# CONFIG_ZPL_ARM_VFP is not set
# CONFIG_ZPL_ARM_NEON_VFP is not set
ZPL_TOOLCHAIN_CFLAGS=""

#
# Platform configuration
#

#
# OS Abstract Layer Module
#
ZPL_OS_MODULE=true
ZPL_OS_JSON=true
ZPL_OS_UCI=true
ZPL_OS_TLV=true
ZPL_OS_RNG=true
ZPL_OS_QUEUE=true
ZPL_OS_NVRAM=true
ZPL_OS_AVL=true
ZPL_OS_TTYCOM=true
ZPL_OS_XYZ_MODEM=true
ZPL_OS_CPPJSON=true
ZPL_LIB_MODULE=true
ZPL_IPV6_MODULE=true
# CONFIG_ZPL_LIBEVENT_MODULE is not set
# CONFIG_ZPL_LIBEVENT_SIGNAL is not set
# CONFIG_ZPL_LIBMXML_MODULE is not set
ZPL_IP_FILTER=true
ZPL_IP_PLIST=true
ZPL_WORKQUEUE=true
ZPL_SHELL_MODULE=true
# CONFIG_ZPL_SHRL_MODULE is not set
# CONFIG_ZPL_ACTIVE_STANDBY is not set
# CONFIG_ZPL_NSM_SNMP is not set
ZPL_NSM_MODULE=true
ZPL_VRF_MODULE=true
ZPL_KEYCHAIN=true
ZPL_DISTRIBUTE=true
ZPL_NSM_L3MODULE=true
ZPL_NSM_NEXTHOP=true
ZPL_NSM_ROUTEMAP=true
ZPL_NSM_8021X=true
ZPL_NSM_ARP=true
# CONFIG_ZPL_NSM_DHCP is not set
# CONFIG_ZPL_DHCPS_MODULE is not set
# CONFIG_ZPL_DHCPC_MODULE is not set
# CONFIG_ZPL_DHCPR_MODULE is not set
# CONFIG_ZPL_NSM_DNS is not set
ZPL_NSM_DOS=true
# CONFIG_ZPL_NSM_FIREWALLD is not set
ZPL_NSM_MAC=true
ZPL_NSM_MIRROR=true
ZPL_NSM_PPP=true
ZPL_NSM_QOS=true
ZPL_NSM_SERIAL=true
ZPL_NSM_TRUNK=true
ZPL_NSM_TUNNEL=true
ZPL_NSM_VLAN=true
ZPL_NSM_IGMP=true
ZPL_NSM_MSTP=true
ZPL_NSM_PORT=true
ZPL_NSM_VLANETH=true
# CONFIG_ZPL_NSM_WIRELESS is not set
ZPL_NSM_BRIDGE=true
ZPL_NSM_SECURITY=true
ZPL_NSM_RTADV=true
ZPL_NSM_IRDP=true
ZPL_IPCBC_MODULE=true

#
# Component Module Config
#
# CONFIG_ZPL_COMPONENT_MODULE is not set
# CONFIG_ZPL_MODEM_MODULE is not set
# CONFIG_ZPL_MQTT_MODULE is not set
# CONFIG_ZPL_MQTT_SSL is not set
# CONFIG_ZPL_MQTT_SRV is not set
# CONFIG_ZPL_PJSIP_MODULE is not set
# CONFIG_ZPL_PJSIP_PJSUA2 is not set
# CONFIG_ZPL_SQLITE_MODULE is not set
# CONFIG_ZPL_SQLITE_EXE_MODULE is not set
# CONFIG_ZPL_LIBSSH_MODULE is not set
# CONFIG_ZPL_LIBSSH_NACL is not set
# CONFIG_ZPL_LIBSSH_ZLIB is not set
# CONFIG_ZPL_LIBSSH_SFTP is not set
# CONFIG_ZPL_LIBSSH_GCRYPT is not set
# CONFIG_ZPL_LIBSSH_MBEDTLS is not set
# CONFIG_ZPL_LIBSSH_CRYPTO is not set
# CONFIG_ZPL_LIBSSH_OPENSSL_ED25519 is not set
# CONFIG_ZPL_LIBSSH_BLOWFISH is not set
# CONFIG_ZPL_LIBSSH_SERVER is not set
# CONFIG_ZPL_LIBSSH_PTHREAD is not set
# CONFIG_ZPL_LIBSSH_GSSAPI is not set
# CONFIG_ZPL_LIBSSH_GEX is not set
# CONFIG_ZPL_LIBSSH_PCAP is not set
# CONFIG_ZPL_WEBSERVER_MODULE is not set
# CONFIG_ZPL_WEBSERVER_NONESSL is not set
# CONFIG_ZPL_WEBSERVER_OPENSSL is not set
# CONFIG_ZPL_WEBSERVER_MATRIXSSL is not set
# CONFIG_ZPL_WEBSERVER_MBEDTLS is not set
# CONFIG_ZPL_WEBSERVER_NANOSSL is not set
# CONFIG_ZPL_WEBAPP_MODULE is not set
# CONFIG_ZPL_WIFI_MODULE is not set
# CONFIG_ZPL_MODBUS_MODULE is not set
# CONFIG_ZPL_ONVIF_MODULE is not set
# CONFIG_ZPL_ONVIF_SSL is not set

#
# HAL/PAL Module Config
#
ZPL_HALPAL_MODULE=true
ZPL_HAL_MODULE=true
ZPL_PAL_MODULE=true
# CONFIG_ZPL_IPCOM_MODULE is not set
ZPL_KERNEL_MODULE=true
ZPL_KERNEL_NETLINK=true
ZPL_IPCOM_ROOT_PATH=""

#
# Service Module Config
#
ZPL_SERVICE_MODULE=true
ZPL_SERVICE_SNTPC=true
ZPL_SERVICE_SNTPS=true
ZPL_SERVICE_SYSLOG=true
ZPL_SERVICE_FTPC=true
ZPL_SERVICE_FTPD=true
ZPL_SERVICE_TFTPC=true
ZPL_SERVICE_TFTPD=true
ZPL_SERVICE_PING=true
ZPL_SERVICE_TELNET=true
# CONFIG_ZPL_SERVICE_TELENTD is not set
ZPL_SERVICE_TRACEROUTE=true
ZPL_SERVICE_UBUS_SYNC=true

#
# Multimedia Module Config
#
ZPL_MULTIMEDIA_MODULE=true
ZPL_LIBRTSP_MODULE=true
ZPL_LIBORTP_MODULE=true
# CONFIG_ZPL_EXOSIP_MODULE is not set
# CONFIG_ZPL_LIBRTMP_MODULE is not set
# CONFIG_ZPL_LIBJPEG_MODULE is not set
ZPL_ZPLMEDIA_MODULE=true
# CONFIG_ZPL_HISIMPP_MODULE is not set
# CONFIG_ZPL_HISIMPP_HWDEBUG is not set
# CONFIG_ZPL_PJPROJECT_MODULE is not set
# CONFIG_ZPL_PJPROJECT_OPTIONS is not set
# CONFIG_ZPL_PJ_RESAMPLE_ENABLE is not set
# CONFIG_ZPL_PJ_SIMPLE_ENABLE is not set
# CONFIG_ZPL_PJ_SRTP_ENABLE is not set
# CONFIG_ZPL_PJ_VIDEO_ENABLE is not set
# CONFIG_ZPL_PJ_VIDEO_YUV_ENABLE is not set
# CONFIG_ZPL_PJ_VIDEO_H264_ENABLE is not set
ZPL_PJ_CODEC_H264_LIB_PATH=""
# CONFIG_ZPL_PJ_CODEC_GSM_ENABLE is not set
# CONFIG_ZPL_PJ_CODEC_SPEEX_ENABLE is not set
# CONFIG_ZPL_PJ_CODEC_ILBC_ENABLE is not set
# CONFIG_ZPL_PJ_CODEC_G722_ENABLE is not set
# CONFIG_ZPL_PJ_CODEC_WEBRTC_ENABLE is not set
# CONFIG_ZPL_PJ_CODEC_VPX_ENABLE is not set
ZPL_PJ_CODEC_VPX_LIB_PATH=""
# CONFIG_ZPL_PJ_FFMPEG_ENABLE is not set
ZPL_PJ_FFMPEG_LIB_PATH=""
# CONFIG_ZPL_PJ_SDL_ENABLE is not set
ZPL_PJ_SDL_LIB_PATH=""
# CONFIG_ZPL_PJ_AUDIO_ALSA is not set
# CONFIG_ZPL_PJ_AUDIO_PORTAUDIO is not set
# CONFIG_ZPL_LIVE555_MODULE is not set
# CONFIG_ZPL_SOFTCODEC_MODULE is not set
# CONFIG_ZPL_LIBX264_MODULE is not set
# CONFIG_ZPL_OPENH264_MODULE is not set
# CONFIG_ZPL_LIBVPX_MODULE is not set
# CONFIG_ZPL_FFMPEG_MODULE is not set
# CONFIG_ZPL_MEDIAAPP_MODULE is not set
# CONFIG_ZPL_LIBMEDIA_MODULE is not set

#
# BSP Module Config
#
ZPL_BSP_MODULE=true
ZPL_SDK_MODULE=true
ZPL_SDK_NONE=true
# CONFIG_ZPL_SDK_USER is not set
# CONFIG_ZPL_SDK_KERNEL is not set
# CONFIG_ZPL_SDK_BCM53125 is not set

#
# Externsions Module Config
#
ZPL_EXTERNSIONS_MODULE=true
# CONFIG_ZPL_ZLIB_MODULE is not set
ZPL_OPENSSL_MODULE=true
# CONFIG_ZPL_EXFREETYPE_MODULE is not set
ZPL_EXFREETYPE_LIB_PATH=""
ZPL_EXFREETYPE_INC_PATH=""
# CONFIG_ZPL_READLINE_MODULE is not set
# CONFIG_ZPL_LIBNL_MODULE is not set

#
# Application Module Config
#
# CONFIG_ZPL_APPLICATION_MODULE is not set
# CONFIG_ZPL_APP_X5_MODULE is not set
# CONFIG_ZPL_APP_V9_MODULE is not set

#
# Tools Module Config
#
# CONFIG_ZPL_TOOLS_MODULE is not set
# CONFIG_ZPL_TOOLS_PROCESS is not set
# CONFIG_ZPL_TOOLS_QUECTEL_CM is not set
# CONFIG_ZPL_TOOLS_SYSTEM is not set
# CONFIG_ZPL_VTYSH_MODULE is not set
# CONFIG_ZPL_WATCHDOG_MODULE is not set
# CONFIG_ZPL_SWCONFIG_MODULE is not set
