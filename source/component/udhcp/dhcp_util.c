/*
 * util.c
 *
 *  Created on: Apr 20, 2019
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "log.h"
#include "nsm_include.h"
#include "dhcp_def.h"
#include "dhcpd.h"
#include "dhcp_lease.h"
#include "dhcp_util.h"
#include "dhcp_main.h"
#include "dhcpc.h"
#include "dhcp_util.h"

#include "nsm_dns.h"

const zpl_uint8 DHCP_MAC_BCAST_ADDR[6] ALIGN2 = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/* Parse string to IP in network order */
int udhcp_str2nip(const char *str, void *arg)
{
	zpl_uint32 addr = 0;
	addr = ipstack_inet_addr(str);
	/* arg maybe unaligned */
	move_to_unaligned32((zpl_uint32 *)arg, addr);
	return 1;
}



int udhcp_interface_mac(ifindex_t ifindex, zpl_uint32  *nip, zpl_uint8 *mac)
{
	struct interface * ifp = if_lookup_by_index (ifindex);
	if(ifp)
	{
		if(nip)
		{
			struct prefix address;
			if(nsm_interface_address_get_api(ifp, &address) == OK)
			{
				*nip = address.u.prefix4.s_addr;
			}
		}
		if(mac)
			nsm_interface_mac_get_api(ifp, mac, ETHER_ADDR_LEN);
		return 0;
	}
	return -1;
}



ssize_t  safe_read(zpl_socket_t fd, void *buf, zpl_uint32 count)
{
	ssize_t n;

	do {
		n = ipstack_read(fd, buf, count);
	} while (n < 0 && ipstack_errno == IPSTACK_ERRNO_EINTR);

	return n;
}
ssize_t  safe_write(zpl_socket_t fd, const void *buf, size_t count)
{
	ssize_t n;

	for (;;) {
		n = ipstack_write(fd, buf, count);
		if (n >= 0 || ipstack_errno != IPSTACK_ERRNO_EINTR)
			break;
		/* Some callers set ipstack_errno=0, are upset when they see EINTR.
		 * Returning EINTR is wrong since we retry write(),
		 * the "error" was transient.
		 */
		ipstack_errno = 0;
		/* repeat the write() */
	}

	return n;
}

#if 0
/*
 * Read all of the supplied buffer from a file.
 * This does multiple reads as necessary.
 * Returns the amount read, or -1 on an error.
 * A zpl_int16 read is returned on an end of file.
 */
ssize_t  full_read(int fd, void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_read(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already have some! */
				/* user can do another read to know the error code */
				return total;
			}
			return cc; /* read() returns -1 on failure. */
		}
		if (cc == 0)
			break;
		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
	}

	return total;
}



ssize_t  full_write(int fd, const void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_write(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already wrote some! */
				/* user can do another write to know the error code */
				return total;
			}
			return cc;  /* write() returns -1 on failure. */
		}

		total += cc;
		buf = ((const char *)buf) + cc;
		len -= cc;
	}

	return total;
}

/* Die with an error message if we can't read the entire buffer. */
void  xread(int fd, void *buf, size_t count)
{
	if (count) {
		ssize_t size = full_read(fd, buf, count);
		if ((size_t)size != count)
			;//bb_error_msg_and_die("zpl_int16 read");
	}
}
#endif

/* Wrapper which restarts poll on EINTR or ENOMEM.
 * On other errors does perror("poll") and returns.
 * Warning! May take longer than timeout_ms to return! */
int  safe_poll(zpl_socket_t ufds, int maxfd, zpl_uint32 timeout)
{
	int n = 0;
	ipstack_fd_set rfdset;
	while(1)
	{
		IPSTACK_FD_ZERO(&rfdset);
		IPSTACK_FD_SET(ipstack_fd(ufds), &rfdset);
		n = ipstack_select_wait(IPSTACK_OS, maxfd, &rfdset, NULL, timeout);
		if (n >= 0)
			return n;
		if (timeout > 0)
			timeout--;
		if (ipstack_errno == IPSTACK_ERRNO_EINTR)
			continue;
		if (ipstack_errno == IPSTACK_ERRNO_ENOMEM)
			continue;
		return n;
	}
#if 0
	while (1) {
		int n = poll(ufds, nfds, timeout);
		if (n >= 0)
			return n;
		/* Make sure we inch towards completion */
		if (timeout > 0)
			timeout--;
		/* E.g. strace causes poll to return this */
		if (ipstack_errno == IPSTACK_ERRNO_EINTR)
			continue;
		/* Kernel is very low on memory. Retry. */
		/* I doubt many callers would handle this correctly! */
		if (ipstack_errno == IPSTACK_ERRNO_ENOMEM)
			continue;
		//bb_perror_msg("poll");
		return n;
	}
#endif	
}


