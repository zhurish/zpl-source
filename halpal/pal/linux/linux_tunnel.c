/*
 * ip_tunnel.c
 *
 *  Created on: Jul 14, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"
#include "nsm_debug.h"
#include "nsm_include.h"
#include "nsm_arp.h"
#include "nsm_bridge.h"
#include "nsm_firewalld.h"
#include "nsm_vlaneth.h"
#include "linux_driver.h"
#define _LINUX_IP_H

#include "linux/if_tunnel.h"
#include "linux/sockios.h"

#ifdef ZPL_KERNEL_NETLINK

static int _if_tunnel_create(const char *dev, struct ip_tunnel_parm *p)
{
	struct ipstack_ifreq ifr;
	os_memset(&ifr, 0, sizeof(ifr));
	strcpy (ifr.ifr_name, dev);
	ifr.ifr_ifru.ifru_data = (void*)p;
	if (linux_ioctl_if_ioctl (IPSTACK_SIOCADDTUNNEL, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}

static int _if_tunnel_delete(const char *dev, struct ip_tunnel_parm *p)
{
	struct ipstack_ifreq ifr;
	os_memset(&ifr, 0, sizeof(ifr));
	strcpy (ifr.ifr_name, dev);
	ifr.ifr_ifru.ifru_data = (void*)p;
	if (linux_ioctl_if_ioctl (IPSTACK_SIOCDELTUNNEL, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}

static int _if_tunnel_change(const char *dev, struct ip_tunnel_parm *p)
{
	struct ipstack_ifreq ifr;
	os_memset(&ifr, 0, sizeof(ifr));
	strcpy (ifr.ifr_name, dev);
	ifr.ifr_ifru.ifru_data = (void*)p;
	if (linux_ioctl_if_ioctl (IPSTACK_SIOCCHGTUNNEL, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}


/*
cmd = 0x89f1
basedev = gre0
p.name = gre12131312
iph.version = 4
iph.ihl = 5
iph.frag_off = 64
iph.protocol = 47
iph.tos = 0
iph.saddr = 0x1010101
iph.daddr = 0x2010101
p.link = 0x0
p.i_flags = 0x0
p.o_flags = 0x0
p.i_key = 0x0
p.o_key = 0x0
 */

int linux_ioctl_tunnel_create(struct interface *ifp)
{
	nsm_tunnel_t *tunnel;
	struct ip_tunnel_parm p;
	os_memset(&p, 0, sizeof(p));
	tunnel = nsm_tunnel_get(ifp);
	if(tunnel == NULL)
		return ERROR;	
	os_strcpy(p.name, tunnel->ifp->k_name);
	nsm_tunnel_make_iphdr(tunnel, &p.iph);

/*	p.link;
	p.i_flags;
	p.o_flags;
	p.i_key;
	p.o_key;*/
	//p.i_flags |= GRE_KEY;
	//p.o_flags |= GRE_KEY;
	switch(tunnel->mode)
	{
	case NSM_TUNNEL_IPIP:
		break;
	case NSM_TUNNEL_GRE:
		break;
	case NSM_TUNNEL_VTI:
		p.i_flags |= VTI_ISVTI;
		break;
	case NSM_TUNNEL_SIT:
		p.i_flags |= SIT_ISATAP;
		break;
	case NSM_TUNNEL_IPIPV6:
		break;
	case NSM_TUNNEL_GREV6:
		break;
	default:
		break;
	}
	return _if_tunnel_create(p.name, &p);
	/*
	if(p.iph.protocol == IPSTACK_IPPROTO_IPIP)
		return _if_tunnel_create(p.name, &p);
	else if(p.iph.protocol == IPSTACK_IPPROTO_GRE)
		return _if_tunnel_create("gre0", &p);
	else if(p.iph.protocol == IPSTACK_IPPROTO_IPV6)
		return _if_tunnel_create("sit0", &p);
	else if(p.iph.protocol == IPPROTO_IPIP)
		return _if_tunnel_create("sit0", &p);
		*/
	return ERROR;
}


int linux_ioctl_tunnel_delete(struct interface *ifp)
{
	nsm_tunnel_t *tunnel;
	struct ip_tunnel_parm p;
	os_memset(&p, 0, sizeof(p));
	tunnel = nsm_tunnel_get(ifp);
	if(tunnel == NULL)
		return ERROR;	
	os_strcpy(p.name, tunnel->ifp->k_name);
	nsm_tunnel_make_iphdr(tunnel, &p.iph);

	p.link = tunnel->ifp->k_ifindex;
	switch(tunnel->mode)
	{
	case NSM_TUNNEL_IPIP:
		break;
	case NSM_TUNNEL_GRE:
		break;
	case NSM_TUNNEL_VTI:
		p.i_flags |= VTI_ISVTI;
		break;
	case NSM_TUNNEL_SIT:
		p.i_flags |= SIT_ISATAP;
		break;
	case NSM_TUNNEL_IPIPV6:
		break;
	case NSM_TUNNEL_GREV6:
		break;
	default:
		break;
	}
	return _if_tunnel_delete(p.name, &p);
	/*
	if(p.iph.protocol == IPSTACK_IPPROTO_IPIP)
		return _if_tunnel_delete("tunl0", &p);
	else if(p.iph.protocol == IPSTACK_IPPROTO_GRE)
		return _if_tunnel_delete("gre0", &p);
	else if(p.iph.protocol == IPSTACK_IPPROTO_IPV6)
		return _if_tunnel_delete("sit0", &p);
		*/
	return ERROR;
}

int linux_ioctl_tunnel_change(struct interface *ifp)
{
	nsm_tunnel_t *tunnel;
	struct ip_tunnel_parm p;
	os_memset(&p, 0, sizeof(p));
	tunnel = nsm_tunnel_get(ifp);
	if(tunnel == NULL)
		return ERROR;
	os_strcpy(p.name, tunnel->ifp->k_name);
	nsm_tunnel_make_iphdr(tunnel, &p.iph);

	p.link = tunnel->ifp->k_ifindex;
/*	p.link;
	p.i_flags;
	p.o_flags;
	p.i_key;
	p.o_key;
	if (memcmp(p->name, "vti", 3) == 0)
	p->iph.protocol = IPSTACK_IPPROTO_IPIP;
	p->i_flags |= VTI_ISVTI;
	if (isatap)
		p->i_flags |= SIT_ISATAP;
	*/
	//p.i_flags |= GRE_KEY;
	//p.o_flags |= GRE_KEY;
	switch(tunnel->mode)
	{
	case NSM_TUNNEL_IPIP:
		break;
	case NSM_TUNNEL_GRE:
		break;
	case NSM_TUNNEL_VTI:
		p.i_flags |= VTI_ISVTI;
		break;
	case NSM_TUNNEL_SIT:
		p.i_flags |= SIT_ISATAP;
		break;
	case NSM_TUNNEL_IPIPV6:
		break;
	case NSM_TUNNEL_GREV6:
		break;
	default:
		break;
	}
	return _if_tunnel_change(p.name, &p);
	/*
	if(p.iph.protocol == IPSTACK_IPPROTO_IPIP)
		return _if_tunnel_change("tunl0", &p);
	else if(p.iph.protocol == IPSTACK_IPPROTO_GRE)
		return _if_tunnel_change("gre0", &p);
	else if(p.iph.protocol == IPSTACK_IPPROTO_IPV6)
		return _if_tunnel_change("sit0", &p);
		*/
	return ERROR;
}


#endif