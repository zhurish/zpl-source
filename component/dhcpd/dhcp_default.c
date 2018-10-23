/*	$OpenBSD: print.c,v 1.11 2016/02/06 23:50:10 krw Exp $ */

/* Turn data structures into printable text. */

/*
 * Copyright (c) 1995, 1996, 1997, 1998 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"
#include "dhctoken.h"
#include "dhcpd_utils.h"


#if 1

int dhcpd_global_default_init()
{
#ifdef DHCPD_CONF_DEBUG

	readconf();
	root_group.server_port = htons(SERVER_PORT);
	root_group.client_port = htons(CLIENT_PORT);
#else
	/* Set up the initial dhcp option universe. */
	initialize_universes();

	/* Set up the global defaults. */
	root_group.default_lease_time = 43200; /* 12 hours. */
	root_group.max_lease_time = 86400; /* 24 hours. */
	root_group.bootp_lease_cutoff = MAX_TIME;
	root_group.bootp_lease_length = 1200;

	root_group.boot_unknown_clients = 1;
//	root_group.get_lease_hostnames = 1;
	root_group.use_host_decl_names = 1;

	root_group.use_lease_addr_for_default_route = 1;
	root_group.dynamic_bootp = 1;
	root_group.allow_bootp = 1;
	root_group.allow_booting = 1;
	root_group.always_reply_rfc1048 = 1;
	root_group.authoritative = 1;
	root_group.server_port = htons(SERVER_PORT);
	root_group.client_port = htons(CLIENT_PORT);

/*	struct iaddr start, stop, netmask;
	memset(&start, 0, sizeof(struct iaddr));
	memset(&stop, 0, sizeof(struct iaddr));
	memset(&netmask, 0, sizeof(struct iaddr));
	start.len = 4;
	stop.len = 4;
	netmask.len = 4;
	start.iabuf[0] = 0x01;
	start.iabuf[1] = 0x01;
	start.iabuf[2] = 0x01;
	start.iabuf[3] = 0x02;

	stop.iabuf[0] = 0x01;
	stop.iabuf[1] = 0x01;
	stop.iabuf[2] = 0x01;
	stop.iabuf[3] = 0xee;

	netmask.iabuf[0] = 0xff;
	netmask.iabuf[1] = 0xff;
	netmask.iabuf[2] = 0xff;
	netmask.iabuf[3] = 0;*/
#endif
	return 0;
}

#else
void dhcp_set_allow_deny(struct group *group, int token, int flag)
{
	switch (token) 
	{
	case TOK_BOOTP:
		group->allow_bootp = flag;
		break;

	case TOK_BOOTING:
		group->allow_booting = flag;
		break;

	case TOK_DYNAMIC_BOOTP:
		group->dynamic_bootp = flag;
		break;

	case TOK_UNKNOWN_CLIENTS:
		group->boot_unknown_clients = flag;
		break;

	default:
		parse_warn("expecting allow/deny key");
		return;
	}
}

/*
 * host-declaration :== hostname '{' parameters declarations '}'
 */
void dhcp_add_host_declaration(struct group *group, struct host_decl *inhost)
{
	struct host_decl *host = NULL;
	host = calloc(1, sizeof (struct host_decl));
	if (!host)
	{
		dhcpd_error("can't allocate host decl struct %s.", inhost->name);
		return;
	}
	host->name = strdup(inhost->name);
	host->group = clone_group(group, "parse_host_declaration");

	if (!host->group->options[DHO_HOST_NAME] &&
	    host->group->use_host_decl_names) 
	{
		host->group->options[DHO_HOST_NAME] = new_tree_cache("parse_host_declaration");
		if (!host->group->options[DHO_HOST_NAME])
			dhcpd_error("can't allocate a tree cache for hostname.");
		host->group->options[DHO_HOST_NAME]->len = strlen(host->name);
		host->group->options[DHO_HOST_NAME]->value = (unsigned char *)host->name;
		host->group->options[DHO_HOST_NAME]->buf_size = host->group->options[DHO_HOST_NAME]->len;
		host->group->options[DHO_HOST_NAME]->timeout = -1;
		host->group->options[DHO_HOST_NAME]->tree = NULL;
	}

	enter_host(host);
}



