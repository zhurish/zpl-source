/*
 * Automatically generated C config: don't edit
 * Busybox version: 
 */
#define AUTOCONF_TIMESTAMP "2022-07-02 21:52:19 CST"

#define CONFIG_HAVE_DOT_CONFIG 1

/*
 * Global Settings
 */
#define ZPL_RUNNING_BASE_PATH "/tmp/app"
#define ZPL_REAL_SYSCONFIG_PATH "/home/app"
#define ZPL_INSTALL_PATH "./_install"

/*
 * Arch Config
 */
#define ZPL_SYSTEM_MODULE 1
#define ZPL_SYSTEM_LINUX 1
#undef ZPL_SYSTEM_OPENWRT
#undef ZPL_SYSTEM_MINGW32
#undef ZPL_SYSTEM_MINGW64
#undef ZPL_SYSTEM_MSVC32
#undef ZPL_SYSTEM_MSVC64
#define ZPL_ARCH_MODULE 1
#define ZPL_ARCH_ARM 1
#undef ZPL_ARCH_ARM64
#undef ZPL_ARCH_AARCH64
#undef ZPL_ARCH_X86
#undef ZPL_ARCH_X86_64
#undef ZPL_ARM_A5
#undef ZPL_ARM_A6
#define ZPL_ARM_A7 1
#undef ZPL_ARM_A8
#undef ZPL_ARM_A9

/*
 * Toolchain Config
 */
#undef ZPL_HOST_TOOLCHAIN_MODULE
#define ZPL_EXTERNAL_TOOLCHAIN_MODULE 1
#define ZPL_TOOLCHAIN_PREFIX "arm-linux-gnueabihf-"
#define ZPL_TOOLCHAIN_GLIBC 1
#undef ZPL_TOOLCHAIN_UCLIBC
#define ZPL_TOOLCHAIN_PATH "/opt/gcc-linaro-7.5.0-arm-linux-gnueabihf"
#define ZPL_TOOLCHAIN_INC_PATH ""
#define ZPL_TOOLCHAIN_LIB_PATH ""
#define ZPL_COMPILE_OPTIONS "-g"
#define ZPL_COREDUMP_ENABLE 1
#define ZPL_FPU_HARD 1
#undef ZPL_FPU_SOFT
#undef ZPL_FPU_SOFTFP
#undef ZPL_FPU_NEON
#undef ZPL_ARM_VFP
#undef ZPL_ARM_NEON_VFP
#define ZPL_TOOLCHAIN_CFLAGS "-D__ARM_PCS_VFP"

/*
 * Platform configuration
 */

/*
 * OS Abstract Layer Module
 */
#define ZPL_OS_MODULE 1
#define ZPL_OS_JSON 1
#define ZPL_OS_UCI 1
#define ZPL_OS_TLV 1
#define ZPL_OS_RNG 1
#define ZPL_OS_QUEUE 1
#define ZPL_OS_NVRAM 1
#define ZPL_OS_AVL 1
#define ZPL_OS_TTYCOM 1
#define ZPL_OS_XYZ_MODEM 1
#define ZPL_OS_CPPJSON 1
#define ZPL_LIB_MODULE 1
#define ZPL_IPV6_MODULE 1
#undef ZPL_LIBEVENT_MODULE
#undef ZPL_LIBEVENT_SIGNAL
#undef ZPL_LIBMXML_MODULE
#define ZPL_IP_FILTER 1
#define ZPL_IP_PLIST 1
#define ZPL_WORKQUEUE 1
#define ZPL_SHELL_MODULE 1
#undef ZPL_SHRL_MODULE
#undef ZPL_ACTIVE_STANDBY
#define ZPL_NSM_SNMP 1
#define ZPL_NSM_MODULE 1
#define ZPL_VRF_MODULE 1
#define ZPL_KEYCHAIN 1
#define ZPL_DISTRIBUTE 1
#define ZPL_NSM_L3MODULE 1
#define ZPL_NSM_NEXTHOP 1
#define ZPL_NSM_ROUTEMAP 1
#define ZPL_NSM_8021X 1
#define ZPL_NSM_ARP 1
#undef ZPL_NSM_DHCP
#undef ZPL_DHCPS_MODULE
#undef ZPL_DHCPC_MODULE
#undef ZPL_DHCPR_MODULE
#undef ZPL_NSM_DNS
#define ZPL_NSM_DOS 1
#undef ZPL_NSM_FIREWALLD
#define ZPL_NSM_MAC 1
#define ZPL_NSM_MIRROR 1
#define ZPL_NSM_PPP 1
#define ZPL_NSM_QOS 1
#define ZPL_NSM_SERIAL 1
#define ZPL_NSM_TRUNK 1
#define ZPL_NSM_TUNNEL 1
#define ZPL_NSM_VLAN 1
#define ZPL_NSM_IGMP 1
#define ZPL_NSM_MSTP 1
#define ZPL_NSM_PORT 1
#undef ZPL_NSM_VLANETH
#undef ZPL_NSM_WIRELESS
#undef ZPL_NSM_BRIDGE
#define ZPL_NSM_SECURITY 1
#define ZPL_NSM_RTADV 1
#define ZPL_NSM_IRDP 1
#define ZPL_IPCBC_MODULE 1

