/*
 * ip_.c
 *
 *  Created on: Jan 28, 2018
 *      Author: zhurish
 */


#define IPCOM_USE_HOOK

#include "zplos_include.h"
#include "zmemory.h"
#include "command.h"
#include "zmemory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>

#include "nsm_mac.h"
#include "nsm_arp.h"

#include "ip_ipstack.h"
#include "pal_include.h"

/* clear and set interface name string */



#ifdef ZPL_KERNEL_MODULE

#else

static void _ipkernel_ifreq_set_name (struct Ip_ifreq *ipstack_ifreq, struct interface *ifp)
{
  if(NULL == ifp)
  {
    return;
  }
  strncpy (ipstack_ifreq->ifr_name, ifp->k_name, MIN(INTERFACE_NAMSIZ, 16));//IP_IFNAMSIZ
  return;
}

static int ip_stack_ioctl(zpl_ulong cmd, void *data, vrf_id_t vrf_id)
{
	int ret = 0;
	int fd = ipcom_socket(IP_AF_INET, IP_SOCK_DGRAM, 0);
	if(fd < 0)
	{
		zlog_err(MODULE_PAL,"failed to create ipstack_socket");
		return -1;
	}
    if(vrf_id > 0)
    {
    	vrf_id_t vrf = vrf_id;
        ret = ipcom_setsockopt (fd, IP_SOL_SOCKET, IP_SO_X_VR, &vrf, sizeof (vrf));
        if(ret < 0)
        {
        	ipcom_socketclose (fd);
        	zlog_err(MODULE_PAL,"failed to set ipstack_socket to VRF");
            return -1;
        }
    }
	ret = ipcom_socketioctl(fd, (zpl_ulong)cmd, data);
	if(fd < 0)
	{
		zlog_err(MODULE_PAL,"failed to ioctl ipstack_socket");
		ipcom_socketclose (fd);
		return -1;
	}
	ipcom_socketclose (fd);
	return 0;
}