/*
 * class-declaration :== STRING '{' parameters declarations '}'
 */
void dhcp_class_declaration(struct group *group, int type, char *val)
{
	struct class *class = NULL;
	class = add_class (type, val);
	if (!class)
	{
		dhcpd_error("No memory for class %s.", val);
		return;
	}
	class->group = clone_group(group, "parse_class_declaration");
}

/*
 * shared-network-declaration :==
 *			hostname LBRACE declarations parameters RBRACE
 */
struct shared_network * dhcp_shared_net_declaration(struct group *group, char *name)
{
	char *n = NULL;
	struct shared_network *share = NULL;
	//char *name;
	int declaration = 0;

	share = calloc(1, sizeof(struct shared_network));
	if (!share)
	{
		dhcpd_error("No memory for shared subnet");
		return NULL;
	}
	share->leases = NULL;
	share->last_lease = NULL;
	share->insertion_point = NULL;
	share->next = NULL;
	share->interface = NULL;
	share->group = clone_group(group, "parse_shared_net_declaration");
	share->group->shared_network = share;
	/* share->subnets is the subnet we just parsed. */
	if (share->subnets)
	{
		share->interface = share->subnets->interface;
		/* Make the shared network name from network number. */
		n = piaddr(share->subnets->net);
		share->name = strdup(n);
		if (share->name == NULL)
			dhcpd_error("no memory for subnet name");
		/*
		 * Copy the authoritative parameter from the subnet,
		 * since there is no opportunity to declare it here.
		 */
		share->group->authoritative = share->subnets->group->authoritative;
		enter_shared_network(share);
	}
	else
	{
		/* Get the name of the shared network. */
		share->name = strdup(name);

		enter_shared_network(share);
	}
	return share;
}


struct tree *dhcp_host_tree_get(char *name, unsigned char *addr, int len)
{
	struct hostent *h = NULL;
	struct tree *tree = NULL;
	if (name)
		h = gethostbyname(name);
	if (name && h)
	{
		tree = tree_const(h->h_addr_list[0], h->h_length);
		tree = tree_limit(tree, len);
	}
	else
	{
		tree = tree_const(addr, len);
	}
	if (!tree)
		return NULL;
	return tree;
}


int dhcp_next_server(struct group *group, char *name, unsigned char *addr)
{
	struct tree *tree = NULL;
	struct tree_cache *cache = NULL;
	tree = dhcp_host_tree_get(name, addr, 4);
	if (!tree)
	{
		dhcpd_error("next-server is not known");
		return -1;
	}
	cache = tree_cache(tree);
	if (!tree_evaluate (cache))
		dhcpd_error("next-server is not known");
	group->next_server.len = 4;
	memcpy(group->next_server.iabuf, cache->value, group->next_server.len);
	//parse_semi(cfile);
	return 0;
}

int dhcp_server_identifier(struct group *group, unsigned char *addr)
{
	struct tree *tree = NULL;
	tree = dhcp_host_tree_get(NULL, addr, 4);
	if (!tree)
	{
		dhcpd_error("server address is not known");
		return -1;
	}
	group->options[DHO_DHCP_SERVER_IDENTIFIER] = tree_cache(tree);
	return 0;
}

int dhcp_host_decl(struct host_decl *host_decl, struct hardware hardware, unsigned char *addr)
{
	struct tree *tree = NULL;
	struct tree_cache *cache = NULL;
/*	struct hardware hardware;
	hardware.htype = HTYPE_ETHER;
	hardware.htype = HTYPE_IPSEC_TUNNEL;
	hardware.hlen = 6;*/
	//memcpy((unsigned char *)&hardware->haddr[0], t, hardware->hlen);

	if (host_decl)
		host_decl->interface = hardware;
	else
		parse_warn("hardware address parameter %s","not allowed here.");

	tree = dhcp_host_tree_get(NULL, addr, 4);
	if (!tree)
	{
		dhcpd_error("host fixed address is not known");
		return -1;
	}
	cache = tree_cache(tree);
	if (!cache)
	{
		dhcpd_error("host fixed address is not known");
		return -1;
	}
	if (host_decl)
		host_decl->fixed_addr = cache;
	else
		parse_warn("fixed-address parameter not %s", "allowed here.");
	return 0;
}


