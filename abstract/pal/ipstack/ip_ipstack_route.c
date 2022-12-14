/*
 * ip_stack_route.c
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */


#define IPCOM_USE_HOOK
#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "linklist.h"
#include "if.h"
#include "interface.h"
#include <log.h>
#include "table.h"
#include "nsm_mac.h"
#include "nsm_arp.h"
#include "rib.h"
#include "thread.h"
#include "vrf.h"
#include "nexthop.h"
#include "zserv.h"
#include "redistribute.h"
#include "interface.h"
#include "debug.h"

#include "ip_ipstack.h"
#include "../kernel/pal_interface.h"

#if 0
#include "ipnet_cmd.h"

#define IPNET_CMD_ROUTE_ADD        0   /* Add a route */
#define IPNET_CMD_ROUTE_FLUSH      1   /* Remove all routes */
#define IPNET_CMD_ROUTE_DELETE     2   /* Delete a specific route */
#define IPNET_CMD_ROUTE_CHANGE     3   /* Change aspects of a route (such as its gateway) */

#define IPNET_CMD_ROUTE_NAME_BUF   256

static int ip_ipstack_route_issue_request(Ip_fd sock, Ipnet_cmd_route *cmd,
		struct Ipnet_rt_msghdr **rtm_reply, Ip_bool silent)
{
	int err;
	Ip_pid_t pid;
	Ip_u32 seq;
	int size;
	Ip_u16 bufsize;
	struct Ipnet_rt_msghdr *rtm;
	struct Ip_sockaddr *sa;
	int addrs;
	Ip_u8 type;

	ip_assert(rtm_reply && *rtm_reply == IP_NULL);

	addrs = 0;
	size = sizeof(struct Ipnet_rt_msghdr);
	/*
	 if (cmd->dst.sa.sa_family == 0 && cmd->command != IPNET_CMD_ROUTE_VR)
	 {
	 IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("No destination address specified." IP_LF));
	 return -IP_ERRNO_EINVAL;
	 }
	 */

	if (cmd->dst.sa.sa_family != 0)
	{
		IP_BIT_SET(addrs, IPNET_RTA_DST);
		size += IPCOM_SA_LEN_GET(&cmd->dst.sa);
	}

	if (cmd->gateway.sa.sa_family != 0)
	{
		IP_BIT_SET(addrs, IPNET_RTA_GATEWAY);
		size += IPCOM_SA_LEN_GET(&cmd->gateway.sa);
	}
	else
	{
		if (IP_BIT_ISSET(cmd->rt_flags, IPNET_RTF_GATEWAY | IPNET_RTF_TUNNELEND)
				&& cmd->command == IPNET_CMD_ROUTE_ADD)
		{
			IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("No gateway address specified." IP_LF));
			return -IP_ERRNO_EINVAL;
		}
	}

	if (cmd->netmask.sa.sa_family != 0)
	{
		IP_BIT_SET(addrs, IPNET_RTA_NETMASK);
		size += IPCOM_SA_LEN_GET(&cmd->netmask.sa);
	}

	bufsize = sizeof(struct Ipnet_rt_msghdr) + 8 * sizeof(struct Ip_sockaddr_in6);
	rtm = ipcom_malloc(bufsize);
	if (rtm == IP_NULL)
	return -IP_ERRNO_ENOMEM;
	*rtm_reply = rtm;

	pid = ipcom_getpid();
	seq = (int) ipcom_random();

	ipcom_memset(rtm, 0, sizeof(struct Ipnet_rt_msghdr));
	rtm->rtm_msglen = (Ip_u16) size;
	rtm->rtm_version = IPNET_RTM_VERSION;
	rtm->rtm_addrs = addrs;
	rtm->rtm_pid = pid;
	rtm->rtm_seq = seq;
	rtm->rtm_table = cmd->table;
	switch (cmd->command)
	{
		case IPNET_CMD_ROUTE_ADD :
		type = IPNET_RTM_ADD;
		rtm->rtm_flags = cmd->rt_flags;
		break;
		case IPNET_CMD_ROUTE_DELETE :
		type = IPNET_RTM_DELETE;
		break;
		/*    case IPNET_CMD_ROUTE_GET :
		 type = IPNET_RTM_GET;
		 break;*/
		case IPNET_CMD_ROUTE_CHANGE :
		type = IPNET_RTM_CHANGE;
		rtm->rtm_flags = cmd->rt_flags;
		break;
		/*    case IPNET_CMD_ROUTE_VR:
		 if (cmd->add_vr)
		 {
		 if (cmd->vr_name != IP_NULL)
		 {
		 struct Ip_sioc_route_table sioc_rt;
		 sioc_rt.vr = IPCOM_VR_ANY;
		 ipcom_strcpy(sioc_rt.name, cmd->vr_name);
		 if (ipcom_socketioctl(sock, IP_SIOCGETROUTETAB, &sioc_rt) < 0)
		 {
		 ipcom_printf("Failed to add virtual router by name: %s"IP_LF,
		 ipcom_strerror(ipcom_errno));
		 return -ipcom_errno;
		 }
		 ipcom_printf("Added virtual router default table. VR=%u, table=%u, name=%s"IP_LF,
		 sioc_rt.vr,
		 (unsigned)sioc_rt.table,
		 sioc_rt.name);
		 return 0;
		 }
		 type = IPNET_RTM_NEWVR;
		 }
		 else
		 type = IPNET_RTM_DELVR;
		 break;*/
		default:
		IP_PANIC2();
		return -IP_ERRNO_EINVAL;
	}

	rtm->rtm_type = type;
	rtm->rtm_index = cmd->ifindex;

	rtm->rtm_inits = cmd->initis;
	rtm->rtm_rmx.rmx_rtt = cmd->rtt;
	rtm->rtm_rmx.rmx_rttvar = cmd->rttvar;
	rtm->rtm_rmx.rmx_mtu = cmd->mtu;
	rtm->rtm_rmx.rmx_hopcount = cmd->hopcount;
	rtm->rtm_rmx.rmx_expire = cmd->expire;

	sa = (struct Ip_sockaddr *) (rtm + 1);
	ipcom_memcpy(sa, &cmd->dst, IPCOM_SA_LEN_GET(&cmd->dst.sa));

	if (cmd->gateway.sa.sa_family)
	{
		sa = (struct Ip_sockaddr *) ((Ip_u8 *) sa + IPCOM_SA_LEN_GET(sa));
		ipcom_memcpy(sa, &cmd->gateway, IPCOM_SA_LEN_GET(&cmd->gateway.sa));
	}
	if (cmd->netmask.sa.sa_family)
	{
		sa = (struct Ip_sockaddr *) ((Ip_u8 *) sa + IPCOM_SA_LEN_GET(sa));
		ipcom_memcpy(sa, &cmd->netmask, IPCOM_SA_LEN_GET(&cmd->netmask.sa));
	}

	if (ipcom_setsockopt(sock, IP_SOL_SOCKET, IP_SO_X_VR, &cmd->vr_new, sizeof(cmd->vr_new)) != 0)
	return -ipcom_errno;

	err = (int)ipcom_send(sock, rtm, rtm->rtm_msglen, 0);
	if (err < 0)
	return -ipcom_errno;

	do
	{
		size = ipcom_recv(sock, rtm, bufsize, 0);
		if (size < 0)
		return -ipcom_errno;
		ip_assert(rtm->rtm_msglen == size);
	}while (rtm->rtm_type != type || rtm->rtm_seq != seq || rtm->rtm_pid != pid);

	return - (int) rtm->rtm_errno;
}

static int
ip_ipstack_route_add(Ip_fd sock, Ipnet_cmd_route *cmd)
{
	char str[IPNET_CMD_ROUTE_NAME_BUF];
	int err;
	struct Ipnet_rt_msghdr *rtm = IP_NULL;

	cmd->command = IPNET_CMD_ROUTE_ADD;
	err = ip_ipstack_route_issue_request(sock, cmd, &rtm, cmd->silent);
	if (err < 0)
	goto cleanup;
	cleanup:
	if (rtm != IP_NULL)
	ipcom_free(rtm);
	return err;
}

