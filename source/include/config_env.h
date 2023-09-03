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


#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
#ifndef BASE_DIR
#define BASE_DIR	      "/home/zhurish/workspace/working/zpl-source/source/debug/tmp/app"
#endif
#ifndef SYS_REAL_DIR
#define SYS_REAL_DIR		"/home/zhurish/workspace/working/zpl-source/source/debug"
#endif
#else //
#ifndef BASE_DIR
#define BASE_DIR	      "/tmp/app"
#endif
#ifndef SYS_REAL_DIR
#define SYS_REAL_DIR		"/home/app"
#endif
#endif



#define REAL_SYSCONFDIR 		SYS_REAL_DIR "/etc"     //flash的etc 目录
#define REAL_SYSLOGDIR 		   SYS_REAL_DIR "/log"	   //flash的log 目录
#define REAL_SYSWWWDIR 		   SYS_REAL_DIR "/www"	   //flash的www 目录
#define REAL_SYSWEBDIR 		   SYS_REAL_DIR "/etc/web"	//flash的/etc/web 目录
/* 运行时配置文件目录 */
#define SYSCONF_REAL_DIR 	REAL_SYSCONFDIR


#define SYSCONFDIR 	   BASE_DIR "/etc"	//缓存目录
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

#define STARTUP_CONFIG_FILE	SYSCONFDIR "/startup-config.cfg"
#define DEFAULT_CONFIG_FILE	SYSCONFDIR "/default-config.cfg"
#define FACTORY_CONFIG_FILE	SYSCONFDIR "/factory-config.cfg"
#define SERVICE_CONFIG_FILE	SYSCONFDIR "/os_netservice.cfg"





#define PLCLI_VTY_PORT 2610




/* bgpd vty socket */
#define SHELL_SOCKET_PATH  DAEMON_VTY_DIR "/bgpd.sock"

#define NSM_SERV_PATH      DAEMON_VTY_DIR "/zserv.sock"


#define PL_PID DAEMON_VTY_DIR "/bgpd.pid"

/* zebra api socket */


/* default oem file */
#define HOST_DEFAULT_OEM_FILE SYSCONFDIR "/oem.bin"


/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
//#define LT_OBJDIR ".libs/"


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CONFIG_ENV_H_ */
