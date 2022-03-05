/*
 * nsm_hook.h
 *
 *  Created on: 2019年7月30日
 *      Author: DELL
 */

#ifndef __NSM_HOOK_H__
#define __NSM_HOOK_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "if.h"
#include "prefix.h"
#include "nsm_vrf.h"

//接口创建删除的时候触发创建删除对应模块的数据结构
//接口UP/DOWN，设置删除IP的时候通知其他模块
//参数变化
typedef int (*nsm_intf_hook)(struct interface *);
typedef int (*nsm_intf_state_hook)(struct interface *, zpl_bool);
typedef int (*nsm_intf_show_hook)(struct interface *, struct vty *, zpl_bool);
typedef int (*nsm_intf_write_hook)(struct vty *, struct interface *, zpl_bool);


typedef struct nsm_hook_node
{
	nsm_intf_hook 	intf_add;
	nsm_intf_hook	intf_del;
	nsm_intf_state_hook intf_state_cb;
	nsm_intf_show_hook intf_show_cb;
	nsm_intf_write_hook intf_write_cb;
}nsm_hook_node_t;

struct nsm_hook_tbl
{
	nsm_hook_node_t	intf_hook_tbl[NSM_INTF_MAX];
};

int nsm_hook_module_init (void);
void nsm_hook_module_exit (void);
void * nsm_hook_lookup (nsm_hook_em type);

int nsm_hook_install (nsm_hook_em type, void *hook);
int nsm_hook_uninstall (nsm_hook_em type, void *hook);
int nsm_hook_execute (nsm_hook_em type, void *p1, void *p2, zpl_bool b);

 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_HOOK_H__ */