static int
ip_ipstack_route_delete(Ip_fd sock, Ipnet_cmd_route *cmd)
{
	char str[IPNET_CMD_ROUTE_NAME_BUF];
	int err;
	struct Ipnet_rt_msghdr *rtm = IP_NULL;

	cmd->command = IPNET_CMD_ROUTE_DELETE;
	err = ip_ipstack_route_issue_request(sock, cmd, &rtm, cmd->silent);
	if (err < 0)
	goto cleanup;
	cleanup:
	if (rtm != IP_NULL)
	ipcom_free(rtm);
	return err;
}

static int
ip_ipstack_route_change(Ip_fd sock, Ipnet_cmd_route *cmd)
{
	char str[IPNET_CMD_ROUTE_NAME_BUF];
	int err;
	struct Ipnet_rt_msghdr *rtm = IP_NULL;
	cmd->command = IPNET_CMD_ROUTE_CHANGE;
	err = ip_ipstack_route_issue_request(sock, cmd, &rtm, cmd->silent);
	if (err < 0)
	goto cleanup;
	cleanup:
	if (rtm != IP_NULL)
	ipcom_free(rtm);
	return err;
}

static int
ip_ipstack_route_rib(int cmd, struct prefix *p, struct rib *rib)
{
	Ipnet_cmd_route cmd;
	int sock_addr_len = 0, max_prefixlen = 0;
	struct nexthop *nexthop = NULL;
	struct prefix tmp;
	int nexthop_num = 0;
	int discard;
	os_memset(&tmp, 0, sizeof(struct prefix));
	ipcom_memset(cmd, 0, sizeof(Ipnet_cmd_route));
	cmd.command = -1;
	cmd.domain = IP_AF_INET;
	cmd.expire = IPCOM_ADDR_INFINITE;
	cmd.rt_flags = IPNET_RTF_UP | IPNET_RTF_DONE;
#if IPCOM_VR_MAX > 1
	cmd.vr = ipcom_proc_vr_get();
	cmd.vr_new = cmd.vr;
#endif
	cmd.table = rib->table; //IPCOM_ROUTE_TABLE_DEFAULT;

	/*if(rib->type == ZEBRA_ROUTE_CONNECT)
	 cmd.rt_flags |= IPNET_RTF_HOST;
	 else */if(rib->type == ZEBRA_ROUTE_STATIC)
	cmd.rt_flags |= IPNET_RTF_STATIC;
	else
	cmd.rt_flags |= IPNET_RTF_DYNAMIC;

	if(p->family == AF_INET)
	{
		cmd.domain = IP_AF_INET;
		if(p->prefixlen == IPV4_MAX_PREFIXLEN)
		cmd.rt_flags |= IPNET_RTF_HOST;

		sock_addr_len = sizeof(struct Ip_sockaddr_in);
		max_prefixlen = IPV4_MAX_PREFIXLEN;

		IPCOM_SA_LEN_SET(&cmd.dst.sa, sizeof(union Ipnet_cmd_route_addr));
		cmd.dst.sa.sa_family = (Ip_sa_family_t) cmd.domain;
		cmd.dst.sa_in.sin_addr.s_addr = p->u.prefix4.s_addr;

		IPCOM_SA_LEN_SET(&cmd.netmask.sa, sizeof(union Ipnet_cmd_route_addr));
		cmd.netmask.sa.sa_family = (Ip_sa_family_t)cmd.domain;
		masklen2ip (p->prefixlen, &cmd.netmask.sa_in.sin_addr);
	}
#ifdef HAVE_IPV6
	else if(p->family == AF_INET6)
	{
		cmd.domain = IP_AF_INET6;
		if(p->prefixlen == IPV6_MAX_PREFIXLEN)
		cmd.rt_flags |= IPNET_RTF_HOST;
		sock_addr_len = sizeof(struct Ip_sockaddr_in6);
		max_prefixlen = IPV6_MAX_PREFIXLEN;

		IPCOM_SA_LEN_SET(&cmd.dst.sa, sizeof(union Ipnet_cmd_route_addr));
		cmd.dst.sa.sa_family = (Ip_sa_family_t) cmd.domain;
		os_memcpy(&cmd.dst.sa_in6.sin6_addr, &p->u.prefix6, sizeof(struct in6_addr));

		IPCOM_SA_LEN_SET(&cmd.netmask.sa, sizeof(union Ipnet_cmd_route_addr));
		cmd.netmask.sa.sa_family = (Ip_sa_family_t)cmd.domain;
		masklen2ip6 (p->prefixlen, &cmd.netmask.sa_in6.sin6_addr);
	}
#endif

	if(p->u.prefix4.s_addr == 0
#ifdef HAVE_IPV6
			|| os_memcmp(&p->u.prefix6, &tmp.u.prefix6 sizeof(struct in6_addr)) == 0
#endif
	)
	{
		IP_BIT_CLR(cmd.rt_flags, IPNET_RTF_HOST);
		IP_BIT_SET(cmd.rt_flags, IPNET_RTF_MASK);
		cmd.dst.sa.sa_family = (Ip_sa_family_t)cmd.domain;
		IPCOM_SA_LEN_SET(&cmd.dst.sa, sock_addr_len);
		cmd.netmask.sa.sa_family = (Ip_sa_family_t)cmd.domain;
		IPCOM_SA_LEN_SET(&cmd.netmask.sa, sock_addr_len);
	}
	IP_BIT_SET(cmd.initis, IPNET_RTV_RTTVAR);
	cmd.rttvar = rib->metric;
	cmd.vr = rib->vrf_id;
	nexthop_num = 0;
	for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
	{
		if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
		continue;
		if (cmd == RTM_NEWROUTE && !CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
		continue;
		if (cmd == RTM_DELROUTE && !CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB))
		continue;
		if( nexthop->type == NEXTHOP_TYPE_IFINDEX ||
				nexthop->type == NEXTHOP_TYPE_IFNAME ||
				nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX ||
				nexthop->type == NEXTHOP_TYPE_IPV4_IFNAME ||
				nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX ||
				nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME )
		cmd.ifindex = ipcom_if_nametoindex(nexthop->ifindex);

		if( nexthop->type == NEXTHOP_TYPE_BLACKHOLE )
		IP_BIT_SET(cmd.rt_flags, IPNET_RTF_BLACKHOLE);

		if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
		continue;

		nexthop_num++;
	}
}