/*
 * address-range-declaration :== ip-address ip-address SEMI
 *			       | DYNAMIC_BOOTP ip-address ip-address SEMI
 */
void dhcp_address_range(struct subnet *subnet, struct iaddr *start, struct iaddr *stop)
{
	struct iaddr low, high;
	int dynamic = 0;

	if (1 == TOK_DYNAMIC_BOOTP) {
		subnet->group->dynamic_bootp = dynamic = 1;
	}
	/* Get the bottom address in the range. */
	memcpy(&low, start, sizeof(struct iaddr));
	/* Get the top address in the range. */
	memcpy(&high, stop, sizeof(struct iaddr));

	/* Create the new address range. */
	new_address_range(low, high, subnet, dynamic);
}


/*
 * subnet-declaration :==
 *	net NETMASK netmask RBRACE parameters declarations LBRACE
 */
struct subnet * dhcp_subnet_declaration(struct shared_network *share, struct iaddr *net, struct iaddr *netmask)
{
	struct subnet *subnet = NULL, *t = NULL, *u = NULL;
	subnet = calloc(1, sizeof(struct subnet));
	if (!subnet)
	{
		dhcpd_error("No memory for new subnet");
		return NULL;
	}
	subnet->shared_network = share;
	subnet->group = clone_group(share->group, "parse_subnet_declaration");
	subnet->group->subnet = subnet;

	/* Get the network number. */
	memcpy(&subnet->net, net, sizeof(struct iaddr));
	/* Get the netmask. */
	memcpy(&subnet->netmask, netmask, sizeof(struct iaddr));
	/* Save only the subnet number. */
	subnet->net = subnet_number(subnet->net, subnet->netmask);

	enter_subnet(subnet);

	/*
	 * If this subnet supports dynamic bootp, flag it so in the
	 * shared_network containing it.
	 */
	if (subnet->group->dynamic_bootp)
		share->group->dynamic_bootp = 1;

	/* Add the subnet to the list of subnets in this shared net. */
	if (!share->subnets)
		share->subnets = subnet;
	else {
		u = NULL;
		for (t = share->subnets; t; t = t->next_sibling) {
			if (subnet_inner_than(subnet, t, 0)) {
				if (u)
					u->next_sibling = subnet;
				else
					share->subnets = subnet;
				subnet->next_sibling = t;
				return subnet;
			}
			u = t;
		}
		u->next_sibling = subnet;
	}
	return subnet;
}



/*
 * option_parameter :== identifier DOT identifier <syntax> SEMI
 *		      | identifier <syntax> SEMI
 *
 * Option syntax is handled specially through format strings, so it
 * would be painful to come up with BNF for it. However, it always
 * starts as above and ends in a SEMI.
 */
