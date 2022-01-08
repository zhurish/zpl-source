/*
 * os_start.h
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */

#ifndef OS_START_H_
#define OS_START_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OS_START_TEST

#define ZEBRA_VTY_PORT 2610

extern struct zebra_privs_t os_privs;

extern int os_base_env_init(void);
extern int os_base_env_load(void);
extern int os_base_signal_init(int daemon_mode);

extern int os_base_stack_init(const char *tty);
extern int os_base_zlog_open(char *progname);

extern int os_base_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *);
extern int os_base_start_pid(int pro, char *pid_file, int *pid);

extern int os_base_module_start_all(void);
extern int os_base_module_exit_all(void);

 
#ifdef __cplusplus
}
#endif


#endif /* OS_START_H_ */
