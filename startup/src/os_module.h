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

int os_module_init(void);
int os_module_exit(void);
int os_module_task_init(void);
int os_module_task_exit(void);
int os_module_cmd_init(void);
int os_module_cmd_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* __OS_MODULE_H_ */
