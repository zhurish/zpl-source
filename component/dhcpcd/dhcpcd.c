/*
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2006-2018 Roy Marples <roy@marples.name>
 * All rights reserved

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "dhcp-config.h"
#include "arp.h"
#include "common.h"
#include "control.h"
//#include "dev.h"
#include "dhcpcd.h"
#include "dhcp6.h"
#include "duid.h"
#include "dhcp-eloop.h"
#include "dhcpc-if.h"
#include "if-options.h"
#include "ipv4.h"
#include "ipv4ll.h"
#include "ipv6.h"
#include "ipv6nd.h"
#include "logerr.h"
#include "script.h"

#ifdef HAVE_UTIL_H
#include <util.h>
#endif

struct dhcpcd_ctx dhcpcd_ctx;
static void dhcpcd_handlelink(void *arg);


static void dhcpcd_v4_drop(struct dhcpc_interface *ifp, int stop);
static void stop_v4_interface(struct dhcpc_interface *ifp);
static void if_v4_reboot(struct dhcpc_interface *ifp);
static void dhcpcd_v4_ifrenew(struct dhcpc_interface *ifp);
#ifdef INET6
static void dhcpcd_v6_startinterface(void *arg);
static void dhcpcd_v6_drop(struct dhcpc_interface *ifp, int stop);
static void stop_v6_interface(struct dhcpc_interface *ifp);
static void if_v6_reboot(struct dhcpc_interface *ifp);
static void dhcpcd_v6_ifrenew(struct dhcpc_interface *ifp);
#endif

static void free_globals(struct dhcpcd_ctx *ctx)
{
	struct dhcp_opt *opt;

	if (ctx->ifac)
	{
		for (; ctx->ifac > 0; ctx->ifac--)
			free(ctx->ifav[ctx->ifac - 1]);
		free(ctx->ifav);
		ctx->ifav = NULL;
	}
	if (ctx->ifdc)
	{
		for (; ctx->ifdc > 0; ctx->ifdc--)
			free(ctx->ifdv[ctx->ifdc - 1]);
		free(ctx->ifdv);
		ctx->ifdv = NULL;
	}
	if (ctx->ifcc)
	{
		for (; ctx->ifcc > 0; ctx->ifcc--)
			free(ctx->ifcv[ctx->ifcc - 1]);
		free(ctx->ifcv);
		ctx->ifcv = NULL;
	}

#ifdef INET
	if (ctx->dhcp_opts)
	{
		for (opt = ctx->dhcp_opts; ctx->dhcp_opts_len > 0;
				opt++, ctx->dhcp_opts_len--)
			free_dhcp_opt_embenc(opt);
		free(ctx->dhcp_opts);
		ctx->dhcp_opts = NULL;
	}
#endif
#ifdef INET6
	if (ctx->nd_opts)
	{
		for (opt = ctx->nd_opts; ctx->nd_opts_len > 0;
				opt++, ctx->nd_opts_len--)
			free_dhcp_opt_embenc(opt);
		free(ctx->nd_opts);
		ctx->nd_opts = NULL;
	}
	if (ctx->dhcp6_opts)
	{
		for (opt = ctx->dhcp6_opts; ctx->dhcp6_opts_len > 0;
				opt++, ctx->dhcp6_opts_len--)
			free_dhcp_opt_embenc(opt);
		free(ctx->dhcp6_opts);
		ctx->dhcp6_opts = NULL;
	}
#endif
	if (ctx->vivso)
	{
		for (opt = ctx->vivso; ctx->vivso_len > 0; opt++, ctx->vivso_len--)
			free_dhcp_opt_embenc(opt);
		free(ctx->vivso);
		ctx->vivso = NULL;
	}
}

static void handle_exit_timeout(void *arg)
{
	struct dhcpcd_ctx *ctx;

	ctx = arg;
	logerrx("timed out");
	if (!(ctx->options & DHCPCD_MASTER))
	{
		eloop_exit(ctx->eloop, EXIT_FAILURE);
		return;
	}
	ctx->options |= DHCPCD_NOWAITIP;
	dhcpcd_daemonise(ctx);
}

/* Returns the pid of the child, otherwise 0. */
pid_t dhcpcd_daemonise(struct dhcpcd_ctx *ctx)
{
	eloop_timeout_delete(ctx->eloop, handle_exit_timeout, ctx);
	errno = ENOSYS;
	return 0;
}



static void warn_iaid_conflict(struct dhcpc_interface *ifp, uint16_t ia_type,
		uint8_t *iaid)
{
	struct dhcpc_interface *ifn;
#ifdef INET6
	size_t i;
	struct if_ia *ia;
#endif

	TAILQ_FOREACH(ifn, ifp->ctx->ifaces, next)
	{
		if (ifn == ifp || !ifn->active)
			continue;
		if (ia_type == 0
				&& memcmp(ifn->options->iaid, iaid, sizeof(ifn->options->iaid))
						== 0)
			break;
#ifdef INET6
		for (i = 0; i < ifn->options->ia_len; i++)
		{
			ia = &ifn->options->ia[i];
			if (ia->ia_type == ia_type
					&& memcmp(ia->iaid, iaid, sizeof(ia->iaid)) == 0)
				break;
		}
#endif
	}

	/* This is only a problem if the interfaces are on the same network. */
	if (ifn)
		logerrx("%s: IAID conflicts with one assigned to %s", ifp->name,
				ifn->name);
}

