/*
 * iplink.c		"ip link".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */
#include "auto_include.h"
#include "zpl_type.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <linux/mpls.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "namespace.h"

#define IPLINK_IOCTL_COMPAT	1
#ifndef LIBDIR
#define LIBDIR "/usr/lib"
#endif

#ifndef GSO_MAX_SIZE
#define GSO_MAX_SIZE		65536
#endif
#ifndef GSO_MAX_SEGS
#define GSO_MAX_SEGS		65535
#endif


static void usage(void) __attribute__((noreturn));
static int iplink_have_newlink(void);

void iplink_usage(void)
{
	if (iplink_have_newlink()) {
		fprintf(stderr,
			"Usage: ip link add [link DEV] [ name ] NAME\n"
			"		    [ txqueuelen PACKETS ]\n"
			"		    [ address LLADDR ]\n"
			"		    [ broadcast LLADDR ]\n"
			"		    [ mtu MTU ] [index IDX ]\n"
			"		    [ numtxqueues QUEUE_COUNT ]\n"
			"		    [ numrxqueues QUEUE_COUNT ]\n"
			"		    type TYPE [ ARGS ]\n"
			"\n"
			"	ip link delete { DEVICE | dev DEVICE | group DEVGROUP } type TYPE [ ARGS ]\n"
			"\n"
			"	ip link set { DEVICE | dev DEVICE | group DEVGROUP }\n"
			"			[ { up | down } ]\n"
			"			[ type TYPE ARGS ]\n");
	} else
		fprintf(stderr,
			"Usage: ip link set DEVICE [ { up | down } ]\n");

	fprintf(stderr,
		"		[ arp { on | off } ]\n"
		"		[ dynamic { on | off } ]\n"
		"		[ multicast { on | off } ]\n"
		"		[ allmulticast { on | off } ]\n"
		"		[ promisc { on | off } ]\n"
		"		[ trailers { on | off } ]\n"
		"		[ carrier { on | off } ]\n"
		"		[ txqueuelen PACKETS ]\n"
		"		[ name NEWNAME ]\n"
		"		[ address LLADDR ]\n"
		"		[ broadcast LLADDR ]\n"
		"		[ mtu MTU ]\n"
		"		[ netns { PID | NAME } ]\n"
		"		[ link-netns NAME | link-netnsid ID ]\n"
		"		[ alias NAME ]\n"
		"		[ vf NUM [ mac LLADDR ]\n"
		"			 [ vlan VLANID [ qos VLAN-QOS ] [ proto VLAN-PROTO ] ]\n"
		"			 [ rate TXRATE ]\n"
		"			 [ max_tx_rate TXRATE ]\n"
		"			 [ min_tx_rate TXRATE ]\n"
		"			 [ spoofchk { on | off} ]\n"
		"			 [ query_rss { on | off} ]\n"
		"			 [ state { auto | enable | disable} ]\n"
		"			 [ trust { on | off} ]\n"
		"			 [ node_guid EUI64 ]\n"
		"			 [ port_guid EUI64 ] ]\n"
		"		[ { xdp | xdpgeneric | xdpdrv | xdpoffload } { off |\n"
		"			  object FILE [ section NAME ] [ verbose ] |\n"
		"			  pinned FILE } ]\n"
		"		[ master DEVICE ][ vrf NAME ]\n"
		"		[ nomaster ]\n"
		"		[ addrgenmode { eui64 | none | stable_secret | random } ]\n"
		"		[ protodown { on | off } ]\n"
		"		[ gso_max_size BYTES ] | [ gso_max_segs PACKETS ]\n"
		"\n"
		"	ip link show [ DEVICE | group GROUP ] [up] [master DEV] [vrf NAME] [type TYPE]\n"
		"\n"
		"	ip link xstats type TYPE [ ARGS ]\n"
		"\n"
		"	ip link afstats [ dev DEVICE ]\n"
		"	ip link property add dev DEVICE [ altname NAME .. ]\n"
		"	ip link property del dev DEVICE [ altname NAME .. ]\n");

	if (iplink_have_newlink()) {
		fprintf(stderr,
			"\n"
			"	ip link help [ TYPE ]\n"
			"\n"
			"TYPE := { vlan | veth | vcan | vxcan | dummy | ifb | macvlan | macvtap |\n"
			"	   bridge | bond | team | ipoib | ip6tnl | ipip | sit | vxlan |\n"
			"	   gre | gretap | erspan | ip6gre | ip6gretap | ip6erspan |\n"
			"	   vti | nlmon | team_slave | bond_slave | bridge_slave |\n"
			"	   ipvlan | ipvtap | geneve | bareudp | vrf | macsec | netdevsim | rmnet |\n"
			"	   xfrm }\n");
	}
	exit(-1);
}

