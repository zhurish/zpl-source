/*
 * kernel_arp.c
 *
 *  Created on: Sep 29, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "vrf.h"
#include "command.h"
#include "prefix.h"
#include "nsm_arp.h"
#include "nsm_firewalld.h"
#include "pal_include.h"
#include "linux_driver.h"

#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>

#ifdef ZPL_NSM_ARP
/* Delete an entry from the ARP cache. */
int kernel_arp_del(struct interface *ifp, struct prefix *address)
{
	struct ipstack_arpreq ipstack_arpreq;
	struct ipstack_sockaddr_in *sin;
	int rc;

	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&ipstack_arpreq, 0, sizeof(struct ipstack_arpreq));
	sin = (struct ipstack_sockaddr_in *) &ipstack_arpreq.arp_pa;
	memset(sin, 0, sizeof(struct ipstack_sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct ipstack_in_addr));
	strcpy(ipstack_arpreq.arp_dev, ifp->k_name);

	rc = linux_ioctl_if_ioctl (IPSTACK_SIOCDARP, &ipstack_arpreq);
	if (rc < 0)
	{
		return -1;
	}
	return 0;
}

/* Set an entry in the ARP cache. */
int kernel_arp_set(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	struct ipstack_arpreq ipstack_arpreq;
	struct ipstack_sockaddr_in *sin;
	int rc;
	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&ipstack_arpreq, 0, sizeof(struct ipstack_arpreq));
	sin = (struct ipstack_sockaddr_in *) &ipstack_arpreq.arp_pa;
	memset(sin, 0, sizeof(struct ipstack_sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct ipstack_in_addr));
	strcpy(ipstack_arpreq.arp_dev, ifp->k_name);
	memcpy((zpl_uint8 *) ipstack_arpreq.arp_ha.sa_data, mac, 6);

	ipstack_arpreq.arp_flags = IPSTACK_ATF_PERM | IPSTACK_ATF_COM; //note, must set flag, if not,you will get error

	rc = linux_ioctl_if_ioctl (IPSTACK_SIOCSARP, &ipstack_arpreq);
	if (rc < 0)
	{
		return -1;
	}
	return 0;
}

int kernel_arp_get(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	struct ipstack_arpreq ipstack_arpreq;
	struct ipstack_sockaddr_in *sin;
	int rc;

	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&ipstack_arpreq, 0, sizeof(struct ipstack_arpreq));
	sin = (struct ipstack_sockaddr_in *) &ipstack_arpreq.arp_pa;
	memset(sin, 0, sizeof(struct ipstack_sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct ipstack_in_addr));
	strcpy(ipstack_arpreq.arp_dev, ifp->k_name);

	rc = linux_ioctl_if_ioctl (IPSTACK_SIOCGARP, &ipstack_arpreq);
	if (rc < 0)
	{
		return -1;
	}
	memcpy(mac, (zpl_uint8 *) ipstack_arpreq.arp_ha.sa_data, 6);
	return 0;
}


int kernel_arp_gratuitousarp_enable(int enable)
{
	return 0;
}
#endif
#ifdef ZPL_LIBNL_MODULE
static int rtnl_arp_opt(int op, int family, char *name, char *addr, char *mac, int state)
{
	struct rtnl_neigh *neigh;
	struct nl_cache *link_cache;
	struct nl_addr *a;
	struct nl_addr *lla;
	int err, ival, nlflags = NLM_F_REPLACE | NLM_F_CREATE;

	if ((err = rtnl_link_alloc_cache(netlink_cmd.libnl_sock, AF_UNSPEC, &link_cache)) < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}
	neigh = rtnl_neigh_alloc();

	if (!(ival = rtnl_link_name2i(link_cache, name)))
		nl_perror(err, "Link does not exist");

	rtnl_neigh_set_ifindex(neigh, ival);

	nl_addr_parse(mac, AF_UNSPEC, &lla);
	rtnl_neigh_set_lladdr(neigh, lla);
	nl_addr_put(lla);

	nl_addr_parse(addr, rtnl_neigh_get_family(neigh), &a);
	rtnl_neigh_set_dst(neigh, a);
	nl_addr_put(a);
	if(op)
	{
		rtnl_neigh_set_state(neigh, state);

		if ((err = rtnl_neigh_add(netlink_cmd.libnl_sock, neigh, nlflags)) < 0)
			nl_perror(err, "Unable to add neighbour");
	}
	else
	{
	if ((err = rtnl_neigh_delete(netlink_cmd.libnl_sock, neigh, 0)) < 0)
		nl_perror(err, "Unable to delete neighbour");
	}
	return err;
}
#endif
#if 0
static int quiet = 0, default_yes = 0, deleted = 0, interactive = 0;
static struct nl_sock *sock;

static void print_usage(void)
{
	printf(
	"Usage: nl-neigh-delete [OPTION]... [NEIGHBOUR]\n"
	"\n"
	"Options\n"
	" -i, --interactive     Run interactively\n"
	"     --yes             Set default answer to yes\n"
	" -q, --quiet           Do not print informal notifications\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	"\n"
	"Neighbour Options\n"
	" -a, --addr=ADDR       Destination address of neighbour\n"
	" -l, --lladdr=ADDR     Link layer address of neighbour\n"
	" -d, --dev=DEV         Device the neighbour is connected to\n"
	"     --family=FAMILY   Destination address family\n"
	"     --state=STATE     Neighbour state, (default = permanent)\n"
	);

	exit(0);
}

static void delete_cb(struct nl_object *obj, void *arg)
{
	struct rtnl_neigh *neigh = nl_object_priv(obj);
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err;

	if (interactive && !nl_cli_confirm(obj, &params, default_yes))
		return;

	if ((err = rtnl_neigh_delete(sock, neigh, 0)) < 0)
		nl_cli_fatal(err, "Unable to delete neighbour: %s\n",
			     nl_geterror(err));

	if (!quiet) {
		printf("Deleted ");
		nl_object_dump(obj, &params);
	}

	deleted++;
}

int main(int argc, char *argv[])
{
	struct rtnl_neigh *neigh;
	struct nl_cache *link_cache, *neigh_cache;

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	neigh_cache = nl_cli_neigh_alloc_cache(sock);
	neigh = nl_cli_neigh_alloc();

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_FAMILY = 257,
			ARG_STATE = 258,
			ARG_YES,
		};
		static struct option long_opts[] = {
			{ "interactive", 0, 0, 'i' },
			{ "yes", 0, 0, ARG_YES },
			{ "quiet", 0, 0, 'q' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "addr", 1, 0, 'a' },
			{ "lladdr", 1, 0, 'l' },
			{ "dev", 1, 0, 'd' },
			{ "family", 1, 0, ARG_FAMILY },
			{ "state", 1, 0, ARG_STATE },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "qhva:l:d:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'i': interactive = 1; break;
		case ARG_YES: default_yes = 1; break;
		case 'q': quiet = 1; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'a': nl_cli_neigh_parse_dst(neigh, optarg); break;
		case 'l': nl_cli_neigh_parse_lladdr(neigh, optarg); break;
		case 'd': nl_cli_neigh_parse_dev(neigh, link_cache, optarg); break;
		case ARG_FAMILY: nl_cli_neigh_parse_family(neigh, optarg); break;
		case ARG_STATE: nl_cli_neigh_parse_state(neigh, optarg); break;
		}
	}

	nl_cache_foreach_filter(neigh_cache, OBJ_CAST(neigh), delete_cb, NULL);

	if (!quiet)
		printf("Deleted %d neighbours\n", deleted);

	return 0;
}
#endif