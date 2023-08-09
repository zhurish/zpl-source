/*
 * Automatically generated C config: don't edit
 * ZPLSource version: 
 */
#define MAKECONF_TIMESTAMP "2023-07-31 22:59:31 CST"

#define CONFIG_HAVE_DOT_CONFIG 1

/*
 * Global Settings
 */
#define CONFIG_ZPL_RUNNING_BASE_PATH "/tmp/app"
#define CONFIG_ZPL_REAL_SYSCONFIG_PATH "/home/app"
#define CONFIG_ZPL_INSTALL_PATH "./_install"

/*
 * Arch Config
 */
#define CONFIG_ZPL_SYSTEM_MODULE 1
#define CONFIG_ZPL_SYSTEM_LINUX 1
#undef CONFIG_ZPL_SYSTEM_OPENWRT
#undef CONFIG_ZPL_SYSTEM_MINGW32
#undef CONFIG_ZPL_SYSTEM_MINGW64
#undef CONFIG_ZPL_SYSTEM_MSVC32
#undef CONFIG_ZPL_SYSTEM_MSVC64
#define CONFIG_ZPL_ARCH_MODULE 1
#undef CONFIG_ZPL_ARCH_ARM
#undef CONFIG_ZPL_ARCH_ARM64
#undef CONFIG_ZPL_ARCH_AARCH64
#undef CONFIG_ZPL_ARCH_X86
#define CONFIG_ZPL_ARCH_X86_64 1
#undef CONFIG_ZPL_ARM_A5
#undef CONFIG_ZPL_ARM_A6
#undef CONFIG_ZPL_ARM_A7
#undef CONFIG_ZPL_ARM_A8
#undef CONFIG_ZPL_ARM_A9

/*
 * Toolchain Config
 */
#define CONFIG_ZPL_HOST_TOOLCHAIN_MODULE 1
#undef CONFIG_ZPL_EXTERNAL_TOOLCHAIN_MODULE
#define CONFIG_ZPL_TOOLCHAIN_PREFIX ""
#undef CONFIG_ZPL_TOOLCHAIN_GLIBC
#undef CONFIG_ZPL_TOOLCHAIN_UCLIBC
#define CONFIG_ZPL_TOOLCHAIN_PATH ""
#define CONFIG_ZPL_TOOLCHAIN_INC_PATH ""
#define CONFIG_ZPL_TOOLCHAIN_LIB_PATH ""
#define CONFIG_ZPL_COMPILE_OPTIONS "-g"
#undef CONFIG_ZPL_COREDUMP_ENABLE
#define CONFIG_ZPL_FPU_HARD 1
#undef CONFIG_ZPL_FPU_SOFT
#undef CONFIG_ZPL_FPU_SOFTFP
#undef CONFIG_ZPL_FPU_NEON
#undef CONFIG_ZPL_ARM_VFP
#undef CONFIG_ZPL_ARM_NEON_VFP
#define CONFIG_ZPL_TOOLCHAIN_CFLAGS ""

/*
 * Platform configuration
 */

/*
 * OS Abstract Layer Module
 */