static void usage(void)
{
	iplink_usage();
}

static int on_off(const char *msg, const char *realval)
{
	fprintf(stderr,
		"Error: argument of \"%s\" must be \"on\" or \"off\", not \"%s\"\n",
		msg, realval);
	return -1;
}

static void *BODY;		/* cached dlopen(NULL) handle */
static struct link_util *linkutil_list;

struct link_util *get_link_kind(const char *id)
{
	void *dlh;
	char buf[256];
	struct link_util *l;

	for (l = linkutil_list; l; l = l->next)
		if (strcmp(l->id, id) == 0)
			return l;

	snprintf(buf, sizeof(buf), LIBDIR "/ip/link_%s.so", id);
	dlh = dlopen(buf, RTLD_LAZY);
	if (dlh == NULL) {
		/* look in current binary, only open once */
		dlh = BODY;
		if (dlh == NULL) {
			dlh = BODY = dlopen(NULL, RTLD_LAZY);
			if (dlh == NULL)
				return NULL;
		}
	}

	snprintf(buf, sizeof(buf), "%s_link_util", id);
	l = dlsym(dlh, buf);
	if (l == NULL)
		return NULL;

	l->next = linkutil_list;
	linkutil_list = l;
	return l;
}

static int get_link_mode(const char *mode)
{
	if (strcasecmp(mode, "default") == 0)
		return IF_LINK_MODE_DEFAULT;
	if (strcasecmp(mode, "dormant") == 0)
		return IF_LINK_MODE_DORMANT;
	return -1;
}

static int get_addr_gen_mode(const char *mode)
{
	if (strcasecmp(mode, "eui64") == 0)
		return IN6_ADDR_GEN_MODE_EUI64;
	if (strcasecmp(mode, "none") == 0)
		return IN6_ADDR_GEN_MODE_NONE;
	if (strcasecmp(mode, "stable_secret") == 0)
		return IN6_ADDR_GEN_MODE_STABLE_PRIVACY;
	if (strcasecmp(mode, "random") == 0)
		return IN6_ADDR_GEN_MODE_RANDOM;
	return -1;
}

#if IPLINK_IOCTL_COMPAT
static int have_rtnl_newlink = -1;

static int accept_msg(struct rtnl_ctrl_data *ctrl,
		      struct nlmsghdr *n, void *arg)
{
	struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(n);

	if (n->nlmsg_type == NLMSG_ERROR &&
	    (err->error == -EOPNOTSUPP || err->error == -EINVAL))
		have_rtnl_newlink = 0;
	else
		have_rtnl_newlink = 1;
	return -1;
}

static int iplink_have_newlink(void)
{
	struct {
		struct nlmsghdr		n;
		struct ifinfomsg	i;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
		.n.nlmsg_type = RTM_NEWLINK,
		.i.ifi_family = AF_UNSPEC,
	};

	if (have_rtnl_newlink < 0) {
		if (rtnl_send(&rth, &req.n, req.n.nlmsg_len) < 0) {
			perror("request send failed");
			exit(1);
		}
		rtnl_listen(&rth, accept_msg, NULL);
	}
	return have_rtnl_newlink;
}
#else /* IPLINK_IOCTL_COMPAT */
static int iplink_have_newlink(void)
{
	return 1;
}
#endif /* ! IPLINK_IOCTL_COMPAT */

static int nl_get_ll_addr_len(const char *ifname)
{
	int len;
	int dev_index = ll_name_to_index(ifname);
	struct iplink_req req = {
		.n = {
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
			.nlmsg_type = RTM_GETLINK,
			.nlmsg_flags = NLM_F_REQUEST
		},
		.i = {
			.ifi_family = preferred_family,
			.ifi_index = dev_index,
		}
	};
	struct nlmsghdr *answer;
	struct rtattr *tb[IFLA_MAX+1];

	if (dev_index == 0)
		return -1;

	if (rtnl_talk(&rth, &req.n, &answer) < 0)
		return -1;

	len = answer->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
	if (len < 0) {
		free(answer);
		return -1;
	}

	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(NLMSG_DATA(answer)),
			   len, NLA_F_NESTED);
	if (!tb[IFLA_ADDRESS]) {
		free(answer);
		return -1;
	}

	len = RTA_PAYLOAD(tb[IFLA_ADDRESS]);
	free(answer);
	return len;
}

static void iplink_parse_vf_vlan_info(int vf, int *argcp, char ***argvp,
				      struct ifla_vf_vlan_info *ivvip)
{
	int argc = *argcp;
	char **argv = *argvp;
	unsigned int vci;

