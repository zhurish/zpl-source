/*
 * nsm_veth.h
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */

#ifndef __NSM_VETH_H__
#define __NSM_VETH_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SKB_QOS_MAX	8

typedef struct nsm_veth_s
{
	struct interface *ifp;
	int fd;

	struct interface *root;	//just for sub interface
	vlan_t vlanid;//vlan id
	vlan_t oldvlanid;//vlan id
	ospl_uint32 egress_vlan_qos[SKB_QOS_MAX];//出口skb的优先级到qos的映射
	ospl_uint32 ingress_vlan_qos[SKB_QOS_MAX];//入口skb的优先级到qos的映射

	ospl_bool	active;
} nsm_veth_t;


extern nsm_veth_t * nsm_veth_get(struct interface *ifp);
extern int nsm_veth_client_init();
extern int nsm_veth_client_exit();

extern int nsm_veth_interface_vid_set_api(struct interface *ifp, vlan_t vlan);

 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_VETH_H__ */