static void configure_interface_default(struct dhcpc_interface *ifp)
{
	struct if_options *ifo = ifp->options;

	/* Do any platform specific configuration */
	//if_conf(ifp);
	logdebugx("%s: \n", __func__);
	/* If we want to release a lease, we can't really persist the
	 * address either. */
	if (ifo->options & DHCPCD_RELEASE)
		ifo->options &= ~DHCPCD_PERSISTENT;

	if (ifp->flags & (IFF_POINTOPOINT | IFF_LOOPBACK))
	{
		ifo->options &= ~DHCPCD_ARP;
		if (!(ifp->flags & IFF_MULTICAST))
			ifo->options &= ~DHCPCD_IPV6RS;
		if (!(ifo->options & DHCPCD_INFORM))
			ifo->options |= DHCPCD_STATIC;
	}
	if (ifp->flags & IFF_NOARP || !(ifo->options & DHCPCD_ARP)
			|| ifo->options & (DHCPCD_INFORM | DHCPCD_STATIC))
		ifo->options &= ~DHCPCD_IPV4LL;

	if (ifo->metric != -1)
		ifp->metric = (unsigned int) ifo->metric;

	if (!(ifo->options & DHCPCD_IPV4))
		ifo->options &= ~(DHCPCD_DHCP | DHCPCD_IPV4LL | DHCPCD_WAITIP4);

#ifdef INET6
	if (!(ifo->options & DHCPCD_IPV6))
		ifo->options &= ~(DHCPCD_IPV6RS | DHCPCD_DHCP6 | DHCPCD_WAITIP6);

	/* We want to setup INET6 on the interface as soon as possible. */
	if (/*ifp->active == IF_ACTIVE_USER && */ifo->options & DHCPCD_IPV6
			&& !(ifp->ctx->options & (DHCPCD_DUMPLEASE | DHCPCD_TEST)))
	{
		/* If not doing any DHCP, disable the RDNSS requirement. */
		if (!(ifo->options & (DHCPCD_DHCP | DHCPCD_DHCP6)))
			ifo->options &= ~DHCPCD_IPV6RA_REQRDNSS;
		if_setup_inet6(ifp);
	}
#endif

	if (!(ifo->options & DHCPCD_IAID))
	{
		/*
		 * An IAID is for identifying a unqiue interface within
		 * the client. It is 4 bytes long. Working out a default
		 * value is problematic.
		 *
		 * Interface name and number are not stable
		 * between different OS's. Some OS's also cannot make
		 * up their mind what the interface should be called
		 * (yes, udev, I'm looking at you).
		 * Also, the name could be longer than 4 bytes.
		 * Also, with pluggable interfaces the name and index
		 * could easily get swapped per actual interface.
		 *
		 * The MAC address is 6 bytes long, the final 3
		 * being unique to the manufacturer and the initial 3
		 * being unique to the organisation which makes it.
		 * We could use the last 4 bytes of the MAC address
		 * as the IAID as it's the most stable part given the
		 * above, but equally it's not guaranteed to be
		 * unique.
		 *
		 * Given the above, and our need to reliably work
		 * between reboots without persitent storage,
		 * generating the IAID from the MAC address is the only
		 * logical default.
		 * Saying that, if a VLANID has been specified then we
		 * can use that. It's possible that different interfaces
		 * can have the same VLANID, but this is no worse than
		 * generating the IAID from the duplicate MAC address.
		 *
		 * dhclient uses the last 4 bytes of the MAC address.
		 * dibbler uses an increamenting counter.
		 * wide-dhcpv6 uses 0 or a configured value.
		 * odhcp6c uses 1.
		 * Windows 7 uses the first 3 bytes of the MAC address
		 * and an unknown byte.
		 * dhcpcd-6.1.0 and earlier used the interface name,
		 * falling back to interface index if name > 4.
		 */
		if (ifp->vlanid != 0)
		{
			uint32_t vlanid;

			/* Maximal VLANID is 4095, so prefix with 0xff
			 * so we don't conflict with an interface index. */
			vlanid = htonl(ifp->vlanid | 0xff000000);
			memcpy(ifo->iaid, &vlanid, sizeof(vlanid));
		}
		else if (ifp->hwlen >= sizeof(ifo->iaid))
		{
			memcpy(ifo->iaid, ifp->hwaddr + ifp->hwlen - sizeof(ifo->iaid),
					sizeof(ifo->iaid));
		}
		else
		{
			uint32_t len;

			len = (uint32_t) strlen(ifp->name);
			if (len <= sizeof(ifo->iaid))
			{
				memcpy(ifo->iaid, ifp->name, len);
				if (len < sizeof(ifo->iaid))
					memset(ifo->iaid + len, 0, sizeof(ifo->iaid) - len);
			}
			else
			{
				/* IAID is the same size as a uint32_t */
				len = htonl(ifp->index);
				memcpy(ifo->iaid, &len, sizeof(ifo->iaid));
			}
		}
		ifo->options |= DHCPCD_IAID;
	}

#ifdef INET6
	if (ifo->ia_len == 0 && ifo->options & DHCPCD_IPV6 && ifp->name[0] != '\0')
	{
		ifo->ia = malloc(sizeof(*ifo->ia));
		if (ifo->ia == NULL)
			logerr(__func__);
		else
		{
			ifo->ia_len = 1;
			ifo->ia->ia_type = D6_OPTION_IA_NA;
			memcpy(ifo->ia->iaid, ifo->iaid, sizeof(ifo->iaid));
			memset(&ifo->ia->addr, 0, sizeof(ifo->ia->addr));
#ifndef SMALL
			ifo->ia->sla = NULL;
			ifo->ia->sla_len = 0;
#endif
		}
	}
	else
	{
		size_t i;

		for (i = 0; i < ifo->ia_len; i++)
		{
			if (!ifo->ia[i].iaid_set)
			{
				memcpy(&ifo->ia[i].iaid, ifo->iaid, sizeof(ifo->ia[i].iaid));
				ifo->ia[i].iaid_set = 1;
			}
		}
	}
#endif
}


static void configure_interface(struct dhcpc_interface *ifp,
		unsigned long long options)
{
	time_t old;
	logdebugx("%s: \n", __func__);
	old = ifp->options ? ifp->options->mtime : 0;
	dhcpcd_selectprofile(ifp, NULL);
	if (ifp->options == NULL)
	{
		loginfox("%s: if (ifp->options == NULL)", __func__);
		/* dhcpcd cannot continue with this interface. */
		ifp->active = IF_INACTIVE;
		return;
	}
	//add_options(ifp->ctx, ifp->name, ifp->options, argc, argv);
	ifp->options->options |= options;
	configure_interface_default(ifp);

	/* If the mtime has changed drop any old lease */
	if (old != 0 && ifp->options->mtime != old)
	{
		logwarnx("%s: confile file changed, expiring leases", ifp->name);
#ifdef INET6
		if(	IF_IS_V6ACTIVE(ifp->active) && dhcp6_state(ifp) > DH6S_INIT)
			dhcpcd_v6_drop(ifp, 0);
#endif
		if(	IF_IS_ACTIVE(ifp->active) && dhcp_state(ifp) > DHS_INIT)
			dhcpcd_v4_drop(ifp, 0);
		//dhcpcd_drop(ifp, DHCP_CMD_BOTH, 0);
	}
}


