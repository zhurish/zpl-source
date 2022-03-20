/*
 * startup_start.h
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */

#ifndef __STARTUP_START_H__
#define __STARTUP_START_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OS_START_TEST

#define ZEBRA_VTY_PORT 2610

struct startup_option
{
	char *progname;
	char *config_file;
	char *pid_file;
	char *vty_addr;
	char *zserv_path;

	int vty_port;
	int daemon_mode;
	char *user;
	char *group;
	char *tty;
	/* process id. */
	pid_t pid;
};

extern struct startup_option startup_option;


extern int zpl_base_signal_init(int daemon_mode);

extern int startup_option_default(void);


#ifdef __cplusplus
}
#endif


#endif /* __STARTUP_START_H__ */