#if 0
IP_STATIC int
ipnet_cmd_route_parse_arg(int argc, char **argv, Ipnet_cmd_route *cmd)
{
	Ip_bool iface = IP_FALSE;
	Ip_bool done = IP_TRUE;
	Ipcom_cmd_int_str_map cmd_map[] =
	{
		{	IPNET_CMD_ROUTE_ADD, "add"},
		{	IPNET_CMD_ROUTE_FLUSH, "flush"},
		{	IPNET_CMD_ROUTE_DELETE, "delete"},
		{	IPNET_CMD_ROUTE_CHANGE, "change"},
		{	IPNET_CMD_ROUTE_GET, "get"},
		{	IPNET_CMD_ROUTE_SHOW, "show"},
		{	IPNET_CMD_ROUTE_MONITOR, "monitor"},
		{	IPNET_CMD_ROUTE_VR, "rtab"},
		{	IPNET_CMD_ROUTE_VR, "vr"},
		/* Generic flags */
		{	IPNET_CMD_ROUTE_SWITCH_NO_DNS, "-n"},
		{	IPNET_CMD_ROUTE_SWITCH_SILENT, "-silent"},
		{	IPNET_CMD_ROUTE_SWITCH_SILENT, "-q"},
		{	IPNET_CMD_ROUTE_SWITCH_DEVICE, "-dev"},
		{	IPNET_CMD_ROUTE_SWITCH_NO_LLINFO, "-nollinfo"},
		/* IP domain */
		{	IPNET_CMD_ROUTE_SWITCH_INET, "-inet"},
		{	IPNET_CMD_ROUTE_SWITCH_INET6, "-inet6"},
		/* Mask */
		{	IPNET_CMD_ROUTE_SWITCH_NETMASK, "-netmask"},
		{	IPNET_CMD_ROUTE_SWITCH_PREFIXLEN, "-prefixlen"},
		/* IP domain */
		{	IPNET_CMD_ROUTE_SWITCH_HOST, "-host"},
		{	IPNET_CMD_ROUTE_SWITCH_NET, "-net"},
		/* Route flags */
		{	IPNET_CMD_ROUTE_SWITCH_CLONING, "-cloning"},
		{	IPNET_CMD_ROUTE_SWITCH_IFACE, "-iface"},
		{	IPNET_CMD_ROUTE_SWITCH_NOTDONE, "-notdone"},
		{	IPNET_CMD_ROUTE_SWITCH_STATIC, "-static"},
		{	IPNET_CMD_ROUTE_SWITCH_NOSTATIC, "-nostatic"},
		{	IPNET_CMD_ROUTE_SWITCH_REJECT, "-reject"},
		{	IPNET_CMD_ROUTE_SWITCH_BLACKHOLE, "-blackhole"},
		{	IPNET_CMD_ROUTE_SWITCH_LLINFO, "-llinfo"},
		{	IPNET_CMD_ROUTE_SWITCH_XRESOLVE, "-xresolve"},
		{	IPNET_CMD_ROUTE_SWITCH_PROTO1, "-proto1"},
		{	IPNET_CMD_ROUTE_SWITCH_PROTO2, "-proto2"},
		{	IPNET_CMD_ROUTE_SWITCH_PREF, "-pref"},
		{	IPNET_CMD_ROUTE_SWITCH_TUNNELEND, "-tunnelend"},
		{	IPNET_CMD_ROUTE_SWITCH_MBINDING, "-mbinding"},
		{	IPNET_CMD_ROUTE_SWITCH_SKIP, "-skip"},
		/* Metric flags */
		{	IPNET_CMD_ROUTE_SWITCH_RTT, "-rtt"},
		{	IPNET_CMD_ROUTE_SWITCH_RTTVAR, "-rttvar"},
		{	IPNET_CMD_ROUTE_SWITCH_MTU, "-mtu"},
		{	IPNET_CMD_ROUTE_SWITCH_HOPCOUNT, "-hopcount"},
		{	IPNET_CMD_ROUTE_SWITCH_EXPIRE, "-expire"},
		/* Vr flags */
		{	IPNET_CMD_ROUTE_SWITCH_VR, "-V"},
		{	IPNET_CMD_ROUTE_SWITCH_VR_ADD, "-add"},
		{	IPNET_CMD_ROUTE_SWITCH_VR_DEL, "-delete"},
		{	IPNET_CMD_ROUTE_SWITCH_VR_BYNAME, "-addbyname"},
		/* Table index */
		{	IPNET_CMD_ROUTE_SWITCH_RT_TABLE, "-T"},
		/* MPLS shortcut route */
#ifdef IPMPLS
		{	IPNET_CMD_ROUTE_SWITCH_MPLS_PW, "-mpls"},
#endif
		{	IPNET_CMD_ROUTE_SWITCH_EOL, IP_NULL},
	};
	int max_prefixlen;
	int sock_addr_len;
	int i;
	int arg;
	Ip_bool silent = IP_FALSE;

	if (argc == 1)
	{
		ipnet_cmd_route_print_usage();
		return -1;
	}

	ipcom_memset(cmd, 0, sizeof(Ipnet_cmd_route));
	cmd->command = -1;
	cmd->domain = IP_AF_INET;
	cmd->expire = IPCOM_ADDR_INFINITE;
	cmd->rt_flags = IPNET_RTF_UP | IPNET_RTF_DONE | IPNET_RTF_STATIC;
#if IPCOM_VR_MAX > 1
	cmd->vr = ipcom_proc_vr_get();
	cmd->vr_new = cmd->vr;
#endif
	cmd->table = IPCOM_ROUTE_TABLE_DEFAULT;

	sock_addr_len = sizeof(struct Ip_sockaddr_in);
	max_prefixlen = 32;

	/* Parse command and options */
	arg = 1;
	for (i = 0; cmd_map[i].str != IP_NULL; i++)
	{
		if (ipcom_strcmp(argv[arg], cmd_map[i].str) == 0)
		{
			if (IPNET_CMD_ROUTE_IS_COMMAND(cmd_map[i].key))
			cmd->command = cmd_map[i].key;
			else
			{
				switch (cmd_map[i].key)
				{
					case IPNET_CMD_ROUTE_SWITCH_NO_DNS: /* -n */
					cmd->no_dns = IP_TRUE;
					break;
					case IPNET_CMD_ROUTE_SWITCH_DEVICE: /* -dev ifname */
					if (++arg >= argc)
					{
						IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("ifname must follow %s" IP_LF, cmd_map[i].str));
						return -IP_ERRNO_EINVAL;
					}
					cmd->ifindex = ipcom_if_nametoindex(argv[arg]);
					if (cmd->ifindex == 0)
					{
						IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("%s is not a valid interface name" IP_LF, argv[arg]));
						return -IP_ERRNO_EINVAL;
					}
					break;
					case IPNET_CMD_ROUTE_SWITCH_NO_LLINFO: /* -nollinfo */
					cmd->no_llinfo = IP_TRUE;
					break;
					case IPNET_CMD_ROUTE_SWITCH_INET: /* -inet */
					cmd->show_domain = IP_AF_INET;
					break;
					case IPNET_CMD_ROUTE_SWITCH_INET6: /* -inet6 */
					cmd->domain = IP_AF_INET6;
					cmd->show_domain = IP_AF_INET6;
					sock_addr_len = sizeof(struct Ip_sockaddr_in6);
					max_prefixlen = 128;
					break;
					case IPNET_CMD_ROUTE_SWITCH_NETMASK: /* -netmask x */
					if (++arg >= argc)
					{
						IPNET_CMD_ROUTE_CHECK_SILENT(
								ipcom_printf("A IPv4 style netmask must follow %s" IP_LF, cmd_map[i].str));
						return -IP_ERRNO_EINVAL;
					}
					i = ipcom_getsockaddrbyaddrname(cmd->domain, cmd->no_dns,
							argv[arg], &cmd->netmask.sa);
					if (i < 0)
					{
						IPNET_CMD_ROUTE_CHECK_SILENT(
								ipcom_printf("%s is an invalid IPv4 netmask format." IP_LF, argv[arg]));
						return i;
					}
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_MASK);
					break;
					case IPNET_CMD_ROUTE_SWITCH_PREFIXLEN: /* -prefixlen x */
					if (++arg >= argc)
					{
						IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("An integer between 0-%d must follow %s" IP_LF,
										max_prefixlen, cmd_map[i].str));
						return -IP_ERRNO_EINVAL;
					}

					if ((i = ipnet_cmd_route_set_mask_from_prefixlen(cmd, max_prefixlen, argv[arg])) < 0)
					{
						IPNET_CMD_ROUTE_CHECK_SILENT(
								ipcom_printf("An integer between 0-%d must follow '-prefixlen'" IP_LF,
										max_prefixlen));
						return i;
					}
					break;
					case IPNET_CMD_ROUTE_SWITCH_HOST: /* -host */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_HOST);
					break;
					case IPNET_CMD_ROUTE_SWITCH_NET: /* -net */
					IP_BIT_CLR(cmd->rt_flags, IPNET_RTF_HOST);
					break;
					case IPNET_CMD_ROUTE_SWITCH_CLONING: /* -cloning */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_CLONING);
					break;
					case IPNET_CMD_ROUTE_SWITCH_IFACE: /* -iface */
					iface = IP_TRUE;
					break;
					case IPNET_CMD_ROUTE_SWITCH_NOTDONE: /* -notdone */
					done = IP_FALSE;
					break;
					case IPNET_CMD_ROUTE_SWITCH_STATIC: /* -static */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_STATIC);
					break;
					case IPNET_CMD_ROUTE_SWITCH_NOSTATIC: /* -nostatic */
					IP_BIT_CLR(cmd->rt_flags, IPNET_RTF_STATIC);
					break;
					case IPNET_CMD_ROUTE_SWITCH_REJECT: /* -reject */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_REJECT);
					break;
					case IPNET_CMD_ROUTE_SWITCH_BLACKHOLE: /* -blackhole */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_BLACKHOLE);
					break;
					case IPNET_CMD_ROUTE_SWITCH_LLINFO: /* -llinfo */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_LLINFO);
					break;
					case IPNET_CMD_ROUTE_SWITCH_XRESOLVE: /* -xresolve */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_XRESOLVE);
					break;
					case IPNET_CMD_ROUTE_SWITCH_PROTO1: /* -proto1 */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_PROTO1);
					break;
					case IPNET_CMD_ROUTE_SWITCH_PROTO2: /* -proto2 */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_PROTO2);
					break;
					case IPNET_CMD_ROUTE_SWITCH_PREF: /* -pref */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_PREF);
					break;
					case IPNET_CMD_ROUTE_SWITCH_TUNNELEND: /* -tunnelend */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_TUNNELEND);
					IP_BIT_CLR(cmd->rt_flags, IPNET_RTF_GATEWAY);
					break;
					case IPNET_CMD_ROUTE_SWITCH_MBINDING: /* -mbinding */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_MBINDING);
					break;
					case IPNET_CMD_ROUTE_SWITCH_SKIP: /* -skip */
					IP_BIT_SET(cmd->rt_flags, IPNET_RTF_SKIP);
					break;
					case IPNET_CMD_ROUTE_SWITCH_VR_BYNAME:
					cmd->vr_name = argv[++arg];
					cmd->add_vr = IP_TRUE;
					break;
					case IPNET_CMD_ROUTE_SWITCH_RTT: /* -rtt x */
					case IPNET_CMD_ROUTE_SWITCH_RTTVAR: /* -rttvar x */
					case IPNET_CMD_ROUTE_SWITCH_MTU: /* -mtu x */
					case IPNET_CMD_ROUTE_SWITCH_HOPCOUNT: /* -hopcount x */
					case IPNET_CMD_ROUTE_SWITCH_EXPIRE: /* -expire x */
					case IPNET_CMD_ROUTE_SWITCH_VR: /* -V x */
					case IPNET_CMD_ROUTE_SWITCH_VR_ADD: /* -add x */
					case IPNET_CMD_ROUTE_SWITCH_VR_DEL: /* -delete x */
					case IPNET_CMD_ROUTE_SWITCH_RT_TABLE: /* -T */
