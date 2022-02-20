/*
 * module.h
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */

#ifndef __LIB_MODULE_TABLE_H__
#define __LIB_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_include.h"
#include "zpl_include.h"
#include "moduletypes.h"



extern struct module_alllist module_lists_tbl[MODULE_MAX];

extern int pl_module_name_show(void);

extern int pl_module_allinit(void);
extern int pl_module_allexit(void);
extern int pl_module_task_allinit(void);
extern int pl_module_task_allexit(void);
extern int pl_module_cmd_allinit(void);

extern int pl_module_name_init(const char * name);
extern int pl_module_init(zpl_uint32 module);
extern int pl_module_exit(zpl_uint32 module);
extern int pl_module_task_name_init(const char * name);
extern int pl_module_task_init(zpl_uint32 module);
extern int pl_module_task_exit(zpl_uint32 module);
extern int pl_module_cmd_name_init(const char * name);
extern int pl_module_cmd_init(zpl_uint32 module);
extern struct module_list * pl_module_info(zpl_uint32 module);

extern int submodule_setup(zpl_uint32 module, zpl_uint32 submodule, char *name, zpl_uint32 taskid);


 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_MODULE_TABLE_H__ */
