/*
 * nsm_arp.h
 *
 *  Created on: Apr 8, 2018
 *      Author: zhurish
 */

#ifndef __NSM_ARP_H_
#define __NSM_ARP_H_

#ifdef __cplusplus
extern "C" {
#endif


#define NSM_ARP_TTL_DEFAULT 30

typedef enum arp_class_s
{
	ARP_STATIC = 1,
	ARP_DYNAMIC,
}arp_class_t;

typedef struct ip_arp_s
{
	NODE			node;
	arp_class_t		class;
	struct prefix   address;
	mac_t 			mac[NSM_MAC_MAX];
	ifindex_t		ifindex;
	vrf_id_t		vrfid;
	zpl_uint32				ttl;
}ip_arp_t;


typedef struct Gip_arp_s
{
	zpl_uint32		ageing_time;
	zpl_uint32		retry_interval;
	zpl_uint32		timeout;		//ARP IPSTACK_TTL
	zpl_uint32		arp_proxy;
	zpl_uint32		arp_proxy_local;
	zpl_uint32		grat_arp;		//gratuitous arp
	LIST	*arpList;
	void	*mutex;
	zpl_uint32		dynamic_cnt;
	zpl_uint32		static_cnt;
}Gip_arp_t;

typedef int (*ip_arp_cb)(ip_arp_t *, void *);

extern int nsm_ip_arp_init(void);
extern int nsm_ip_arp_exit(void);

extern int nsm_ip_arp_add_api(struct interface *ifp, struct prefix *address, zpl_char *mac);
extern int nsm_ip_arp_del_api(struct interface *ifp, struct prefix *address);
extern int nsm_ip_arp_lookup_api(struct prefix *address);
extern int nsm_ip_arp_callback_api(ip_arp_cb cb, void *pVoid);

extern int nsm_ip_arp_get_api(struct prefix *address, ip_arp_t *gip_arp);
extern int ip_arp_cleanup_api(arp_class_t type, zpl_bool all, ifindex_t ifindex);

extern int nsm_ip_arp_ageing_time_set_api(zpl_uint32 );
extern int nsm_ip_arp_ageing_time_get_api(zpl_uint32 *);


extern int nsm_ip_arp_timeout_set_api(zpl_uint32 );
extern int nsm_ip_arp_timeout_get_api(zpl_uint32 *);

extern int nsm_ip_arp_retry_interval_set_api(zpl_uint32 );
extern int nsm_ip_arp_retry_interval_get_api(zpl_uint32 *);

extern int nsm_ip_arp_proxy_set_api(zpl_uint32 );
extern int nsm_ip_arp_proxy_get_api(zpl_uint32 *);

extern int nsm_ip_arp_proxy_local_set_api(zpl_uint32 );
extern int nsm_ip_arp_proxy_local_get_api(zpl_uint32 *);


extern int ip_arp_dynamic_cb(zpl_action action, void *pVoid);
#ifdef ZPL_SHELL_MODULE
extern int nsm_ip_arp_config(struct vty *vty);
extern int nsm_ip_arp_ageing_config(struct vty *vty);

extern void cmd_arp_init(void);
#endif
 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_ARP_H_ */
