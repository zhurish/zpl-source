/*
 * nsm_dns.h
 *
 *  Created on: Oct 13, 2018
 *      Author: zhurish
 */

#ifndef __NSM_DNS_H__
#define __NSM_DNS_H__


#define IP_DNS_NAME_MAX	64
#define IP_DNS_METRIC_STATIC_DEFAULT 1


#define IP_DNS_DEBUG		0x01
#define IP_DNS_EVENT_DEBUG	0x02

#define NSM_DNS_DEBUG_ON(n) 	(_dns_debug |= (n))
#define NSM_DNS_DEBUG_OFF(n) 	(_dns_debug &= ~(n))
#define NSM_DNS_IS_DEBUG(n) 	(_dns_debug & (n))


typedef enum
{
	IP_DNS_EV_NONE = 0,
	IP_DNS_UP_IF,
	IP_DNS_DOWN_IF,

	IP_DNS_ADD_IF,
	IP_DNS_DEL_IF,

	IP_DNS_ADD_ADDRESS,
	IP_DNS_DEL_ADDRESS,

	IP_DNS_ADD_ROUTE,
	IP_DNS_DEL_ROUTE,

	IP_DNS_ADD_STATIC,
	IP_DNS_ADD_DYNAMIC,
	IP_DNS_DEL_STATIC,
	IP_DNS_DEL_DYNAMIC,

	IP_DNS_ADD_DOMAIN,
	IP_DNS_DEL_DOMAIN,
}dns_cmd_t;


typedef enum
{
	IP_DNS_STATIC = 1,
	IP_DNS_DYNAMIC,

	IP_HOST_STATIC,
	IP_HOST_DYNAMIC,
}dns_class_t;

typedef struct dns_opt
{
	ifindex_t		ifindex;
	vrf_id_t		vrfid;
	int				metric;
	BOOL			secondly;
	BOOL			active;
}ip_dns_opt_t;

typedef struct ip_dns_s
{
	NODE			node;
	dns_class_t		type;
	struct prefix   address;
	union
	{
		char 			name[IP_DNS_NAME_MAX];
		struct dns_opt	dns_opt;

	}data;

}ip_dns_t, ip_host_t;

#define _host_name		data.name
#define _dns_ifindex	data.dns_opt.ifindex
#define _dns_vrfid		data.dns_opt.vrfid
#define _dns_metric		data.dns_opt.metric
#define _dns_secondly	data.dns_opt.secondly
#define _dns_active		data.dns_opt.active


typedef struct Gip_dns_s
{
	LIST			*dnsList;
	void			*mutex;
	char 			domain_name1[IP_DNS_NAME_MAX];
	char 			domain_name2[IP_DNS_NAME_MAX];
	BOOL			domain_dynamic1;
	BOOL			domain_dynamic2;
	ip_dns_t		*dns1;
	ip_dns_t		*dns2;
	ip_dns_t		*dns3;
}Gip_dns_t;

typedef int (*ip_dns_cb)(ip_dns_t *, void *);
typedef int (*ip_host_cb)(ip_host_t *, void *);


extern u_int8 _dns_debug;


extern int nsm_ip_dns_init(void);
extern int nsm_ip_dns_exit(void);

extern int ip_dns_add_job(dns_cmd_t cmd, void *p);

extern int nsm_ip_dns_add(struct prefix *address, ip_dns_opt_t *, BOOL	secondly, dns_class_t type);
extern int nsm_ip_dns_del(struct prefix *address, dns_class_t type);
extern int nsm_ip_dns_del_by_ifindex(ifindex_t ifindex, dns_class_t type);

extern int nsm_ip_dns_add_api(struct prefix *address, BOOL	secondly);
extern int nsm_ip_dns_del_api(struct prefix *address);
extern int nsm_ip_dns_get_api(ifindex_t ifindex, struct prefix *address, BOOL	secondly);

extern ip_dns_t * nsm_ip_dns_lookup_api(struct prefix *address, dns_class_t type);
extern int nsm_ip_dns_callback_api(ip_dns_cb cb, void *pVoid);



extern int nsm_ip_host_add(struct prefix *address, char *name, dns_class_t type);
extern int nsm_ip_host_del(struct prefix *address, dns_class_t type);

extern int nsm_ip_host_add_api(struct prefix *address, char *name);
extern int nsm_ip_host_del_api(struct prefix *address);


extern ip_host_t * nsm_ip_host_lookup_api(struct prefix *address, dns_class_t type);
extern int nsm_ip_host_callback_api(ip_host_cb cb, void *pVoid);

extern int nsm_dns_domain_name_add_api(char *name, BOOL secondly);
extern int nsm_dns_domain_name_del_api(BOOL secondly);
extern int nsm_dns_domain_name_dynamic_api(BOOL dynamic, BOOL secondly);

extern int nsm_ip_dns_host_show(struct vty *vty);
extern int nsm_ip_dns_host_config(struct vty *vty);

extern int nsm_dns_debug_write(struct vty *vty);

extern void cmd_dns_init(void);


#endif /* __NSM_DNS_H__ */
