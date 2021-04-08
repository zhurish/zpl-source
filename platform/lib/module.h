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

#include "ospl_type.h"
#include "moduletypes.h"
/*
typedef enum
{
  MODULE_NONE,
  MODULE_DEFAULT,		//Default
  MODULE_TIMER,		//Default
  MODULE_JOB,		//Default
  MODULE_CONSOLE,		//Console
  MODULE_TELNET,		//telnet
  MODULE_LIBSSH,		//telnet
  MODULE_NSM,			//route table manage
  MODULE_MODEM,			//MODEM
  MODULE_WIFI,			//wifi
  MODULE_DHCP,			//DHCP
  MODULE_DHCPD,			//DHCPD
  MODULE_RIP,
  MODULE_BGP,
  MODULE_OSPF,
  MODULE_RIPNG,
  MODULE_BABEL,
  MODULE_OSPF6,
  MODULE_ISIS,
  MODULE_PIM,
  MODULE_MASC,
  MODULE_NHRP,
  MODULE_HSLS,
  MODULE_OLSR,
  MODULE_VRRP,
  MODULE_FRP,
  MODULE_LLDP,
  MODULE_BFD,
  MODULE_LDP,
  MODULE_SNTP,
  MODULE_IMISH,
  MODULE_UTILS,
  MODULE_KERNEL,		//Kernel
  MODULE_VOIP,
  MODULE_APP_START,
  MODULE_APP_STOP = MODULE_APP_START + 16,
  MODULE_MAX,
} module_t;
*/

/* For pretty printing of memory allocate information. */
struct module_list
{
  ospl_uint32 module;
	const char 	*name;

  int	(*module_init)(void);
	int	(*module_exit)(void);
	int	(*module_task_init)(void);
	int	(*module_task_exit)(void);
	int	(*module_cmd_init)(void);

  int	(*module_write_config)(struct vty *, void *);
	int	(*module_show_config)(struct vty *, void *, ospl_bool);
	int	(*module_show_debug)(struct vty *, void *, ospl_bool);

	ospl_uint32		taskid;
};

struct module_alllist
{
  struct module_list *tbl;
};


extern struct module_alllist module_lists_tbl[MODULE_MAX];

extern int pl_module_name_show();
extern int pl_module_name_init(const char * name);
extern int pl_module_init(ospl_uint32 module);
extern int pl_module_exit(ospl_uint32 module);
extern int pl_module_task_name_init(const char * name);
extern int pl_module_task_init(ospl_uint32 module);
extern int pl_module_task_exit(ospl_uint32 module);
extern int pl_module_cmd_name_init(const char * name);
extern int pl_module_cmd_init(ospl_uint32 module);

extern const char * module2name(ospl_uint32 module);
extern ospl_uint32 name2module(const char *name);
extern ospl_uint32 module2task(ospl_uint32 module);
extern ospl_uint32 task2module(ospl_uint32 taskid);
extern ospl_uint32 task_module_self(void);
extern int module_setup_task(ospl_uint32 module, ospl_uint32 taskid);


extern ospl_char *zlog_backtrace_module();
extern ospl_char *zlog_backtrace_funcname();
extern ospl_char *zlog_backtrace_schedfrom();
extern ospl_uint32 zlog_backtrace_schedfrom_line();


 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_MODULE_H__ */
