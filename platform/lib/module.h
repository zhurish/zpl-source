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

#include "os_include.h"
#include "zpl_include.h"
#include "moduletypes.h"

#define ZPL_MODULE_NEED_INIT  0x00000001
#define ZPL_MODULE_IS_INIT    0x00000100
#define ZPL_MODULE_INIT_TASK  0x00000200
#define ZPL_MODULE_INIT_CMD   0x00000400

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
#ifdef ZPL_SHELL_MODULE
  	int	(*module_write_config)(struct vty *, void *);
	int	(*module_show_config)(struct vty *, void *, zpl_bool);
	int	(*module_show_debug)(struct vty *, void *, zpl_bool);
#else
  	int	(*module_write_config)(void *, void *);
	int	(*module_show_config)(void *, void *, zpl_bool);
	int	(*module_show_debug)(void *, void *, zpl_bool);
#endif

  zpl_uint32		flags;//模块是否初始化标志
  zpl_void		  *master;    
  zpl_uint32		taskid;       //模块任务ID
  struct submodule
  {
    zpl_uint32 module;
	  char 	*name;
    zpl_uint32		taskid;
  }submodule[ZPL_SUB_MODULE_MAX];
};

struct module_table
{
	zpl_uint32 module;  //模块ID
	const char 	*name;  //模块名称  
	zpl_uint32	taskid;       //模块任务ID
};

struct module_alllist
{
  struct module_list *tbl;
};

extern struct module_table module_tbl[MODULE_MAX];

extern const char * module2name(zpl_uint32 module);//
extern zpl_uint32 name2module(const char *name);//
extern zpl_uint32 module2task(zpl_uint32 module);
extern zpl_uint32 task2module(zpl_uint32 taskid);
extern zpl_uint32 task_module_self(void);
extern int module_setup_task(zpl_uint32 module, zpl_uint32 taskid);//


 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_MODULE_H__ */