static int ip_stack_change_state(struct interface *ifp, zpl_bool if_up)
{
    struct Ip_ifreq ipstack_ifreq;
    os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
    _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

    if (ip_stack_ioctl(IP_SIOCGIFFLAGS, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
        zlog_err(MODULE_PAL, "failed to get interface flags: %s", ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }

    if (if_up)
    	SET_FLAG(ipstack_ifreq.ip_ifr_flags, IP_IFF_UP);
    else
    	UNSET_FLAG(ipstack_ifreq.ip_ifr_flags, IP_IFF_UP);
    if (ip_stack_ioctl(IP_SIOCSIFFLAGS, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to set interface flags: %s", ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}

static int ip_stack_change_up(struct interface *ifp)
{
    return ip_stack_change_state(ifp, zpl_true);
}

static int ip_stack_change_down(struct interface *ifp)
{
    return ip_stack_change_state(ifp, zpl_false);
}

static int ip_stack_update_flag(struct interface *ifp)
{
    struct Ip_ifreq ipstack_ifreq;
    os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
    _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

    if (ip_stack_ioctl(IP_SIOCGIFFLAGS, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
        zlog_err(MODULE_PAL, "failed to get interface flags: %s", ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    ifp->flags = ipstack_ifreq.ip_ifr_flags;

    return 0;
}

static int ip_stack_set_vr(struct interface *ifp, vrf_id_t vrf_id)
{
    struct Ip_ifreq ipstack_ifreq;

    os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
    _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
    ipstack_ifreq.ip_ifr_vr = (Ip_u16) vrf_id;

    if (ip_stack_ioctl(IP_SIOCSIFVR, &ipstack_ifreq, 0) < 0)
    {
    	zlog_err(MODULE_PAL, "set vr failed: %s", ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}


static int ip_stack_set_mtu(struct interface *ifp, int mtu)
{
    struct Ip_ifreq ipstack_ifreq;
    os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
    _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
    ipstack_ifreq.ip_ifr_mtu = mtu;

    if (ip_stack_ioctl(IP_SIOCSIFMTU, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "set MTU failed: %s" , ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}



static int ip_stack_set_lladdr(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len)
{
    struct Ip_ifreq       ipstack_ifreq;

	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

    ipstack_ifreq.ip_ifr_addr.sa_family = IP_AF_LINK;
    IPCOM_SA_LEN_SET(&ipstack_ifreq.ip_ifr_addr, len);
    os_memcpy(ipstack_ifreq.ip_ifr_addr.sa_data, mac, len);

    if (ip_stack_ioctl(IP_SIOCSIFLLADDR, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "Set LLADDR failed: %s" , ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}

static int ip_stack_create(struct interface *ifp)
{
    struct Ip_ifreq       ipstack_ifreq;

	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

    if (ip_stack_ioctl(IP_SIOCIFCREATE, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "Creating %s failed: %s" , ipstack_ifreq.ifr_name, ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
    ipstack_ifreq.ip_ifr_addr.sa_family = IP_AF_INET;
    if (ip_stack_ioctl(IP_SIOCGIFINDEX, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "get ifindex %s failed: %s" , ipstack_ifreq.ifr_name, ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    ifp->k_ifindex = ipstack_ifreq.ip_ifr_ifindex;
    ip_stack_change_state(ifp, zpl_true);
    if(ifp->hw_addr_len)
    	ip_stack_set_lladdr(ifp, ifp->hw_addr, ifp->hw_addr_len);

    ip_stack_update_flag(ifp);
    return 0;
}

static int ip_stack_destroy(struct interface *ifp)
{
    struct Ip_ifreq ipstack_ifreq;
	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

    if (ip_stack_ioctl(IP_SIOCIFDESTROY, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "Destroying %s failed: %s" , ipstack_ifreq.ifr_name, ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}



static int ip_stack_vlan_set(struct interface *ifp, int vlan)
{
    struct Ip_ifreq ipstack_ifreq;
    struct Ip_vlanreq vlanreq;
	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

    strcpy (vlanreq.vlr_parent, strtok(ipstack_ifreq.ifr_name, "."));
    ip_stack_change_state(ifp, zpl_false);
    vlanreq.vlr_tag = vlan;

    ipstack_ifreq.ip_ifr_data = &vlanreq;
    if (ip_stack_ioctl(IP_SIOCSETVLAN, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "Setting VLAN parameters for %s failed: %s" ,
                     ipstack_ifreq.ifr_name,
                     ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    ip_stack_change_state(ifp, zpl_true);
    return 0;
}


static int ip_stack_vlanpri_set(struct interface *ifp, int pri)
{
    struct Ip_ifreq ipstack_ifreq;
    struct Ip_vlanreq vlanreq;
	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
    strcpy (vlanreq.vlr_parent, strtok(ipstack_ifreq.ifr_name, "."));
    vlanreq.vlr_pri = pri;
    ip_stack_change_state(ifp, zpl_false);
    ipstack_ifreq.ip_ifr_data = &vlanreq;
    if (ip_stack_ioctl(IP_SIOCSETVLANPRI, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "Setting VLAN priority for %s failed: %s" ,
                     ipstack_ifreq.ifr_name,
                     ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    ip_stack_change_state(ifp, zpl_true);
    return 0;
}

static int ip_stack_promisc_link(struct interface *ifp, zpl_bool enable)
{
    struct Ip_ifreq ipstack_ifreq;
	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
    ipstack_ifreq.ifr_ifru.ifru_opt = enable;
    if (ip_stack_ioctl(IP_SIOCXPROMISC, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to set interface flags: %s", ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;

}


static int ip_stack_change_dhcp(struct interface *ifp, zpl_bool enable)
{
    struct Ip_ifreq ipstack_ifreq;

	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

    ipstack_ifreq.ifr_ifru.ifru_opt = enable;
    if (ip_stack_ioctl(IP_SIOCXSDHCPRUNNING, &ipstack_ifreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to enable/disabled DHCP: %s", ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}

static int ip_stack_add_dstaddr(struct interface *ifp, struct connected *ifc)
{
	struct Ip_ifreq ipstack_ifreq;
	struct Ip_sockaddr_in *ipstack_in_addr;
	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
	ipstack_in_addr = (struct Ip_sockaddr_in *) &ipstack_ifreq.ip_ifr_dstaddr;
	ipstack_in_addr->sin_family = IP_AF_INET;
	IPCOM_SA_LEN_SET(ipstack_in_addr, sizeof(struct Ip_sockaddr_in));
	ipstack_in_addr->sin_addr.s_addr = ifc->destination->u.prefix4.s_addr;

	if (ip_stack_ioctl(IP_SIOCSIFDSTADDR, &ipstack_ifreq, ifp->vrf_id) < 0) {
		zlog_err(MODULE_PAL, "set destination address failed: %s",
				ipcom_strerror(ipcom_errno));
		return -ipcom_errno;
	}
	return 0;
}

static int ip_stack_del_dstaddr(struct interface *ifp, struct connected *ifc)
{
	struct Ip_ifreq ipstack_ifreq;
	struct Ip_sockaddr_in *ipstack_in_addr;
	os_memset(&ipstack_ifreq, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
	ipstack_in_addr = (struct Ip_sockaddr_in *) &ipstack_ifreq.ip_ifr_dstaddr;
	ipstack_in_addr->sin_family = IP_AF_INET;
	IPCOM_SA_LEN_SET(ipstack_in_addr, sizeof(struct Ip_sockaddr_in));
	ipstack_in_addr->sin_addr.s_addr = 0;

	if (ip_stack_ioctl(IP_SIOCSIFDSTADDR, &ipstack_ifreq, ifp->vrf_id) < 0) {
		zlog_err(MODULE_PAL, "set destination address failed: %s",
				ipcom_strerror(ipcom_errno));
		return -ipcom_errno;
	}
	return 0;
}

static int ip_stack_ipv4_replace(struct interface *ifp,struct connected *ifc)
{
    struct Ip_ifreq        ifr;
    struct Ip_sockaddr_in *in;

    if (ifc->address == IP_NULL)
    {
    	zlog_err(MODULE_PAL, "address is missing" );
        return -IP_ERRNO_EINVAL;
    }

	os_memset(&ifr, 0, sizeof(struct Ip_ifreq));
	_ipkernel_ifreq_set_name(&ifr, ifp);

    in = (struct Ip_sockaddr_in *) &ifr.ip_ifr_addr;
    in->sin_family = IP_AF_INET;
    IPCOM_SA_LEN_SET(in, sizeof(struct Ip_sockaddr_in));
    in->sin_addr   = ifc->address->u.prefix4;
    if (ip_stack_ioctl(IP_SIOCSIFADDR, &ifr, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to set primary IPv4 address: %s" ,
                         ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }

    //if (ifc->address->prefixlen == IP_NULL)
    //    return 0;
    masklen2ip (ifc->address->prefixlen, &in->sin_addr);;

    //in->sin_addr   = *netmask;
    if (ip_stack_ioctl(IP_SIOCSIFNETMASK, &ifr, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to set mask for primary IPv4 address: %s" ,
                         ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}


static int ip_stack_ipv4_add(struct interface *ifp,struct connected *ifc)
{
    struct Ip_ifaliasreq   ifal;
    struct Ip_sockaddr_in *ipstack_in_addr;
    struct Ip_sockaddr_in *in_mask;
    char buf[64];
    union prefix46constptr  addrp;
    if (ifc->address == IP_NULL)
    {
    	zlog_err(MODULE_PAL, "address is missing" );
        return -IP_ERRNO_EINVAL;
    }

	os_memset(&ifal, 0, sizeof(struct Ip_ifaliasreq));
    os_strcpy(ifal.ifra_name, ifp->k_name);

    /* Adding address and mask. */
    ipstack_in_addr = (struct Ip_sockaddr_in *) &ifal.ifra_addr;
    ipstack_in_addr->sin_family = IP_AF_INET;
    IPCOM_SA_LEN_SET(ipstack_in_addr, sizeof(struct Ip_sockaddr_in));
    ipstack_in_addr->sin_addr   = ifc->address->u.prefix4;

    in_mask = (struct Ip_sockaddr_in *) &ifal.ifra_mask;
    in_mask->sin_family = IP_AF_INET;
    IPCOM_SA_LEN_SET(in_mask, sizeof(struct Ip_sockaddr_in));
    //in_mask->sin_addr   = *netmask;
    masklen2ip (ifc->address->prefixlen, &in_mask->sin_addr);
/*
    os_memset (buf, 0, sizeof(buf));
    addrp.p = ifc->address;
    prefix2str (addrp, buf, sizeof(buf));
    printf("%s: %s\r\n", __func__, buf);
*/
    if (ip_stack_ioctl(IP_SIOCAIFADDR, &ifal, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to %s IPv4 address: %s" ,
                "add",
                ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}

static int ip_stack_ipv4_delete(struct interface *ifp,struct connected *ifc)
{
    struct Ip_ifaliasreq   ifal;
    struct Ip_sockaddr_in *ipstack_in_addr;
    struct Ip_sockaddr_in *in_mask;
    char buf[64];
    union prefix46constptr  addrp;
    if (ifc->address == IP_NULL)
    {
    	zlog_err(MODULE_PAL, "address is missing" );
        return -IP_ERRNO_EINVAL;
    }

	os_memset(&ifal, 0, sizeof(struct Ip_ifaliasreq));
    os_strcpy(ifal.ifra_name, ifp->k_name);

    /* Adding address and mask. */
    ipstack_in_addr = (struct Ip_sockaddr_in *) &ifal.ifra_addr;
    ipstack_in_addr->sin_family = IP_AF_INET;
    IPCOM_SA_LEN_SET(ipstack_in_addr, sizeof(struct Ip_sockaddr_in));
    ipstack_in_addr->sin_addr   = ifc->address->u.prefix4;
/*
    os_memset (buf, 0, sizeof(buf));
    addrp.p = ifc->address;
    prefix2str (addrp, buf, sizeof(buf));
    printf("%s: %s\r\n", __func__, buf);
*/
    if (ip_stack_ioctl(IP_SIOCDIFADDR, &ifal, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to %s IPv4 address: %s" ,
                "delete",
                ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}

#ifdef ZPL_BUILD_IPV6
static int ip_stack_ipv6_add(struct interface *ifp,struct connected *ifc)
{
    struct Ip_in6_aliasreq ifareq;

    ipcom_memset(&ifareq, 0, sizeof(ifareq));
    os_strcpy(ifareq.ifra_name, ifp->k_name);
    ifareq.ifra_addr.sin6_family = IP_AF_INET6;
    IPCOM_SA_LEN_SET(&ifareq.ifra_addr, sizeof(struct Ip_sockaddr_in6));
    ifareq.ifra_addr.sin6_addr   = ifc->address->u.prefix6;
    if (IP_IN6_IS_ADDR_LINK_LOCAL(&ifc->address->u.prefix6))
        ifareq.ifra_addr.sin6_scope_id = ifp->k_ifindex;

    ifareq.ifra_prefixmask.sin6_family = IP_AF_INET6;
    IPCOM_SA_LEN_SET(&ifareq.ifra_prefixmask, sizeof(struct Ip_sockaddr_in6));

    masklen2ip (ifc->address->prefixlen, &ifareq.ifra_prefixmask.sin6_addr);
    //ipnet_cmd_ifconfig_create_mask(&ifareq.ifra_prefixmask.sin6_addr, param->prefixlen);

    ifareq.ifra_flags = 0;//param->flags;

    ifareq.ifra_lifetime.ia6t_preferred = (Ip_time_t)IPCOM_ADDR_INFINITE;
    ifareq.ifra_lifetime.ia6t_expire = (Ip_time_t)IPCOM_ADDR_INFINITE;
/*
    if (param->preferred_lifetime != (Ip_time_t)IPCOM_ADDR_INFINITE)
        ifareq.ifra_lifetime.ia6t_preferred = param->preferred_lifetime + ipcom_time(IP_NULL);
    else
        ifareq.ifra_lifetime.ia6t_preferred = (Ip_time_t)IPCOM_ADDR_INFINITE;

    if (ifareq.ifra_lifetime.ia6t_expire != (Ip_time_t)IPCOM_ADDR_INFINITE)
        ifareq.ifra_lifetime.ia6t_expire = param->valid_lifetime + ipcom_time(IP_NULL);
    else
        ifareq.ifra_lifetime.ia6t_expire = (Ip_time_t)IPCOM_ADDR_INFINITE;
*/
    if (ip_stack_ioctl(IP_SIOCAIFADDR_IN6, &ifareq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to %s IPv6 address: %s" ,
                "add",
                ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}


/*
 *===========================================================================
 *                    ipnet_ifconfig_ipv6_delete
 *===========================================================================
 * Description:
 * Parameters:
 * Returns:
 *
 */
static int ip_stack_ipv6_delete(struct interface *ifp,struct connected *ifc)
{
    struct Ip_in6_ifreq ipstack_ifreq;

    ipcom_memset(&ipstack_ifreq, 0, sizeof(ipstack_ifreq));
    os_strcpy(ifareq.ifra_name, ifp->k_name);
    ipstack_ifreq.ifr_ifru.ifru_addr.sin6_family = IP_AF_INET6;
    IPCOM_SA_LEN_SET(&ipstack_ifreq.ifr_ifru.ifru_addr, sizeof(struct Ip_sockaddr_in6));

    ipstack_ifreq.ifr_ifru.ifru_addr.sin6_addr   = ifc->address->u.prefix6;
    if (IP_IN6_IS_ADDR_LINK_LOCAL(&ifc->address->u.prefix6))
        ipstack_ifreq.ifr_ifru.ifru_addr.sin6_scope_id = ifp->k_ifindex;

    if (ip_stack_ioctl(IP_SIOCDIFADDR_IN6, &ifareq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to %s IPv6 address: %s" ,
                "delete",
                ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}
#endif

static int ip_stack_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	int ret = 0;
    struct Ip_arpreq ipstack_arpreq;
    struct Ip_sockaddr_dl *dl = (struct Ip_sockaddr_dl *)&ipstack_arpreq.arp_ha;
    ipcom_memset(&ipstack_arpreq, 0, sizeof(struct Ip_arpreq));
    ipstack_arpreq.arp_flags = IP_ATF_PERM;
    ipstack_arpreq.arp_ha.sdl_type  = IP_IFT_ETHER;
    ipstack_arpreq.arp_ha.sdl_index = (Ip_u16) ifp->k_ifindex;

    if(address->family == IPSTACK_AF_INET)
    {
        struct Ip_sockaddr_in *addr = (struct Ip_sockaddr_in *) &ipstack_arpreq.arp_pa;
    	IPCOM_SA_LEN_SET(&ipstack_arpreq.arp_pa, sizeof(struct Ip_sockaddr_in));
    	ipstack_arpreq.arp_pa.sa_family = IP_AF_INET;
    	addr->sin_addr.s_addr = address->u.prefix4.s_addr;
    }
#ifdef ZPL_BUILD_IPV6
    if(address->family == IPSTACK_AF_INET6)
    {
        struct Ip_sockaddr_in6 *addr = (struct Ip_sockaddr_in6 *) &ipstack_arpreq.arp_pa;
    	IPCOM_SA_LEN_SET(&ipstack_arpreq.arp_pa, sizeof(struct Ip_sockaddr_in6));
    	ipstack_arpreq.arp_pa.sa_family = IP_AF_INET6;
    	os_memcpy(&addr->sin6_addr, &address->u.prefix6, IPV6_MAX_BYTELEN);
    }
#endif
    ipcom_memset(dl, 0, sizeof(struct Ip_sockaddr_dl));
    dl->sdl_family = IP_AF_LINK;
	os_memcpy(dl->sdl_data, mac, IPNET_ETH_ADDR_SIZE);
	ipstack_arpreq.arp_ha.sdl_alen = IPNET_ETH_ADDR_SIZE;
    IPCOM_SA_LEN_SET(dl, (ip_offsetof(struct Ip_sockaddr_dl, sdl_data)
                          + IP_MAX(sizeof(dl->sdl_data), dl->sdl_alen)));
/*

    ret = ipcom_getsockaddrbyaddr(IP_AF_LINK, mac, (struct Ip_sockaddr *)&ipstack_arpreq.arp_ha);
    if (ret < 0 || ipstack_arpreq.arp_ha.sdl_alen != IPNET_ETH_ADDR_SIZE)
    {
        return -1;
    }
*/

    if (ip_stack_ioctl(IP_SIOCSARP, &ipstack_arpreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to %s arp address: %s" ,
                "add",
                ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}

static int ip_stack_arp_delete(struct interface *ifp,  struct prefix *address)
{
    struct Ip_arpreq       ipstack_arpreq;
    int                    ret;
    ipcom_memset(&ipstack_arpreq, 0, sizeof(ipstack_arpreq));
    IPCOM_SA_LEN_SET(&ipstack_arpreq.arp_ha, sizeof(ipstack_arpreq.arp_ha));
    ipstack_arpreq.arp_ha.sdl_family = IP_AF_LINK;
    ipstack_arpreq.arp_ha.sdl_type   = IP_IFT_ETHER;
    ipstack_arpreq.arp_ha.sdl_index  = (Ip_u16) ifp->k_ifindex;

    if(address->family == IPSTACK_AF_INET)
    {
        struct Ip_sockaddr_in *addr = (struct Ip_sockaddr_in *) &ipstack_arpreq.arp_pa;
    	IPCOM_SA_LEN_SET(&ipstack_arpreq.arp_pa, sizeof(struct Ip_sockaddr_in));
    	ipstack_arpreq.arp_pa.sa_family = IP_AF_INET;
    	addr->sin_addr.s_addr = address->u.prefix4.s_addr;
    }
#ifdef ZPL_BUILD_IPV6
    if(address->family == IPSTACK_AF_INET6)
    {
        struct Ip_sockaddr_in6 *addr = (struct Ip_sockaddr_in6 *) &ipstack_arpreq.arp_pa;
    	IPCOM_SA_LEN_SET(&ipstack_arpreq.arp_pa, sizeof(struct Ip_sockaddr_in6));
    	ipstack_arpreq.arp_pa.sa_family = IP_AF_INET6;
    	os_memcpy(&addr->sin6_addr, &address->u.prefix6, IPV6_MAX_BYTELEN);
    }
#endif
    if (ip_stack_ioctl(IP_SIOCDARP, &ipstack_arpreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to %s arp address: %s" ,
                "add",
                ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}

static int ip_stack_arp_request(struct interface *ifp,  struct prefix *address)
{
    struct Ip_arpreq       ipstack_arpreq;
    int                    ret;
    ipcom_memset(&ipstack_arpreq, 0, sizeof(ipstack_arpreq));
    IPCOM_SA_LEN_SET(&ipstack_arpreq.arp_ha, sizeof(ipstack_arpreq.arp_ha));
    ipstack_arpreq.arp_ha.sdl_family = IP_AF_LINK;
    ipstack_arpreq.arp_ha.sdl_type   = IP_IFT_ETHER;
    ipstack_arpreq.arp_ha.sdl_index  = (Ip_u16) ifp->k_ifindex;

    if(address->family == IPSTACK_AF_INET)
    {
        struct Ip_sockaddr_in *addr = (struct Ip_sockaddr_in *) &ipstack_arpreq.arp_pa;
    	IPCOM_SA_LEN_SET(&ipstack_arpreq.arp_pa, sizeof(struct Ip_sockaddr_in));
    	ipstack_arpreq.arp_pa.sa_family = IP_AF_INET;
    	addr->sin_addr.s_addr = address->u.prefix4.s_addr;
    }
#ifdef ZPL_BUILD_IPV6
    if(address->family == IPSTACK_AF_INET6)
    {
        struct Ip_sockaddr_in6 *addr = (struct Ip_sockaddr_in6 *) &ipstack_arpreq.arp_pa;
    	IPCOM_SA_LEN_SET(&ipstack_arpreq.arp_pa, sizeof(struct Ip_sockaddr_in6));
    	ipstack_arpreq.arp_pa.sa_family = IP_AF_INET6;
    	os_memcpy(&addr->sin6_addr, &address->u.prefix6, IPV6_MAX_BYTELEN);
    }
#endif
    if (ip_stack_ioctl(IP_SIOCPARP, &ipstack_arpreq, ifp->vrf_id) < 0)
    {
    	zlog_err(MODULE_PAL, "failed to %s arp address: %s" ,
                "add",
                ipcom_strerror(ipcom_errno));
        return -ipcom_errno;
    }
    return 0;
}


/* call route system call */
static int ip_stack_route_ioctl (zpl_ulong request, void * buffer)
{
	int sock;
	int ret;
	sock = ipcom_socket(IP_AF_ROUTE, IP_SOCK_RAW, 0);
	if (sock < 0)
	{
		int save_errno = ipstack_errno;
		zlog_err(MODULE_PAL, "Cannot create ROUTE RAW ipstack_socket: %s",
				ipstack_strerror(save_errno));
		return -1;
	}
	ret = ipcom_socketioctl(sock, (zpl_ulong)request, buffer);

	if (ret < 0)
	{
		zlog_err(MODULE_PAL, "failed to ioctl ipstack_socket");
		ipcom_socketclose(sock);
		return -1;
	}
	ipcom_socketclose(sock);
	return 0;
}


static int ip_arp_stack_cb (void * buffer)
{
	int action = 0;
	ip_arp_t *value = XMALLOC(MTYPE_VLAN, sizeof(ip_arp_t));
	Ipcom_arp_action_entry *entry = (Ipcom_arp_action_entry *)buffer;


	value->ifindex = ifkernel2ifindex(entry->ifindex);
	value->vrfid = entry->vr_index;
	//value->vlan = entry->vlan;

	os_memcpy(value->mac, entry->link_addr, entry->link_addr_size);

	os_memcpy(&value->address.u.prefix4.s_addr, &entry->address.in.s_addr, 4);

	//value->flag;

	if(entry->action == IPCOM_ENTRY_ADD)
		return ip_arp_dynamic_cb(1, value);
	else if(entry->action == IPCOM_ENTRY_DEL)
		return ip_arp_dynamic_cb(0, value);
	return 0;
}

int ip_arp_stack_gratuitousarp_enable(int enable)
{
	if(enable)
		return ipcom_sysvar_set("ipnet.inet.SendGratuitousArp", "enable", 0);
	else
		return ipcom_sysvar_set("ipnet.inet.SendGratuitousArp", "disable", 0);
}

int ip_arp_stack_ttl_enable(int ttl)
{
	char buf[64];
	os_memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ttl);
	return ipcom_sysvar_set("ipnet.inet.NeighborCacheToLive", buf, 0);
}

int ip_arp_stack_age_timeout_enable(int timeout)
{
	char buf[64];
	os_memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", timeout);
	return ipcom_sysvar_set("ipnet.inet.DelayFirstProbeTime", buf, 0);
}

int ip_arp_stack_retry_interval_enable(int interval)
{
	char buf[64];
	os_memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", interval);
	return ipcom_sysvar_set("ipnet.inet.BaseRetransmitTime", buf, 0);
}


static int ip_stack_new_vr (vrf_id_t vr)
{
    int ret;
    ret = ip_stack_route_ioctl (IP_SIOCADDVR, &vr);
    if (ret < 0)
    {
    	zlog_err(MODULE_PAL, "can't new vr(%d) to ipstack %d", vr, ret);
        return ret;
    }
    return 0;
}

static int ip_stack_del_vr (vrf_id_t vr)
{
    int ret;
    ret = ip_stack_route_ioctl (IP_SIOCDELVR, &vr);
    if (ret < 0)
    {
    	zlog_err(MODULE_PAL, "can't del vr(%d) to ipstack %d", vr, ret);
        return ret;
    }
    return 0;
}




#if 0
int if_lag_add_mem(struct interface *ifp_lag, const char *ifp_mem_name)
{
#ifndef ZPL_BUILD_LINUX
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct Ip_ebondreq ebondreq;

  if(NULL == ifp_lag || ifp_mem_name == NULL)
  {
      return -1;
  }

  memset (&ipstack_ifreq, 0, sizeof(struct ipstack_ifreq));
  _ipkernel_ifreq_set_name (&ipstack_ifreq, ifp_lag);

  ipstack_ifreq.ip_ifr_data = (void *)&ebondreq;

  memcpy(ebondreq.mem_name, ifp_mem_name, IP_IFNAMSIZ);
  ebondreq.lag_mode = ifp_lag->trunk_mode;
  ebondreq.lacp_nego_flag = 1;      /*enable*/

  ret = _ipkernel_if_ioctl (IP_SIOCSETBONDLAG, (caddr_t) &ipstack_ifreq, ifp_lag->vrf_id);
  if (ret < 0)
  {
	  zlog_err(MODULE_PAL, "%s if_lag_add_mem interface %s",ifp_lag->name, ifp_mem_name);
      return ret;
  }

#endif
    return 0;
}

int if_lag_delete_mem(struct interface *ifp_lag, const char *ifp_mem_name)
{

#ifndef ZPL_BUILD_LINUX
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct Ip_ebondreq ebondreq;

  if(NULL == ifp_lag || ifp_mem_name == NULL)
  {
      return -1;
  }

  memset (&ipstack_ifreq, 0, sizeof(struct ipstack_ifreq));
  _ipkernel_ifreq_set_name (&ipstack_ifreq, ifp_lag);

  ipstack_ifreq.ip_ifr_data = (void *)&ebondreq;

  memcpy(ebondreq.mem_name, ifp_mem_name, IP_IFNAMSIZ);

  ret = _ipkernel_if_ioctl (IP_SIOCREMOVEBONDLAG, (caddr_t) &ipstack_ifreq, ifp_lag->vrfid);
  if (ret < 0)
  {
	  zlog_err(MODULE_PAL, "%s if_lag_delete_mem interface %s",ifp_lag->name, ifp_mem_name);
      return ret;
  }

#endif

    return 0;
}



int if_lag_set_mem_flag(struct interface *ifp_lag, const char *ifp_mem_name, int master_slave_flag)
{
#ifndef ZPL_BUILD_LINUX
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct Ip_ebondreq ebondreq;

  if(NULL == ifp_lag || ifp_mem_name == NULL)
  {
      return -1;
  }

  memset (&ipstack_ifreq, 0, sizeof(struct ipstack_ifreq));
  _ipkernel_ifreq_set_name (&ipstack_ifreq, ifp_lag);

  ipstack_ifreq.ip_ifr_data = (void *)&ebondreq;

  memcpy(ebondreq.mem_name, ifp_mem_name, IP_IFNAMSIZ);
  ebondreq.lag_mode = ifp_lag->trunk_mode;
  ebondreq.master_slave_flag = master_slave_flag;      /*enable*/

  ret = _ipkernel_if_ioctl (IP_SIOCSETBONDMS, (caddr_t) &ipstack_ifreq, ifp_lag->vrfid);
  if (ret < 0)
  {
	  zlog_err(MODULE_PAL, "%s if_lag_set_mem_flag interface  %s %d",ifp_lag->name, ifp_mem_name, master_slave_flag);
      return ret;
  }

#endif
    return 0;
}
#endif


int ip_stack_init()
{
	//interface
	pal_stack.ip_stack_up = ip_stack_change_up;
	pal_stack.ip_stack_down = ip_stack_change_down;
	pal_stack.ip_stack_update_flag = ip_stack_update_flag;
	pal_stack.ip_stack_set_vr = ip_stack_set_vr;
	pal_stack.ip_stack_set_mtu = ip_stack_set_mtu;
	pal_stack.ip_stack_set_lladdr = ip_stack_set_lladdr;
	pal_stack.ip_stack_create = ip_stack_create;
	pal_stack.ip_stack_destroy = ip_stack_destroy;
	pal_stack.ip_stack_set_vlan = ip_stack_vlan_set;
	pal_stack.ip_stack_set_vlanpri = ip_stack_vlanpri_set;
	pal_stack.ip_stack_promisc = ip_stack_promisc_link;
	//ip address
	pal_stack.ip_stack_dhcp = ip_stack_change_dhcp;
	pal_stack.ip_stack_ipv4_dstaddr_add = ip_stack_add_dstaddr;
	pal_stack.ip_stack_ipv4_dstaddr_del = ip_stack_del_dstaddr;
	pal_stack.ip_stack_ipv4_replace = ip_stack_ipv4_replace;
	pal_stack.ip_stack_ipv4_add = ip_stack_ipv4_add;
	pal_stack.ip_stack_ipv4_delete = ip_stack_ipv4_delete;

	//arp
	pal_stack.ip_stack_arp_add = ip_stack_arp_add;
	pal_stack.ip_stack_arp_delete = ip_stack_arp_delete;
	pal_stack.ip_stack_arp_request = ip_stack_arp_request;
	pal_stack.ip_stack_arp_gratuitousarp_enable = ip_arp_stack_gratuitousarp_enable;
	pal_stack.ip_stack_arp_ttl = ip_arp_stack_ttl_enable;
	pal_stack.ip_stack_arp_age_timeout = ip_arp_stack_age_timeout_enable;
	pal_stack.ip_stack_arp_retry_interval = ip_arp_stack_retry_interval_enable;

	//route
	pal_stack.ip_stack_vrf_create = ip_stack_new_vr;
	pal_stack.ip_stack_vrf_delete = ip_stack_del_vr;

	ipcom_arp_action_entry_register_notify(ip_arp_stack_cb);
	return OK;
}
#endif