#if defined(IPSTACK_IPV6_PKTINFO) && !defined(IPSTACK_IPV6_RECVPKTINFO)
# define IPSTACK_IPV6_RECVPKTINFO IPSTACK_IPV6_PKTINFO
#endif



/* Emit a string of hex representation of bytes */
char*  bin2hex(char *p, const char *cp, zpl_uint32 count)
{
	const char bb_hexdigits_upcase[] ALIGN1 = "0123456789ABCDEF";
	while (count) {
		zpl_uint8 c = *cp++;
		/* put lowercase hex digits */
		*p++ = 0x20 | bb_hexdigits_upcase[c >> 4];
		*p++ = 0x20 | bb_hexdigits_upcase[c & 0xf];
		count--;
	}
	return p;
}

/* Convert "[x]x[:][x]x[:][x]x[:][x]x" hex string to binary, no more than COUNT bytes */
char* hex2bin(char *dst, const char *str, zpl_uint32 count)
{
	ipstack_errno = IPSTACK_ERRNO_EINVAL;
	while (*str && count) {
		zpl_uint8 val;
		zpl_uint8 c = *str++;
		if (isdigit(c))
			val = c - '0';
		else if ((c|0x20) >= 'a' && (c|0x20) <= 'f')
			val = (c|0x20) - ('a' - 10);
		else
			return NULL;
		val <<= 4;
		c = *str;
		if (isdigit(c))
			val |= c - '0';
		else if ((c|0x20) >= 'a' && (c|0x20) <= 'f')
			val |= (c|0x20) - ('a' - 10);
		else if (c == ':' || c == '\0')
			val >>= 4;
		else
			return NULL;

		*dst++ = val;
		if (c != '\0')
			str++;
		if (*str == ':')
			str++;
		count--;
	}
	ipstack_errno = (*str ? IPSTACK_ERRNO_ERANGE : 0);
	return dst;
}