#define CONFIG_ZPL_OS_MODULE 1
#define CONFIG_ZPL_OS_JSON 1
#define CONFIG_ZPL_OS_UCI 1
#define CONFIG_ZPL_OS_TLV 1
#define CONFIG_ZPL_OS_RNG 1
#define CONFIG_ZPL_OS_QUEUE 1
#define CONFIG_ZPL_OS_NVRAM 1
#define CONFIG_ZPL_OS_AVL 1
#define CONFIG_ZPL_OS_TTYCOM 1
#define CONFIG_ZPL_OS_XYZ_MODEM 1
#define CONFIG_ZPL_OS_CPPJSON 1
#define CONFIG_ZPL_JTHREAD_MODULE 1
#define CONFIG_ZPL_LIB_MODULE 1
#define CONFIG_ZPL_IPV6_MODULE 1
#undef CONFIG_ZPL_LIBEVENT_MODULE
#undef CONFIG_ZPL_LIBEVENT_SIGNAL
#undef CONFIG_ZPL_LIBMXML_MODULE
#define CONFIG_ZPL_IP_FILTER 1
#define CONFIG_ZPL_IP_PLIST 1
#define CONFIG_ZPL_WORKQUEUE 1
#define CONFIG_ZPL_SHELL_MODULE 1
#undef CONFIG_ZPL_SHRL_MODULE
#define CONFIG_ZPL_ACTIVE_STANDBY 1
#undef CONFIG_ZPL_NSM_SNMP
#define CONFIG_ZPL_NSM_MODULE 1
#define CONFIG_ZPL_VRF_MODULE 1
#define CONFIG_ZPL_KEYCHAIN 1
#define CONFIG_ZPL_DISTRIBUTE 1
#define CONFIG_ZPL_NSM_L3MODULE 1
#define CONFIG_ZPL_NSM_NEXTHOP 1
#define CONFIG_ZPL_NSM_ROUTEMAP 1
#define CONFIG_ZPL_NSM_8021X 1
#define CONFIG_ZPL_NSM_ARP 1
#undef CONFIG_ZPL_NSM_DHCP
#undef CONFIG_ZPL_DHCPS_MODULE
#undef CONFIG_ZPL_DHCPC_MODULE
#undef CONFIG_ZPL_DHCPR_MODULE
#undef CONFIG_ZPL_NSM_DNS
#define CONFIG_ZPL_NSM_DOS 1
#undef CONFIG_ZPL_NSM_FIREWALLD
#define CONFIG_ZPL_NSM_MAC 1
#define CONFIG_ZPL_NSM_MIRROR 1
#define CONFIG_ZPL_NSM_PPP 1
#define CONFIG_ZPL_NSM_QOS 1
#define CONFIG_ZPL_NSM_SERIAL 1
#define CONFIG_ZPL_NSM_TRUNK 1
#define CONFIG_ZPL_NSM_TUNNEL 1
#define CONFIG_ZPL_NSM_VLAN 1
#define CONFIG_ZPL_NSM_IGMP 1
#define CONFIG_ZPL_NSM_MSTP 1
#define CONFIG_ZPL_NSM_PORT 1
#define CONFIG_ZPL_NSM_VLANETH 1
#undef CONFIG_ZPL_NSM_WIRELESS
#define CONFIG_ZPL_NSM_BRIDGE 1
#define CONFIG_ZPL_NSM_SECURITY 1
#define CONFIG_ZPL_NSM_RTADV 1
#define CONFIG_ZPL_NSM_IRDP 1
#define CONFIG_ZPL_IPCBC_MODULE 1

/*
 * Component Module Config
 */
#define CONFIG_ZPL_COMPONENT_MODULE 1
#undef CONFIG_ZPL_MODEM_MODULE
#undef CONFIG_ZPL_MQTT_MODULE
#undef CONFIG_ZPL_MQTT_SSL
#undef CONFIG_ZPL_MQTT_SRV
#define CONFIG_ZPL_PJSIP_MODULE 1
#undef CONFIG_ZPL_PJSIP_SRTP
#define CONFIG_ZPL_PJSIP_VIDEO 1
#undef CONFIG_ZPL_PJSIP_VIDEO_V4L2
#define CONFIG_ZPL_PJSIP_ALSA 1
#define CONFIG_ZPL_PJSIP_PJSUA2 1
#undef CONFIG_ZPL_SQLITE_MODULE
#undef CONFIG_ZPL_SQLITE_EXE_MODULE
#undef CONFIG_ZPL_LIBSSH_MODULE
#undef CONFIG_ZPL_LIBSSH_NACL
#undef CONFIG_ZPL_LIBSSH_ZLIB
#undef CONFIG_ZPL_LIBSSH_SFTP
#undef CONFIG_ZPL_LIBSSH_GCRYPT
#undef CONFIG_ZPL_LIBSSH_MBEDTLS
#undef CONFIG_ZPL_LIBSSH_CRYPTO
#undef CONFIG_ZPL_LIBSSH_OPENSSL_ED25519
#undef CONFIG_ZPL_LIBSSH_BLOWFISH
#undef CONFIG_ZPL_LIBSSH_SERVER
#undef CONFIG_ZPL_LIBSSH_PTHREAD
#undef CONFIG_ZPL_LIBSSH_GSSAPI
#undef CONFIG_ZPL_LIBSSH_GEX
#undef CONFIG_ZPL_LIBSSH_PCAP
#undef CONFIG_ZPL_WEBSERVER_MODULE
#undef CONFIG_ZPL_WEBSERVER_NONESSL
#undef CONFIG_ZPL_WEBSERVER_OPENSSL
#undef CONFIG_ZPL_WEBSERVER_MATRIXSSL
#undef CONFIG_ZPL_WEBSERVER_MBEDTLS
#undef CONFIG_ZPL_WEBSERVER_NANOSSL
#undef CONFIG_ZPL_WEBAPP_MODULE
#undef CONFIG_ZPL_WIFI_MODULE
#undef CONFIG_ZPL_MODBUS_MODULE
#undef CONFIG_ZPL_ONVIF_MODULE
#undef CONFIG_ZPL_ONVIF_SSL