#ifdef IPMPLS
					case IPNET_CMD_ROUTE_SWITCH_MPLS_PW: /* -mpls x */
#endif
					if (++arg >= argc)
					{
						IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("A integer must follow %s." IP_LF,
										cmd_map[i].str));
						return -IP_ERRNO_EINVAL;
					}
					{
						int val;
						val = ipcom_atoi(argv[arg]);
						if (val < 0)
						{
							IPNET_CMD_ROUTE_CHECK_SILENT(
									ipcom_printf("A positive integer must follow %s." IP_LF, cmd_map[i].str));
							return -IP_ERRNO_EINVAL;
						}
						switch (cmd_map[i].key)
						{
							case IPNET_CMD_ROUTE_SWITCH_RTT: /* -rtt x */
							IP_BIT_SET(cmd->initis, IPNET_RTV_RTT);
							cmd->rtt = val;
							break;
							case IPNET_CMD_ROUTE_SWITCH_RTTVAR: /* -rttvar x */
							IP_BIT_SET(cmd->initis, IPNET_RTV_RTTVAR);
							cmd->rttvar = val;
							break;
							case IPNET_CMD_ROUTE_SWITCH_MTU: /* -mtu x */
							IP_BIT_SET(cmd->initis, IPNET_RTV_MTU);
							cmd->mtu = val;
							break;
							case IPNET_CMD_ROUTE_SWITCH_HOPCOUNT: /* -hopcount x */
							IP_BIT_SET(cmd->initis, IPNET_RTV_HOPCOUNT);
							cmd->hopcount = val;
							break;
							case IPNET_CMD_ROUTE_SWITCH_EXPIRE: /* -expire x */
							IP_BIT_SET(cmd->initis, IPNET_RTV_EXPIRE);
							cmd->expire = val;
							break;
							case IPNET_CMD_ROUTE_SWITCH_VR_ADD : /* -add x */
							cmd->vr_new = val;
							cmd->add_vr = IP_TRUE;
							break;
							case IPNET_CMD_ROUTE_SWITCH_VR: /* -V x */
							case IPNET_CMD_ROUTE_SWITCH_VR_DEL: /* -delete x */
							cmd->vr_new = val;
							break;
							case IPNET_CMD_ROUTE_SWITCH_RT_TABLE:
							cmd->table = (Ip_u32)val;
							break;
#ifdef IPMPLS
							case IPNET_CMD_ROUTE_SWITCH_MPLS_PW:
							IPCOM_SA_LEN_SET(&cmd->gateway.sa_mpls, sizeof(struct Ip_sockaddr_mpls));
							cmd->gateway.sa_mpls.smpls_family = IP_AF_MPLS;
							cmd->gateway.sa_mpls.smpls_key = val;
							break;
#endif
							default:
							break;
						}
					}
					break;
					case IPNET_CMD_ROUTE_SWITCH_SILENT:
					cmd->silent = IP_TRUE;
					silent = IP_TRUE;
					break;
					default:
					IP_PANIC();
					return -IP_ERRNO_EINVAL;
				}
			}
#if IPCOM_VR_MAX > 1
			if (ipcom_proc_vr_get() != cmd->vr_new)
			ipcom_proc_vr_set(cmd->vr_new);
#endif
			arg++;
			if (arg >= argc)
			break;
			i = -1;
		}
	}

	if (cmd->command == -1)
	{
		if (arg == argc)
		IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("No command specified" IP_LF));
		else
		IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("%s is an unknown command." IP_LF, argv[arg]));
		return -1;
	}

	if (arg < argc)
	{
		if (ipcom_strcmp(argv[arg], "default") == 0)
		{
			IP_BIT_CLR(cmd->rt_flags, IPNET_RTF_HOST);
			IP_BIT_SET(cmd->rt_flags, IPNET_RTF_MASK);
			cmd->dst.sa.sa_family = (Ip_sa_family_t)cmd->domain;
			IPCOM_SA_LEN_SET(&cmd->dst.sa, sock_addr_len);
			cmd->netmask.sa.sa_family = (Ip_sa_family_t)cmd->domain;
			IPCOM_SA_LEN_SET(&cmd->netmask.sa, sock_addr_len);
		}
		else
		{
			char *str_prefixlen;

			str_prefixlen = ipcom_strstr(argv[arg], "/");
			if (str_prefixlen != IP_NULL && ipcom_strlen(str_prefixlen) > 1)
			{
				*str_prefixlen = '\0';
				ipnet_cmd_route_set_mask_from_prefixlen(cmd, max_prefixlen, str_prefixlen + 1);
			}

			/* Read destination */
			i = ipcom_getsockaddrbyaddrname(cmd->domain, cmd->no_dns, argv[arg], &cmd->dst.sa);
			if (i < 0)
			{
				IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("%s is an invalid destination." IP_LF,
								argv[arg]));
				return i;
			}
#ifdef IPCOM_USE_INET6
			if (cmd->domain == IP_AF_INET6 && IPNET_IP6_IS_SCOPE_LINK_LOCAL(&cmd->dst.sa_in6.sin6_addr))
			cmd->netmask.sa_in6.sin6_scope_id = 0xffffffff;
#endif /* IPCOM_USE_INET6 */
		}
		arg++;
	}

	if (cmd->command == IPNET_CMD_ROUTE_ADD
			&& IP_BIT_ISFALSE(cmd->rt_flags, IPNET_RTF_HOST)
			&& cmd->netmask.sa.sa_family == 0)
	{
		IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("No network mask was specified." IP_LF));
		return -IP_ERRNO_EINVAL;
	}

	if (arg < argc)
	{
		/* Read gateway */
		i = ipcom_getsockaddrbyaddrname(IP_BIT_ISSET(cmd->rt_flags, IPNET_RTF_LLINFO) ? IP_AF_LINK : cmd->domain,
				IP_BIT_ISSET(cmd->rt_flags, IPNET_RTF_LLINFO) ? IP_TRUE : cmd->no_dns,
				argv[arg],
				&cmd->gateway.sa);
		if (i < 0)
		{
			IPNET_CMD_ROUTE_CHECK_SILENT(ipcom_printf("%s is an invalid gateway." IP_LF,
							argv[arg]));
			return i;
		}
		if (!iface && IP_BIT_ISFALSE(cmd->rt_flags, IPNET_RTF_TUNNELEND))
		IP_BIT_SET(cmd->rt_flags, IPNET_RTF_GATEWAY);
		if (!done)
		IP_BIT_CLR(cmd->rt_flags, IPNET_RTF_DONE);
	}

	return 0;
}
#endif
#else

