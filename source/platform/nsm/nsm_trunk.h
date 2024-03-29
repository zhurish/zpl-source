/*
 * nsm_trunk.h
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */

#ifndef __NSM_TRUNK_H__
#define __NSM_TRUNK_H__


#ifdef __cplusplus
extern "C" {
#endif
#include "os_list.h"
#ifdef ZPL_SHELL_MODULE
#include "vty.h"
#endif
#include "if.h"
#define NSM_TRUNK_ID_MAX	2
#define NSM_TRUNK_MEMBER_MAX	4



#define LACP_PORT_PRIORITY_DEFAULT		32768
#define LACP_SYSTEM_PRIORITY_DEFAULT	32768

#define LOAD_BALANCE_DEFAULT	TRUNK_LOAD_BALANCE_DSTSRCMAC

#define LACP_TIMEOUT_SHORT	1
#define LACP_TIMEOUT_LONG	2
#define LACP_TIMEOUT_DEFAULT	LACP_TIMEOUT_SHORT

typedef enum trunk_type_s
{
	TRUNK_STATIC = 1,
	TRUNK_DYNAMIC,
}trunk_type_t;

typedef enum trunk_mode_s
{
	TRUNK_ACTIVE = 1,
	TRUNK_PASSIVE,
}trunk_mode_t;

typedef enum load_balance_s
{
	TRUNK_LOAD_BALANCE_NONE = 0,
	TRUNK_LOAD_BALANCE_DSTMAC,
	TRUNK_LOAD_BALANCE_SRCMAC,
	TRUNK_LOAD_BALANCE_DSTSRCMAC,
	TRUNK_LOAD_BALANCE_DSTIP,
	TRUNK_LOAD_BALANCE_SRCIP,
	TRUNK_LOAD_BALANCE_DSTSRCIP,
}load_balance_t;

struct l2trunk_group_s;
struct Gl2trunk_s;

typedef struct l2trunk_member_s
{
	NODE			node;
	zpl_uint32			trunkId;

	trunk_type_t	type;
	trunk_mode_t	mode;
	zpl_uint32			lacp_port_priority;
	zpl_uint32			lacp_timeout;

	ifindex_t 		ifindex;

	struct l2trunk_group_s *group;
}l2trunk_member_t;// port-channel member interface

typedef struct l2trunk_group_s
{
	LIST				*trunkList;
	zpl_uint32				trunkId;
	trunk_type_t		type;
	zpl_uint32				lacp_system_priority;
	load_balance_t		load_balance;
	struct Gl2trunk_s 	*global;

	struct interface	*ifp;
}l2trunk_group_t;// port-channel interface

typedef struct Gl2trunk_s
{
	//l2trunk_group_t	*group[NSM_TRUNK_ID_MAX];
	l2trunk_group_t	*group;
	//zpl_uint32			lacp_system_priority;
	//load_balance_t	load_balance;
	void		*mutex;
	zpl_bool		enable;
}Gl2trunk_t;


typedef int (*l2trunk_group_cb)(l2trunk_group_t *, void *);
typedef int (*l2trunk_member_cb)(l2trunk_member_t *, void *);

extern int nsm_trunk_init(void);
extern int nsm_trunk_exit(void);

extern int nsm_trunk_enable(void);
extern zpl_bool nsm_trunk_is_enable(void);
/* 创建/删除汇聚接口 */
extern int nsm_trunk_interface_create_api(struct interface *ifp);
extern int nsm_trunk_interface_del_api(struct interface *ifp);
/* 获取汇聚接口的信息 */
extern l2trunk_group_t * nsm_port_channel_get(struct interface *ifp);

/* 查询汇聚组是否存在 */
extern zpl_bool l2trunk_lookup_api(zpl_uint32 trunkid);
/* 获取汇聚祖的接口数量 */
extern int l2trunk_lookup_interface_count_api(zpl_uint32 trunkid);

/* 创建/删除汇聚组 */
extern int nsm_trunk_create_api(zpl_uint32 trunkid, trunk_type_t type);
extern int nsm_trunk_destroy_api(zpl_uint32 trunkid);

/* 查询该接口是否在汇聚接口下 */
extern zpl_bool l2trunk_lookup_interface_api(ifindex_t ifindex);
/* 获取该接口所在的汇聚组ID */
extern int nsm_trunk_get_ID_interface_api(ifindex_t ifindex, zpl_uint32 *trunkId);

/* 把接口加入trunk组，从trunk组删除 */
extern int nsm_trunk_add_interface_api(zpl_uint32 trunkid, trunk_type_t type, trunk_mode_t mode, struct interface *ifp);
extern int nsm_trunk_del_interface_api(zpl_uint32 trunkid, struct interface *ifp);
extern int nsm_trunk_lacp_port_priority_api(ifindex_t ifindex, zpl_uint32 pri);
extern int nsm_trunk_lacp_timeout_api(ifindex_t ifindex, zpl_uint32 timeout);

/* 汇聚组 优先级和负载分担摸索 */
extern int nsm_trunk_load_balance_api(zpl_uint32 trunkid, load_balance_t mode);
extern int nsm_trunk_lacp_system_priority_api(zpl_uint32 trunkid, zpl_uint32 pri);

extern int nsm_trunk_group_callback_api(l2trunk_group_cb cb, void *pVoid);
extern int nsm_trunk_member_callback_api(l2trunk_member_cb cb, void *pVoid);

#ifdef ZPL_SHELL_MODULE
extern void cmd_trunk_init(void);
extern int nsm_trunk_interface_write_config(struct vty *vty, struct interface *ifp);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NSM_TRUNK_H__ */