	NEXT_ARG();
	if (get_unsigned(&vci, *argv, 0) || vci > 4095)
		invarg("Invalid \"vlan\" value\n", *argv);

	ivvip->vlan = vci;
	ivvip->vf = vf;
	ivvip->qos = 0;
	ivvip->vlan_proto = htons(ETH_P_8021Q);
	if (NEXT_ARG_OK()) {
		NEXT_ARG();
		if (matches(*argv, "qos") == 0) {
			NEXT_ARG();
			if (get_unsigned(&ivvip->qos, *argv, 0))
				invarg("Invalid \"qos\" value\n", *argv);
		} else {
			/* rewind arg */
			PREV_ARG();
		}
	}
	if (NEXT_ARG_OK()) {
		NEXT_ARG();
		if (matches(*argv, "proto") == 0) {
			NEXT_ARG();
			if (ll_proto_a2n(&ivvip->vlan_proto, *argv))
				invarg("protocol is invalid\n", *argv);
			if (ivvip->vlan_proto != htons(ETH_P_8021AD) &&
			    ivvip->vlan_proto != htons(ETH_P_8021Q)) {
				SPRINT_BUF(b1);
				SPRINT_BUF(b2);
				char msg[64 + sizeof(b1) + sizeof(b2)];

				sprintf(msg,
					"Invalid \"vlan protocol\" value - supported %s, %s\n",
					ll_proto_n2a(htons(ETH_P_8021Q),
					     b1, sizeof(b1)),
					ll_proto_n2a(htons(ETH_P_8021AD),
					     b2, sizeof(b2)));
				invarg(msg, *argv);
			}
		} else {
			/* rewind arg */
			PREV_ARG();
		}
	}

	*argcp = argc;
	*argvp = argv;
}

static int iplink_parse_vf(int vf, int *argcp, char ***argvp,
			   struct iplink_req *req, const char *dev)
{
	char new_rate_api = 0, count = 0, override_legacy_rate = 0;
	struct ifla_vf_rate tivt;
	int len, argc = *argcp;
	char **argv = *argvp;
	struct rtattr *vfinfo;

	tivt.min_tx_rate = -1;
	tivt.max_tx_rate = -1;

	vfinfo = addattr_nest(&req->n, sizeof(*req), IFLA_VF_INFO);

	while (NEXT_ARG_OK()) {
		NEXT_ARG();
		count++;
		if (!matches(*argv, "max_tx_rate")) {
			/* new API in use */
			new_rate_api = 1;
			/* override legacy rate */
			override_legacy_rate = 1;
		} else if (!matches(*argv, "min_tx_rate")) {
			/* new API in use */
			new_rate_api = 1;
		}
	}

	while (count--) {
		/* rewind arg */
		PREV_ARG();
	}