/*
 * Component Module Config
 */
#undef ZPL_COMPONENT_MODULE
#undef ZPL_MODEM_MODULE
#undef ZPL_MQTT_MODULE
#undef ZPL_MQTT_SSL
#undef ZPL_MQTT_SRV
#undef ZPL_PJSIP_MODULE
#undef ZPL_PJSIP_PJSUA2
#undef ZPL_SQLITE_MODULE
#undef ZPL_SQLITE_EXE_MODULE
#undef ZPL_LIBSSH_MODULE
#undef ZPL_LIBSSH_NACL
#undef ZPL_LIBSSH_ZLIB
#undef ZPL_LIBSSH_SFTP
#undef ZPL_LIBSSH_GCRYPT
#undef ZPL_LIBSSH_MBEDTLS
#undef ZPL_LIBSSH_CRYPTO
#undef ZPL_LIBSSH_OPENSSL_ED25519
#undef ZPL_LIBSSH_BLOWFISH
#undef ZPL_LIBSSH_SERVER
#undef ZPL_LIBSSH_PTHREAD
#undef ZPL_LIBSSH_GSSAPI
#undef ZPL_LIBSSH_GEX
#undef ZPL_LIBSSH_PCAP
#undef ZPL_WEBSERVER_MODULE
#undef ZPL_WEBSERVER_NONESSL
#undef ZPL_WEBSERVER_OPENSSL
#undef ZPL_WEBSERVER_MATRIXSSL
#undef ZPL_WEBSERVER_MBEDTLS
#undef ZPL_WEBSERVER_NANOSSL
#undef ZPL_WEBAPP_MODULE
#undef ZPL_WIFI_MODULE
#undef ZPL_MODBUS_MODULE
#undef ZPL_ONVIF_MODULE
#undef ZPL_ONVIF_SSL

/*
 * HAL/PAL Module Config
 */
#define ZPL_HALPAL_MODULE 1
#define ZPL_HAL_MODULE 1
#define ZPL_PAL_MODULE 1

/*
 * Service Module Config
 */
#undef ZPL_SERVICE_MODULE
#undef ZPL_SERVICE_SNTPC
#undef ZPL_SERVICE_SNTPS
#undef ZPL_SERVICE_SYSLOG
#undef ZPL_SERVICE_FTPC
#undef ZPL_SERVICE_FTPD
#undef ZPL_SERVICE_TFTPC
#undef ZPL_SERVICE_TFTPD
#undef ZPL_SERVICE_PING
#undef ZPL_SERVICE_TELNET
#undef ZPL_SERVICE_TELENTD
#undef ZPL_SERVICE_TRACEROUTE
#undef ZPL_SERVICE_UBUS_SYNC

/*
 * Multimedia Module Config
 */
