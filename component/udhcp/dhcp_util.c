/*
 * util.c
 *
 *  Created on: Apr 20, 2019
 *      Author: zhurish
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"


#include "dhcp_def.h"
#include "dhcp_lease.h"
#include "dhcp_util.h"
#include "dhcp_main.h"
#include "dhcpc.h"
#include "dhcp_util.h"

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

/* note: ip is a pointer to an IPv6 in network order, possibly misaliged */
int FAST_FUNC sprint_nip6(char *dest, /*const char *pre,*/ const zpl_uint8 *ip)
{
	char hexstrbuf[16 * 2];
	bin2hex(hexstrbuf, (void*)ip, 16);
	return sprintf(dest, /* "%s" */
		"%.4s:%.4s:%.4s:%.4s:%.4s:%.4s:%.4s:%.4s",
		/* pre, */
		hexstrbuf + 0 * 4,
		hexstrbuf + 1 * 4,
		hexstrbuf + 2 * 4,
		hexstrbuf + 3 * 4,
		hexstrbuf + 4 * 4,
		hexstrbuf + 5 * 4,
		hexstrbuf + 6 * 4,
		hexstrbuf + 7 * 4
	);
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

#if 0
zpl_uint16 FAST_FUNC inet_cksum(zpl_uint16 *addr, zpl_uint32 nleft)
{
	/*
	 * Our algorithm is simple, using a 32 bit accumulator,
	 * we add sequential 16 bit words to it, and at the end, fold
	 * back all the carry bits from the top 16 bits into the lower
	 * 16 bits.
	 */
	unsigned sum = 0;
	while (nleft > 1) {
		sum += *addr++;
		nleft -= 2;
	}

	/* Mop up an odd byte, if necessary */
	if (nleft == 1) {
		if (BB_LITTLE_ENDIAN)
			sum += *(zpl_uint8*)addr;
		else
			sum += *(zpl_uint8*)addr << 8;
	}

	/* Add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */

	return (zpl_uint16)~sum;
}
#endif
/*
void FAST_FUNC udhcp_sp_fd_set(struct pollfd pfds[], int extra_fd)
{
	pfds[0].events = POLLIN;
	pfds[0].fd = -1;
	if (extra_fd >= 0) {
		//close_on_exec_on(extra_fd);
		pfds[0].fd = extra_fd;
		pfds[0].events = POLLIN;
	}
	pfds[0].revents = 0;
}
*/
/*
void* FAST_FUNC xrealloc_vector(void *vector, unsigned sizeof_and_shift, int idx)
{
	int mask = 1 << (zpl_uint8)sizeof_and_shift;

	if (!(idx & (mask - 1))) {
		sizeof_and_shift >>= 8;
		vector = realloc(vector, sizeof_and_shift * (idx + mask + 1));
		memset((char*)vector + (sizeof_and_shift * idx), 0, sizeof_and_shift * (mask + 1));
	}
	return vector;
}*/

char* FAST_FUNC xasprintf(const char *format, ...)
{
	va_list p;
	int r;
	char *string_ptr;

	va_start(p, format);
	r = vasprintf(&string_ptr, format, p);
	va_end(p);

/*	if (r < 0)
		bb_error_msg_and_die(bb_msg_memory_exhausted);*/
	return string_ptr;
}



ssize_t FAST_FUNC safe_read(zpl_socket_t fd, void *buf, zpl_uint32 count)
{
	ssize_t n;

	do {
		n = ipstack_read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

#if 0
/*
 * Read all of the supplied buffer from a file.
 * This does multiple reads as necessary.
 * Returns the amount read, or -1 on an error.
 * A zpl_int16 read is returned on an end of file.
 */
ssize_t FAST_FUNC full_read(int fd, void *buf, size_t len)
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


ssize_t FAST_FUNC safe_write(int fd, const void *buf, size_t count)
{
	ssize_t n;

	for (;;) {
		n = ipstack_write(fd, buf, count);
		if (n >= 0 || errno != EINTR)
			break;
		/* Some callers set errno=0, are upset when they see EINTR.
		 * Returning EINTR is wrong since we retry write(),
		 * the "error" was transient.
		 */
		errno = 0;
		/* repeat the write() */
	}

	return n;
}

ssize_t FAST_FUNC full_write(int fd, const void *buf, size_t len)
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
void FAST_FUNC xread(int fd, void *buf, size_t count)
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
int FAST_FUNC safe_poll(zpl_socket_t ufds, int maxfd, zpl_uint32 timeout)
{
	int n = 0;
	ipstack_fd_set rfdset;
	while(1)
	{
		IPSTACK_FD_ZERO(&rfdset);
		IPSTACK_FD_SET(ufds._fd, &rfdset);
		n = ipstack_select_wait( maxfd, &rfdset, NULL, timeout);
		if (n >= 0)
			return n;
		if (timeout > 0)
			timeout--;
		if (errno == EINTR)
			continue;
		if (errno == ENOMEM)
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
		if (errno == EINTR)
			continue;
		/* Kernel is very low on memory. Retry. */
		/* I doubt many callers would handle this correctly! */
		if (errno == ENOMEM)
			continue;
		//bb_perror_msg("poll");
		return n;
	}
#endif	
}


#if defined(IPV6_PKTINFO) && !defined(IPV6_RECVPKTINFO)
# define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif


#if 0
int FAST_FUNC index_in_strings(const char *strings, const char *key)
{
	int idx = 0;

	while (*strings) {
		if (strcmp(strings, key) == 0) {
			return idx;
		}
		strings += strlen(strings) + 1; /* skip NUL */
		idx++;
	}
	return -1;
}
#endif

/* Emit a string of hex representation of bytes */
char* FAST_FUNC bin2hex(char *p, const char *cp, zpl_uint32 count)
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
char* FAST_FUNC hex2bin(char *dst, const char *str, zpl_uint32 count)
{
	errno = EINVAL;
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
	errno = (*str ? ERANGE : 0);
	return dst;
}

/*
void* FAST_FUNC xmemdup(const void *s, int n)
{
	char *p = malloc(n);
	if(p)
		return memcpy(p, s, n);
	return NULL;
}
*/
static int dhcp_client_lease_set_kernel(client_interface_t *ifter, client_lease_t *lease)
{
	int ret = 0;
	struct interface * ifp = if_lookup_by_index (ifter->ifindex);
	struct prefix cp;
	struct in_addr gate;
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
		if(p1->family == AF_INET)
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
		SET_FLAG(ifc->conf, ZEBRA_IFC_DHCPC);
		ret = nsm_pal_interface_set_address (ifp, ifc, 0);
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
		dnscp.family = AF_INET;
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
			dnscp.family = AF_INET;
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
		cp.family = AF_INET;
		cp.prefixlen = 0;
		cp.u.prefix4.s_addr = 0;
		gate.s_addr = lease->lease_gateway;
		rib_add_ipv4 (ZEBRA_ROUTE_DHCP, 0, &cp,
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
		rib_add_ipv4 (ZEBRA_ROUTE_DHCP, 0, &cp,
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
	struct in_addr gate;
	char tmp[128], tmp1[128];
	memset(tmp, 0, sizeof(tmp));
	memset(tmp1, 0, sizeof(tmp1));
	if(lease->lease_gateway)
	{
		memset(&cp, 0, sizeof(struct prefix));
		cp.family = AF_INET;
		cp.prefixlen = 0;
		cp.u.prefix4.s_addr = 0;
		gate.s_addr = lease->lease_gateway;
		rib_delete_ipv4 (ZEBRA_ROUTE_DHCP, 0, &cp,
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
		rib_delete_ipv4 (ZEBRA_ROUTE_DHCP, 0, &cp,
					 &gate,
					 ifter->ifindex, ifp->vrf_id, SAFI_UNICAST);
	}

	if(lease->lease_dns1)
	{
		struct prefix dnscp;
		ip_dns_opt_t dnsopt;
		memset(&dnscp, 0, sizeof(struct prefix));
		memset(&dnsopt, 0, sizeof(ip_dns_opt_t));
		dnscp.family = AF_INET;
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
			dnscp.family = AF_INET;
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
			if(p1->family == AF_INET)
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
			SET_FLAG(ifc->conf, ZEBRA_IFC_DHCPC);
			ret = nsm_pal_interface_unset_address (ifp, ifc, 0);
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
#if 0
#!/bin/sh
[ -z "$1" ] && echo "Error: should be run by udhcpc" && exit 1

set_classless_routes() {
	local max=128
	local type
	while [ -n "$1" -a -n "$2" -a $max -gt 0 ]; do
		[ ${1##*/} -eq 32 ] && type=host || type=net
		echo "udhcpc: adding route for $type $1 via $2"
		route add -$type "$1" gw "$2" dev "$interface"
		max=$(($max-1))
		shift 2
	done
}

setup_interface() {
	echo "udhcpc: ifconfig $interface $ip netmask ${subnet:-255.255.255.0} broadcast ${broadcast:-+}"
	ifconfig $interface $ip netmask ${subnet:-255.255.255.0} broadcast ${broadcast:-+}

	[ -n "$router" ] && [ "$router" != "0.0.0.0" ] && [ "$router" != "255.255.255.255" ] && {
		echo "udhcpc: setting default routers: $router"

		local valid_gw=""
		for i in $router ; do
			route add default gw $i dev $interface
			valid_gw="${valid_gw:+$valid_gw|}$i"
		done

		eval $(route -n | awk '
			/^0.0.0.0\W{9}('$valid_gw')\W/ {next}
			/^0.0.0.0/ {print "route del -net "$1" gw "$2";"}
		')
	}

	# CIDR STATIC ROUTES (rfc3442)
	[ -n "$staticroutes" ] && set_classless_routes $staticroutes
	[ -n "$msstaticroutes" ] && set_classless_routes $msstaticroutes
}


applied=
case "$1" in
	deconfig)
		ifconfig "$interface" 0.0.0.0
	;;
	renew)
		setup_interface update
	;;
	bound)
		setup_interface ifup
	;;
esac

# user rules
[ -f /etc/udhcpc.user ] && . /etc/udhcpc.user

exit 0

/* put all the parameters into the environment */
static char **fill_envp(struct dhcp_packet *packet)
{
	int envc;
	int i;
	char **envp, **curr;
	const char *opt_name;
	zpl_uint8 *temp;
	zpl_uint8 overload = 0;

#define BITMAP unsigned
#define BBITS (sizeof(BITMAP) * 8)
#define BMASK(i) (1 << (i & (sizeof(BITMAP) * 8 - 1)))
#define FOUND_OPTS(i) (found_opts[(unsigned)i / BBITS])
	BITMAP found_opts[256 / BBITS];

	memset(found_opts, 0, sizeof(found_opts));

	/* We need 6 elements for:
	 * "interface=IFACE"
	 * "ip=N.N.N.N" from packet->yiaddr
	 * "siaddr=IP" from packet->siaddr_nip (unless 0)
	 * "boot_file=FILE" from packet->file (unless overloaded)
	 * "sname=SERVER_HOSTNAME" from packet->sname (unless overloaded)
	 * terminating NULL
	 */
	envc = 6;
	/* +1 element for each option, +2 for subnet option: */
	if (packet) {
		/* note: do not search for "pad" (0) and "end" (255) options */
//TODO: change logic to scan packet _once_
		for (i = 1; i < 255; i++) {
			temp = udhcp_get_option(packet, i);
			if (temp) {
				if (i == DHCP_OPTION_OVERLOAD)
					overload |= *temp;
				else if (i == DHCP_SUBNET)
					envc++; /* for $mask */
				envc++;
				/*if (i != DHCP_MESSAGE_TYPE)*/
				FOUND_OPTS(i) |= BMASK(i);
			}
		}
	}
	curr = envp = xzalloc(sizeof(envp[0]) * envc);

	*curr = xasprintf("interface=%s", client_config.interface);
	putenv(*curr++);

	if (!packet)
		return envp;

	/* Export BOOTP fields. Fields we don't (yet?) export:
	 * zpl_uint8 op;      // always BOOTREPLY
	 * zpl_uint8 htype;   // hardware address type. 1 = 10mb ethernet
	 * zpl_uint8 hlen;    // hardware address length
	 * zpl_uint8 hops;    // used by relay agents only
	 * zpl_uint32  xid;
	 * zpl_uint16 secs;   // elapsed since client began acquisition/renewal
	 * zpl_uint16 flags;  // only one flag so far: bcast. Never set by server
	 * zpl_uint32  ciaddr; // client IP (usually == yiaddr. can it be different
	 *                  // if during renew server wants to give us different IP?)
	 * zpl_uint32  gateway_nip; // relay agent IP address
	 * zpl_uint8 chaddr[16]; // link-layer client hardware address (MAC)
	 * TODO: export gateway_nip as $giaddr?
	 */
	/* Most important one: yiaddr as $ip */
	*curr = xmalloc(sizeof("ip=255.255.255.255"));
	sprint_nip(*curr, "ip=", (zpl_uint8 *) &packet->yiaddr);
	putenv(*curr++);
	if (packet->siaddr_nip) {
		/* IP address of next server to use in bootstrap */
		*curr = xmalloc(sizeof("siaddr=255.255.255.255"));
		sprint_nip(*curr, "siaddr=", (zpl_uint8 *) &packet->siaddr_nip);
		putenv(*curr++);
	}
	if (!(overload & FILE_FIELD) && packet->file[0]) {
		/* watch out for invalid packets */
		*curr = xasprintf("boot_file=%."DHCP_PKT_FILE_LEN_STR"s", packet->file);
		putenv(*curr++);
	}
	if (!(overload & SNAME_FIELD) && packet->sname[0]) {
		/* watch out for invalid packets */
		*curr = xasprintf("sname=%."DHCP_PKT_SNAME_LEN_STR"s", packet->sname);
		putenv(*curr++);
	}

	/* Export known DHCP options */
	opt_name = dhcp_option_strings;
	i = 0;
	while (*opt_name) {
		zpl_uint8 code = dhcp_optflags[i].code;
		BITMAP *found_ptr = &FOUND_OPTS(code);
		BITMAP found_mask = BMASK(code);
		if (!(*found_ptr & found_mask))
			goto next;
		*found_ptr &= ~found_mask; /* leave only unknown options */
		temp = udhcp_get_option(packet, code);
		*curr = xmalloc_optname_optval(temp, &dhcp_optflags[i], opt_name);
		putenv(*curr++);
		if (code == DHCP_SUBNET) {
			/* Subnet option: make things like "$ip/$mask" possible */
			zpl_uint32  subnet;
			move_from_unaligned32(subnet, temp);
			*curr = xasprintf("mask=%u", mton(subnet));
			putenv(*curr++);
		}
 next:
		opt_name += strlen(opt_name) + 1;
		i++;
	}
	/* Export unknown options */
	for (i = 0; i < 256;) {
		BITMAP bitmap = FOUND_OPTS(i);
		if (!bitmap) {
			i += BBITS;
			continue;
		}
		if (bitmap & BMASK(i)) {
			unsigned len, ofs;

			temp = udhcp_get_option(packet, i);
			/* udhcp_get_option returns ptr to data portion,
			 * need to go back to get len
			 */
			len = temp[-OPT_DATA + OPT_LEN];
			*curr = xmalloc(sizeof("optNNN=") + 1 + len*2);
			ofs = sprintf(*curr, "opt%u=", i);
			*bin2hex(*curr + ofs, (void*) temp, len) = '\0';
			putenv(*curr++);
		}
		i++;
	}

	return envp;
}
#endif

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
	//routes
	//static_route_list
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
/*		else
		{
			var = tp = xstrndup(var, sz);
		}*/
	}
	unsetenv(var);
	//free(tp);
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
	argv[0] = (char*) CONFIG_UDHCPC_DEFAULT_SCRIPT;
	argv[1] = (char*) name;
	argv[2] = NULL;

	//spawn_and_wait(argv);
	pid = child_process_create();
	if(pid < 0)
	{
		  zlog_warn(MODULE_DEFAULT, "Can't create child process (%s), continuing", safe_strerror(errno));
		  return ;
	}
	else if(pid == 0)
	{
		super_system_execvp(CONFIG_UDHCPC_DEFAULT_SCRIPT, argv);
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
			 struct in_addr *gate, struct in_addr *src,
			 ifindex_t ifindex, vrf_id_t vrf_id, int table_id,
			 zpl_uint32, zpl_uint32, zpl_uchar, safi_t);

extern int rib_delete_ipv4 (zpl_uint32 type, zpl_uint32 flags, struct prefix_ipv4 *p,
		            struct in_addr *gate, ifindex_t ifindex,
		            vrf_id_t, safi_t safi);
*/