void dhcp_option_param(struct group *group, char *optionname, char *val)
{
	//char *val = NULL;
	int token;
	unsigned char buf[4];
	unsigned char cprefix;
	char *fmt = NULL;
	struct universe *universe = NULL;
	struct dhcpd_option *option = NULL;
	struct tree *tree = NULL;
	struct tree *t = NULL;

	/*
	 * Look up the option name hash table for the specified
	 * vendor.
	 */
	//expecting identifier after option keyword.
	universe = ((struct universe *)dhcp_hash_lookup(&universe_hash,
		    (unsigned char *)"dhcp", 0));
	/*
	 * If it's not there, we can't parse the rest of the
	 * declaration.
	 */
	if (!universe) {
		parse_warn("no vendor named %s.", "dhcp");
		//return;
	}
	else {
		/*
		 * Use the default hash table, which contains all the
		 * standard dhcp option names.
		 */
		//val = optionname;
		universe = &dhcp_universe;
	}

	/* Look up the actual option info. */
	option = (struct dhcpd_option *)dhcp_hash_lookup(universe->hash,
	    (unsigned char *)optionname, 0);

	/* If we didn't get an option structure, it's an undefined option. */
	if (!option) {
/*		if (val == optionname)
			parse_warn("no option named %s", val);
		else*/
			parse_warn("no option named %s for vendor %s",
				    val, optionname);
		//free(vendor);
		return;
	}

	/* Free the initial identifier token. */
	//free(vendor);

	/* Parse the option data. */
	//do {
		/*
		 * Set a flag if this is an array of a simple type (i.e.,
		 * not an array of pairs of IP addresses, or something
		 * like that.
		 */
		int uniform = option->format[1] == 'A';

		for (fmt = option->format; *fmt; fmt++) {
			if (*fmt == 'A')
				break;
			switch (*fmt) {
			case 'X':
				//token = peek_token(&val, cfile);
/*				if (token == TOK_NUMBER_OR_NAME) {
						convert_num(buf, val, 16, 8);
						tree = tree_concat(tree,
						    tree_const(buf, 1));
				
				} else if (token == TOK_STRING)*/ {
					//token = next_token(&val, cfile);
					tree = tree_concat(tree,
					    tree_const((unsigned char *)val,
					    strlen(val)));
				} 
				break;

			case 't': /* Text string. */
				tree = tree_concat(tree,
				    tree_const((unsigned char *)val,
				    strlen(val)));
				break;

			case 'I': /* IP address or hostname. */
				//t = parse_ip_addr_or_hostname(cfile, uniform);
				t = dhcp_host_tree_get(val, val, 4);
				if (!t)
					return;
				tree = tree_concat(tree, t);
				break;

			case 'L': /* Unsigned 32-bit integer. */
			case 'l': /* Signed 32-bit integer. */
				convert_num(buf, val, 0, 32);
				tree = tree_concat(tree, tree_const(buf, 4));
				break;
			case 's': /* Signed 16-bit integer. */
			case 'S': /* Unsigned 16-bit integer. */
				convert_num(buf, val, 0, 16);
				tree = tree_concat(tree, tree_const(buf, 2));
				break;
			case 'b': /* Signed 8-bit integer. */
			case 'B': /* Unsigned 8-bit integer. */
				convert_num(buf, val, 0, 8);
				tree = tree_concat(tree, tree_const(buf, 1));
				break;
			case 'f': /* Boolean flag. */
				if (!strcasecmp(val, "true")|| !strcasecmp(val, "on"))
					buf[0] = 1;
				else if (!strcasecmp(val, "false") || !strcasecmp(val, "off"))
					buf[0] = 0;
				else
				{
					parse_warn("expecting boolean.");
					return;
				}
				tree = tree_concat(tree, tree_const(buf, 1));
				break;
			case 'C':
				tree = tree_concat(tree, tree_const(&cprefix,
				    sizeof(cprefix)));
				if (cprefix > 0)
					tree = tree_concat(tree, tree_const(buf,
					    (cprefix + 7) / 8));
				break;
			default:
				dhcpd_warning("Bad format %c in parse_option_param.",
				    *fmt);
				return;
			}
		}

//	} while (*fmt == 'A');

	group->options[option->code] = tree_cache(tree);
}


int dhcpd_pool_add(char *name)
{
	struct shared_network *share = dhcp_shared_net_declaration(&root_group, name);
	if(share)
		return 0;
	return -1;
}

int dhcpd_pool_set_address(char *name, struct iaddr *start, struct iaddr *stop, struct iaddr *netmask)
{
	struct shared_network *share = find_shared_network(name);
	if(share)
	{
		struct subnet *subnet = dhcp_subnet_declaration(share, start, netmask);
		if(subnet)
		{
			dhcp_address_range(subnet, start, stop);
			return 0;
		}
		return -1;
	}
	return -1;
}