#undef ZPL_MULTIMEDIA_MODULE
#undef ZPL_LIVE555_MODULE
#undef ZPL_SOFTCODEC_MODULE
#undef ZPL_LIBX264_MODULE
#undef ZPL_OPENH264_MODULE
#undef ZPL_LIBVPX_MODULE
#undef ZPL_FFMPEG_MODULE
#undef ZPL_EXOSIP_MODULE
#undef ZPL_LIBRTMP_MODULE
#undef ZPL_PJPROJECT_MODULE
#undef ZPL_PJPROJECT_OPTIONS
#undef ZPL_PJ_RESAMPLE_ENABLE
#undef ZPL_PJ_SIMPLE_ENABLE
#undef ZPL_PJ_SRTP_ENABLE
#undef ZPL_PJ_VIDEO_ENABLE
#undef ZPL_PJ_VIDEO_YUV_ENABLE
#undef ZPL_PJ_VIDEO_H264_ENABLE
#define ZPL_PJ_CODEC_H264_LIB_PATH ""
#undef ZPL_PJ_CODEC_GSM_ENABLE
#undef ZPL_PJ_CODEC_SPEEX_ENABLE
#undef ZPL_PJ_CODEC_ILBC_ENABLE
#undef ZPL_PJ_CODEC_G722_ENABLE
#undef ZPL_PJ_CODEC_WEBRTC_ENABLE
#undef ZPL_PJ_CODEC_VPX_ENABLE
#define ZPL_PJ_CODEC_VPX_LIB_PATH ""
#undef ZPL_PJ_FFMPEG_ENABLE
#define ZPL_PJ_FFMPEG_LIB_PATH ""
#undef ZPL_PJ_SDL_ENABLE
#define ZPL_PJ_SDL_LIB_PATH ""
#undef ZPL_PJ_AUDIO_ALSA
#undef ZPL_PJ_AUDIO_PORTAUDIO
#undef ZPL_MEDIAAPP_MODULE
#undef ZPL_LIBMEDIA_MODULE
#undef ZPL_LIBRTSP_MODULE
#undef ZPL_LIBORTP_MODULE
#undef ZPL_LIBJPEG_MODULE

/*
 * hwmedia Module Config
 */
#undef ZPL_ZPLMEDIA_MODULE
#undef ZPL_HISIMPP_MODULE
#undef ZPL_HISIMPP_HWDEBUG

/*
 * BSP Module Config
 */
#define ZPL_BSP_MODULE 1
#undef ZPL_IPCOM_MODULE
#define ZPL_KERNEL_MODULE 1
#define ZPL_KERNEL_NETLINK 1
#undef ZPL_KERNEL_FORWARDING
#define ZPL_IPCOM_ROOT_PATH ""
#define ZPL_SDK_MODULE 1
#undef ZPL_SDK_NONE
#define ZPL_SDK_USER 1
#undef ZPL_SDK_KERNEL
#define ZPL_SDK_BCM53125 1

/*
 * Externsions Module Config
 */
#undef ZPL_EXTERNSIONS_MODULE
#undef ZPL_ZLIB_MODULE
#undef ZPL_OPENSSL_MODULE
#undef ZPL_EXFREETYPE_MODULE
#define ZPL_EXFREETYPE_LIB_PATH ""
#define ZPL_EXFREETYPE_INC_PATH ""
#undef ZPL_READLINE_MODULE

/*
 * Application Module Config
 */
#undef ZPL_APPLICATION_MODULE
#undef ZPL_APP_X5_MODULE
#undef ZPL_APP_V9_MODULE

/*
 * Tools Module Config
 */
#define ZPL_TOOLS_MODULE 1
#undef ZPL_TOOLS_PROCESS
#undef ZPL_TOOLS_QUECTEL_CM
#undef ZPL_TOOLS_SYSTEM
#undef ZPL_VTYSH_MODULE
#undef ZPL_WATCHDOG_MODULE
#define ZPL_SWCONFIG_MODULE 1
