/*
 * host.h
 *
 *  Created on: Jan 1, 2018
 *      Author: zhurish
 */

#ifndef PLATFORM_LIB_HOST_H_
#define PLATFORM_LIB_HOST_H_


/* Host configuration variable */
struct host
{
  /* Host name of this router. */
  char *name;
  char *description;
  struct list *userlist;

  /* System wide terminal lines. */
  int lines;
  /* Log filename. */
  char *logfile;
  /* config file name of this host */
  char *config;
  char *default_config;
  char *factory_config;
  /* Flags for services */

  int encrypt;
  /* Banner configuration. */
  const char *motd;
  char *motdfile;

  unsigned long vty_timeout_val;
  /* Vty access-class command */
  char *vty_accesslist_name;
  /* Vty access-calss for IPv6. */
  char *vty_ipv6_accesslist_name;
  /* VTY server thread. */
  vector Vvty_serv_thread;
  /* Current directory. */
  char *vty_cwd;
  /* Configure lock. */
  int vty_config;
  /* Login password check. */
  int no_password_check;
  enum{LOAD_NONE, LOAD_INIT, LOADING, LOAD_DONE} load;
  void *mutx;
  void *cli_mutx;
};


/* struct host global, ick */
extern struct host host;

extern int host_config_init(char *motd);
extern int host_config_exit(void);
extern const char *host_config_get (void);
extern void host_config_set (char *);

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
};


extern int host_config_set_api (int cmd, void *pVoid);
extern int host_config_get_api (int cmd, void *pVoid);


#endif /* PLATFORM_LIB_HOST_H_ */
