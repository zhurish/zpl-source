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



struct startup_option
{
	char *progname;
	char *config_file;
	char *service_file;
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
extern int zpl_base_signal_reload(void);
extern int startup_option_default(void);
extern int zplmain_getopt(int argc, char **argv);
extern int os_test(void);
#ifdef __cplusplus
}
#endif


#endif /* __STARTUP_START_H__ */