	while (NEXT_ARG_OK()) {
		NEXT_ARG();
		if (matches(*argv, "mac") == 0) {
			struct ifla_vf_mac ivm = { 0 };
			int halen = nl_get_ll_addr_len(dev);

			NEXT_ARG();
			ivm.vf = vf;
			len = ll_addr_a2n((char *)ivm.mac, 32, *argv);
			if (len < 0)
				return -1;
			if (halen > 0 && len != halen) {
				fprintf(stderr,
					"Invalid address length %d - must be %d bytes\n",
					len, halen);
				return -1;
			}
			addattr_l(&req->n, sizeof(*req), IFLA_VF_MAC,
				  &ivm, sizeof(ivm));
		} else if (matches(*argv, "vlan") == 0) {
			struct ifla_vf_vlan_info ivvi;

			iplink_parse_vf_vlan_info(vf, &argc, &argv, &ivvi);
			/* support the old interface in case of older kernel*/
			if (ivvi.vlan_proto == htons(ETH_P_8021Q)) {
				struct ifla_vf_vlan ivv;

				ivv.vf = ivvi.vf;
				ivv.vlan = ivvi.vlan;
				ivv.qos = ivvi.qos;
				addattr_l(&req->n, sizeof(*req),
					  IFLA_VF_VLAN, &ivv, sizeof(ivv));
			} else {
				struct rtattr *vfvlanlist;

				vfvlanlist = addattr_nest(&req->n, sizeof(*req),
							  IFLA_VF_VLAN_LIST);
				addattr_l(&req->n, sizeof(*req),
					  IFLA_VF_VLAN_INFO, &ivvi,
					  sizeof(ivvi));

				while (NEXT_ARG_OK()) {
					NEXT_ARG();
					if (matches(*argv, "vlan") != 0) {
						PREV_ARG();
						break;
					}
					iplink_parse_vf_vlan_info(vf, &argc,
								  &argv, &ivvi);
					addattr_l(&req->n, sizeof(*req),
						  IFLA_VF_VLAN_INFO, &ivvi,
						  sizeof(ivvi));
				}
				addattr_nest_end(&req->n, vfvlanlist);
			}
		} else if (matches(*argv, "rate") == 0) {
			struct ifla_vf_tx_rate ivt;

			NEXT_ARG();
			if (get_unsigned(&ivt.rate, *argv, 0))
				invarg("Invalid \"rate\" value\n", *argv);

			ivt.vf = vf;
			if (!new_rate_api)
				addattr_l(&req->n, sizeof(*req),
					  IFLA_VF_TX_RATE, &ivt, sizeof(ivt));
			else if (!override_legacy_rate)
				tivt.max_tx_rate = ivt.rate;

		} else if (matches(*argv, "max_tx_rate") == 0) {
			NEXT_ARG();
			if (get_unsigned(&tivt.max_tx_rate, *argv, 0))
				invarg("Invalid \"max tx rate\" value\n",
				       *argv);
			tivt.vf = vf;

		} else if (matches(*argv, "min_tx_rate") == 0) {
			NEXT_ARG();
			if (get_unsigned(&tivt.min_tx_rate, *argv, 0))
				invarg("Invalid \"min tx rate\" value\n",
				       *argv);
			tivt.vf = vf;

		} else if (matches(*argv, "spoofchk") == 0) {
			struct ifla_vf_spoofchk ivs;

			NEXT_ARG();
			if (matches(*argv, "on") == 0)
				ivs.setting = 1;
			else if (matches(*argv, "off") == 0)
				ivs.setting = 0;
			else
				return on_off("spoofchk", *argv);
			ivs.vf = vf;
			addattr_l(&req->n, sizeof(*req), IFLA_VF_SPOOFCHK,
				  &ivs, sizeof(ivs));

		} else if (matches(*argv, "query_rss") == 0) {
			struct ifla_vf_rss_query_en ivs;

			NEXT_ARG();
			if (matches(*argv, "on") == 0)
				ivs.setting = 1;
			else if (matches(*argv, "off") == 0)
				ivs.setting = 0;
			else
				return on_off("query_rss", *argv);
			ivs.vf = vf;
			addattr_l(&req->n, sizeof(*req), IFLA_VF_RSS_QUERY_EN,
				  &ivs, sizeof(ivs));

		} else if (matches(*argv, "trust") == 0) {
			struct ifla_vf_trust ivt;

			NEXT_ARG();
			if (matches(*argv, "on") == 0)
				ivt.setting = 1;
			else if (matches(*argv, "off") == 0)
				ivt.setting = 0;
			else
				invarg("Invalid \"trust\" value\n", *argv);
			ivt.vf = vf;
			addattr_l(&req->n, sizeof(*req), IFLA_VF_TRUST,
				  &ivt, sizeof(ivt));

		} else if (matches(*argv, "state") == 0) {
			struct ifla_vf_link_state ivl;

			NEXT_ARG();
			if (matches(*argv, "auto") == 0)
				ivl.link_state = IFLA_VF_LINK_STATE_AUTO;
			else if (matches(*argv, "enable") == 0)
				ivl.link_state = IFLA_VF_LINK_STATE_ENABLE;
			else if (matches(*argv, "disable") == 0)
				ivl.link_state = IFLA_VF_LINK_STATE_DISABLE;
			else
				invarg("Invalid \"state\" value\n", *argv);
			ivl.vf = vf;
			addattr_l(&req->n, sizeof(*req), IFLA_VF_LINK_STATE,
				  &ivl, sizeof(ivl));
		} else if (matches(*argv, "node_guid") == 0) {
			struct ifla_vf_guid ivg;

			NEXT_ARG();
			ivg.vf = vf;
			if (get_guid(&ivg.guid, *argv)) {
				invarg("Invalid GUID format\n", *argv);
				return -1;
			}
			addattr_l(&req->n, sizeof(*req), IFLA_VF_IB_NODE_GUID,
				  &ivg, sizeof(ivg));
		} else if (matches(*argv, "port_guid") == 0) {
			struct ifla_vf_guid ivg;

			NEXT_ARG();
			ivg.vf = vf;
			if (get_guid(&ivg.guid, *argv)) {
				invarg("Invalid GUID format\n", *argv);
				return -1;
			}
			addattr_l(&req->n, sizeof(*req), IFLA_VF_IB_PORT_GUID,
				  &ivg, sizeof(ivg));
		} else {
			/* rewind arg */
			PREV_ARG();
			break;
		}
	}

