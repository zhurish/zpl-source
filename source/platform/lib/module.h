/*
 * module.h
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */

#ifndef __LIB_MODULE_H__
#define __LIB_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "route_types.h"
#include "nsm_event.h"
#include "moduletypes.h"


#define ZPL_MODULE_NODE(module, name, init, exit, taskinit, taskexit, cmdinit) \
struct module_list module ##_list = \
{ \
    .module = module, \
    .name = name, \
    .module_init = init, \
    .module_exit = exit, \
    .module_task_init = taskinit, \
    .module_task_exit = taskexit, \
    .module_cmd_init = cmdinit, \
    .flags = 0, \
    .master = NULL, \
    .taskid = 0, \
}; 

/* For pretty printing of memory allocate information. */
struct module_list
{
  	zpl_uint32 module;  //模块ID
	const char 	*name;  //模块名称    
  	int	(*module_init)(void);
	int	(*module_exit)(void);
	int	(*module_task_init)(void);
	int	(*module_task_exit)(void);
	int	(*module_cmd_init)(void);
	zpl_taskid_t	taskid;       //模块任务ID
	zpl_uint32		flags;
	zpl_void		*master;  
#define ZPL_MODULE_NEED_INIT	0x00000010	//需要手动初始化
#define ZPL_MODULE_IS_INIT  	0x00000100  //已经初始化
#define ZPL_MODULE_IS_TASK  	0x00000200	//初始化task
#define ZPL_MODULE_IS_CMD   	0x00000400	//初始化CLI

	int index_num;
	/*struct submodule
	{
		zpl_uint32 module;
		char 	*name;
		zpl_taskid_t		taskid;
	}submodule[ZPL_SUB_MODULE_MAX];*/
};

struct module_alllist
{
  struct module_list *tbl;
};

extern const char * module2name(zpl_uint32 module);//
extern zpl_uint32 name2module(const char *name);//
extern zpl_taskid_t module2task(zpl_uint32 module);
extern zpl_uint32 task2module(zpl_taskid_t taskid);
extern zpl_taskid_t task_module_self(void);
extern int module_setup_task(zpl_uint32 module, zpl_taskid_t taskid);//

extern int zplib_module_install(struct module_alllist *_m_table);
extern int zplib_module_name_show(void);

extern int zplib_module_initall(void);
extern int zplib_module_exitall(void);
extern int zplib_module_task_startall(void);
extern int zplib_module_task_stopall(void);
extern int zplib_module_cmd_all(void);

extern int zplib_module_init(zpl_uint32 module);
extern int zplib_module_exit(zpl_uint32 module);
extern int zplib_module_task_init(zpl_uint32 module);
extern int zplib_module_task_exit(zpl_uint32 module);  
extern int zplib_module_cmd_init(zpl_uint32 module);


extern int submodule_setup(zpl_uint32 module, zpl_uint32 submodule, char *name, zpl_taskid_t taskid);


 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_MODULE_H__ */