static void dhcpcd_initstate(struct dhcpc_interface *ifp,
		unsigned long long options)
{
	struct if_options *ifo;
	ifo = ifp->options;
	if(!ifo)
		ifo = ifp->options = default_config(ifp->ctx);
	ifo->options |= options;
	configure_interface(ifp, options);
#ifdef INET6
	logerr("%s: ipv6_init",__func__);
	if (ifo->options & DHCPCD_IPV6 && ipv6_init(ifp->ctx) == -1)
	{
		logerr("%s: ipv6_init false",__func__);
		ifo->options &= ~DHCPCD_IPV6;
		IF_SET_V6ACTIVE(ifp->active, IF_INACTIVE);
	}
	ifp->state = DHCP_STATE_INIT;
#endif
}

int dhcpcd_selectprofile(struct dhcpc_interface *ifp, const char *profile)
{
	char pssid[PROFILE_LEN];
	logdebugx("%s: \n", __func__);
	if (ifp->ssid_len)
	{
		ssize_t r;

		r = print_string(pssid, sizeof(pssid), OT_ESCSTRING, ifp->ssid,
				ifp->ssid_len);
		if (r == -1)
		{
			logerr(__func__);
			pssid[0] = '\0';
		}
	}
	else
		pssid[0] = '\0';
#ifndef DHCPC_THREAD
	if (profile != NULL)
	{
		strlcpy(ifp->profile, profile, sizeof(ifp->profile));
		loginfox("%s: selected profile %s", ifp->name, profile);
	}
	else
		*ifp->profile = '\0';
#endif
	if (profile)
	{
		configure_interface_default(ifp);
	}
	return 1;
}

/*
 * link state change
 */
void dhcpcd_handlecarrier(struct dhcpcd_ctx *ctx, int carrier,
		unsigned int flags, const char *ifname)
{
	struct dhcpc_interface *ifp;

	ifp = if_find(ctx->ifaces, ifname);
	if (ifp == NULL || ifp->options == NULL
			|| !(ifp->options->options & DHCPCD_LINK) || !ifp->active)
		return;
	ifp->flags = flags;
	if (carrier == LINK_DOWN)
	{
		if (ifp->carrier != LINK_DOWN)//down
		{
			if (ifp->carrier == LINK_UP)
				loginfox("%s: carrier lost", ifp->name);
			ifp->carrier = LINK_DOWN;
			script_runreason(ifp, "NOCARRIER");
#ifdef NOCARRIER_PRESERVE_IP
#ifdef ARP
			arp_drop(ifp);
#endif
			dhcp_abort(ifp);
#ifdef INET6
			if(	IF_IS_V6ACTIVE(ifp->active))
				ipv6nd_expire(ifp, 0);
#endif
#else
#ifdef INET6
			if(	IF_IS_V6ACTIVE(ifp->active))
				dhcpcd_v6_drop(ifp, 0);
#endif
			if(	IF_IS_ACTIVE(ifp->active))
				dhcpcd_v4_drop(ifp, 0);
#endif
		}
	}
	else if (carrier == LINK_UP)
	{
		if (ifp->carrier != LINK_UP)
		{
			loginfox("%s: carrier acquired", ifp->name);
			ifp->carrier = LINK_UP;
			if (ifp->wireless)
			{
				uint8_t ossid[IF_SSIDLEN];
#ifdef NOCARRIER_PRESERVE_IP
				size_t olen;

				olen = ifp->ssid_len;
#endif
				memcpy(ossid, ifp->ssid, ifp->ssid_len);
				if_getssid(ifp);
#ifdef NOCARRIER_PRESERVE_IP
				/* If we changed SSID network, drop leases */
				if (ifp->ssid_len != olen ||
						memcmp(ifp->ssid, ossid, ifp->ssid_len))
				{
#ifdef INET6
					if(	IF_IS_V6ACTIVE(ifp->active))
						dhcpcd_v6_drop(ifp, 0);
#endif
					if(	IF_IS_ACTIVE(ifp->active))
						dhcpcd_v4_drop(ifp, 0);
				}
#endif
			}
			//dhcpcd_initstate(ifp, 0);
			script_runreason(ifp, "CARRIER");
#ifdef NOCARRIER_PRESERVE_IP
			/* Set any IPv6 Routers we remembered to expire
			 * faster than they would normally as we
			 * maybe on a new network. */
			if(	IF_IS_V6ACTIVE(ifp->active))
				ipv6nd_expire(ifp, RTR_CARRIER_EXPIRE);
#endif
			/* RFC4941 Section 3.5 */
#ifdef INET6
			if(	IF_IS_V6ACTIVE(ifp->active))
			{
				ipv6_gentempifid(ifp);
				dhcpcd_v6_startinterface(ifp);
			}
#endif
			if(	IF_IS_ACTIVE(ifp->active))
				dhcpcd_startinterface(ifp);
		}
	}
}

static void dhcpcd_startinterface_check(void *arg)
{
	struct dhcpc_interface *ifp = arg;

	dhcpcd_initstate(ifp, 0);
	ifp->t_chk = NULL;
	if ((!(ifp->ctx->options & DHCPCD_MASTER)
			|| ifp->options->options & DHCPCD_IF_UP) &&
			dhcpc_if_up(ifp) == -1)
		logerr("%s: %s", __func__, ifp->name);

	logdebugx("%s:ifp->active=0x%x\n", __func__, ifp->active);

/*	if (ifp->options->options & DHCPCD_LINK && ifp->carrier == LINK_UNKNOWN)
	{
		int carrier;

		logdebugx("%s:carrier=%x options=%x\n", __func__, ifp->carrier,
				ifp->options->options);
		if ((carrier = if_carrier(ifp)) != LINK_UNKNOWN)
		{
			dhcpcd_handlecarrier(ifp->ctx, carrier, ifp->flags, ifp->name);
		}
		//loginfox("%s: unknown carrier, waiting for interface flags", ifp->name);
	}*/
}


