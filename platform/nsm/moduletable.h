/*
 * module.h
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */

#ifndef __ZPLIB_MODULE_TABLE_H__
#define __ZPLIB_MODULE_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_include.h"
#include "zpl_include.h"
#include "moduletypes.h"



extern struct module_alllist module_lists_tbl[MODULE_MAX];

extern int zplib_module_name_show(void);

extern int zplib_module_initall(void);
extern int zplib_module_exitall(void);
extern int zplib_module_task_startall(void);
extern int zplib_module_task_stopall(void);
extern int zplib_module_cmd_all(void);

extern int zplib_module_name_init(const char * name);
extern int zplib_module_init(zpl_uint32 module);
extern int zplib_module_exit(zpl_uint32 module);
extern int zplib_module_task_name_init(const char * name);
extern int zplib_module_task_init(zpl_uint32 module);
extern int zplib_module_task_exit(zpl_uint32 module);
extern int zplib_module_cmd_name_init(const char * name);
extern int zplib_module_cmd_init(zpl_uint32 module);
extern struct module_list * zplib_module_info(zpl_uint32 module);

extern int submodule_setup(zpl_uint32 module, zpl_uint32 submodule, char *name, zpl_uint32 taskid);


 
#ifdef __cplusplus
}
#endif

#endif /* __ZPLIB_MODULE_TABLE_H__ */