/* Hack for GNU libc version 2. */
#ifndef MSG_TRUNC
#define MSG_TRUNC      0x20
#endif /* MSG_TRUNC */
#define NL_PKT_BUF_SIZE 8192
#define NL_DEFAULT_ROUTE_METRIC 20

/* Socket interface to kernel */
struct nlsock
{
	int sock;
	int seq;
	int nl_pid;
	struct sockaddr_nl snl;
	const char *name;
} netlink_cmd =
{ -1, 0, 0,
{ 0 }, "netlink-cmd" }; /* command channel */

static struct
{
	char *p;
	size_t size;
} nl_rcvbuf;

static const struct message nlmsg_str[] =
{
{ RTM_NEWROUTE, "RTM_NEWROUTE" },
{ RTM_DELROUTE, "RTM_DELROUTE" },
{ RTM_GETROUTE, "RTM_GETROUTE" },
{ RTM_NEWLINK, "RTM_NEWLINK" },
{ RTM_DELLINK, "RTM_DELLINK" },
{ RTM_GETLINK, "RTM_GETLINK" },
{ RTM_NEWADDR, "RTM_NEWADDR" },
{ RTM_DELADDR, "RTM_DELADDR" },
{ RTM_GETADDR, "RTM_GETADDR" },
{ 0, NULL } };

static int addattr32(struct nlmsghdr *n, size_t maxlen, int type, int data);
static int addattr_l(struct nlmsghdr *n, size_t maxlen, int type, void *data,
		size_t alen);
static int rta_addattr_l(struct rtattr *rta, size_t maxlen, int type,
		void *data, size_t alen);
static const char * nl_msg_type_to_str(uint16_t msg_type);
static const char * nl_rtproto_to_str(u_char rtproto);

#ifndef SO_RCVBUFFORCE
#define SO_RCVBUFFORCE  (33)
#endif

/* Make socket for Linux netlink interface. */
static int netlink_socket(struct nlsock *nl, unsigned long groups,
		vrf_id_t vrf_id)
{
	int ret;
	struct sockaddr_nl snl;
	int sock;
	int namelen;
	int save_errno;
	printf("%s open %s socket\r\n", __func__, nl->name);
	sock = ip_socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock < 0)
	{
		zlog(ZLOG_NSM, LOG_ERR, "Can't open %s socket: %s", nl->name,
				ipcom_strerror(ipcom_errno));
		printf("Can't open %s socket: %s", nl->name,
				ipcom_strerror(ipcom_errno));
		return -1;
	}
	nl->nl_pid = getpid();
#if 1
	memset(&snl, 0, sizeof snl);
	snl.nl_family = AF_NETLINK;
	snl.nl_groups = groups;

	/* Bind the socket to the netlink structure for anything. */
	ret = ip_bind(sock, (struct sockaddr *) &snl, sizeof snl);
	save_errno = ipcom_errno;
	if (ret < 0)
	{
		zlog(ZLOG_NSM, LOG_ERR, "Can't bind %s socket to group 0x%x: %s",
				nl->name, snl.nl_groups, ipcom_strerror(save_errno));
		ip_close(sock);
		return -1;
	}

	/* multiple netlink sockets will have different nl_pid */
	namelen = sizeof snl;
	ret = ip_getsockname(sock, (struct sockaddr *) &snl,
			(socklen_t *) &namelen);
	if (ret < 0 || namelen != sizeof snl)
	{
		zlog(ZLOG_NSM, LOG_ERR, "Can't get %s socket name: %s", nl->name,
				ipcom_strerror(ipcom_errno));
		ip_close(sock);
		return -1;
	}

	nl->snl = snl;
#endif
	nl->sock = sock;
	return ret;
}

/* Receive message from netlink interface and pass those information
 to the given function. */
static int netlink_parse_info(
		int (*filter)(struct sockaddr_nl *, struct nlmsghdr *, vrf_id_t),
		struct nlsock *nl, struct nsm_vrf *zvrf)
{
	int status;
	int ret = 0;
	int error;

	while (1)
	{
		struct iovec iov =
		{ .iov_base = nl_rcvbuf.p, .iov_len = nl_rcvbuf.size, };
		struct sockaddr_nl snl;
		struct msghdr msg =
		{ .msg_name = (void *) &snl, .msg_namelen = sizeof snl, .msg_iov = &iov,
				.msg_iovlen = 1 };
		struct nlmsghdr *h;

		status = ip_recvmsg(nl->sock, &msg, 0);
		if (status < 0)
		{
			if (ipcom_errno == EINTR)
				continue;
			if (ipcom_errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			zlog(ZLOG_NSM, LOG_ERR, "%s recvmsg overrun: %s", nl->name,
					ipcom_strerror(ipcom_errno));
			continue;
		}

		if (status == 0) //netlink-cmd EOF
		{
			zlog(ZLOG_NSM, LOG_ERR, "%s EOF", nl->name);
			return -1;
		}

		if (msg.msg_namelen != sizeof snl)
		{
			zlog(ZLOG_NSM, LOG_ERR, "%s sender address length error: length %d",
					nl->name, msg.msg_namelen);
			return -1;
		}

		for (h = (struct nlmsghdr *) nl_rcvbuf.p;
				NLMSG_OK(h, (unsigned int) status); h = NLMSG_NEXT(h, status))
		{
			/* Finish of reading. */
			if (h->nlmsg_type == NLMSG_DONE)
				return ret;

			/* Error handling. */
			if (h->nlmsg_type == NLMSG_ERROR)
			{
				struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA(h);
				int errnum = err->error;
				int msg_type = err->msg.nlmsg_type;

				/* If the error field is zero, then this is an ACK */
				if (err->error == 0)
				{
					if (IS_ZEBRA_DEBUG_KERNEL)
					{
						zlog_debug(ZLOG_NSM,
								"%s: %s ACK: type=%s(%u), seq=%u, pid=%u",
								__FUNCTION__, nl->name,
								lookup(nlmsg_str, err->msg.nlmsg_type),
								err->msg.nlmsg_type, err->msg.nlmsg_seq,
								err->msg.nlmsg_pid);
					}

					/* return if not a multipart message, otherwise continue */
					if (!(h->nlmsg_flags & NLM_F_MULTI))
					{
						return 0;
					}
					continue;
				}

				if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr)))
				{
					zlog(ZLOG_NSM, LOG_ERR, "%s error: message truncated",
							nl->name);
					return -1;
				}

				/* Deal with errors that occur because of races in link handling */
				if (nl == &netlink_cmd
						&& ((msg_type == RTM_DELROUTE
								&& (-errnum == ENODEV || -errnum == ESRCH))
								|| (msg_type == RTM_NEWROUTE
										&& -errnum == EEXIST)))
				{
					if (IS_ZEBRA_DEBUG_KERNEL)
						zlog_debug(ZLOG_NSM,
								"%s: error: %s type=%s(%u), seq=%u, pid=%u",
								nl->name, ipcom_strerror(-errnum),
								lookup(nlmsg_str, msg_type), msg_type,
								err->msg.nlmsg_seq, err->msg.nlmsg_pid);
					return 0;
				}

				zlog_err(ZLOG_NSM, "%s error: %s, type=%s(%u), seq=%u, pid=%u",
						nl->name, ipcom_strerror(-errnum),
						lookup(nlmsg_str, msg_type), msg_type,
						err->msg.nlmsg_seq, err->msg.nlmsg_pid);
				return -1;
			}

