/*
 * os_module.h
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_MODULE_H_
#define __OS_MODULE_H_


extern int nsm_module_init ();
extern int nsm_task_init ();
extern int nsm_module_exit ();

extern int ospfd_module_init();
extern int ospfd_task_init ();
extern int ospfd_module_exit ();


extern int os_module_init(void);
extern int os_module_task_init(void);
extern int os_module_cmd_init(int terminal);


extern int os_module_exit(void);
extern int os_module_task_exit(void);
extern int os_module_cmd_exit(void);

#endif /* __OS_MODULE_H_ */
