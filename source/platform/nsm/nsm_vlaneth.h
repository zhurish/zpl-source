/*
 * nsm_vlaneth.h
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */

#ifndef __NSM_VLANETH_H__
#define __NSM_VLANETH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "if.h"
#define NSM_VLAN_SUB
/* vlan 接口 或者是 vlan 子接口*/
#define SKB_QOS_MAX	8

typedef struct nsm_vlaneth_s
{
	struct interface *ifp;
	zpl_socket_t fd;

	vlan_t vlanid;//vlan id
	vlan_t oldvlanid;//vlan id
#ifdef NSM_VLAN_SUB
	struct interface *root;	//just for sub interface
	zpl_uint32 egress_vlan_qos[SKB_QOS_MAX];//出口skb的优先级到qos的映射
	zpl_uint32 ingress_vlan_qos[SKB_QOS_MAX];//入口skb的优先级到qos的映射
#endif
	zpl_bool	active;
} nsm_vlaneth_t;


extern nsm_vlaneth_t * nsm_vlaneth_get(struct interface *ifp);
extern int nsm_vlaneth_init(void);
extern int nsm_vlaneth_exit(void);
extern int nsm_vlaneth_interface_create_api(struct interface *ifp);
extern int nsm_vlaneth_interface_del_api(struct interface *ifp);

 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_VLANETH_H__ */