/*static void run_preinit(struct dhcpc_interface *ifp)
{
	if (ifp->ctx->options & DHCPCD_TEST)
		return;
	script_runreason(ifp, "PREINIT");
	if (ifp->options->options & DHCPCD_LINK && ifp->carrier != LINK_UNKNOWN)
		script_runreason(ifp,
				ifp->carrier == LINK_UP ? "CARRIER" : "NOCARRIER");
}*/

void dhcpcd_activateinterface(struct dhcpc_interface *ifp,
		unsigned long long options)
{
#ifdef INET6
	if((options & (DHCPCD_IPV6 | DHCPCD_DHCP6)) == (DHCPCD_IPV6 | DHCPCD_DHCP6))
	{
		if(!IF_IS_V6ACTIVE(ifp->active))
		{
			ifp->options->options |= options;
			IF_SET_V6ACTIVE(ifp->active, IF_ACTIVE);
			dhcpcd_v6_startinterface(ifp);
		}
	}
	else
#endif
	{
		if(!IF_IS_ACTIVE(ifp->active))
		{
			IF_SET_ACTIVE(ifp->active, IF_ACTIVE);
			dhcpcd_startinterface(ifp);
		}
	}
}

int dhcpcd_handleinterface(void *arg, int action, const char *ifname)
{
	if (action == -1)
		logdebugx("delete interface %s", ifname);
	else if (action)
		logdebugx("add interface %s", ifname);
	return 0;
}

static void dhcpcd_linkoverflow(struct dhcpcd_ctx *ctx)
{
	struct dhcpc_interface *ifp, *ifn;

	logerrx("route socket overflowed - learning interface state");

	/* Close the existing socket and open a new one.
	 * This is easier than draining the kernel buffer of an
	 * in-determinate size. */
	eloop_event_delete(ctx->eloop, ctx->link_fd);
	close(ctx->link_fd);
	if_closesockets_os(ctx);
	if (if_opensockets_os(ctx) == -1)
	{
		logerr("%s: if_opensockets", __func__);
		eloop_exit(ctx->eloop, EXIT_FAILURE);
		return;
	}
	eloop_event_add(ctx->eloop, ctx->link_fd, dhcpcd_handlelink, ctx);

	/* Punt departed interfaces */
	TAILQ_FOREACH_SAFE(ifp, ctx->ifaces, next, ifn)
	{
#ifdef INET6
		if(	IF_IS_V6ACTIVE(ifp->active))
		{
			logdebugx("%s: v6 interface departed", ifp->name);
			ifp->options->options |= DHCPCD_DEPARTED;
			stop_v6_interface(ifp);
		}
#endif
		if(	IF_IS_ACTIVE(ifp->active))
		{
			logdebugx("%s: interface departed", ifp->name);
			ifp->options->options |= DHCPCD_DEPARTED;
			stop_v4_interface(ifp);
		}
		if (ifp->active == IF_INACTIVE)
		{
			TAILQ_REMOVE(ctx->ifaces, ifp, next);
			if_free(ifp);
		}
	}

	/* Update address state. */
	if_markaddrsstale(ctx->ifaces);
	if_deletestaleaddrs(ctx->ifaces);
}

static void dhcpcd_handlelink(void *arg)
{
	struct dhcpcd_ctx *ctx = arg;

	if (if_handlelink(ctx) == -1)
	{
		if (errno == ENOBUFS || errno == ENOMEM)
		{
			dhcpcd_linkoverflow(ctx);
			return;
		}
		logerr(__func__);
	}
}

void dhcpcd_handlehwaddr(struct dhcpcd_ctx *ctx, const char *ifname,
		const void *hwaddr, uint8_t hwlen)
{
	struct dhcpc_interface *ifp;
	char buf[sizeof(ifp->hwaddr) * 3];

	ifp = if_find(ctx->ifaces, ifname);
	if (ifp == NULL)
		return;

	if (!if_valid_hwaddr(hwaddr, hwlen))
		hwlen = 0;

	if (hwlen > sizeof(ifp->hwaddr))
	{
		errno = ENOBUFS;
		logerr("%s: %s", __func__, ifp->name);
		return;
	}

	if (ifp->hwlen == hwlen && memcmp(ifp->hwaddr, hwaddr, hwlen) == 0)
		return;

	loginfox("%s: new hardware address: %s", ifp->name,
			hwaddr_ntoa(hwaddr, hwlen, buf, sizeof(buf)));
	ifp->hwlen = hwlen;
	memcpy(ifp->hwaddr, hwaddr, hwlen);
}