int dhcp_global_default_init()
{
#ifdef DHCPD_CONF_DEBUG

	readconf();
	root_group.server_port = htons(SERVER_PORT);
	root_group.client_port = htons(CLIENT_PORT);
#else
	/* Set up the initial dhcp option universe. */
	initialize_universes();
	/* Set up the global defaults. */
	root_group.default_lease_time = 43200; /* 12 hours. */
	root_group.max_lease_time = 86400; /* 24 hours. */
	root_group.bootp_lease_cutoff = MAX_TIME;
	root_group.bootp_lease_length = 1200;

	root_group.boot_unknown_clients = 1;
//	root_group.get_lease_hostnames = 1;
	root_group.use_host_decl_names = 1;

/*
	root_group.next_server;
	root_group.options[256];

	root_group.filename;
	root_group.server_name;
*/

	root_group.use_lease_addr_for_default_route = 1;
	root_group.dynamic_bootp = 1;
	root_group.allow_bootp = 1;
	root_group.allow_booting = 1;
	root_group.always_reply_rfc1048 = 1;
	root_group.authoritative = 1;
	root_group.server_port = htons(SERVER_PORT);
	root_group.client_port = htons(CLIENT_PORT);
#if 0
	dhcp_option_param(&root_group, "subnet-mask", "255.255.255.0");
	dhcp_option_param(&root_group, "routers", "1.1.1.1");
	dhcp_option_param(&root_group, "domain-name-servers", "1.1.1.11");
	dhcp_option_param(&root_group, "host-name", "abcdefg");

	dhcp_option_param(&root_group, "domain-name", "test.com");
	dhcp_option_param(&root_group, "default-ip-ttl", "64");
	dhcp_option_param(&root_group, "interface-mtu", "1450");

	dhcp_option_param(&root_group, "vendor-encapsulated-options", "test.test.com");
	dhcp_option_param(&root_group, "dhcp-lease-time", "64");
	dhcp_option_param(&root_group, "dhcp-server-identifier", "1.1.1.1");

	dhcp_option_param(&root_group, "dhcp-max-message-size", "1200");
	dhcp_option_param(&root_group, "dhcp-renewal-time", "640");
	dhcp_option_param(&root_group, "dhcp-rebinding-time", "650");
	dhcp_option_param(&root_group, "dhcp-class-identifier", "test-test");
	dhcp_option_param(&root_group, "dhcp-parameter-request-list", "1");
	dhcp_option_param(&root_group, "dhcp-parameter-request-list", "3");
	dhcp_option_param(&root_group, "dhcp-parameter-request-list", "6");
	dhcp_option_param(&root_group, "dhcp-parameter-request-list", "12");
	dhcp_option_param(&root_group, "dhcp-parameter-request-list", "15");
	dhcp_option_param(&root_group, "dhcp-parameter-request-list", "28");
	dhcp_option_param(&root_group, "dhcp-parameter-request-list", "42");

	dhcpd_pool_add("test");
	struct iaddr start, stop, netmask;
	memset(&start, 0, sizeof(struct iaddr));
	memset(&stop, 0, sizeof(struct iaddr));
	memset(&netmask, 0, sizeof(struct iaddr));
	start.len = 4;
	stop.len = 4;
	netmask.len = 4;
	start.iabuf[0] = 0x01;
	start.iabuf[1] = 0x01;
	start.iabuf[2] = 0x01;
	start.iabuf[3] = 0x02;

	stop.iabuf[0] = 0x01;
	stop.iabuf[1] = 0x01;
	stop.iabuf[2] = 0x01;
	stop.iabuf[3] = 0xee;

	netmask.iabuf[0] = 0xff;
	netmask.iabuf[1] = 0xff;
	netmask.iabuf[2] = 0xff;
	netmask.iabuf[3] = 0;

	dhcpd_pool_set_address("test", &start, &stop, &netmask);
#endif

#endif
	return 0;
}
#endif
/*
 *
					             root-group
					|		 |							|
			option and     shared-network, group	   host
			global config	      |
								subnet, group
								  |		  |
								pool    option and global-config


shared-network		: ip address pool
 subnet				:   ip range under ip address pool
   option			:	 option under ip range


 *
 *
 *
readconf: ROOT_GROUP root group=0x753340
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=domain-name val=domain-name under group=0x753340
parse_option_param: add option->code = 15 under group=0x753340
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=domain-name-servers val=domain-name-servers under group=0x753340
parse_option_param: add option->code = 6 under group=0x753340
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=name-servers val=name-servers under group=0x753340
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=subnet-mask val=subnet-mask under group=0x753340
parse_option_param: add option->code = 1 under group=0x753340

parse_statement: TOK_DEFAULT_LEASE_TIME val=default-lease-time
parse_statement: TOK_MAX_LEASE_TIME val=max-lease-time

parse_statement: TOK_SUBNET val=subnet
parse_statement: add shared_network=0x763340 under group=0x753340
parse_subnet_declaration: add subnet=0x763230 network=24.25.29.0 under share network=name0 group=0x763390
parse_statement: TOK_RANGE val=range
parse_address_range: add addr range 24.25.29.10 - 24.25.29.20 under subnet=0x763230 shared=0x763340 group=0x763c30
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=broadcast-address val=broadcast-address under group=0x763c30
parse_option_param: add option->code = 28 under group=0x763c30
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=routers val=routers under group=0x763c30
parse_statement: TOK_SUBNET val=subnet


parse_statement: add shared_network=0x765a50 under group=0x753340
parse_subnet_declaration: add subnet=0x766400 network=204.254.239.0 under share network=name0 group=0x765b60
parse_statement: TOK_RANGE val=range
parse_address_range: add addr range 204.254.239.10 - 204.254.239.20 under subnet=0x766400 shared=0x765a50 group=0x766470
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=broadcast-address val=broadcast-address under group=0x766470
parse_option_param: add option->code = 28 under group=0x766470
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=routers val=routers under group=0x766470


parse_statement: TOK_SUBNET val=subnet
parse_statement: add shared_network=0x767a70 under group=0x753340
parse_subnet_declaration: add subnet=0x767930 network=192.5.5.0 under share network=name0 group=0x767ac0
parse_statement: TOK_RANGE val=range
parse_address_range: add addr range 192.5.5.26 - 192.5.5.30 under subnet=0x767930 shared=0x767a70 group=0x768360
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=name-servers val=name-servers under group=0x768360
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=domain-name val=domain-name under group=0x768360
parse_option_param: add option->code = 15 under group=0x768360
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=routers val=routers under group=0x768360
parse_option_param: add option->code = 3 under group=0x768360
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=subnet-mask val=subnet-mask under group=0x768360
parse_option_param: add option->code = 1 under group=0x768360
parse_statement: TOK_OPTION val=option
parse_option_param: add option  vendor=broadcast-address val=broadcast-address under group=0x768360
parse_option_param: add option->code = 28 under group=0x768360
parse_statement: TOK_DEFAULT_LEASE_TIME val=default-lease-time
parse_statement: TOK_MAX_LEASE_TIME val=max-lease-time


parse_statement: TOK_HOST val=host
parse_statement: TOK_HARDWARE val=hardware
parse_statement: TOK_FILENAME val=filename
parse_statement: TOK_SERVER_NAME val=server-name
parse_host_declaration: add hostname=passacaglia under group=0x753340
parse_statement: TOK_HOST val=host
parse_statement: TOK_HARDWARE val=hardware
parse_statement: TOK_FIXED_ADDR val=fixed-address
parse_statement: TOK_HOST val=host
parse_statement: TOK_HOST val=host
parse_host_declaration: add hostname=fantasia under group=0x753340

 *
 * dhcpd.lease
lease 192.168.1.154 {
	starts 1 2000/05/15 13:36:42;
	ends 1 2000/05/15 21:36:42;
	hardware ethernet 00:00:21:4e:3f:58;
	uid 01:00:00:21:4e:3f:58;
	client-hostname "one";
}
 *
 *
#
# DHCP Server Configuration file.
#   see /usr/share/doc/dhcpd/dhcpd.conf.example
#   see dhcpd.conf(5) man page
ddns-update-style none;　　　　#设置DNS服务不自动进行动态更新
ignore client-updates;　　　　#忽略客户端更新DNS记录
subnet 192.168.38.0 netmask 255.255.255.0 {　　　　#作用域为192.168.38.0/24网段
        range 192.168.38.10 192.168.38.110;　　　　#IP地址池为192.168.38.10-110
        option subnet-mask 255.255.255.0;　　　　#定义客户端默认的子网掩码
        option routers 192.168.38.251;　　　　#定义客户端的网关地址
        option domain-name "http://cnblogs.com/zhangjianghua";　　　　#定义默认的搜索域
        option domain-name-servers 192.168.38.251;　　　　#定义客户端的DNS地址
        default-lease-time 21600;　　　　#定义默认租约时间（单位：秒）
        max-lease-time 43200;　　　　#定义最大预约时间（单位：秒）
		host zhangjianghua{　　　　　　#指定主机名
			hardware ethernet 00:0c:29:e4:ee:ff;　　　　#该主机的MAC地址
			fixed-address 192.168.38.88;　　　　#想要给主机绑定的IP地址
		}
}
参数	作用
ddns-update-style 类型	定义DNS服务动态更新的类型，类型包括：
none（不支持动态更新）、interim（互动更新模式）与ad-hoc（特殊更新模式）
allow/ignore client-updates	允许/忽略客户端更新DNS记录
default-lease-time 21600	默认超时时间
max-lease-time 43200	最大超时时间
option domain-name-servers 8.8.8.8	定义DNS服务器地址
option domain-name "domain.org"	定义DNS域名
range	定义用于分配的IP地址池
option subnet-mask	定义客户端的子网掩码
option routers	定义客户端的网关地址
broadcast-address 广播地址	定义客户端的广播地址
ntp-server IP地址	定义客户端的网络时间服务器（NTP）
nis-servers IP地址	定义客户端的NIS域服务器的地址
hardware 硬件类型 MAC地址	指定网卡接口的类型与MAC地址
server-name 主机名	向DHCP客户端通知DHCP服务器的主机名
fixed-address IP地址	将某个固定的IP地址分配给指定主机
time-offset 偏移差	指定客户端与格林尼治时间的偏移差
*/

