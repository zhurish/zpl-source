/*
 * config_env.h
 *
 *  Created on: Oct 12, 2018
 *      Author: zhurish
 */

#ifndef INCLUDE_CONFIG_ENV_H_
#define INCLUDE_CONFIG_ENV_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define ZPL_KERNEL_STACK_MODULE
	//ZPL_KERNEL_STACK_MODULE
#undef BASE_DIR         //运行时缓存目录
#undef SYS_REAL_DIR     //flash存储目录

#ifndef BASE_DIR
#define BASE_DIR	"/tmp/app"
#endif

#ifdef ZPL_BUILD_ARCH_X86
#ifndef SYS_REAL_DIR
#define SYS_REAL_DIR		"/home/zhurish/workspace/zpl-source/debug"
#undef BASE_DIR
#define BASE_DIR	SYS_REAL_DIR"/tmp/app"
#endif
#define SYSCONF_REAL_DIR 	SYS_REAL_DIR "/etc"
#else
#ifdef ZPL_BUILD_OS_OPENWRT
#ifndef SYS_REAL_DIR
#define SYS_REAL_DIR		"/app"
#endif
#define SYSCONF_REAL_DIR 	SYS_REAL_DIR "/etc"
#else
#ifndef SYS_REAL_DIR
#define SYS_REAL_DIR		"/home/app"
#endif
#define SYSCONF_REAL_DIR 	SYS_REAL_DIR "/etc"
#endif
#endif

//#ifdef ZPL_BUILD_OS_OPENWRT
//#define SYSCONFDIR 		"/etc/app"
//#else
//#define SYSCONFDIR 		BASE_DIR "/etc"
#define SYSCONFDIR 		SYSCONF_REAL_DIR	//flash的etc 目录
#define RSYSLOGDIR 		SYS_REAL_DIR "/log"	//flash的log 目录
#define RSYSWWWDIR 		SYS_REAL_DIR "/www"	//flash的www 目录
#define SYSWEBDIR 		SYSCONF_REAL_DIR "/web"	//flash的/etc/web 目录
//#endif

#define PLSYSCONFDIR 	BASE_DIR "/etc"	//缓存目录
#define SYSLIBDIR 		BASE_DIR "/lib"
#define SYSSBINDIR 		BASE_DIR "/sbin"
#define SYSBINDIR 		BASE_DIR "/bin"
#define SYSRUNDIR 		BASE_DIR "/run"
#define SYSLOGDIR 		BASE_DIR "/log"
#define SYSVARDIR 		BASE_DIR "/var"
#define SYSTMPDIR 		BASE_DIR "/tmp"
#define SYSWWWDIR 		BASE_DIR "/www"
#define SYSWWWCACHEDIR 		BASE_DIR "/www/cache"
#define SYSTFTPBOOTDIR 		BASE_DIR "/tftpboot"
#define SYSUPLOADDIR 		SYSTFTPBOOTDIR


/* default daemon app root dir */
#define DAEMON_CONFIG_DIR SYSCONFDIR

/* default daemon logmsg directory */
#define DAEMON_LOG_FILE_DIR SYSLOGDIR

/* daemon vty directory */
#define DAEMON_VTY_DIR SYSVARDIR

/* daemon vty directory */
#define DAEMON_ENV_DIR SYSRUNDIR


#define CONF_BACKUP_EXT ".sav"

#ifdef ZPL_BUILD_OS_OPENWRT
#define STARTUP_CONFIG_FILE	SYSCONFDIR "/startup-config.cfg"
#define DEFAULT_CONFIG_FILE	SYSCONFDIR "/default-config.cfg"
#define FACTORY_CONFIG_FILE	SYSCONFDIR "/factory-config.cfg"
#else
#ifdef ZPL_BUILD_ARCH_X86
#define STARTUP_CONFIG_FILE	PLSYSCONFDIR "/startup-config.cfg"
#define DEFAULT_CONFIG_FILE	PLSYSCONFDIR "/default-config.cfg"
#define FACTORY_CONFIG_FILE	PLSYSCONFDIR "/factory-config.cfg"
#else
#define STARTUP_CONFIG_FILE	SYSCONFDIR "/startup-config.cfg"
#define DEFAULT_CONFIG_FILE	SYSCONFDIR "/default-config.cfg"
#define FACTORY_CONFIG_FILE	SYSCONFDIR "/factory-config.cfg"
#endif
#endif