void dhcpcd_startinterface(void *arg)
{
	struct dhcpc_interface *ifp = arg;
	struct if_options *ifo = ifp->options;
	ifp->t_event = NULL;
	logdebugx("%s:\n", __func__);
	if (!IF_IS_ACTIVE(ifp->active))
		return;

#ifdef INET
	if (ifo->options & DHCPCD_IPV4 && IF_IS_ACTIVE(ifp->active) == IF_ACTIVE_USER)
	{
		/* Ensure we have an IPv4 state before starting DHCP */
		if (ipv4_getstate(ifp) != NULL)
		{
			dhcp_start(ifp);
			ifp->state = DHCP_STATE_RUNNING;
		}
	}
#endif
}
#ifdef INET6
static void dhcpcd_v6_startinterface(void *arg)
{
	struct dhcpc_interface *ifp = arg;
	struct if_options *ifo = ifp->options;
	char buf[DUID_LEN * 3];

	logdebugx("%s:\n", __func__);
	ifp->t_v6_event = NULL;
	if (!IF_IS_V6ACTIVE(ifp->active))
		return;

	dhcpcd_startinterface_check(ifp);

	if (ifo->options & (DHCPCD_DUID | DHCPCD_IPV6))
	{
		/* Report client DUID */
		if (ifp->ctx->duid == NULL)
		{
			if (duid_init(ifp) == 0)
			{
				logdebugx("%s:duid_init\n", __func__);
				return;
			}
			loginfox("DUID %s",
					hwaddr_ntoa(ifp->ctx->duid, ifp->ctx->duid_len, buf,
							sizeof(buf)));
		}
	}

	if (ifo->options & (DHCPCD_DUID | DHCPCD_IPV6))
	{
#ifdef INET6
		size_t i;
		struct if_ia *ia;
#endif

		/* Report IAIDs */
		loginfox("%s: IAID %s", ifp->name,
				hwaddr_ntoa(ifo->iaid, sizeof(ifo->iaid), buf, sizeof(buf)));
		warn_iaid_conflict(ifp, 0, ifo->iaid);
#ifdef INET6
		for (i = 0; i < ifo->ia_len; i++)
		{
			ia = &ifo->ia[i];
			if (memcmp(ifo->iaid, ia->iaid, sizeof(ifo->iaid)))
			{
				loginfox("%s: IA type %u IAID %s", ifp->name, ia->ia_type,
						hwaddr_ntoa(ia->iaid, sizeof(ia->iaid), buf,
								sizeof(buf)));
				warn_iaid_conflict(ifp, ia->ia_type, ia->iaid);
			}
		}
#endif
	}
	if (ifo->options & DHCPCD_IPV6 && ipv6_start(ifp) == -1)
	{
		logerr("%s: ipv6_start", ifp->name);
		ifo->options &= ~DHCPCD_IPV6;
	}
	if (ifo->options & DHCPCD_IPV6)
	{
		if (IF_IS_V6ACTIVE(ifp->active) == IF_ACTIVE_USER)
		{
			ipv6_startstatic(ifp);

			if (ifo->options & DHCPCD_IPV6RS)
				ipv6nd_startrs(ifp);
		}

		if (ifo->options & DHCPCD_DHCP6)
		{
			dhcp6_find_delegates(ifp);

			if (IF_IS_V6ACTIVE(ifp->active) == IF_ACTIVE_USER)
			{
				enum DH6S d6_state;

				if (ifo->options & DHCPCD_IA_FORCED)
					d6_state = DH6S_INIT;
				else if (ifo->options & DHCPCD_INFORM6)
					d6_state = DH6S_INFORM;
				else
					d6_state = DH6S_CONFIRM;
				if (dhcp6_start(ifp, d6_state) == -1)
					logerr("%s: dhcp6_start", ifp->name);
			}
		}
		if(dhcp6_state(ifp) >= DH6S_INIT)
			ifp->state = DHCP_STATE_RUNNING;
	}
}
#endif
static void dhcpcd_v4_drop(struct dhcpc_interface *ifp, int stop)
{
#ifdef IPV4LL
	ipv4ll_drop(ifp);
#endif
#ifdef INET
	dhcp_drop(ifp, stop ? "STOP" : "EXPIRE");
#endif
#ifdef ARP
	arp_drop(ifp);
#endif
#if !defined(DHCP6) && !defined(DHCP)
	UNUSED(stop);
#endif
}
#ifdef INET6
static void dhcpcd_v6_drop(struct dhcpc_interface *ifp, int stop)
{
#ifdef DHCP6
	dhcp6_drop(ifp, stop ? NULL : "EXPIRE6");
#endif
#ifdef INET6
	ipv6nd_drop(ifp);
	ipv6_drop(ifp);
#endif

#if !defined(DHCP6) && !defined(DHCP)
	UNUSED(stop);
#endif
}
#endif

static void stop_v4_interface(struct dhcpc_interface *ifp)
{
	struct dhcpcd_ctx *ctx;

	ctx = ifp->ctx;
	loginfox("%s: removing interface", ifp->name);
	ifp->t_event = NULL;
/*
	if (cmd == DHCP_CMD_BOTH)
		ifp->options->options |= DHCPCD_STOPPING;
*/

	dhcpcd_v4_drop(ifp, 1);

	if (ifp->options->options & DHCPCD_DEPARTED)
		script_runreason(ifp, "DEPARTED");
	else
		script_runreason(ifp, "STOPPED");

	/* Delete all timeouts for the interfaces */
	dhcp_timer_cancel(ifp);

	/* De-activate the interface */
	IF_SET_ACTIVE(ifp->active, IF_INACTIVE);
	if (ifp->active == IF_INACTIVE)
	{
		ifp->options->options &= ~DHCPCD_STOPPING;
		ifp->carrier = LINK_UNKNOWN;
		ifp->state = DHCP_STATE_INIT;
	}
}
#ifdef INET6
static void stop_v6_interface(struct dhcpc_interface *ifp)
{
	struct dhcpcd_ctx *ctx;
	ifp->t_v6_event = NULL;
	ctx = ifp->ctx;
	loginfox("%s: removing interface", ifp->name);

/*
	if (cmd == DHCP_CMD_BOTH)
		ifp->options->options |= DHCPCD_STOPPING;
*/

	dhcpcd_v6_drop(ifp, 1);

	if (ifp->options->options & DHCPCD_DEPARTED)
		script_runreason(ifp, "DEPARTED");
	else
		script_runreason(ifp, "STOPPED");

	/* Delete all timeouts for the interfaces */
	dhcp6_timer_cancel(ifp);

	/* De-activate the interface */
	IF_SET_V6ACTIVE(ifp->active, IF_INACTIVE);
	if (ifp->active == IF_INACTIVE)
	{
		ifp->options->options &= ~DHCPCD_STOPPING;
		ifp->carrier = LINK_UNKNOWN;
		ifp->state = DHCP_STATE_INIT;
	}
}
#endif
static void if_v4_reboot(struct dhcpc_interface *ifp)
{
	unsigned long long oldopts;
	ifp->t_event = NULL;
	oldopts = ifp->options->options;
	script_runreason(ifp, "RECONFIGURE");

	//dhcpcd_initstate(ifp, 0);

	dhcp_reboot_newopts(ifp, oldopts);
}