#ifndef DHCPD_CONF_DEBUG
void
convert_num(unsigned char *buf, char *str, int base, int size)
{
	int negative = 0, tval, max;
	u_int32_t val = 0;
	char *ptr = str;

	if (*ptr == '-') {
		negative = 1;
		ptr++;
	}

	/* If base wasn't specified, figure it out from the data. */
	if (!base) {
		if (ptr[0] == '0') {
			if (ptr[1] == 'x') {
				base = 16;
				ptr += 2;
			} else if (isascii((unsigned char)ptr[1]) &&
			    isdigit((unsigned char)ptr[1])) {
				base = 8;
				ptr += 1;
			} else
				base = 10;
		} else
			base = 10;
	}

	do {
		tval = *ptr++;
		/* XXX assumes ASCII... */
		if (tval >= 'a')
			tval = tval - 'a' + 10;
		else if (tval >= 'A')
			tval = tval - 'A' + 10;
		else if (tval >= '0')
			tval -= '0';
		else {
			dhcpd_warning("Bogus number: %s.", str);
			break;
		}
		if (tval >= base) {
			dhcpd_warning("Bogus number: %s: digit %d not in base %d",
			    str, tval, base);
			break;
		}
		val = val * base + tval;
	} while (*ptr);

	if (negative)
		max = (1 << (size - 1));
	else
		max = (1 << (size - 1)) + ((1 << (size - 1)) - 1);
	if (val > max) {
		switch (base) {
		case 8:
			dhcpd_warning("value %s%o exceeds max (%d) for precision.",
			    negative ? "-" : "", val, max);
			break;
		case 16:
			dhcpd_warning("value %s%x exceeds max (%d) for precision.",
			    negative ? "-" : "", val, max);
			break;
		default:
			dhcpd_warning("value %s%u exceeds max (%d) for precision.",
			    negative ? "-" : "", val, max);
			break;
		}
	}

	if (negative) {
		switch (size) {
		case 8:
			*buf = -(unsigned long)val;
			break;
		case 16:
			putShort(buf, -(unsigned long)val);
			break;
		case 32:
			putLong(buf, -(unsigned long)val);
			break;
		default:
			dhcpd_warning("Unexpected integer size: %d", size);
			break;
		}
	} else {
		switch (size) {
		case 8:
			*buf = (u_int8_t)val;
			break;
		case 16:
			putUShort(buf, (u_int16_t)val);
			break;
		case 32:
			putULong(buf, val);
			break;
		default:
			dhcpd_warning("Unexpected integer size: %d", size);
			break;
		}
	}
}
#endif
