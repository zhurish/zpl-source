/*
 * os_module.h
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_MODULE_H_
#define __OS_MODULE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PL_PJSIP_MODULE
extern int _pl_pjsip_module_init();
extern int _pl_pjsip_module_exit();
extern int _pl_pjsip_module_task_init();
extern int _pl_pjsip_module_task_exit();
#endif

extern int console_enable;


extern int ospfd_module_init();
extern int ospfd_task_init ();
extern int ospfd_module_exit ();


extern int os_module_init(void);
extern int os_module_task_init(void);
extern int os_module_cmd_init(int terminal);


extern int os_module_exit(void);
extern int os_module_task_exit(void);
extern int os_module_cmd_exit(void);
 
#ifdef __cplusplus
}
#endif

#endif /* __OS_MODULE_H_ */