#ifdef INET6
static void if_v6_reboot(struct dhcpc_interface *ifp)
{
	//unsigned long long oldopts;
	ifp->t_v6_event = NULL;
	//oldopts = ifp->options->options;
	script_runreason(ifp, "RECONFIGURE");

	//dhcpcd_initstate(ifp, 0);

	dhcp6_reboot(ifp);

	//dhcpcd_prestartinterface(ifp);
}
#endif
static void dhcpcd_v4_ifrenew(struct dhcpc_interface *ifp)
{
	ifp->t_event = NULL;
	if(!IF_IS_ACTIVE(ifp->active))
	{
		return;
	}
	if (ifp->options->options & DHCPCD_LINK && ifp->carrier == LINK_DOWN)
	{
		return;
	}
	dhcp_renew(ifp);
}
#ifdef INET6
static void dhcpcd_v6_ifrenew(struct dhcpc_interface *ifp)
{
	ifp->t_v6_event = NULL;
	if (!IF_IS_V6ACTIVE(ifp->active))
	{
		return;
	}
	if (ifp->options->options & DHCPCD_LINK && ifp->carrier == LINK_DOWN)
	{
		return;
	}

#define DHCPCD_RARENEW (DHCPCD_IPV6 | DHCPCD_IPV6RS)
	if ((ifp->options->options & DHCPCD_RARENEW) == DHCPCD_RARENEW)
		ipv6nd_startrs(ifp);
	dhcp6_renew(ifp);
}
#endif

static void dhcpcd_v4_add(struct dhcpc_interface *ifp)
{
	ifp->t_event = NULL;
	if(!if_findindex(ifp->ctx->ifaces, ifp->index))
		TAILQ_INSERT_TAIL(ifp->ctx->ifaces, ifp, next);
	if(!IF_IS_ACTIVE(ifp->active))
	{
		IF_SET_ACTIVE(ifp->active, IF_ACTIVE_USER);
		dhcpcd_startinterface(ifp);
	}
//	/dhcpcd_startinterface(ifp);
}
#ifdef INET6
static void dhcpcd_v6_add(struct dhcpc_interface *ifp)
{
	ifp->t_v6_event = NULL;
	if(!if_findindex(ifp->ctx->ifaces, ifp->index))
		TAILQ_INSERT_TAIL(ifp->ctx->ifaces, ifp, next);
	if (!IF_IS_V6ACTIVE(ifp->active))
	{
		IF_SET_V6ACTIVE(ifp->active, IF_ACTIVE_USER);
		dhcpcd_v6_startinterface(ifp);
	}
}
#endif
static void dhcpcd_v4_del(struct dhcpc_interface *ifp)
{
	ifp->t_event = NULL;
	if(!IF_IS_ACTIVE(ifp->active))
	{
		return;
	}
	IF_SET_V6ACTIVE(ifp->active, IF_INACTIVE);
	stop_v4_interface(ifp);

	if(ifp->active == IF_INACTIVE)
	{
		TAILQ_REMOVE(ifp->ctx->ifaces, ifp, next);
		if_free(ifp);
	}
}
#ifdef INET6
static void dhcpcd_v6_del(struct dhcpc_interface *ifp)
{
	ifp->t_v6_event = NULL;
	if (!IF_IS_V6ACTIVE(ifp->active))
	{
		return;
	}
	IF_SET_V6ACTIVE(ifp->active, IF_INACTIVE);
	stop_v6_interface(ifp);
	if(ifp->active == IF_INACTIVE)
	{
		TAILQ_REMOVE(ifp->ctx->ifaces, ifp, next);
		if_free(ifp);
	}
}
#endif
void dhcpcd_event_interface(struct dhcpc_interface *ifp,
		DHCP_EVENT_CMD event, BOOL ipv6)
{
	switch(event)
	{
	case DHCP_EVENT_NONE:
		break;
	case DHCP_EVENT_ADD:
#ifdef INET6
		if(ipv6)
		{
			if(ifp->t_v6_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_v6_event);
			ifp->t_v6_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_v6_add, ifp);
		}
		else
#endif
		{
			if(ifp->t_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_event);
			ifp->t_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_v4_add, ifp);
		}
		break;
	case DHCP_EVENT_DEL:
#ifdef INET6
		if(ipv6)
		{
			if(ifp->t_v6_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_v6_event);
			ifp->t_v6_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_v6_del, ifp);
		}
		else
#endif
		{
			if(ifp->t_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_event);
			ifp->t_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_v4_del, ifp);
		}
		break;
	case DHCP_EVENT_START:
#ifdef INET6
		if(ipv6)
		{
			if(ifp->t_v6_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_v6_event);
			ifp->t_v6_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_v6_startinterface, ifp);
		}
		else
#endif
		{
			if(ifp->t_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_event);
			ifp->t_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_startinterface, ifp);
		}
		break;
	case DHCP_EVENT_STOP:
#ifdef INET6
		if(ipv6)
		{
			if(ifp->t_v6_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_v6_event);
			ifp->t_v6_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, stop_v6_interface, ifp);
		}
		else
#endif
		{
			if(ifp->t_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_event);
			ifp->t_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, stop_v4_interface, ifp);
		}
		break;
	case DHCP_EVENT_REBOOT:
#ifdef INET6
		if(ipv6)
		{
			if(ifp->t_v6_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_v6_event);
			ifp->t_v6_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, if_v6_reboot, ifp);
		}
		else
#endif
		{
			if(ifp->t_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_event);
			ifp->t_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, if_v4_reboot, ifp);
		}
		break;
	case DHCP_EVENT_RENEW:
#ifdef INET6
		if(ipv6)
		{
			if(ifp->t_v6_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_v6_event);
			ifp->t_v6_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_v6_ifrenew, ifp);
		}
		else
#endif
		{
			if(ifp->t_event)
				eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_event);
			ifp->t_event = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_v4_ifrenew, ifp);
		}
		break;
	case DHCP_EVENT_CHK:
		if(ifp->t_chk)
			eloop_timeout_cancel(ifp->ctx->eloop, ifp->t_chk);
		ifp->t_chk = eloop_timeout_add_sec(ifp->ctx->eloop, 1, dhcpcd_startinterface_check, ifp);
		break;

	}
}