/*
 * HAL/PAL Module Config
 */
#define CONFIG_ZPL_HALPAL_MODULE 1
#define CONFIG_ZPL_HAL_MODULE 1
#define CONFIG_ZPL_PAL_MODULE 1
#undef CONFIG_ZPL_IPCOM_MODULE
#define CONFIG_ZPL_KERNEL_MODULE 1
#define CONFIG_ZPL_KERNEL_NETLINK 1
#define CONFIG_ZPL_IPCOM_ROOT_PATH ""

/*
 * Service Module Config
 */
#define CONFIG_ZPL_SERVICE_MODULE 1
#define CONFIG_ZPL_SERVICE_SNTPC 1
#define CONFIG_ZPL_SERVICE_SNTPS 1
#define CONFIG_ZPL_SERVICE_SYSLOG 1
#define CONFIG_ZPL_SERVICE_FTPC 1
#define CONFIG_ZPL_SERVICE_FTPD 1
#define CONFIG_ZPL_SERVICE_TFTPC 1
#define CONFIG_ZPL_SERVICE_TFTPD 1
#define CONFIG_ZPL_SERVICE_PING 1
#define CONFIG_ZPL_SERVICE_TELNET 1
#undef CONFIG_ZPL_SERVICE_TELENTD
#define CONFIG_ZPL_SERVICE_TRACEROUTE 1
#define CONFIG_ZPL_SERVICE_UBUS_SYNC 1

/*
 * Multimedia Module Config
 */
#define CONFIG_ZPL_MULTIMEDIA_MODULE 1
#define CONFIG_ZPL_LIBRTSP_MODULE 1
#define CONFIG_ZPL_JRTPLIB_MODULE 1
#undef CONFIG_ZPL_EXOSIP_MODULE
#define CONFIG_ZPL_LIBRTMP_MODULE 1
#define CONFIG_ZPL_LIBJPEG_MODULE 1
#define CONFIG_ZPL_LIBMEDIA_MODULE 1
#undef CONFIG_ZPL_HISIMPP_MODULE
#undef CONFIG_ZPL_HISIMPP_HWDEBUG
#undef CONFIG_ZPL_LIVE555_MODULE
#undef CONFIG_ZPL_SOFTCODEC_MODULE
#undef CONFIG_ZPL_LIBX264_MODULE
#undef CONFIG_ZPL_OPENH264_MODULE
#undef CONFIG_ZPL_LIBVPX_MODULE
#undef CONFIG_ZPL_FFMPEG_MODULE

/*
 * BSP Module Config
 */
#define CONFIG_ZPL_BSP_MODULE 1
#define CONFIG_ZPL_SDK_MODULE 1
#define CONFIG_ZPL_SDK_NONE 1
#undef CONFIG_ZPL_SDK_USER
#undef CONFIG_ZPL_SDK_KERNEL
#undef CONFIG_ZPL_SDK_BCM53125

/*
 * Externsions Module Config
 */
#define CONFIG_ZPL_EXTERNSIONS_MODULE 1
#undef CONFIG_ZPL_ZLIB_MODULE
#undef CONFIG_ZPL_EXZLIB_MODULE
#undef CONFIG_ZPL_OPENSSL_MODULE
#define CONFIG_ZPL_MBEDTLS_MODULE 1
#define CONFIG_ZPL_FREETYPE_MODULE 1
#undef CONFIG_ZPL_EXFREETYPE_MODULE
#define CONFIG_ZPL_EXFREETYPE_INC_PATH ""
#define CONFIG_ZPL_EXFREETYPE_LIB_PATH ""
#undef CONFIG_ZPL_READLINE_MODULE
#undef CONFIG_ZPL_LIBNL_MODULE

/*
 * Application Module Config
 */
#undef CONFIG_ZPL_APPLICATION_MODULE
#undef CONFIG_ZPL_APP_X5_MODULE
#undef CONFIG_ZPL_APP_V9_MODULE

/*
 * Tools Module Config
 */
#undef CONFIG_ZPL_TOOLS_MODULE
#undef CONFIG_ZPL_TOOLS_PROCESS
#undef CONFIG_ZPL_TOOLS_QUECTEL_CM
#undef CONFIG_ZPL_TOOLS_SYSTEM
#undef CONFIG_ZPL_VTYSH_MODULE
#undef CONFIG_ZPL_WATCHDOG_MODULE
#undef CONFIG_ZPL_SWCONFIG_MODULE