/* bgpd vty socket */
#define BGP_VTYSH_PATH DAEMON_VTY_DIR "/bgpd.sock"

/* frpd vty socket */
#define FRP_VTYSH_PATH DAEMON_VTY_DIR "/frpd.sock"

/* IMI Mdoule socket */
#define IMISH_UNIX_PATH DAEMON_VTY_DIR "/imi-cli.sock"
/* isisd vty socket */
#define ISIS_VTYSH_PATH DAEMON_VTY_DIR "/isisd.sock"
/* ldpd vty socket */
#define LDP_VTYSH_PATH DAEMON_VTY_DIR "/ldpd.sock"
/* lldpd vty socket */
#define LLDP_VTYSH_PATH DAEMON_VTY_DIR "/lldpd.sock"
/* olsrd vty socket */
#define OLSR_VTYSH_PATH DAEMON_VTY_DIR "/olsrd.sock"
/* nhrpd vty socket */
#define NHRP_VTYSH_PATH DAEMON_VTY_DIR "/nhrpd.sock"
/* ospf6d vty socket */
#define OSPF6_VTYSH_PATH DAEMON_VTY_DIR "/ospf6d.sock"
/* ospfd vty socket */
#define OSPF_VTYSH_PATH DAEMON_VTY_DIR "/ospfd.sock"
/* pimd vty socket */
#define PIM_VTYSH_PATH DAEMON_VTY_DIR "/pimd.sock"
/* ripng vty socket */
#define RIPNG_VTYSH_PATH DAEMON_VTY_DIR "/ripngd.sock"
/* rip vty socket */
#define RIP_VTYSH_PATH DAEMON_VTY_DIR "/ripd.sock"
/* utils vty socket */
#define UTILS_VTYSH_PATH DAEMON_VTY_DIR "/utils.sock"
/* vrrpd vty socket */
#define VRRP_VTYSH_PATH DAEMON_VTY_DIR "/vrrpd.sock"
/* zebra vty socket */
#define ZEBRA_VTYSH_PATH DAEMON_VTY_DIR "/zebra.sock"





/* bfdd PID */
/* #undef PATH_BFDD_PID */

/* bgpd PID */
#define PATH_BGPD_PID DAEMON_VTY_DIR "/bgpd.pid"

/* frpd PID */
#define PATH_FRPD_PID DAEMON_VTY_DIR "/frpd.pid"

/* IMI Mdoule PID */
#define PATH_IMISH_PID DAEMON_VTY_DIR "/imi-cli.pid"

/* isisd PID */
#define PATH_ISISD_PID DAEMON_VTY_DIR "/isisd.pid"

/* ldpd PID */
#define PATH_LDPD_PID DAEMON_VTY_DIR "/ldpd.pid"

/* lldpd PID */
#define PATH_LLDPD_PID DAEMON_VTY_DIR "/lldpd.pid"

/* nhrpd PID */
#define PATH_NHRPD_PID DAEMON_VTY_DIR "/nhrpd.pid"

/* olsrd PID */
#define PATH_OLSRD_PID DAEMON_VTY_DIR "/olsrd.pid"

/* ospf6d PID */
#define PATH_OSPF6D_PID DAEMON_VTY_DIR "/ospf6d.pid"

/* ospfd PID */
#define PATH_OSPFD_PID DAEMON_VTY_DIR "/ospfd.pid"

/* pimd PID */
#define PATH_PIMD_PID DAEMON_VTY_DIR "/pimd.pid"

/* ripd PID */
#define PATH_RIPD_PID DAEMON_VTY_DIR "/ripd.pid"

/* ripngd PID */
#define PATH_RIPNGD_PID DAEMON_VTY_DIR "/ripngd.pid"

/* utils PID */
#define PATH_UTILS_PID DAEMON_VTY_DIR "/utils.pid"

/* vrrpd PID */
#define PATH_VRRPD_PID DAEMON_VTY_DIR "/vrrpd.pid"

/* watchquagga PID */
#define PATH_WATCHQUAGGA_PID DAEMON_VTY_DIR "/watchquagga.pid"

/* zebra PID */
#define PATH_ZEBRA_PID DAEMON_VTY_DIR "/zebra.pid"






/* zebra api socket */
#define ZEBRA_SERV_PATH DAEMON_VTY_DIR "/zserv.api"

/* default oem file */
#define HOST_DEFAULT_OEM_FILE SYSCONFDIR "/oem.bin"


/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
//#define LT_OBJDIR ".libs/"


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CONFIG_ENV_H_ */
