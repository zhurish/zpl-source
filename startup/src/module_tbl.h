/*
 * module_tbl.h
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_MODULE_TBL_H_
#define __OS_MODULE_TBL_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int pl_def_module_init();
extern int pl_def_module_exit();
extern int pl_def_module_task_init();
extern int pl_def_module_task_exit();
extern int pl_def_module_cmd_init();
 
#ifdef __cplusplus
}
#endif

#endif /* __OS_MODULE_TBL_H_ */