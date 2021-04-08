/*
 * Automatically generated C config: don't edit
 * Busybox version: 
 */
#define AUTOCONF_TIMESTAMP "2021-04-08 20:35:41 CST"

#define CONFIG_HAVE_DOT_CONFIG 1

/*
 * Global Settings
 */
#define PL_RUNNING_BASE_PATH "/tmp/app"
#define PL_REAL_SYSCONFIG_PATH "/home/zhurish/workspace/SWPlatform/source/debug"
#define PL_INSTALL_PATH "./_install"

/*
 * Platform Arch Config
 */
#define PL_SYSTEM_MODULE 1
#define PL_SYSTEM_LINUX 1
#undef PL_SYSTEM_OPENWRT
#undef PL_SYSTEM_MINGW32
#undef PL_SYSTEM_MINGW64
#undef PL_SYSTEM_MSVC32
#undef PL_SYSTEM_MSVC64
#define PL_ARCH_MODULE 1
#undef PL_ARCH_ARM
#undef PL_ARCH_ARM64
#undef PL_ARCH_AARCH64
#undef PL_ARCH_X86
#define PL_ARCH_X86_64 1

/*
 * Toolchain Config
 */
#define PL_HOST_TOOLCHAIN_MODULE 1
#undef PL_EXTERNAL_TOOLCHAIN_MODULE
#define PL_EXTERNAL_TOOLCHAIN_PREFIX ""
#undef PL_EXTERNAL_TOOLCHAIN_GLIBC
#undef PL_EXTERNAL_TOOLCHAIN_UCLIBC
#define PL_TOOLCHAIN_PATH ""
#define PL_TOOLCHAIN_INC_PATH ""
#define PL_TOOLCHAIN_LIB_PATH ""
#define PL_COMPILE_OPTIONS "-g"

/*
 * Product Module Config
 */
#define PL_PRODUCT_MODULE 1
#undef PL_PRODUCT_L2SWITCH
#undef PL_PRODUCT_L3ROUTE
#undef PL_VPN_MODULE
#undef PL_IPCOM_STACK_MODULE
#define PL_IPCOM_ROOT_PATH ""
#define PL_KERNEL_STACK_MODULE 1
#define PL_PRODUCT_BSP_MODULE 1
#undef PL_PRODUCT_SDK_MODULE

/*
 * Platform configuration
 */

/*
 * Platform OS Abstract Layer Module
 */
#define PL_OS_MODULE 1
#define PL_OS_JSON 1
#define PL_OS_UCI 1
#define PL_OS_TLV 1
#define PL_OS_RNG 1
#define PL_OS_QUEUE 1
#define PL_OS_NVRAM 1
#define PL_OS_AVL 1
#define PL_OS_TTYCOM 1
#define PL_OS_XYZ_MODEM 1
#define PL_OS_CPPJSON 1

/*
 * Platform NSM Layer Module
 */
#define PL_NSM_MODULE 1
#undef PL_NSM_8021X
#undef PL_NSM_ARP
#undef PL_NSM_BRIDGE
#undef PL_NSM_DHCP
#undef PL_NSM_DNS
#undef PL_NSM_DOS
#undef PL_NSM_FIREWALLD
#undef PL_NSM_MAC
#undef PL_NSM_MIRROR
#undef PL_NSM_PPP
#undef PL_NSM_QOS
#undef PL_NSM_SERIAL
#undef PL_NSM_TRUNK
#undef PL_NSM_TUNNEL
#undef PL_NSM_VLAN
#undef PL_NSM_VETH
#undef PL_NSM_SECURITY

/*
 * Platform Shell Module
 */
#define PL_SHELL_MODULE 1

/*
 * Component Module Config
 */
