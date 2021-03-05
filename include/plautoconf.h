/*
 * Automatically generated C config: don't edit
 * Busybox version: 
 */
#define AUTOCONF_TIMESTAMP "2021-03-05 22:20:21 CST"

#define CONFIG_HAVE_DOT_CONFIG 1

/*
 * Global Settings
 */
#define PL_RUNNING_BASE_PATH "/tmp/app"
#define PL_REAL_SYSCONFIG_PATH "/home/zhurish/workspace/SWPlatform/source/debug/etc"
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
#define PL_PRODUCT_L2SWITCH 1
#define PL_PRODUCT_L3ROUTE 1
#define PL_VPN_MODULE 1
#undef PL_IPCOM_STACK_MODULE
#define PL_IPCOM_ROOT_PATH ""
#define PL_KERNEL_STACK_MODULE 1
#define PL_PRODUCT_BSP_MODULE 1
#define PL_PRODUCT_SDK_MODULE 1

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
#define PL_NSM_8021X 1
#define PL_NSM_ARP 1
#define PL_NSM_BRIDGE 1
#define PL_NSM_DHCP 1
#define PL_NSM_DNS 1
#define PL_NSM_DOS 1
#define PL_NSM_FIREWALLD 1
#define PL_NSM_MAC 1
#define PL_NSM_MIRROR 1
#define PL_NSM_PPP 1
#define PL_NSM_QOS 1
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
#define PL_MQTT_MODULE 1
#undef PL_MQTT_SSL
#define PL_MQTT_SRV 1
#define PL_PJSIP_MODULE 1
#define PL_PJSIP_PJSUA2 1
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
#define PL_WEBSERVER_MODULE 1
#define PL_WEBSERVER_OPENSSL 1
#undef PL_WEBSERVER_MATRIXSSL
#undef PL_WEBSERVER_MBEDTLS
#undef PL_WEBSERVER_NANOSSL
#define PL_WIFI_MODULE 1

/*
 * Externsions Module Config
 */
#define PL_EXTERNSIONS_MODULE 1
#define PL_ZLIB_MODULE 1
#undef PL_OPENSSL_MODULE

/*
 * Abstract Module Config
 */
#define PL_ABSTRACT_MODULE 1
#define PL_HAL_MODULE 1
#define PL_PAL_MODULE 1
#undef PL_PAL_IPCOM_STACK
#define PL_PAL_KERNEL_STACK 1

/*
 * Service Module Config
 */
#define PL_SERVICE_MODULE 1
#define PL_SERVICE_SNTPC 1
#define PL_SERVICE_SNTPS 1
#define PL_SERVICE_SYSLOG 1
#define PL_SERVICE_FTPC 1
#define PL_SERVICE_FTPD 1
#define PL_SERVICE_TFTPC 1
#define PL_SERVICE_TFTPD 1
#define PL_SERVICE_PING 1
#define PL_SERVICE_TELNET 1
#define PL_SERVICE_TELENTD 1
#define PL_SERVICE_TRACEROUTE 1
#define PL_SERVICE_UBUS_SYNC 1

/*
 * CLI Module Config
 */
#define PL_CLI_MODULE 1

/*
 * Multimedia Module Config
 */
#define PL_MULTIMEDIA_MODULE 1
#define PL_LIVE555_MODULE 1
#undef PL_LIBX264_MODULE
#define PL_OPENH264_MODULE 1
#undef PL_LIBVPX_MODULE
#define PL_FFMPEG_MODULE 1
#define PL_PJPROJECT_MODULE 1
#define PL_PJPROJECT_OPTIONS 1
#define PL_PJ_RESAMPLE_ENABLE 1
#define PL_PJ_SIMPLE_ENABLE 1
#define PL_PJ_SRTP_ENABLE 1
#define PL_PJ_VIDEO_ENABLE 1
#define PL_PJ_VIDEO_YUV_ENABLE 1
#define PL_PJ_VIDEO_H264_ENABLE 1
#define PL_PJ_CODEC_H264_LIB_PATH "/usr/local"
#define PL_PJ_CODEC_GSM_ENABLE 1
#define PL_PJ_CODEC_SPEEX_ENABLE 1
#define PL_PJ_CODEC_ILBC_ENABLE 1
#define PL_PJ_CODEC_G722_ENABLE 1
#define PL_PJ_CODEC_WEBRTC_ENABLE 1
#undef PL_PJ_CODEC_VPX_ENABLE
#define PL_PJ_CODEC_VPX_LIB_PATH ""
#undef PL_PJ_FFMPEG_ENABLE
#define PL_PJ_FFMPEG_LIB_PATH ""
#undef PL_PJ_SDL_ENABLE
#define PL_PJ_SDL_LIB_PATH ""
#define PL_PJ_AUDIO_ALSA 1
#undef PL_PJ_AUDIO_PORTAUDIO

/*
 * Application Module Config
 */
#define PL_APPLICATION_MODULE 1
#define PL_APP_X5_MODULE 1
#undef PL_APP_V9_MODULE

/*
 * Tools Module Config
 */
#define PL_TOOLS_MODULE 1
#define PL_TOOLS_PROCESS 1
#define PL_TOOLS_QUECTEL_CM 1
#undef PL_TOOLS_SYSTEM
