/*
 * nsm_tunnel.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */
#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include <log.h>
#include "nsm_client.h"
#include "os_list.h"
#include "tty_com.h"
#include "nsm_tunnel.h"

// ip_tunnel.c
extern int _ipkernel_tunnel_create(nsm_tunnel_t *tunnel);
extern int _ipkernel_tunnel_delete(nsm_tunnel_t *tunnel);
extern int _ipkernel_tunnel_change(nsm_tunnel_t *tunnel);


static int (*ipkernel_tunnel_create)(nsm_tunnel_t *tunnel);
static int (*ipkernel_tunnel_delete)(nsm_tunnel_t *tunnel);
static int (*ipkernel_tunnel_change)(nsm_tunnel_t *tunnel);


nsm_tunnel_t * nsm_tunnel_get(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	return (nsm_tunnel_t *)nsm->nsm_client[NSM_TUNNEL];
}


static int nsm_tunnel_iph_protocol(tunnel_mode mode)
{
	int pro = 0;
	switch(mode)
	{
	case NSM_TUNNEL_IPIP:
	case NSM_TUNNEL_IPV4V6:
		pro = IPPROTO_IPIP;
		break;
	case NSM_TUNNEL_GRE:
		pro = IPPROTO_GRE;
		break;
	case NSM_TUNNEL_VTI:
		pro = IPPROTO_IPIP;
		break;
	case NSM_TUNNEL_SIT:
		pro = IPPROTO_IPV6;
		break;
/*	case NSM_TUNNEL_SIT://isatap
		pro = IPPROTO_IPV6;
		break;*/
	case NSM_TUNNEL_IPIPV6:
		pro = IPPROTO_IPV6;
		break;
	case NSM_TUNNEL_GREV6:
		pro = IPPROTO_GRE;
		break;
	default:
		break;
	}
	return pro;
}


int nsm_tunnel_make_iphdr(nsm_tunnel_t *tunnel, struct iphdr *iph)
{
	struct interface *ifp = NULL;
	if(!tunnel || !tunnel->ifp)
		return ERROR;
	ifp = tunnel->ifp;
	iph->version 	= 4;
	iph->ihl 		= 5;
#ifndef IP_DF
#define IP_DF 0x4000  /* Flag: "Don't Fragment" */
#endif
	iph->frag_off 	= htons(IP_DF);
	iph->protocol 	= nsm_tunnel_iph_protocol(tunnel->mode);
	iph->tos 		= tunnel->tun_tos;
    iph->ttl 		= tunnel->tun_ttl;

    iph->saddr 		= tunnel->source.u.prefix4.s_addr;
    iph->daddr 		= tunnel->remote.u.prefix4.s_addr;
    return OK;
}

static int nsm_tunnel_kname(nsm_tunnel_t *tunnel)
{
	char kname[65];
	os_memset(kname, 0, sizeof(kname));
	switch(tunnel->mode)
	{
	case NSM_TUNNEL_IPIP:
		os_snprintf(kname, sizeof(kname), "tunl%d", tunnel->ifp->ifindex);
		break;
	case NSM_TUNNEL_GRE:
		os_snprintf(kname, sizeof(kname), "gre%d", tunnel->ifp->ifindex);
		break;
	case NSM_TUNNEL_VTI:
		os_snprintf(kname, sizeof(kname), "vit%d", tunnel->ifp->ifindex);
		break;
	case NSM_TUNNEL_SIT:
		os_snprintf(kname, sizeof(kname), "sit%d", tunnel->ifp->ifindex);
		break;
	case NSM_TUNNEL_IPIPV6:
		os_snprintf(kname, sizeof(kname), "tunv%d", tunnel->ifp->ifindex);
		break;
	case NSM_TUNNEL_GREV6:
		os_snprintf(kname, sizeof(kname), "grev%d", tunnel->ifp->ifindex);
		break;
	default:
		break;
	}
	//if(os_strlen(kname))
	//	if_kname_set(tunnel->ifp, kname);
	return OK;
}

static int nsm_tunnel_ipkernel_create(nsm_tunnel_t *tunnel)
{
	//TODO create tunnel interface
	if(ipkernel_tunnel_create)
	{
		if((ipkernel_tunnel_create)(tunnel) == OK)
		{
			return nsm_interface_up_set_api(tunnel->ifp);
		}
	}
	return OK;
}

static int nsm_tunnel_ipkernel_change(nsm_tunnel_t *tunnel)
{
	if(tunnel->active)
	{
		//TODO change tunnel interface
		if(ipkernel_tunnel_change)
		{
			return (ipkernel_tunnel_change)(tunnel);
		}
		return OK;
	}
	return ERROR;
}

static int nsm_tunnel_ipkernel_destroy(nsm_tunnel_t *tunnel)
{
	if(ipkernel_tunnel_delete)
	{
		if((ipkernel_tunnel_delete)(tunnel) == OK)
		{
			if_kname_set(tunnel->ifp, NULL);
			tunnel->active = FALSE;
			tunnel->ready = FALSE;
			return OK;
		}
	}
	return ERROR;
}

static int nsm_tunnel_create_thread(struct interface *ifp)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
		return nsm_tunnel_ipkernel_create(tunnel);
	return ERROR;
}

