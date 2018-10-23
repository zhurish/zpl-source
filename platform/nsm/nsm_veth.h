/*
 * nsm_veth.h
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */

#ifndef __NSM_VETH_H__
#define __NSM_VETH_H__

#define SKB_QOS_MAX	8

typedef struct nsm_veth_s
{
	struct interface *ifp;
	int fd;

	struct interface *root;	//just for sub interface
	int vlanid;//vlan id
	int oldvlanid;//vlan id
	int egress_vlan_qos[SKB_QOS_MAX];//出口skb的优先级到qos的映射
	int ingress_vlan_qos[SKB_QOS_MAX];//入口skb的优先级到qos的映射

	BOOL	active;
} nsm_veth_t;


extern nsm_veth_t * nsm_veth_get(struct interface *ifp);
extern int nsm_veth_client_init();
extern int nsm_veth_client_exit();

extern int nsm_veth_interface_vid_set_api(struct interface *ifp, int vlan);


#endif /* __NSM_VETH_H__ */