#if 0
static void dhcpcd_drop(struct dhcpc_interface *ifp, DHCP_CMD cmd, int stop)
{
	if (cmd == DHCP_CMD_IPV6 || cmd == DHCP_CMD_BOTH)
	{
#ifdef DHCP6
		dhcp6_drop(ifp, stop ? NULL : "EXPIRE6");
#endif
#ifdef INET6
		ipv6nd_drop(ifp);
		ipv6_drop(ifp);
#endif
	}
	if (cmd == DHCP_CMD_IPV4 || cmd == DHCP_CMD_BOTH)
	{
#ifdef IPV4LL
		ipv4ll_drop(ifp);
#endif
#ifdef INET
		dhcp_drop(ifp, stop ? "STOP" : "EXPIRE");
#endif
#ifdef ARP
		arp_drop(ifp);
#endif
	}
#if !defined(DHCP6) && !defined(DHCP)
	UNUSED(stop);
#endif
}
static void stop_interface(struct dhcpc_interface *ifp, DHCP_CMD cmd)
{
	struct dhcpcd_ctx *ctx;

	ctx = ifp->ctx;
	loginfox("%s: removing interface", ifp->name);

	if (cmd == DHCP_CMD_BOTH)
		ifp->options->options |= DHCPCD_STOPPING;

	dhcpcd_drop(ifp, cmd, 1);

	if (ifp->options->options & DHCPCD_DEPARTED)
		script_runreason(ifp, "DEPARTED");
	else
		script_runreason(ifp, "STOPPED");

	/* Delete all timeouts for the interfaces */
	if (cmd == DHCP_CMD_BOTH)
		eloop_q_timeout_delete(ctx->eloop, 0, NULL, ifp);

	/* De-activate the interface */
	if (cmd == DHCP_CMD_BOTH)
	{
		ifp->active = IF_INACTIVE;
		ifp->options->options &= ~DHCPCD_STOPPING;
		ifp->carrier = LINK_UNKNOWN;
	}
}
void if_reboot(struct dhcpc_interface *ifp)
{
	unsigned long long oldopts;

	oldopts = ifp->options->options;
	script_runreason(ifp, "RECONFIGURE");

	dhcpcd_initstate(ifp, 0);

	if (ifp->ipv4_state > DHCP_RUN_STATE_NONE
			&& (ifp->cmd == DHCP_CMD_IPV4 || ifp->cmd == DHCP_CMD_BOTH))
		dhcp_reboot_newopts(ifp, oldopts);
	if (ifp->ipv6_state > DHCP_RUN_STATE_NONE
			&& (ifp->cmd == DHCP_CMD_IPV6 || ifp->cmd == DHCP_CMD_BOTH))
		dhcp6_reboot(ifp);

	dhcpcd_prestartinterface(ifp);
	ifp->cmd = DHCP_CMD_NONE;
}
void stop_one_interfaces(struct dhcpc_interface *ifp)
{
	/* Drop the last interface first */
/*	if (ifp && ifp->ctx)
	{
		ifp->ctx->options |= DHCPCD_EXITING;
		if (ifp->active)
		{
			DHCP_CMD cmd = DHCP_CMD_NONE;
			//ifp->options->options |= opts;
			if (ifp->options->options & DHCPCD_RELEASE)
				ifp->options->options &= ~DHCPCD_PERSISTENT;
			ifp->options->options |= DHCPCD_EXITING;
			stop_interface(ifp, ifp->cmd);
		}
		ifp->cmd = DHCP_CMD_NONE;
	}*/
}

void dhcpcd_ifrenew(struct dhcpc_interface *ifp)
{
	if (!ifp->active)
	{
		ifp->cmd = DHCP_CMD_NONE;
		return;
	}
	if (ifp->options->options & DHCPCD_LINK && ifp->carrier == LINK_DOWN)
	{
		ifp->cmd = DHCP_CMD_NONE;
		return;
	}
	if (ifp->ipv4_state > DHCP_RUN_STATE_NONE
			&& (ifp->cmd == DHCP_CMD_IPV4 || ifp->cmd == DHCP_CMD_BOTH))
		dhcp_renew(ifp);
	if (ifp->ipv6_state > DHCP_RUN_STATE_NONE
			&& (ifp->cmd == DHCP_CMD_IPV6 || ifp->cmd == DHCP_CMD_BOTH))
	{
#define DHCPCD_RARENEW (DHCPCD_IPV6 | DHCPCD_IPV6RS)
		if ((ifp->options->options & DHCPCD_RARENEW) == DHCPCD_RARENEW)
			ipv6nd_startrs(ifp);
		dhcp6_renew(ifp);
	}
	ifp->cmd = DHCP_CMD_NONE;
}

#endif