static int nsm_tunnel_state_update(nsm_tunnel_t *tunnel)
{
	if(tunnel->active || tunnel->ready)
	{
		if(nsm_tunnel_iph_protocol(tunnel->mode) == 0)			//隧道模式
		{
/*			if(!tunnel->source.u.prefix4.s_addr || !tunnel->remote.u.prefix4.s_addr)
			{
				tunnel->active = FALSE;
				tunnel->ready = FALSE;
				return OK;
			}
			else*/
			{
				tunnel->active = FALSE;
				tunnel->ready = FALSE;
				nsm_interface_down_set_api(tunnel->ifp);
				return OK;
			}
		}
		if(!tunnel->source.u.prefix4.s_addr || !tunnel->remote.u.prefix4.s_addr)
		{
			tunnel->active = FALSE;
			tunnel->ready = FALSE;
			nsm_interface_down_set_api(tunnel->ifp);
			return OK;
		}
	}
	return OK;
}

static int nsm_tunnel_create_update(nsm_tunnel_t *tunnel)
{
	if(!tunnel)
		return ERROR;
	nsm_tunnel_state_update(tunnel);
	if(tunnel->active)
	{
		return nsm_tunnel_ipkernel_change(tunnel);
	}
	if(nsm_tunnel_iph_protocol(tunnel->mode))			//隧道模式
	{
		if(tunnel->source.u.prefix4.s_addr && tunnel->remote.u.prefix4.s_addr)
		{
			tunnel->ready = TRUE;
			nsm_tunnel_kname(tunnel);
			os_job_add(nsm_tunnel_create_thread, tunnel->ifp);
			return OK;
		}
	}
	return ERROR;
}




int nsm_tunnel_mode_set_api(struct interface *ifp, tunnel_mode mode)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		tunnel->mode = mode;
		nsm_tunnel_create_update(tunnel);
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_mode_get_api(struct interface *ifp, tunnel_mode *mode)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		if(mode)
			*mode = tunnel->mode;
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_source_set_api(struct interface *ifp, struct prefix  *source)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		prefix_copy(&tunnel->source, source);
		nsm_tunnel_create_update(tunnel);
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_source_get_api(struct interface *ifp, struct prefix *source)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		if(source)
			prefix_copy(source, &tunnel->remote);
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_destination_set_api(struct interface *ifp, struct prefix *dest)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		prefix_copy(&tunnel->remote, dest);
		nsm_tunnel_create_update(tunnel);
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_destination_get_api(struct interface *ifp, struct prefix *dest)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		if(dest)
			prefix_copy(dest, &tunnel->remote);
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_ttl_set_api(struct interface *ifp, int ttl)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		tunnel->tun_ttl = ttl;
		nsm_tunnel_create_update(tunnel);
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_ttl_get_api(struct interface *ifp, int *ttl)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		if(ttl)
			*ttl = tunnel->tun_ttl;
		return OK;
	}
	return ERROR;
}


int nsm_tunnel_mtu_set_api(struct interface *ifp, int mtu)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		tunnel->tun_mtu = mtu;
		nsm_tunnel_create_update(tunnel);
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_mtu_get_api(struct interface *ifp, int *mtu)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		if(mtu)
			*mtu = tunnel->tun_mtu;
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_tos_set_api(struct interface *ifp, int tos)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		tunnel->tun_tos = tos;
		nsm_tunnel_create_update(tunnel);
		return OK;
	}
	return ERROR;
}

int nsm_tunnel_tos_get_api(struct interface *ifp, int *tos)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		if(tos)
			*tos = tunnel->tun_tos;
		return OK;
	}
	return ERROR;
}

static int nsm_tunnel_add_interface(struct interface *ifp)
{
	nsm_tunnel_t * tunnel = NULL;
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!if_is_tunnel(ifp))
		return OK;
	if(!nsm->nsm_client[NSM_TUNNEL])
		nsm->nsm_client[NSM_TUNNEL] = XMALLOC(MTYPE_IF, sizeof(nsm_tunnel_t));
	zassert(nsm->nsm_client[NSM_TUNNEL]);
	os_memset(nsm->nsm_client[NSM_TUNNEL], 0, sizeof(nsm_tunnel_t));
	tunnel = nsm->nsm_client[NSM_TUNNEL];

	tunnel->ifp = ifp;
	tunnel->tun_ttl = NSM_TUNNEL_TTL_DEFAULT;		//change: ip tunnel change tunnel0 ttl
	tunnel->tun_mtu = NSM_TUNNEL_MTU_DEFAULT;		//change: ip link set dev tunnel0 mtu 1400

	tunnel->tun_tos = NSM_TUNNEL_TOS_DEFAULT;
	//serial->serial_index = serial_index_make();
	return OK;
}


static int nsm_tunnel_del_interface(struct interface *ifp)
{
	nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
	if(tunnel)
	{
		struct nsm_interface *nsm = ifp->info[MODULE_NSM];
		nsm_tunnel_ipkernel_destroy(tunnel);
		XFREE(MTYPE_IF, tunnel);
		nsm->nsm_client[NSM_TUNNEL] = NULL;
	}
	return OK;
}


int nsm_tunnel_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->notify_add_cb = nsm_tunnel_add_interface;
	nsm->notify_delete_cb = nsm_tunnel_del_interface;
	nsm->interface_write_config_cb = NULL;//nsm_tunnel_interface_write_config;
	nsm_client_install (nsm, NSM_TUNNEL);

	ipkernel_tunnel_create = _ipkernel_tunnel_create;
	ipkernel_tunnel_delete = _ipkernel_tunnel_delete;
	ipkernel_tunnel_change = _ipkernel_tunnel_change;

	return OK;
}


int nsm_tunnel_client_exit()
{
	struct nsm_client *nsm = nsm_client_lookup (NSM_TUNNEL);
	if(nsm)
		nsm_client_free (nsm);
	return OK;
}
