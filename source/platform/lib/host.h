/*
 * host.h
 *
 *  Created on: Jan 1, 2018
 *      Author: zhurish
 */

#ifndef __LIB_HOST_H__
#define __LIB_HOST_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "hash.h"
#include "prefix.h"
#ifdef ZPL_SHELL_MODULE
#include "vty.h"
#endif

#define IPMC_SLOT_MAX	32

struct ipmc_slot
{
	zpl_uint8 slot;
	zpl_uint8 state;
	zpl_uint32	power;
	zpl_uint32	fan;
	zpl_uint32	temp;
};

/* Host configuration variable */
struct zpl_host
{
	/* Host name of this router. */
	zpl_char *name;
	zpl_char *description;
	struct list *userlist;

	/* System wide terminal lines. */
	zpl_int32 lines;
	/* Log filename. */
	zpl_char *logfile;
	/* config file name of this host */
	zpl_char *config;
	zpl_char *default_config;
	zpl_char *factory_config;
	/* Flags for services */

	zpl_bool encrypt;
	/* Banner configuration. */
	const char *motd;
	zpl_char *motdfile;

	zpl_ulong vty_timeout_val;
	/* Vty access-class command */
	zpl_char *vty_accesslist_name;
	/* Vty access-calss for IPv6. */
	zpl_char *vty_ipv6_accesslist_name;

	/* Current directory. */
	zpl_char *vty_cwd;
	/* Configure lock. */
	zpl_bool vty_config;
	/* Login password check. */
	zpl_bool no_password_check;
	zpl_bool console_enable;

	os_mutex_t *mutex;

	zpl_int8	serial[64];

	zpl_uint8	sysmac[6];

	struct ipmc_slot ipmctable[IPMC_SLOT_MAX];
	int slot;
#ifdef ZPL_ACTIVE_STANDBY
    zpl_bool preempt_mode;
    zpl_int32 switch_delay;
    zpl_bool active_standby; // 主:0;备:1
#endif
	zpl_bool control_access;	//控制板卡还是接入板卡
	
	void *bspinit_sem;

	enum{LOAD_NONE, LOAD_INIT, LOADING, LOAD_DONE} load;
};

struct host_system
{
	zpl_uint8		process;
	zpl_double		freq;
#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
	zpl_char		*model_name;
#else
#ifdef ZPL_BUILD_ARCH_MIPS
//#error "aaaaaaaaaaaaaaaaaaaa"
#endif
	zpl_char		*system_type;
	zpl_char		*cpu_model;
	zpl_char		*ase;
#endif
	zpl_uint32		mem_total;
	zpl_uint32		mem_free;
	zpl_uint32		mem_uses;
	struct sysinfo s_info;
};


/* struct host global, ick */
extern struct zpl_host _global_host;
extern const char *default_motd;

extern int host_sysconfig_sync(void);
extern int host_config_init(void);
extern int host_config_exit(void);
extern const char *host_config_get (void);
extern void host_config_set (zpl_char *);

const char * host_name_get (void);
#ifdef ZPL_ACTIVE_STANDBY
extern zpl_bool host_isstandby(void);
extern int host_standby(zpl_bool val);
extern zpl_bool host_isactive(void);
extern int host_active(zpl_bool val);
extern int host_preempt_mode(zpl_bool enable);
extern zpl_bool host_ispreempt_mode(void);
extern int host_switch_delay_set(zpl_int32 val);
extern int host_switch_delay_get(void);
#endif

extern zpl_bool host_is_access(void);//接入板卡
extern int host_access_set(zpl_bool val);
extern zpl_bool host_is_control(void);//控制板卡
extern int host_control_set(zpl_bool val);

extern int host_config_loading(char *config);
extern zpl_bool host_waitting_loadconfig(void);
extern zpl_bool host_loadconfig_done(void);
extern int host_loadconfig_state(int);

extern int host_waitting_bspinit(int);
extern int host_bspinit_done(void);

enum
{
	API_SET_NONE_CMD,
	API_SET_HOSTNAME_CMD,
	API_SET_DESC_CMD,
	API_SET_LINES_CMD,
	API_SET_LOGFILE_CMD,
	API_SET_CONFIGFILE_CMD,
	API_SET_ENCRYPT_CMD,
	API_SET_MOTD_CMD,
	API_SET_MOTDFILE_CMD,
	API_SET_VTY_TIMEOUT_CMD,
	API_SET_ACCESS_CMD,
	API_SET_IPV6ACCESS_CMD,
	API_SET_NOPASSCHK_CMD,
	API_SET_SYSMAC_CMD,
	API_SET_SERIAL_CMD,

	API_GET_HOSTNAME_CMD,
	API_GET_DESC_CMD,
	API_GET_LINES_CMD,
	API_GET_LOGFILE_CMD,
	API_GET_CONFIGFILE_CMD,
	API_GET_ENCRYPT_CMD,
	API_GET_MOTD_CMD,
	API_GET_MOTDFILE_CMD,
	API_GET_VTY_TIMEOUT_CMD,
	API_GET_ACCESS_CMD,
	API_GET_IPV6ACCESS_CMD,
	API_GET_NOPASSCHK_CMD,
	API_GET_SYSMAC_CMD,
	API_GET_SERIAL_CMD,
};


extern int host_config_set_api (zpl_uint32 cmd, void *pVoid);
extern int host_config_get_api (zpl_uint32 cmd, void *pVoid);

extern int cmd_host_init(zpl_bool terminal);

extern int host_system_information_get(struct host_system *host_system);
#ifdef ZPL_SHELL_MODULE
extern int show_host_system_information(struct host_system *host_system, struct vty *vty);
#endif 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_HOST_H__ */
