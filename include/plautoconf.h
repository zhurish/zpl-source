/*
 * Automatically generated C config: don't edit
 * Busybox version: 
 */
#define AUTOCONF_TIMESTAMP "2021-01-31 20:53:20 CST"

#define CONFIG_HAVE_DOT_CONFIG 1

/*
 * Global Settings
 */
#define PL_RUNNING_BASE_PATH "/tmp/app"
#define PL_REAL_SYSCONFIG_PATH "/dm/zhurish/work/busybox-1.28.4/SWPlatform/debug/etc"
#define PL_INSTALL_PATH "./_install"

/*
 * Platform Arch Config
 */
#define PL_SYSTEM_MODULE 1
#define PL_USEING_LINUX 1
#undef PL_USEING_OPENWRT
#define PL_ARCH_MODULE 1
#undef PL_USEING_ARM
#undef PL_USEING_ARM64
#undef PL_USEING_AARCH64
#undef PL_USEING_X86
#define PL_USEING_X86_64 1

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
#define PL_PRODUCT_L3ROUTE 1
#define PL_VPN_MODULE 1
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
#define PL_NSM_ARP 1
#define PL_NSM_BRIDGE 1
#define PL_NSM_DHCP 1
#define PL_NSM_DNS 1
#undef PL_NSM_DOS
#define PL_NSM_FIREWALLD 1
#define PL_NSM_MAC 1
#undef PL_NSM_MIRROR
#define PL_NSM_PPP 1
#undef PL_NSM_QOS
#define PL_NSM_SERIAL 1
#define PL_NSM_TRUNK 1
#define PL_NSM_TUNNEL 1
#define PL_NSM_VLAN 1
#define PL_NSM_VETH 1
#define PL_NSM_SECURITY 1

/*
 * Platform Shell Module
 */
#define PL_SHELL_MODULE 1

/*
 * Component Module Config
 */
#define PL_COMPONENT_MODULE 1
#define PL_MODEM_MODULE 1
#undef PL_MQTT_MODULE
#undef PL_MQTT_SSL
#undef PL_MQTT_SRV
#undef PL_PJSIP_MODULE
#undef PL_PJSIP_PJSUA2
#define PL_SQLITE_MODULE 1
#define PL_SQLITE_EXE_MODULE 1
#undef PL_LIBSSH_MODULE
#undef PL_LIBSSH_NACL
#undef PL_LIBSSH_ZLIB
#undef PL_LIBSSH_SFTP
#undef PL_LIBSSH_SSH1
#undef PL_LIBSSH_GCRYPT
#undef PL_LIBSSH_SERVER
#undef PL_LIBSSH_PTHREAD
#undef PL_LIBSSH_GSSAPI
#define PL_UDHCP_MODULE 1
#define PL_UDHCPS_MODULE 1
#define PL_UDHCPC_MODULE 1
#define PL_UDHCPR_MODULE 1
#undef PL_WEBSERVER_MODULE
#undef PL_WEBSERVER_OPENSSL
#undef PL_WEBSERVER_MATRIXSSL
#undef PL_WEBSERVER_MBEDTLS
#undef PL_WEBSERVER_NANOSSL
#define PL_WIFI_MODULE 1

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
#define PL_SERVICE_MODULE 1
#define PL_SERVICE_SNTPC 1
#undef PL_SERVICE_SNTPS
#define PL_SERVICE_SYSLOG 1
#undef PL_SERVICE_FTPC
#undef PL_SERVICE_FTPD
#undef PL_SERVICE_TFTPC
#undef PL_SERVICE_TFTPD
#define PL_SERVICE_PING 1
#define PL_SERVICE_TELNET 1
#undef PL_SERVICE_TELENTD
#define PL_SERVICE_TRACEROUTE 1
#define PL_SERVICE_UBUS_SYNC 1

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
#define PL_TOOLS_MODULE 1
#define PL_TOOLS_PROCESS 1
#undef PL_TOOLS_QUECTEL_CM
#undef PL_TOOLS_SYSTEM