int dhcpcd_main(void *p)
{
	//struct dhcpcd_ctx ctx;
	struct dhcpc_interface *ifp;
	//struct if_options *ifo;
	/*	struct dhcpc_interface *ifp;
	 uint16_t family = 0;*/
	int i;
	//time_t t;

	memset(&dhcpcd_ctx, 0, sizeof(dhcpcd_ctx));

	//ifo = NULL;
	//dhcpcd_ctx.cffile = NULL;	//CONFIG;
	dhcpcd_ctx.control_fd = dhcpcd_ctx.control_unpriv_fd = dhcpcd_ctx.link_fd =
			-1;
	dhcpcd_ctx.pf_inet_fd = -1;
#ifdef IFLR_ACTIVE
	dhcpcd_ctx.pf_link_fd = -1;
#endif

	TAILQ_INIT(&dhcpcd_ctx.control_fds);
#ifdef PLUGIN_DEV
	dhcpcd_ctx.dev_fd = -1;
#endif
#ifdef INET
	dhcpcd_ctx.udp_fd = -1;
#endif
	rt_init(&dhcpcd_ctx);

	//logopts = LOGERR_ERR|LOGERR_LOG|LOGERR_LOG_DATE|LOGERR_LOG_PID;
	/*
	 logsetopts(logopts);
	 logopen(dhcpcd_ctx.logfile);
	 */

	/*
	 dhcpcd_ctx.argv = argv;
	 dhcpcd_ctx.argc = argc;
	 dhcpcd_ctx.ifc = argc - optind;
	 dhcpcd_ctx.ifv = argv + optind;
	 */
	/*	ifo = default_config(&dhcpcd_ctx);
	 if (ifo == NULL) {
	 goto exit_failure;
	 }*/
	/*	opt = add_options(&dhcpcd_ctx, NULL, ifo, argc, argv);
	 if (opt != 1) {
	 if (dhcpcd_ctx.options & DHCPCD_PRINT_PIDFILE)
	 goto printpidfile;
	 if (opt == 0)
	 usage();
	 goto exit_failure;
	 }*/

	if ((dhcpcd_ctx.ifaces = malloc(sizeof(struct if_head))) == NULL)
	{
		logerr(__func__);
		return -1;
	}
	TAILQ_INIT(dhcpcd_ctx.ifaces);
	dhcpcd_ctx.options |= global_ctx_options();
	//dhcpcd_ctx.options |= ifo->options;
	/*	if (i == 1 || i == 3) {
	 if (i == 1)
	 dhcpcd_ctx.options |= DHCPCD_TEST;
	 else
	 dhcpcd_ctx.options |= DHCPCD_DUMPLEASE;
	 dhcpcd_ctx.options |= DHCPCD_PERSISTENT;
	 dhcpcd_ctx.options &= ~DHCPCD_DAEMONISE;
	 }*/

#ifdef THERE_IS_NO_FORK
	dhcpcd_ctx.options &= ~DHCPCD_DAEMONISE;
#endif

	/* Freeing allocated addresses from dumping leases can trigger
	 * eloop removals as well, so init here. */
	if ((dhcpcd_ctx.eloop = eloop_new()) == NULL)
	{
		logerr("%s: eloop_init", __func__);
		goto exit_failure;
	}
	eloop_install_chk_cb(dhcpcd_ctx.eloop, dhcpcd_eloop_timeout_delete_chk_cb);

	if (control_start(&dhcpcd_ctx, "eth0") == -1)
	{
		logerr("%s: control_start", __func__);
		goto exit_failure;
	}

	logdebugx(DHCPCD_PACKAGE "-" DHCPCD_VERSION " starting");
	dhcpcd_ctx.options |= DHCPCD_STARTED;
	dhcpcd_ctx.options |= DHCPCD_MASTER;

#ifdef HAVE_SETPROCTITLE
	setproctitle("%s%s%s",
			dhcpcd_ctx.options & DHCPCD_MASTER ? "[master]" : argv[optind],
			dhcpcd_ctx.options & DHCPCD_IPV4 ? " [ip4]" : "",
			dhcpcd_ctx.options & DHCPCD_IPV6 ? " [ip6]" : "");
#endif

	if (if_opensockets(&dhcpcd_ctx) == -1)
	{
		logerr("%s: if_opensockets", __func__);
		goto exit_failure;
	}

	/* When running dhcpcd against a single interface, we need to retain
	 * the old behaviour of waiting for an IP address */
	if (dhcpcd_ctx.ifc == 1 && !(dhcpcd_ctx.options & DHCPCD_BACKGROUND))
		dhcpcd_ctx.options |= DHCPCD_WAITIP;

	/*	if (dhcpcd_ctx.options & DHCPCD_BACKGROUND && dhcpcd_daemonise(&dhcpcd_ctx))
	 goto exit_success;
	 */
	/* Start handling kernel messages for interfaces, addresses and
	 * routes. */
	eloop_event_add(dhcpcd_ctx.eloop, dhcpcd_ctx.link_fd, dhcpcd_handlelink,
			&dhcpcd_ctx);

	if_sortinterfaces(&dhcpcd_ctx);
/*	TAILQ_FOREACH(ifp, dhcpcd_ctx.ifaces, next)
	{
		if (ifp->active)
			dhcpcd_event_interface(ifp, DHCP_EVENT_START, TRUE);
	}*/
#ifdef NO_SIGNALS
	i = eloop_start(dhcpcd_ctx.eloop, NULL);
#else
	i = eloop_start(dhcpcd_ctx.eloop, &dhcpcd_ctx.sigset);
#endif
	if (i < 0)
	{
		logerr("%s: eloop_start", __func__);
		goto exit_failure;
	}
	goto exit1;

	exit_success: i = EXIT_SUCCESS;
	goto exit1;

	exit_failure: i = EXIT_FAILURE;

	exit1:

	if (control_stop(&dhcpcd_ctx) == -1)
		logerr("%s: control_stop", __func__);
	if (dhcpcd_ctx.ifaces)
	{
		while ((ifp = TAILQ_FIRST(dhcpcd_ctx.ifaces)))
		{
			TAILQ_REMOVE(dhcpcd_ctx.ifaces, ifp, next);
			if_free(ifp);
		}
		free(dhcpcd_ctx.ifaces);
	}
	//free_options(&dhcpcd_ctx, ifo);
	rt_dispose(&dhcpcd_ctx);
	free(dhcpcd_ctx.duid);
	if (dhcpcd_ctx.link_fd != -1)
	{
		eloop_event_delete(dhcpcd_ctx.eloop, dhcpcd_ctx.link_fd);
		close(dhcpcd_ctx.link_fd);
	}
	if_closesockets(&dhcpcd_ctx);
	free_globals(&dhcpcd_ctx);
	ipv6_ctxfree(&dhcpcd_ctx);

	eloop_free(dhcpcd_ctx.eloop);
	free(dhcpcd_ctx.iov[0].iov_base);

	if (dhcpcd_ctx.options & DHCPCD_STARTED
			&& !(dhcpcd_ctx.options & DHCPCD_FORKED))
		loginfox(DHCPCD_PACKAGE " exited");
	logclose();
/*	if (dhcpcd_ctx.logfile)
		free(dhcpcd_ctx.logfile);*/
#ifdef USE_SIGNALS
	if (dhcpcd_ctx.options & DHCPCD_FORKED)
	_exit(i); /* so atexit won't remove our pidfile */
#endif
	return i;

}

