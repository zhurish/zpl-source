/*
 * nsm_hook.h
 *
 *  Created on: 2019年7月30日
 *      Author: DELL
 */

#ifndef __NSM_HOOK_H__
#define __NSM_HOOK_H__


#include "if.h"
#include "prefix.h"
#include "vrf.h"

#define NSM_HOOK_TBL_MAX	128 * 2

typedef enum
{
	NSM_HOOK_NONE = 0,
	NSM_HOOK_IFP_ADD,	//增加接口
	NSM_HOOK_IFP_DEL,	//删除接口
	NSM_HOOK_IFP_UP,	//接口UP/DOWN
	NSM_HOOK_IFP_DOWN,
	NSM_HOOK_IP_ADD,	//接口添加IP
	NSM_HOOK_IP_DEL,	//接口删除IP
	NSM_HOOK_IFP_CHANGE,//接口参数变化
	NSM_HOOK_IFP_SHOW,	//show 接口信息
	NSM_HOOK_IFP_CONFIG,//write config 接口信息
	NSM_HOOK_SERVICE,	//接口删除IP
	NSM_HOOK_DEBUG,//接口参数变化
	NSM_HOOK_MAX,
}nsm_hook_em;

//接口创建删除的时候触发创建删除对应模块的数据结构
//接口UP/DOWN，设置删除IP的时候通知其他模块
//参数变化
typedef int (*ifp_hook)(struct interface *, BOOL);
typedef int (*ifp_address_hook)(struct interface *, struct connected *, BOOL);
typedef int (*service_hook)(void *, BOOL);
typedef int (*debug_hook)(void *, BOOL);
typedef int (*ifp_show_hook)(struct interface *, struct vty *, BOOL);

typedef struct nsm_hook_node
{
	nsm_hook_em type;
	union
	{
		ifp_hook ifp_cb;
		ifp_hook ifp_state_cb;
		ifp_address_hook ifp_address_cb;
		ifp_hook ifp_change_cb;
		ifp_show_hook ifp_show_cb;
		ifp_show_hook ifp_write_cb;
		service_hook service_cb;
		debug_hook debug_cb;
	}hook;
}nsm_hook_node_t;

struct nsm_hook
{
	nsm_hook_node_t *ifplist;
	nsm_hook_node_t *ifp_state_list;
	nsm_hook_node_t *ifp_address_list;
	nsm_hook_node_t *ifp_change_list;
	nsm_hook_node_t *ifp_show_list;
	nsm_hook_node_t *ifp_write_list;
	nsm_hook_node_t *service_list;
	nsm_hook_node_t *debug_list;
};

int nsm_hook_module_init (void);
void nsm_hook_module_exit (void);
void * nsm_hook_lookup (nsm_hook_em type);

int nsm_hook_install (nsm_hook_em type, void *hook);
int nsm_hook_uninstall (nsm_hook_em type, void *hook);
int nsm_hook_execute (nsm_hook_em type, void *p1, void *p2, BOOL b);


#endif /* __NSM_HOOK_H__ */