			/* OK we got netlink message. */
			if (IS_ZEBRA_DEBUG_KERNEL)
				zlog_debug(ZLOG_NSM,
						"netlink_parse_info: %s type %s(%u), seq=%u, pid=%u",
						nl->name, lookup(nlmsg_str, h->nlmsg_type),
						h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid);

			/* skip unsolicited messages originating from command socket
			 * linux sets the originators port-id for {NEW|DEL}ADDR messages,
			 * so this has to be checked here. */
			if (nl != &netlink_cmd
					&& h->nlmsg_pid
							== /*netlink_cmd.nl_pid*/netlink_cmd.snl.nl_pid
					&& (h->nlmsg_type != RTM_NEWADDR
							&& h->nlmsg_type != RTM_DELADDR))
			{
				if (IS_ZEBRA_DEBUG_KERNEL)
					zlog_debug(ZLOG_NSM,
							"netlink_parse_info: %s packet comes from %s",
							netlink_cmd.name, nl->name);
				continue;
			}

			error = (*filter)(&snl, h, zvrf->vrf_id);
			if (error < 0)
			{
				zlog(ZLOG_NSM, LOG_ERR, "%s filter function error", nl->name);
				ret = error;
			}
		}

		/* After error care. */
		if (msg.msg_flags & MSG_TRUNC)
		{
			zlog(ZLOG_NSM, LOG_ERR, "%s error: message truncated!", nl->name);
			zlog(ZLOG_NSM, LOG_ERR,
					"Must restart with larger --nl-bufsize value!");
			continue;
		}
		if (status)
		{
			zlog(ZLOG_NSM, LOG_ERR, "%s error: data remnant size %d", nl->name,
					status);
			return -1;
		}
	}
	return ret;
}

static const struct message rtproto_str[] =
{
{ RTPROT_REDIRECT, "redirect" },
{ RTPROT_KERNEL, "kernel" },
{ RTPROT_BOOT, "boot" },
{ RTPROT_STATIC, "static" },
{ RTPROT_GATED, "GateD" },
{ RTPROT_RA, "router advertisement" },
{ RTPROT_MRT, "MRT" },
{ RTPROT_ZEBRA, "Zebra" },
#ifdef RTPROT_BIRD
		{	RTPROT_BIRD, "BIRD"},
#endif /* RTPROT_BIRD */
		{ 0, NULL } };

/* Utility function  comes from iproute2.
 Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
static int addattr_l(struct nlmsghdr *n, size_t maxlen, int type, void *data,
		size_t alen)
{
	size_t len;
	struct rtattr *rta;

	len = RTA_LENGTH(alen);

	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;

	rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

	return 0;
}

static int rta_addattr_l(struct rtattr *rta, size_t maxlen, int type,
		void *data, size_t alen)
{
	size_t len;
	struct rtattr *subrta;

	len = RTA_LENGTH(alen);

	if (RTA_ALIGN(rta->rta_len) + len > maxlen)
		return -1;

	subrta = (struct rtattr *) (((char *) rta) + RTA_ALIGN(rta->rta_len));
	subrta->rta_type = type;
	subrta->rta_len = len;
	memcpy(RTA_DATA(subrta), data, alen);
	rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;

	return 0;
}

/* Utility function comes from iproute2.
 Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */

static int addattr32(struct nlmsghdr *n, size_t maxlen, int type, int data)
{
	size_t len;
	struct rtattr *rta;

	len = RTA_LENGTH(4);

	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;

	rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), &data, 4);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

	return 0;
}

static int netlink_talk_filter(struct sockaddr_nl *snl, struct nlmsghdr *h,
		vrf_id_t vrf_id)
{
	zlog_warn(ZLOG_NSM, "netlink_talk: ignoring message type 0x%04x vrf %u",
			h->nlmsg_type, vrf_id);
	return 0;
}

/* sendmsg() to netlink socket then recvmsg(). */
static int netlink_talk(struct nlmsghdr *n, struct nlsock *nl,
		struct nsm_vrf *zvrf)
{
	int status;
	struct sockaddr_nl snl;
	struct iovec iov =
	{ .iov_base = (void *) n, .iov_len = n->nlmsg_len };
	struct msghdr msg =
	{ .msg_name = (void *) &snl, .msg_namelen = sizeof snl, .msg_iov = &iov,
			.msg_iovlen = 1, };
	int save_errno;

	memset(&snl, 0, sizeof snl);
	snl.nl_family = AF_NETLINK;

	n->nlmsg_seq = ++nl->seq;

	/* Request an acknowledgement by setting NLM_F_ACK */
	n->nlmsg_flags |= NLM_F_ACK;

	if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(ZLOG_NSM, "netlink_talk: %s type %s(%u), seq=%u", nl->name,
				lookup(nlmsg_str, n->nlmsg_type), n->nlmsg_type, n->nlmsg_seq);

	/* Send message to netlink interface. */
	ipcom_errno = 0;
	status = ip_sendmsg(nl->sock, &msg, 0);
	save_errno = ipcom_errno; //IP_ERRNO_ENXIO

	if (status < 0)
	{
		zlog(ZLOG_NSM, LOG_ERR, "netlink_talk sendmsg() error: %s",
				ipcom_strerror(save_errno));
		return -1;
	}

	/*
	 * Get reply from netlink socket.
	 * The reply should either be an acknowlegement or an error.
	 */
	return netlink_parse_info(netlink_talk_filter, nl, zvrf);
}

/* This function takes a nexthop as argument and adds
 * the appropriate netlink attributes to an existing
 * netlink message.
 *
 * @param routedesc: Human readable description of route type
 *                   (direct/recursive, single-/multipath)
 * @param bytelen: Length of addresses in bytes.
 * @param nexthop: Nexthop information
 * @param nlmsg: nlmsghdr structure to fill in.
 * @param req_size: The size allocated for the message.
 */