static int dhcp_client_lease_set_kernel(client_interface_t *ifter, client_lease_t *lease)
{
	int ret = 0;
	struct interface * ifp = if_lookup_by_index (ifter->ifindex);
	struct prefix cp;
	struct ipstack_in_addr gate;
	struct connected *ifc;
	char tmp[128], tmp1[128];
	memset(tmp, 0, sizeof(tmp));
	memset(tmp1, 0, sizeof(tmp1));
	if(!lease->lease_address)
		return ERROR;
	sprintf(tmp1, "%s", inet_address(ntohl(lease->lease_address)));
	netmask_str2prefix_str (tmp1, inet_address(ntohl(lease->lease_netmask)), tmp);
	memset(&cp, 0, sizeof(struct prefix));
	if(str2prefix_ipv4 (tmp, (struct prefix_ipv4 *)&cp) <= 0)
	{
		return ERROR;
	}
	ifc = connected_check(ifp, (struct prefix *)&cp);
	if (!ifc)
	{
		struct prefix_ipv4 *p1, *p2;
		ifc = connected_new();
		ifc->ifp = ifp;
		/* Address. */
		p1 = prefix_new();
		p1->family = cp.family;

		prefix_copy ((struct prefix *)p1, (struct prefix *)&cp);
		ifc->address = (struct prefix *) p1;
		if(p1->family == IPSTACK_AF_INET)
		{
			/* Broadcast. */
			if (p1->prefixlen <= IPV4_MAX_PREFIXLEN - 2)
			{
				p2 = prefix_new();
				p1->family = cp.family;
				prefix_copy ((struct prefix *)p2, (struct prefix *)&cp);
				p2->prefix.s_addr = ipv4_broadcast_addr(p2->prefix.s_addr, p2->prefixlen);
				ifc->destination = (struct prefix *) p2;
			}
		}
		if (DHCPC_DEBUG_ISON(KERNEL))
		{
			zlog_debug(MODULE_DHCP," dhcp set kernel ip address %s on interface %s", tmp, ifindex2ifname(ifter->ifindex));
		}
		SET_FLAG(ifc->conf, IF_IFC_DHCPC);
		ret = nsm_halpal_interface_set_address (ifp, ifc, 0);
		if(ret != OK)
			return ERROR;
	}
	else
		return ERROR;

	if(lease->lease_dns1)
	{
		struct prefix dnscp;
		ip_dns_opt_t dnsopt;
		memset(&dnscp, 0, sizeof(struct prefix));
		memset(&dnsopt, 0, sizeof(ip_dns_opt_t));
		dnscp.family = IPSTACK_AF_INET;
		dnscp.prefixlen = IPV4_MAX_PREFIXLEN;
		dnscp.u.prefix4.s_addr = lease->lease_dns1;
		dnsopt.ifindex = ifter->ifindex;
		dnsopt.vrfid = ifp->vrf_id;

		if (DHCPC_DEBUG_ISON(KERNEL))
		{
			zlog_debug(MODULE_DHCP," dhcp set kernel dns %s ", inet_address(ntohl(lease->lease_dns1)));
		}

		nsm_ip_dns_add(&dnscp, &dnsopt, zpl_false, IP_DNS_DYNAMIC);

		if(lease->lease_dns2)
		{
			memset(&dnscp, 0, sizeof(struct prefix));
			memset(&dnsopt, 0, sizeof(ip_dns_opt_t));
			dnscp.family = IPSTACK_AF_INET;
			dnscp.prefixlen = IPV4_MAX_PREFIXLEN;
			dnscp.u.prefix4.s_addr = lease->lease_dns2;
			dnsopt.ifindex = ifter->ifindex;
			dnsopt.vrfid = ifp->vrf_id;
			nsm_ip_dns_add(&dnscp, &dnsopt, zpl_false, IP_DNS_DYNAMIC);
		}
	}
	if(strlen(lease->domain_name))
	{
		if (DHCPC_DEBUG_ISON(KERNEL))
		{
			zlog_debug(MODULE_DHCP," dhcp set kernel domain name %s ", lease->domain_name);
		}
		nsm_dns_domain_name_add_api(lease->domain_name, zpl_false);
		nsm_dns_domain_name_dynamic_api(zpl_true, zpl_false);
	}
	if(lease->lease_gateway)
	{
		memset(&cp, 0, sizeof(struct prefix));
		cp.family = IPSTACK_AF_INET;
		cp.prefixlen = 0;
		cp.u.prefix4.s_addr = 0;
		gate.s_addr = lease->lease_gateway;
		rib_add_ipv4 (ZPL_ROUTE_PROTO_DHCP, 0, &cp,
					 &gate, NULL,
					 ifter->ifindex, ifp->vrf_id, 0,
					 ifter->instance + 1000, 0, 0, SAFI_UNICAST);

		if (DHCPC_DEBUG_ISON(KERNEL))
		{
			zlog_debug(MODULE_DHCP," dhcp set kernel gateway %s ", inet_address(ntohl(lease->lease_gateway)));
		}
	}
	if(lease->lease_gateway2)
	{
		gate.s_addr = lease->lease_gateway2;
		rib_add_ipv4 (ZPL_ROUTE_PROTO_DHCP, 0, &cp,
					 &gate, NULL,
					 ifter->ifindex, ifp->vrf_id, 0,
					 ifter->instance + 1000, 0, 0, SAFI_UNICAST);
	}
	return OK;
}

