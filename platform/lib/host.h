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


extern int host_sysconfig_sync(void);
extern int host_config_init(zpl_char *motd);
extern int host_config_exit(void);
extern const char *host_config_get (void);
extern void host_config_set (zpl_char *);

const char * host_name_get (void);

extern int host_config_loading(char *config);
extern zpl_bool host_waitting_loadconfig(void);
extern zpl_bool host_loadconfig_done(void);
extern int host_loadconfig_stats(int);

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
