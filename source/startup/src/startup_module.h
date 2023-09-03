/*
 * startup_module.h
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */

#ifndef __STARTUP_MODULE_H__
#define __STARTUP_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

int zpl_stack_init(char *cfg);
int zpl_stack_start(const char* progname, int localport);
int startup_module_init(int console_enable);
int startup_module_load(void);
int startup_module_waitting(void);
int startup_module_stop(void);
int startup_module_exit(void);

int zpl_base_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *tty);
int zpl_base_start_pid(int pro, char *pid_file, int *pid);


#ifdef __cplusplus
}
#endif

#endif /* __STARTUP_MODULE_H__ */