static void _netlink_route_build_singlepath(const char *routedesc, int bytelen,
		struct nexthop *nexthop, struct nlmsghdr *nlmsg, struct rtmsg *rtmsg,
		size_t req_size)
{
	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ONLINK))
		rtmsg->rtm_flags |= RTNH_F_ONLINK;
	if (nexthop->type == NEXTHOP_TYPE_IPV4
			|| nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
	{
		addattr_l(nlmsg, req_size, RTA_GATEWAY, &nexthop->gate.ipv4, bytelen);
		if (nexthop->src.ipv4.s_addr)
			addattr_l(nlmsg, req_size, RTA_PREFSRC, &nexthop->src.ipv4,
					bytelen);

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(ZLOG_NSM, "netlink_route_multipath() (%s): "
					"nexthop via %s if %s", routedesc,
					inet_ntoa(nexthop->gate.ipv4),
					ifindex2ifname(nexthop->ifindex));
	}
#ifdef HAVE_IPV6
	if (nexthop->type == NEXTHOP_TYPE_IPV6
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
	{
		addattr_l (nlmsg, req_size, RTA_GATEWAY,
				&nexthop->gate.ipv6, bytelen);

		if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(ZLOG_NSM, "netlink_route_multipath() (%s): "
				"nexthop via %s if %s",
				routedesc,
				inet6_ntoa (nexthop->gate.ipv6),
				ifindex2ifname(nexthop->ifindex));
	}
#endif /* HAVE_IPV6 */
	if (nexthop->type == NEXTHOP_TYPE_IFINDEX
			|| nexthop->type == NEXTHOP_TYPE_IFNAME
			|| nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
	{
		addattr32(nlmsg, req_size, RTA_OIF, ifindex2ifkernel(nexthop->ifindex));

		if (nexthop->src.ipv4.s_addr)
			addattr_l(nlmsg, req_size, RTA_PREFSRC, &nexthop->src.ipv4,
					bytelen);

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(ZLOG_NSM, "netlink_route_multipath() (%s): "
					"nexthop via if %s", routedesc,
					ifindex2ifname(nexthop->ifindex));
	}

	if (nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
	{
		addattr32(nlmsg, req_size, RTA_OIF, ifindex2ifkernel(nexthop->ifindex));

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(ZLOG_NSM, "netlink_route_multipath() (%s): "
					"nexthop via if %s", routedesc,
					ifindex2ifname(nexthop->ifindex));
	}
}

/* This function takes a nexthop as argument and
 * appends to the given rtattr/rtnexthop pair the
 * representation of the nexthop. If the nexthop
 * defines a preferred source, the src parameter
 * will be modified to point to that src, otherwise
 * it will be kept unmodified.
 *
 * @param routedesc: Human readable description of route type
 *                   (direct/recursive, single-/multipath)
 * @param bytelen: Length of addresses in bytes.
 * @param nexthop: Nexthop information
 * @param rta: rtnetlink attribute structure
 * @param rtnh: pointer to an rtnetlink nexthop structure
 * @param src: pointer pointing to a location where
 *             the prefsrc should be stored.
 */
static void _netlink_route_build_multipath(const char *routedesc, int bytelen,
		struct nexthop *nexthop, struct rtattr *rta, struct rtnexthop *rtnh,
		union g_addr **src)
{
	rtnh->rtnh_len = sizeof(*rtnh);
	rtnh->rtnh_flags = 0;
	rtnh->rtnh_hops = 0;
	rta->rta_len += rtnh->rtnh_len;

	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ONLINK))
		rtnh->rtnh_flags |= RTNH_F_ONLINK;

	if (nexthop->type == NEXTHOP_TYPE_IPV4
			|| nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
	{
		rta_addattr_l(rta, NL_PKT_BUF_SIZE, RTA_GATEWAY, &nexthop->gate.ipv4,
				bytelen);
		rtnh->rtnh_len += sizeof(struct rtattr) + bytelen;

		if (nexthop->src.ipv4.s_addr)
			*src = &nexthop->src;

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(ZLOG_NSM, "netlink_route_multipath() (%s): "
					"nexthop via %s if %s", routedesc,
					inet_ntoa(nexthop->gate.ipv4),
					ifindex2ifname(nexthop->ifindex));
	}
#ifdef HAVE_IPV6
	if (nexthop->type == NEXTHOP_TYPE_IPV6
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
	{
		rta_addattr_l (rta, NL_PKT_BUF_SIZE, RTA_GATEWAY,
				&nexthop->gate.ipv6, bytelen);
		rtnh->rtnh_len += sizeof (struct rtattr) + bytelen;

		if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(ZLOG_NSM, "netlink_route_multipath() (%s): "
				"nexthop via %s if %s",
				routedesc,
				inet6_ntoa (nexthop->gate.ipv6),
				ifindex2ifname(nexthop->ifindex));
	}
#endif /* HAVE_IPV6 */
	/* ifindex */
	if (nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX
			|| nexthop->type == NEXTHOP_TYPE_IFINDEX
			|| nexthop->type == NEXTHOP_TYPE_IFNAME)
	{
		rtnh->rtnh_ifindex = ifindex2ifkernel(nexthop->ifindex);
		if (nexthop->src.ipv4.s_addr)
			*src = &nexthop->src;
		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(ZLOG_NSM, "netlink_route_multipath() (%s): "
					"nexthop via if %s", routedesc,
					ifindex2ifname(nexthop->ifindex));
	}
	else if (nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
	{
		rtnh->rtnh_ifindex = ifindex2ifkernel(nexthop->ifindex);

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(ZLOG_NSM, "netlink_route_multipath() (%s): "
					"nexthop via if %s", routedesc,
					ifindex2ifname(nexthop->ifindex));
	}
	else
	{
		rtnh->rtnh_ifindex = 0;
	}
}

/* Log debug information for netlink_route_multipath
 * if debug logging is enabled.
 *
 * @param cmd: Netlink command which is to be processed
 * @param p: Prefix for which the change is due
 * @param nexthop: Nexthop which is currently processed
 * @param routedesc: Semantic annotation for nexthop
 *                     (recursive, multipath, etc.)
 * @param family: Address family which the change concerns
 */
static void _netlink_route_debug(int cmd, struct prefix *p,
		struct nexthop *nexthop, const char *routedesc, int family,
		struct nsm_vrf *zvrf)
{
	if (IS_ZEBRA_DEBUG_KERNEL)
	{
		char buf[PREFIX_STRLEN];
		zlog_debug(ZLOG_NSM,
				"netlink_route_multipath() (%s): %s %s vrf %u type %s",
				routedesc, lookup(nlmsg_str, cmd),
				prefix2str(p, buf, sizeof(buf)), zvrf->vrf_id,
				nexthop_type_to_str(nexthop->type));
	}
}
#ifdef IP_STACK_DEBUG
int kernel_rib_table_debug(struct prefix *p, struct rib *rib)
{
	int recursing;
	struct nexthop *nexthop = NULL, *tnexthop = NULL;
	char prefix[PREFIX_STRLEN], buf[256];
	//rib_table_info_t *info = rn->table->info;
	if (p)
	{
		memset(buf, 0, sizeof(buf));
		memset(prefix, 0, sizeof(prefix));
		snprintf(buf, sizeof(buf), "%s vrf %u",
				prefix2str(p, prefix, sizeof(prefix)),
				rib->vrf_id);
	}
	//RNODE_FOREACH_RIB (rn, rib)
	{
		for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
		{
			memset(prefix, 0, sizeof(prefix));
			inet_ntop(p->family, &nexthop->gate, prefix, sizeof(prefix));

			printf(" ====== kernel rib add: (%s) %s %s \r\n", prefix_family_str(p), buf, prefix);
		}
	}
	return OK;
}
#endif
/* Routing table change via netlink interface. */
static int netlink_route_multipath(int cmd, struct prefix *p, struct rib *rib)
{
	int bytelen;
	struct sockaddr_nl snl;
	struct nexthop *nexthop = NULL, *tnexthop;
	int recursing;
	int nexthop_num;
	int discard;
	int family = PREFIX_FAMILY(p);
	const char *routedesc;

	struct
	{
		struct nlmsghdr n;
		struct rtmsg r;
		char buf[NL_PKT_BUF_SIZE];
	} req;

	struct nsm_vrf *zvrf = vrf_info_lookup(rib->vrf_id);

#ifdef IP_STACK_DEBUG
	kernel_rib_table_debug(p, rib);
#endif
	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	bytelen = (family == AF_INET ? 4 : 16);

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_CREATE | NLM_F_REPLACE | NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;
	req.r.rtm_family = family;
	req.r.rtm_table = rib->table;
	req.r.rtm_dst_len = p->prefixlen;
	if(rib->type <= ZEBRA_ROUTE_STATIC)
		req.r.rtm_protocol = RTPROT_STATIC;
	else
		req.r.rtm_protocol = RTPROT_ZEBRA;
	req.r.rtm_scope = RT_SCOPE_UNIVERSE; //;
	req.r.rtm_scope = RT_SCOPE_LINK;

	printf("%s table=%d\r\n", __func__, req.r.rtm_table);
	req.r.rtm_table = IPCOM_ROUTE_TABLE_DEFAULT;

	if ((rib->flags & ZEBRA_FLAG_BLACKHOLE) || (rib->flags & ZEBRA_FLAG_REJECT))
		discard = 1;
	else
		discard = 0;

	if (cmd == RTM_NEWROUTE)
	{
		if (discard)
		{
			if (rib->flags & ZEBRA_FLAG_BLACKHOLE)
				req.r.rtm_type = RTN_BLACKHOLE;
			else if (rib->flags & ZEBRA_FLAG_REJECT)
				req.r.rtm_type = RTN_UNREACHABLE;
			else
				assert(RTN_BLACKHOLE != RTN_UNREACHABLE); /* false */
		}
		else
			req.r.rtm_type = RTN_UNICAST;
	}

	addattr_l(&req.n, sizeof req, RTA_DST, &p->u.prefix, bytelen);

	/* Metric. */
	addattr32(&req.n, sizeof req, RTA_PRIORITY, NL_DEFAULT_ROUTE_METRIC);

	//if (req.r.rtm_table == IPCOM_ROUTE_TABLE_DEFAULT)        /*yinwenjun */

	addattr_l(&req.n, sizeof req, RTA_VR, &rib->vrf_id, sizeof(rib->vrf_id));

	if (rib->mtu || rib->nexthop_mtu)
	{
		char buf[NL_PKT_BUF_SIZE];
		struct rtattr *rta = (void *) buf;
		u_int32_t mtu = rib->mtu;
		if (!mtu || (rib->nexthop_mtu && rib->nexthop_mtu < mtu))
			mtu = rib->nexthop_mtu;
		rta->rta_type = RTA_METRICS;
		rta->rta_len = RTA_LENGTH(0);
		rta_addattr_l(rta, NL_PKT_BUF_SIZE, RTAX_MTU, &mtu, sizeof mtu);
		addattr_l(&req.n, NL_PKT_BUF_SIZE, RTA_METRICS, RTA_DATA(rta),
				RTA_PAYLOAD(rta));
	}

	if (discard)
	{
		if (cmd == RTM_NEWROUTE)
			for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
			{
				/* We shouldn't encounter recursive nexthops on discard routes,
				 * but it is probably better to handle that case correctly anyway.
				 */
				if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
					continue;
				SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);
			}
		goto skip;
	}

	/* Count overall nexthops so we can decide whether to use singlepath
	 * or multipath case. */
	nexthop_num = 0;
	for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
	{
		if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
			continue;
		if (cmd
				== RTM_NEWROUTE&& !CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
			continue;
		if (cmd == RTM_DELROUTE && !CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB))
			continue;

		if (nexthop->type != NEXTHOP_TYPE_IFINDEX
				&& nexthop->type != NEXTHOP_TYPE_IFNAME)
			req.r.rtm_scope = RT_SCOPE_UNIVERSE;

		nexthop_num++;
	}

	/* Singlepath case. */
	if (nexthop_num == 1 || MULTIPATH_NUM == 1)
	{
		nexthop_num = 0;
		for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
		{
			if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
				continue;

			if ((cmd == RTM_NEWROUTE
					&& CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
					|| (cmd == RTM_DELROUTE
							&& CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB)))
			{
				routedesc = recursing ? "recursive, 1 hop" : "single hop";

				_netlink_route_debug(cmd, p, nexthop, routedesc, family, zvrf);
				_netlink_route_build_singlepath(routedesc, bytelen, nexthop,
						&req.n, &req.r, sizeof req);

				if (cmd == RTM_NEWROUTE)
					SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);

				nexthop_num++;
				break;
			}
		}
	}
	else
	{
		char buf[NL_PKT_BUF_SIZE];
		struct rtattr *rta = (void *) buf;
		struct rtnexthop *rtnh;
		union g_addr *src = NULL;

		rta->rta_type = RTA_MULTIPATH;
		rta->rta_len = RTA_LENGTH(0);
		rtnh = RTA_DATA(rta);

		nexthop_num = 0;
		for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
		{
			if (nexthop_num >= MULTIPATH_NUM)
				break;

			if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
				continue;

			if ((cmd == RTM_NEWROUTE
					&& CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
					|| (cmd == RTM_DELROUTE
							&& CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB)))
			{
				routedesc = recursing ? "recursive, multihop" : "multihop";
				nexthop_num++;

				_netlink_route_debug(cmd, p, nexthop, routedesc, family, zvrf);
				_netlink_route_build_multipath(routedesc, bytelen, nexthop, rta,
						rtnh, &src);
				rtnh = RTNH_NEXT(rtnh);

				if (cmd == RTM_NEWROUTE)
					SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);
			}
		}
		if (src)
			addattr_l(&req.n, sizeof req, RTA_PREFSRC, &src->ipv4, bytelen);

		if (rta->rta_len > RTA_LENGTH(0))
			addattr_l(&req.n, NL_PKT_BUF_SIZE, RTA_MULTIPATH, RTA_DATA(rta),
					RTA_PAYLOAD(rta));
	}

	/* If there is no useful nexthop then return. */
	if (nexthop_num == 0)
	{
		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(ZLOG_NSM,
					"netlink_route_multipath(): No useful nexthop.");
		return 0;
	}

	skip:

	/* Destination netlink address. */
	memset(&snl, 0, sizeof snl);
	snl.nl_family = AF_NETLINK;

	/* Talk to netlink socket. */
	return netlink_talk(&req.n, &netlink_cmd, zvrf);
}

int kernel_route_rib(struct prefix *p, struct rib *old, struct rib *new)
{
	if (!old && new)
	{
		//kernel_rib_table_debug(p, new);
		return netlink_route_multipath(RTM_NEWROUTE, p, new);
	}
	if (old && !new)
	{
		//kernel_rib_table_debug(p, old);
		return netlink_route_multipath(RTM_DELROUTE, p, old);
	}
	/* Replace, can be done atomically if metric does not change;
	 * netlink uses [prefix, tos, priority] to identify prefix.
	 * Now metric is not sent to kernel, so we can just do atomic replace. */
	//kernel_rib_table_debug(p, new);
	return netlink_route_multipath(RTM_NEWROUTE, p, new);
}

/* Exported interface function.  This function simply calls
 netlink_socket (). */
void kernel_init(struct nsm_vrf *zvrf)
{

	/*  int groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR;
	 groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
	 netlink_socket (&zvrf->netlink_cmd, groups, zvrf->vrf_id);*/
	int groups = 0; //RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;
	netlink_socket(&netlink_cmd, groups, 0/*zvrf->vrf_id*/);

	if (netlink_cmd.sock > 0)
	{
		nl_rcvbuf.p = XMALLOC(MTYPE_NETLINK_NAME, NL_PKT_BUF_SIZE);
		nl_rcvbuf.size = NL_PKT_BUF_SIZE;
	}
}

/*
 void
 kernel_terminate (struct nsm_vrf *zvrf)
 {
 //  THREAD_READ_OFF (zvrf->t_netlink);

 if (zvrf->netlink.sock >= 0)
 {
 close (zvrf->netlink.sock);
 zvrf->netlink.sock = -1;
 }

 if (zvrf->netlink_cmd.sock >= 0)
 {
 close (zvrf->netlink_cmd.sock);
 zvrf->netlink_cmd.sock = -1;
 }
 }
 */

/*
 * nl_msg_type_to_str
 */
static const char *
nl_msg_type_to_str(uint16_t msg_type)
{
	return lookup(nlmsg_str, msg_type);
}

/*
 * nl_rtproto_to_str
 */
static const char *
nl_rtproto_to_str(u_char rtproto)
{
	return lookup(rtproto_str, rtproto);
}

#endif