#undef PL_COMPONENT_MODULE
#undef PL_MODEM_MODULE
#undef PL_MQTT_MODULE
#undef PL_MQTT_SSL
#undef PL_MQTT_SRV
#undef PL_PJSIP_MODULE
#undef PL_PJSIP_PJSUA2
#undef PL_SQLITE_MODULE
#undef PL_SQLITE_EXE_MODULE
#undef PL_LIBSSH_MODULE
#undef PL_LIBSSH_NACL
#undef PL_LIBSSH_ZLIB
#undef PL_LIBSSH_SFTP
#undef PL_LIBSSH_GCRYPT
#undef PL_LIBSSH_MBEDTLS
#undef PL_LIBSSH_CRYPTO
#undef PL_LIBSSH_OPENSSL_ED25519
#undef PL_LIBSSH_BLOWFISH
#undef PL_LIBSSH_SERVER
#undef PL_LIBSSH_PTHREAD
#undef PL_LIBSSH_GSSAPI
#undef PL_LIBSSH_GEX
#undef PL_LIBSSH_PCAP
#undef PL_UDHCP_MODULE
#undef PL_UDHCPS_MODULE
#undef PL_UDHCPC_MODULE
#undef PL_UDHCPR_MODULE
#undef PL_WEBSERVER_MODULE
#undef PL_WEBSERVER_OPENSSL
#undef PL_WEBSERVER_MATRIXSSL
#undef PL_WEBSERVER_MBEDTLS
#undef PL_WEBSERVER_NANOSSL
#undef PL_WIFI_MODULE
#undef PL_MODBUS_MODULE

/*
 * Externsions Module Config
 */
#undef PL_EXTERNSIONS_MODULE
#undef PL_ZLIB_MODULE
#undef PL_OPENSSL_MODULE

/*
 * Abstract Module Config
 */
#define PL_ABSTRACT_MODULE 1
#undef PL_HAL_MODULE
#define PL_PAL_MODULE 1
#undef PL_PAL_IPCOM_STACK
#define PL_PAL_KERNEL_STACK 1

/*
 * Service Module Config
 */
#undef PL_SERVICE_MODULE
#undef PL_SERVICE_SNTPC
#undef PL_SERVICE_SNTPS
#undef PL_SERVICE_SYSLOG
#undef PL_SERVICE_FTPC
#undef PL_SERVICE_FTPD
#undef PL_SERVICE_TFTPC
#undef PL_SERVICE_TFTPD
#undef PL_SERVICE_PING
#undef PL_SERVICE_TELNET
#undef PL_SERVICE_TELENTD
#undef PL_SERVICE_TRACEROUTE
#undef PL_SERVICE_UBUS_SYNC

/*
 * CLI Module Config
 */
#define PL_CLI_MODULE 1

/*
 * Multimedia Module Config
 */
#undef PL_MULTIMEDIA_MODULE
#undef PL_LIVE555_MODULE
#undef PL_LIBX264_MODULE
#undef PL_OPENH264_MODULE
#undef PL_LIBVPX_MODULE
#undef PL_FFMPEG_MODULE
#undef PL_EXOSIP_MODULE
#undef PL_PJPROJECT_MODULE
#undef PL_PJPROJECT_OPTIONS
#undef PL_PJ_RESAMPLE_ENABLE
#undef PL_PJ_SIMPLE_ENABLE
#undef PL_PJ_SRTP_ENABLE
#undef PL_PJ_VIDEO_ENABLE
#undef PL_PJ_VIDEO_YUV_ENABLE
#undef PL_PJ_VIDEO_H264_ENABLE
#define PL_PJ_CODEC_H264_LIB_PATH ""
#undef PL_PJ_CODEC_GSM_ENABLE
#undef PL_PJ_CODEC_SPEEX_ENABLE
#undef PL_PJ_CODEC_ILBC_ENABLE
#undef PL_PJ_CODEC_G722_ENABLE
#undef PL_PJ_CODEC_WEBRTC_ENABLE
#undef PL_PJ_CODEC_VPX_ENABLE
#define PL_PJ_CODEC_VPX_LIB_PATH ""
#undef PL_PJ_FFMPEG_ENABLE
#define PL_PJ_FFMPEG_LIB_PATH ""
#undef PL_PJ_SDL_ENABLE
#define PL_PJ_SDL_LIB_PATH ""
#undef PL_PJ_AUDIO_ALSA
#undef PL_PJ_AUDIO_PORTAUDIO

/*
 * Application Module Config
 */
#undef PL_APPLICATION_MODULE
#undef PL_APP_X5_MODULE
#undef PL_APP_V9_MODULE

/*
 * Tools Module Config
 */
#undef PL_TOOLS_MODULE
#undef PL_TOOLS_PROCESS
#undef PL_TOOLS_QUECTEL_CM
#undef PL_TOOLS_SYSTEM