	if (new_rate_api) {
		int tmin, tmax;

		if (tivt.min_tx_rate == -1 || tivt.max_tx_rate == -1) {
			ipaddr_get_vf_rate(tivt.vf, &tmin, &tmax, dev);
			if (tivt.min_tx_rate == -1)
				tivt.min_tx_rate = tmin;
			if (tivt.max_tx_rate == -1)
				tivt.max_tx_rate = tmax;
		}

		if (tivt.max_tx_rate && tivt.min_tx_rate > tivt.max_tx_rate) {
			fprintf(stderr,
				"Invalid min_tx_rate %d - must be <= max_tx_rate %d\n",
				tivt.min_tx_rate, tivt.max_tx_rate);
			return -1;
		}

		addattr_l(&req->n, sizeof(*req), IFLA_VF_RATE, &tivt,
			  sizeof(tivt));
	}

	if (argc == *argcp)
		incomplete_command();

	addattr_nest_end(&req->n, vfinfo);

	*argcp = argc;
	*argvp = argv;
	return 0;
}

int iplink_parse(int argc, char **argv, struct iplink_req *req, char **type)
{
	char *name = NULL;
	char *dev = NULL;
	char *link = NULL;
	int ret, len;
	char abuf[32];
	int qlen = -1;
	int mtu = -1;
	int netns = -1;
	int vf = -1;
	int numtxqueues = -1;
	int numrxqueues = -1;
	int link_netnsid = -1;
	int index = 0;
	int group = -1;
	int addr_len = 0;

	ret = argc;

	while (argc > 0) {
		if (strcmp(*argv, "up") == 0) {
			req->i.ifi_change |= IFF_UP;
			req->i.ifi_flags |= IFF_UP;
		} else if (strcmp(*argv, "down") == 0) {
			req->i.ifi_change |= IFF_UP;
			req->i.ifi_flags &= ~IFF_UP;
		} else if (strcmp(*argv, "name") == 0) {
			NEXT_ARG();
			if (name)
				duparg("name", *argv);
			if (check_ifname(*argv))
				invarg("\"name\" not a valid ifname", *argv);
			name = *argv;
			if (!dev)
				dev = name;
		} else if (strcmp(*argv, "index") == 0) {
			NEXT_ARG();
			if (index)
				duparg("index", *argv);
			index = atoi(*argv);
			if (index <= 0)
				invarg("Invalid \"index\" value", *argv);
		} else if (matches(*argv, "link") == 0) {
			NEXT_ARG();
			link = *argv;
		} else if (matches(*argv, "address") == 0) {
			NEXT_ARG();
			addr_len = ll_addr_a2n(abuf, sizeof(abuf), *argv);
			if (addr_len < 0)
				return -1;
			addattr_l(&req->n, sizeof(*req),
				  IFLA_ADDRESS, abuf, addr_len);
		} else if (matches(*argv, "broadcast") == 0 ||
			   strcmp(*argv, "brd") == 0) {
			NEXT_ARG();
			len = ll_addr_a2n(abuf, sizeof(abuf), *argv);
			if (len < 0)
				return -1;
			addattr_l(&req->n, sizeof(*req),
				  IFLA_BROADCAST, abuf, len);
		} else if (matches(*argv, "txqueuelen") == 0 ||
			   strcmp(*argv, "qlen") == 0 ||
			   matches(*argv, "txqlen") == 0) {
			NEXT_ARG();
			if (qlen != -1)
				duparg("txqueuelen", *argv);
			if (get_integer(&qlen,  *argv, 0))
				invarg("Invalid \"txqueuelen\" value\n", *argv);
			addattr_l(&req->n, sizeof(*req),
				  IFLA_TXQLEN, &qlen, 4);
		} else if (strcmp(*argv, "mtu") == 0) {
			NEXT_ARG();
			if (mtu != -1)
				duparg("mtu", *argv);
			if (get_integer(&mtu, *argv, 0))
				invarg("Invalid \"mtu\" value\n", *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_MTU, &mtu, 4);
		} else if (strcmp(*argv, "xdpgeneric") == 0 ||
			   strcmp(*argv, "xdpdrv") == 0 ||
			   strcmp(*argv, "xdpoffload") == 0 ||
			   strcmp(*argv, "xdp") == 0) {
			bool generic = strcmp(*argv, "xdpgeneric") == 0;
			bool drv = strcmp(*argv, "xdpdrv") == 0;
			bool offload = strcmp(*argv, "xdpoffload") == 0;

			NEXT_ARG();
			if (xdp_parse(&argc, &argv, req, dev,
				      generic, drv, offload))
				exit(-1);

			if (offload && name == dev)
				dev = NULL;
		} else if (strcmp(*argv, "netns") == 0) {
			NEXT_ARG();
			if (netns != -1)
				duparg("netns", *argv);
			netns = netns_get_fd(*argv);
			if (netns >= 0)
				addattr_l(&req->n, sizeof(*req), IFLA_NET_NS_FD,
					  &netns, 4);
			else if (get_integer(&netns, *argv, 0) == 0)
				addattr_l(&req->n, sizeof(*req),
					  IFLA_NET_NS_PID, &netns, 4);
			else
				invarg("Invalid \"netns\" value\n", *argv);
		} else if (strcmp(*argv, "multicast") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_MULTICAST;

			if (strcmp(*argv, "on") == 0)
				req->i.ifi_flags |= IFF_MULTICAST;
			else if (strcmp(*argv, "off") == 0)
				req->i.ifi_flags &= ~IFF_MULTICAST;
			else
				return on_off("multicast", *argv);
		} else if (strcmp(*argv, "allmulticast") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_ALLMULTI;

			if (strcmp(*argv, "on") == 0)
				req->i.ifi_flags |= IFF_ALLMULTI;
			else if (strcmp(*argv, "off") == 0)
				req->i.ifi_flags &= ~IFF_ALLMULTI;
			else
				return on_off("allmulticast", *argv);
		} else if (strcmp(*argv, "promisc") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_PROMISC;

			if (strcmp(*argv, "on") == 0)
				req->i.ifi_flags |= IFF_PROMISC;
			else if (strcmp(*argv, "off") == 0)
				req->i.ifi_flags &= ~IFF_PROMISC;
			else
				return on_off("promisc", *argv);
		} else if (strcmp(*argv, "trailers") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_NOTRAILERS;

			if (strcmp(*argv, "off") == 0)
				req->i.ifi_flags |= IFF_NOTRAILERS;
			else if (strcmp(*argv, "on") == 0)
				req->i.ifi_flags &= ~IFF_NOTRAILERS;
			else
				return on_off("trailers", *argv);
		} else if (strcmp(*argv, "arp") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_NOARP;

			if (strcmp(*argv, "on") == 0)
				req->i.ifi_flags &= ~IFF_NOARP;
			else if (strcmp(*argv, "off") == 0)
				req->i.ifi_flags |= IFF_NOARP;
			else
				return on_off("arp", *argv);
		} else if (strcmp(*argv, "carrier") == 0) {
			int carrier;

			NEXT_ARG();
			if (strcmp(*argv, "on") == 0)
				carrier = 1;
			else if (strcmp(*argv, "off") == 0)
				carrier = 0;
			else
				return on_off("carrier", *argv);

			addattr8(&req->n, sizeof(*req), IFLA_CARRIER, carrier);
		} else if (strcmp(*argv, "vf") == 0) {
			struct rtattr *vflist;

			NEXT_ARG();
			if (get_integer(&vf,  *argv, 0))
				invarg("Invalid \"vf\" value\n", *argv);

			vflist = addattr_nest(&req->n, sizeof(*req),
					      IFLA_VFINFO_LIST);
			if (!dev)
				missarg("dev");

			len = iplink_parse_vf(vf, &argc, &argv, req, dev);
			if (len < 0)
				return -1;
			addattr_nest_end(&req->n, vflist);

			if (name == dev)
				dev = NULL;
		} else if (matches(*argv, "master") == 0) {
			int ifindex;

			NEXT_ARG();
			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				invarg("Device does not exist\n", *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_MASTER,
				  &ifindex, 4);
		} else if (strcmp(*argv, "vrf") == 0) {
			int ifindex;

			NEXT_ARG();
			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				invarg("Not a valid VRF name\n", *argv);
			if (!name_is_vrf(*argv))
				invarg("Not a valid VRF name\n", *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_MASTER,
				  &ifindex, sizeof(ifindex));
		} else if (matches(*argv, "nomaster") == 0) {
			int ifindex = 0;

			addattr_l(&req->n, sizeof(*req), IFLA_MASTER,
				  &ifindex, 4);
		} else if (matches(*argv, "dynamic") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_DYNAMIC;

			if (strcmp(*argv, "on") == 0)
				req->i.ifi_flags |= IFF_DYNAMIC;
			else if (strcmp(*argv, "off") == 0)
				req->i.ifi_flags &= ~IFF_DYNAMIC;
			else
				return on_off("dynamic", *argv);
		} else if (matches(*argv, "type") == 0) {
			NEXT_ARG();
			*type = *argv;
			argc--; argv++;
			break;
		} else if (matches(*argv, "alias") == 0) {
			NEXT_ARG();
			len = strlen(*argv);
			if (len >= IFALIASZ)
				invarg("alias too long\n", *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_IFALIAS,
				  *argv, len);
		} else if (strcmp(*argv, "group") == 0) {
			NEXT_ARG();
			if (group != -1)
				duparg("group", *argv);
			if (rtnl_group_a2n(&group, *argv))
				invarg("Invalid \"group\" value\n", *argv);
			addattr32(&req->n, sizeof(*req), IFLA_GROUP, group);
		} else if (strcmp(*argv, "mode") == 0) {
			int mode;

			NEXT_ARG();
			mode = get_link_mode(*argv);
			if (mode < 0)
				invarg("Invalid link mode\n", *argv);
			addattr8(&req->n, sizeof(*req), IFLA_LINKMODE, mode);
		} else if (strcmp(*argv, "state") == 0) {
			int state;

			NEXT_ARG();
			state = get_operstate(*argv);
			if (state < 0)
				invarg("Invalid operstate\n", *argv);

			addattr8(&req->n, sizeof(*req), IFLA_OPERSTATE, state);
		} else if (matches(*argv, "numtxqueues") == 0) {
			NEXT_ARG();
			if (numtxqueues != -1)
				duparg("numtxqueues", *argv);
			if (get_integer(&numtxqueues, *argv, 0))
				invarg("Invalid \"numtxqueues\" value\n",
				       *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_NUM_TX_QUEUES,
				  &numtxqueues, 4);
		} else if (matches(*argv, "numrxqueues") == 0) {
			NEXT_ARG();
			if (numrxqueues != -1)
				duparg("numrxqueues", *argv);
			if (get_integer(&numrxqueues, *argv, 0))
				invarg("Invalid \"numrxqueues\" value\n",
				       *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_NUM_RX_QUEUES,
				  &numrxqueues, 4);
		} else if (matches(*argv, "addrgenmode") == 0) {
			struct rtattr *afs, *afs6;
			int mode;

			NEXT_ARG();
			mode = get_addr_gen_mode(*argv);
			if (mode < 0)
				invarg("Invalid address generation mode\n",
				       *argv);
			afs = addattr_nest(&req->n, sizeof(*req), IFLA_AF_SPEC);
			afs6 = addattr_nest(&req->n, sizeof(*req), AF_INET6);
			addattr8(&req->n, sizeof(*req),
				 IFLA_INET6_ADDR_GEN_MODE, mode);
			addattr_nest_end(&req->n, afs6);
			addattr_nest_end(&req->n, afs);
		} else if (matches(*argv, "link-netns") == 0) {
			NEXT_ARG();
			if (link_netnsid != -1)
				duparg("link-netns/link-netnsid", *argv);
			link_netnsid = get_netnsid_from_name(*argv);
			/* No nsid? Try to assign one. */
			if (link_netnsid < 0)
				set_netnsid_from_name(*argv, -1);
			link_netnsid = get_netnsid_from_name(*argv);
			if (link_netnsid < 0)
				invarg("Invalid \"link-netns\" value\n",
				       *argv);
			addattr32(&req->n, sizeof(*req), IFLA_LINK_NETNSID,
				  link_netnsid);
		} else if (matches(*argv, "link-netnsid") == 0) {
			NEXT_ARG();
			if (link_netnsid != -1)
				duparg("link-netns/link-netnsid", *argv);
			if (get_integer(&link_netnsid, *argv, 0))
				invarg("Invalid \"link-netnsid\" value\n",
				       *argv);
			addattr32(&req->n, sizeof(*req), IFLA_LINK_NETNSID,
				  link_netnsid);
		} else if (strcmp(*argv, "protodown") == 0) {
			unsigned int proto_down;

			NEXT_ARG();
			if (strcmp(*argv, "on") == 0)
				proto_down = 1;
			else if (strcmp(*argv, "off") == 0)
				proto_down = 0;
			else
				return on_off("protodown", *argv);
			addattr8(&req->n, sizeof(*req), IFLA_PROTO_DOWN,
				 proto_down);
		} else if (strcmp(*argv, "gso_max_size") == 0) {
			unsigned int max_size;

			NEXT_ARG();
			if (get_unsigned(&max_size, *argv, 0) ||
			    max_size > GSO_MAX_SIZE)
				invarg("Invalid \"gso_max_size\" value\n",
				       *argv);
			addattr32(&req->n, sizeof(*req),
				  IFLA_GSO_MAX_SIZE, max_size);
		} else if (strcmp(*argv, "gso_max_segs") == 0) {
			unsigned int max_segs;

			NEXT_ARG();
			if (get_unsigned(&max_segs, *argv, 0) ||
			    max_segs > GSO_MAX_SEGS)
				invarg("Invalid \"gso_max_segs\" value\n",
				       *argv);
			addattr32(&req->n, sizeof(*req),
				  IFLA_GSO_MAX_SEGS, max_segs);
		} else {
			if (matches(*argv, "help") == 0)
				usage();

			if (strcmp(*argv, "dev") == 0)
				NEXT_ARG();
			if (dev != name)
				duparg2("dev", *argv);
			if (check_altifname(*argv))
				invarg("\"dev\" not a valid ifname", *argv);
			dev = *argv;
		}
		argc--; argv++;
	}

	ret -= argc;

	/* Allow "ip link add dev" and "ip link add name" */
	if (!name)
		name = dev;
	else if (!dev)
		dev = name;
	else if (!strcmp(name, dev))
		name = dev;

	if (dev && addr_len &&
	    !(req->n.nlmsg_flags & NLM_F_CREATE)) {
		int halen = nl_get_ll_addr_len(dev);

		if (halen >= 0 && halen != addr_len) {
			fprintf(stderr,
				"Invalid address length %d - must be %d bytes\n",
				addr_len, halen);
			return -1;
		}
	}

	if (!(req->n.nlmsg_flags & NLM_F_CREATE) && index) {
		fprintf(stderr,
			"index can be used only when creating devices.\n");
		exit(-1);
	}

	if (group != -1) {
		if (!dev) {
			if (argc) {
				fprintf(stderr,
					"Garbage instead of arguments \"%s ...\". Try \"ip link help\".\n",
					*argv);
				exit(-1);
			}
			if (req->n.nlmsg_flags & NLM_F_CREATE) {
				fprintf(stderr,
					"group cannot be used when creating devices.\n");
				exit(-1);
			}

			*type = NULL;
			return ret;
		}
	}

	if (!(req->n.nlmsg_flags & NLM_F_CREATE)) {
		if (!dev) {
			fprintf(stderr,
				"Not enough information: \"dev\" argument is required.\n");
			exit(-1);
		}

		req->i.ifi_index = ll_name_to_index(dev);
		if (!req->i.ifi_index)
			return nodev(dev);

		/* Not renaming to the same name */
		if (name == dev)
			name = NULL;
	} else {
		if (name != dev) {
			fprintf(stderr,
				"both \"name\" and \"dev\" cannot be used when creating devices.\n");
			exit(-1);
		}

		if (link) {
			int ifindex;

			ifindex = ll_name_to_index(link);
			if (!ifindex)
				return nodev(link);
			addattr32(&req->n, sizeof(*req), IFLA_LINK, ifindex);
		}

		req->i.ifi_index = index;
	}

	if (name) {
		addattr_l(&req->n, sizeof(*req),
			  IFLA_IFNAME, name, strlen(name) + 1);
	}

	return ret;
}

