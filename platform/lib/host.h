/*
 * host.h
 *
 *  Created on: Jan 1, 2018
 *      Author: zhurish
 */

#ifndef PLATFORM_LIB_HOST_H_
#define PLATFORM_LIB_HOST_H_


#ifdef __cplusplus
extern "C" {
#endif

/* Host configuration variable */
struct host
{
  /* Host name of this router. */
  ospl_char *name;
  ospl_char *description;
  struct list *userlist;

  /* System wide terminal lines. */
  ospl_uint32 lines;
  /* Log filename. */
  ospl_char *logfile;
  /* config file name of this host */
  ospl_char *config;
  ospl_char *default_config;
  ospl_char *factory_config;
  /* Flags for services */

  ospl_bool encrypt;
  /* Banner configuration. */
  const char *motd;
  ospl_char *motdfile;

  ospl_ulong vty_timeout_val;
  /* Vty access-class command */
  ospl_char *vty_accesslist_name;
  /* Vty access-calss for IPv6. */
  ospl_char *vty_ipv6_accesslist_name;
  /* VTY server thread. */
  vector Vvty_serv_thread;
  /* Current directory. */
  ospl_char *vty_cwd;
  /* Configure lock. */
  ospl_bool vty_config;
  /* Login password check. */
  ospl_bool no_password_check;
  enum{LOAD_NONE, LOAD_INIT, LOADING, LOAD_DONE} load;
  void *mutx;
  void *cli_mutx;

  ospl_int8	serial[64];

  ospl_uint8	sysmac[6];
};

struct host_system
{
	ospl_uint8		process;
	ospl_double		freq;
#ifdef PL_BUILD_X86
	ospl_char		*model_name;
#else
#ifdef PL_BUILD_MIPS
//#error "aaaaaaaaaaaaaaaaaaaa"
#endif
	ospl_char		*system_type;
	ospl_char		*cpu_model;
	ospl_char		*ase;
#endif
	ospl_uint32		mem_total;
	ospl_uint32		mem_free;
	ospl_uint32		mem_uses;
	struct sysinfo s_info;
};
/* struct host global, ick */
extern struct host host;
extern int host_sysconfig_sync();
extern int host_config_init(ospl_char *motd);
extern int host_config_exit(void);
extern const char *host_config_get (void);
extern void host_config_set (ospl_char *);

const char * host_name_get (void);

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


extern int host_config_set_api (ospl_uint32 cmd, void *pVoid);
extern int host_config_get_api (ospl_uint32 cmd, void *pVoid);

extern int cmd_host_init(ospl_bool terminal);

extern int host_system_information_get(struct host_system *host_system);
extern int show_host_system_information(struct host_system *host_system, struct vty *vty);
 
#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_LIB_HOST_H_ */
