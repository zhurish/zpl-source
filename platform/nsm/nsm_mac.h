/*
 * nsm_mac.h
 *
 *  Created on: Jan 9, 2018
 *      Author: zhurish
 */

#ifndef __NSM_MAC_H__
#define __NSM_MAC_H__

#include "nsm_vlan.h"

typedef unsigned char mac_t;

#ifndef NSM_MAC_MAX
#define NSM_MAC_MAX	6
#endif

#define NSM_MAC_IS_MULTICAST(mac)                ((mac) & 0x01)
#define NSM_MAC_IS_BROADCAST(mac)                (((mac) & 0xFF)==0XFF)

typedef enum mac_action_s
{
	MAC_FORWARD = 1,
	MAC_DISCARDED,
}mac_action_t;

typedef enum mac_type_s
{
	MAC_STATIC = 1,
	MAC_DYNAMIC,

}mac_type_t;

typedef enum mac_class_s
{
	MAC_UNICAST = 1,
	MAC_MULTICAST,
	MAC_BROADCAST,

}mac_class_t;

typedef struct l2mac_s
{
	NODE			node;
	ifindex_t		ifindex;
	vlan_t			vlan;
	mac_t 			mac[NSM_MAC_MAX];
	mac_type_t		type;
	mac_class_t		class;
	mac_action_t	action;
}l2mac_t;

typedef struct Gl2mac_s
{
	int		ageing_time;
	LIST	*macList;
	void	*mutex;
	mac_t	gmac[NSM_MAC_MAX];
	mac_t	gmac1[NSM_MAC_MAX];
	mac_t	gmac2[NSM_MAC_MAX];
}Gl2mac_t;

typedef int (*l2mac_cb)(l2mac_t *, void *);

extern int nsm_mac_init(void);
extern int nsm_mac_exit(void);

extern int nsm_mac_callback_api(l2mac_cb cb, void *);

extern int nsm_mac_cleanall_api(void);
extern int nsm_mac_clean_ifindex_api(mac_type_t type, ifindex_t ifindex);
extern int nsm_mac_clean_vlan_api(mac_type_t type, vlan_t vlan);

extern int nsm_mac_add_api(l2mac_t *mac);
extern int nsm_mac_del_api(l2mac_t *mac);
extern int nsm_mac_lookup_api(mac_t *mac, vlan_t vlan);
extern int nsm_mac_get_api(mac_t *mac, vlan_t vlan, l2mac_t *gmac);

extern int nsm_mac_ageing_time_set_api(int ageing);
extern int nsm_mac_ageing_time_get_api(int *ageing);


extern int nsm_gmac_set_api(int, mac_t *mac, int len);
extern int nsm_gmac_get_api(int, mac_t *mac, int len);


extern int nsm_mac_address_table_config(struct vty *vty);
extern int nsm_mac_address_table_ageing_config(struct vty *vty);
extern void cmd_mac_init(void);


#endif /* __NSM_MAC_H__ */
