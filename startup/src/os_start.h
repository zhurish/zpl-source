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

extern int os_base_init(void);
extern int os_base_load(void);

extern int os_start_init(char *progname, module_t pro, int daemon_mode, char *tty);
extern int os_ip_stack_init(int localport);
extern int os_start_early(module_t pro, char *logpipe);
//extern int os_default_start(zlog_proto_t pro);
extern int os_start_all_module();
extern int os_exit_all_module();
//extern int os_start_module (zlog_proto_t pro, char *config_file, void *argv);

extern int os_start_pid(int pro, char *pid_file, int *pid);

extern int os_load_config(char *config);
extern ospl_bool os_load_config_done(void);

extern int os_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *);

extern int os_start_running(void *master, module_t pro);

extern int eloop_start_running(void *master, module_t pro);

 
#ifdef __cplusplus
}
#endif


#endif /* OS_START_H_ */