static int dhcp_client_lease_unset_kernel(client_interface_t *ifter, client_lease_t *lease)
{
	int ret = 0;
	struct interface * ifp = if_lookup_by_index (ifter->ifindex);
	struct prefix cp;
	struct connected *ifc;
	struct ipstack_in_addr gate;
	char tmp[128], tmp1[128];
	memset(tmp, 0, sizeof(tmp));
	memset(tmp1, 0, sizeof(tmp1));
	if(lease->lease_gateway)
	{
		memset(&cp, 0, sizeof(struct prefix));
		cp.family = IPSTACK_AF_INET;
		cp.prefixlen = 0;
		cp.u.prefix4.s_addr = 0;
		gate.s_addr = lease->lease_gateway;
		rib_delete_ipv4 (ZPL_ROUTE_PROTO_DHCP, 0, &cp,
					 &gate,
					 ifter->ifindex, ifp->vrf_id, SAFI_UNICAST);
		if (DHCPC_DEBUG_ISON(KERNEL))
		{
			zlog_debug(MODULE_DHCP," dhcp unset kernel gateway %s ", inet_address(ntohl(lease->lease_gateway)));
		}
	}
	if(lease->lease_gateway2)
	{
		gate.s_addr = lease->lease_gateway2;
		rib_delete_ipv4 (ZPL_ROUTE_PROTO_DHCP, 0, &cp,
					 &gate,
					 ifter->ifindex, ifp->vrf_id, SAFI_UNICAST);
	}

	if(lease->lease_dns1)
	{
		struct prefix dnscp;
		ip_dns_opt_t dnsopt;
		memset(&dnscp, 0, sizeof(struct prefix));
		memset(&dnsopt, 0, sizeof(ip_dns_opt_t));
		dnscp.family = IPSTACK_AF_INET;
		dnscp.prefixlen = IPV4_MAX_PREFIXLEN;
		dnscp.u.prefix4.s_addr = lease->lease_dns1;
		dnsopt.ifindex = ifter->ifindex;
		dnsopt.vrfid = ifp->vrf_id;

		nsm_ip_dns_del(&dnscp, IP_DNS_DYNAMIC);
		if (DHCPC_DEBUG_ISON(KERNEL))
		{
			zlog_debug(MODULE_DHCP," dhcp set kernel dns %s ", inet_address(ntohl(lease->lease_dns1)));
		}
		if(lease->lease_dns2)
		{
			memset(&dnscp, 0, sizeof(struct prefix));
			memset(&dnsopt, 0, sizeof(ip_dns_opt_t));
			dnscp.family = IPSTACK_AF_INET;
			dnscp.prefixlen = IPV4_MAX_PREFIXLEN;
			dnscp.u.prefix4.s_addr = lease->lease_dns2;
			dnsopt.ifindex = ifter->ifindex;
			dnsopt.vrfid = ifp->vrf_id;
			nsm_ip_dns_del(&dnscp, IP_DNS_DYNAMIC);
		}
	}
	nsm_dns_domain_name_del_api(zpl_false);

	if(lease->lease_address)
	{
		sprintf(tmp1, "%s", inet_address(ntohl(lease->lease_address)));
		netmask_str2prefix_str (tmp1, inet_address(ntohl(lease->lease_netmask)), tmp);
		memset(&cp, 0, sizeof(struct prefix));
		if(str2prefix_ipv4 (tmp, (struct prefix_ipv4 *)&cp) <= 0)
		{
			return ERROR;
		}
		ifc = connected_check(ifp, (struct prefix *)&cp);
		if (!ifc)
		{
			struct prefix_ipv4 *p1, *p2;
			ifc = connected_new();
			ifc->ifp = ifp;
			/* Address. */
			p1 = prefix_new();
			p1->family = cp.family;

			prefix_copy ((struct prefix *)p1, (struct prefix *)&cp);
			ifc->address = (struct prefix *) p1;
			if(p1->family == IPSTACK_AF_INET)
			{
				/* Broadcast. */
				if (p1->prefixlen <= IPV4_MAX_PREFIXLEN - 2)
				{
					p2 = prefix_new();
					p1->family = cp.family;
					prefix_copy ((struct prefix *)p2, (struct prefix *)&cp);
					p2->prefix.s_addr = ipv4_broadcast_addr(p2->prefix.s_addr, p2->prefixlen);
					ifc->destination = (struct prefix *) p2;
				}
			}
			if (DHCPC_DEBUG_ISON(KERNEL))
			{
				zlog_debug(MODULE_DHCP," dhcp unset kernel ip address %s on interface %s", tmp, ifindex2ifname(ifter->ifindex));
			}
			SET_FLAG(ifc->conf, IF_IFC_DHCPC);
			ret = nsm_halpal_interface_unset_address (ifp, ifc, 0);
			if(ret != OK)
				return ERROR;
		}
	}
	else
		return ERROR;
	return OK;
}

int dhcp_client_lease_set(void *p)
{
	client_interface_t *ifter = p;
	return dhcp_client_lease_set_kernel(ifter, &ifter->lease);
}

int dhcp_client_lease_unset(void *p)
{
	client_interface_t *ifter = p;
	return dhcp_client_lease_unset_kernel(ifter, &ifter->lease);
}


/* really simple implementation, just count the bits */
static int mton(zpl_uint32  mask)
{
	int i = 0;
	mask = ntohl(mask); /* 111110000-like bit pattern */
	while (mask) {
		i++;
		mask <<= 1;
	}
	return i;
}