static int iplink_modify(int cmd, unsigned int flags, int argc, char **argv)
{
	char *type = NULL;
	struct iplink_req req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.i.ifi_family = preferred_family,
	};
	int ret;

	ret = iplink_parse(argc, argv, &req, &type);
	if (ret < 0)
		return ret;

	if (type) {
		struct link_util *lu;
		struct rtattr *linkinfo;
		char *ulinep = strchr(type, '_');
		int iflatype;

		linkinfo = addattr_nest(&req.n, sizeof(req), IFLA_LINKINFO);
		addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, type,
			 strlen(type));

		lu = get_link_kind(type);
		if (ulinep && !strcmp(ulinep, "_slave"))
			iflatype = IFLA_INFO_SLAVE_DATA;
		else
			iflatype = IFLA_INFO_DATA;

		argc -= ret;
		argv += ret;

		if (lu && argc) {
			struct rtattr *data;

			data = addattr_nest(&req.n, sizeof(req), iflatype);

			if (lu->parse_opt &&
			    lu->parse_opt(lu, argc, argv, &req.n))
				return -1;

			addattr_nest_end(&req.n, data);
		} else if (argc) {
			if (matches(*argv, "help") == 0)
				usage();
			fprintf(stderr,
				"Garbage instead of arguments \"%s ...\". Try \"ip link help\".\n",
				*argv);
			return -1;
		}
		addattr_nest_end(&req.n, linkinfo);
	} else if (flags & NLM_F_CREATE) {
		fprintf(stderr,
			"Not enough information: \"type\" argument is required\n");
		return -1;
	}

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -2;

	/* remove device from cache; next use can refresh with new data */
	ll_drop_by_index(req.i.ifi_index);

	return 0;
}