static char ** dhcpc_setenv(client_interface_t *ifter)
{
	char tmp[128];
	char **envp = NULL, **curr = NULL;
	client_lease_t *lease = &ifter->lease;
	curr = envp = malloc(sizeof(envp[0]) * 32);

	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp), "interface=%s", ifindex2ifname(ifter->ifindex));
	*curr = strdup(tmp);
	putenv(*curr++);

	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp), "ip=%s", inet_address(ntohl(lease->lease_address)));
	*curr = strdup(tmp);
	putenv(*curr++);

	if(lease->lease_netmask)
	{
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp), "mask=%u", mton(lease->lease_netmask));
		*curr = strdup(tmp);
		putenv(*curr++);
	}
	if (lease->siaddr_nip)
	{
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp), "siaddr=%s", inet_address(ntohl(lease->siaddr_nip)));
		*curr = strdup(tmp);
		putenv(*curr++);
	}

	if(lease->lease_broadcast)
	{
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp), "broadcast=%s", inet_address(ntohl(lease->lease_broadcast)));
		*curr = strdup(tmp);
		putenv(*curr++);
	}

	if(lease->lease_gateway)
	{
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp), "router=%s", inet_address(ntohl(lease->lease_gateway)));
		if(lease->lease_gateway2)
		{
			snprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), " %s", inet_address(ntohl(lease->lease_gateway2)));
		}
		*curr = strdup(tmp);
		putenv(*curr++);
	}

	if(lease->lease_dns1)
	{
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp), "dns=%s", inet_address(ntohl(lease->lease_dns1)));
		if(lease->lease_dns2)
		{
			snprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), " %s", inet_address(ntohl(lease->lease_dns2)));
		}
		*curr = strdup(tmp);
		putenv(*curr++);
	}

	if(strlen(lease->domain_name))
	{
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp), "domain=%s", ((lease->domain_name)));
		*curr = strdup(tmp);
		putenv(*curr++);
	}
	return envp;
}

static void dhcpc_unsetenv(const char *var)
{
	char onstack[128]; /* smaller stack setup code on x86 */
	char *tp = NULL;

	tp = strchr(var, '=');
	if (tp)
	{
		unsigned sz = tp - var;
		if (sz < sizeof(onstack))
		{
			((char*)mempcpy(onstack, var, sz))[0] = '\0';
			tp = NULL;
			var = onstack;
			unsetenv(var);
			return;
		}
	}
	unsetenv(var);
}

/* Call a script with a par file and env vars */
void udhcp_run_script(void *p, const char *name)
{
	int pid = 0;
	char **envp = NULL, **curr = NULL;
	char *argv[3] = { NULL, NULL, NULL };
	client_interface_t *ifter = (client_interface_t *)p;
	if(!ifter)
		return;
	envp = dhcpc_setenv(ifter);

	/* call script */
	argv[0] = (char*) CONFIG_DHCPC_DEFAULT_SCRIPT;
	argv[1] = (char*) name;
	argv[2] = NULL;

	pid = child_process_create();
	if(pid < 0)
	{
		  zlog_warn(MODULE_DHCP, "Can't create child process (%s), continuing", ipstack_strerror(ipstack_errno));
		  return ;
	}
	else if(pid == 0)
	{
		super_system_execvp(CONFIG_DHCPC_DEFAULT_SCRIPT, argv);
	}
	else
	{
		child_process_wait(pid, 0);
		for (curr = envp; *curr; curr++)
		{
			dhcpc_unsetenv(*curr);
		}
		free(envp);
	}
}


/*
int nsm_pal_interface_set_address (struct interface *ifp, struct prefix *cp, int secondry);
int nsm_pal_interface_unset_address (struct interface *ifp, struct prefix *cp, int secondry);
extern int nsm_ip_dns_add(struct prefix *address, ip_dns_opt_t *, zpl_bool	secondly, dns_class_t type);
extern int nsm_ip_dns_del(struct prefix *address, dns_class_t type);
extern int rib_add_ipv4 (zpl_uint32 type, zpl_uint32 flags, struct prefix_ipv4 *p,
			 struct ipstack_in_addr *gate, struct ipstack_in_addr *src,
			 ifindex_t ifindex, vrf_id_t vrf_id, int table_id,
			 zpl_uint32, zpl_uint32, zpl_uchar, safi_t);

extern int rib_delete_ipv4 (zpl_uint32 type, zpl_uint32 flags, struct prefix_ipv4 *p,
		            struct ipstack_in_addr *gate, ifindex_t ifindex,
		            vrf_id_t, safi_t safi);
*/
