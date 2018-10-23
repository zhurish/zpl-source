/* 
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2006-2009 Roy Marples <roy@marples.name>
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


#define THIRTY_YEARS_IN_SECONDS    946707779

#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "dhcpc_config.h"
#include "dhcpcd.h"
#include "dhcpc_common.h"
#include "net.h"
#include "dhcpc_util.h"

#define IF_SSIDSIZE 33

/* Interface comparer for working out ordering. */
static int
ifcmp(struct dhcpc_interface *si, struct dhcpc_interface *ti)
{
	int sill, till;

	if (si->state && !ti->state)
		return -1;
	if (!si->state && ti->state)
		return 1;
	if (!si->state && !ti->state)
		return 0;
	/* If one has a lease and the other not, it takes precedence. */
	if (si->state->new && !ti->state->new)
		return -1;
	if (!si->state->new && ti->state->new)
		return 1;
	/* If we are either, they neither have a lease, or they both have.
	 * We need to check for IPv4LL and make it non-preferred. */
	if (si->state->new && ti->state->new) {
		sill = (si->state->new->cookie == htonl(MAGIC_COOKIE));
		till = (ti->state->new->cookie == htonl(MAGIC_COOKIE));
		if (!sill && till)
			return 1;
		if (sill && !till)
			return -1;
	}
	/* Then carrier status. */
	if (si->carrier > ti->carrier)
		return -1;
	if (si->carrier < ti->carrier)
		return 1;
	/* Finally, metric */
	if (si->metric < ti->metric)
		return -1;
	if (si->metric > ti->metric)
		return 1;
	return 0;
}

/* Sort the interfaces into a preferred order - best first, worst last. */
void
sort_interfaces(void)
{
	struct dhcpc_interface *sorted, *ifp, *ifn, *ift;

	if (!dhcpc_ifaces_list || !dhcpc_ifaces_list->next)
		return;
	sorted = dhcpc_ifaces_list;
	dhcpc_ifaces_list = dhcpc_ifaces_list->next;
	sorted->next = NULL;
	for (ifp = dhcpc_ifaces_list; ifp && (ifn = ifp->next, 1); ifp = ifn) {
		/* Are we the new head? */
		if (ifcmp(ifp, sorted) == -1) {
			ifp->next = sorted;
			sorted = ifp;
			continue;
		}
		/* Do we fit in the middle? */
		for (ift = sorted; ift->next; ift = ift->next) {
			if (ifcmp(ifp, ift->next) == -1) {
				ifp->next = ift->next;
				ift->next = ifp;
				break;
			}
		}
		/* We must be at the end */
		if (!ift->next) {
			ift->next = ifp;
			ifp->next = NULL;
		}
	}
	dhcpc_ifaces_list = sorted;
}


size_t
get_duid(unsigned char *duid, struct dhcpc_interface *iface)
{
	FILE *f;
	uint16_t type = 0;
	uint16_t hw = 0;
	uint32_t ul;
	time_t t;
	int x = 0;
	unsigned char *p = duid;
	size_t len = 0;
	char *line;

	/* If we already have a DUID then use it as it's never supposed
	 * to change once we have one even if the interfaces do */
	if ((f = fopen(DUID, "r"))) {
		while ((line = get_line(f))) {
			len = hwaddr_aton(NULL, line);
			if (len && len <= DUID_LEN) {
				hwaddr_aton(duid, line);
				break;
			}
			len = 0;
		}
		fclose(f);
		if (len)
			return len;
	} else {
		if (errno != ENOENT)
			return 0;
	}

	/* No file? OK, lets make one based on our interface */
	if (!(f = fopen(DUID, "w")))
		return 0;
	type = htons(1); /* DUI-D-LLT */
	memcpy(p, &type, 2);
	p += 2;
	hw = htons(iface->family);
	memcpy(p, &hw, 2);
	p += 2;
	/* time returns seconds from jan 1 1970, but DUID-LLT is
	 * seconds from jan 1 2000 modulo 2^32 */
	t = time(NULL) - THIRTY_YEARS_IN_SECONDS;
	ul = htonl(t & 0xffffffff);
	memcpy(p, &ul, 4);
	p += 4;
	/* Finally, add the MAC address of the interface */
	memcpy(p, iface->hwaddr, iface->hwlen);
	p += iface->hwlen;
	len = p - duid;
	x = fprintf(f, "%s\n", hwaddr_ntoa(duid, len));
	fclose(f);
	/* Failed to write the duid? scrub it, we cannot use it */
	if (x < 1) {
		len = 0;
		unlink(DUID);
	}
	return len;
}


/* We can't include net.h or dhcpcd.h because
 * they would pull in net/if.h, which defeats the purpose of this hack. */
int
getifssid(const char *ifname, char *ssid)
{
#ifdef SIOCGIWESSID
	int s, retval;
	struct iwreq iwr;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return -1;
	memset(&iwr, 0, sizeof(iwr));
	strlcpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));
	iwr.u.essid.pointer = ssid;
	iwr.u.essid.length = IF_SSIDSIZE - 1;

	if (ioctl(s, SIOCGIWESSID, &iwr) == 0) {
		retval = iwr.u.essid.length;
		ssid[retval] = '\0';
	} else
		retval = -1;
	close(s);
	return retval;
#else
	/* Stop gcc warning about unused paramters */
	ifname = ssid;
	return -1;
#endif
}


static const char *mproc =
#if defined(__alpha__)
	"system type"
#elif defined(__arm__)
	"Hardware"
#elif defined(__avr32__)
	"cpu family"
#elif defined(__bfin__)
	"BOARD Name"
#elif defined(__cris__)
	"cpu model"
#elif defined(__frv__)
	"System"
#elif defined(__i386__) || defined(__x86_64__)
	"vendor_id"
#elif defined(__ia64__)
	"vendor"
#elif defined(__hppa__)
	"model"
#elif defined(__m68k__)
	"MMU"
#elif defined(__mips__)
	"system type"
#elif defined(__powerpc__) || defined(__powerpc64__)
	"machine"
#elif defined(__s390__) || defined(__s390x__)
	"Manufacturer"
#elif defined(__sh__)
	"machine"
#elif defined(sparc) || defined(__sparc__)
	"cpu"
#elif defined(__vax__)
	"cpu"
#else
	NULL
#endif
	;

char *
hardware_platform(void)
{
	FILE *fp;
	char *buf, *p;

	if (mproc == NULL) {
		errno = EINVAL;
		return NULL;
	}

	fp = fopen("/proc/cpuinfo", "r");
	if (fp == NULL)
		return NULL;

	p = NULL;
	while ((buf = get_line(fp))) {
		if (strncmp(buf, mproc, strlen(mproc)) == 0) {
			p = strchr(buf, ':');
			if (p != NULL && ++p != NULL) {
				while (*p == ' ')
					p++;
				break;
			}
		}
	}
	fclose(fp);

	if (p == NULL)
		errno = ESRCH;
	return p;
}

static int
check_proc_int(const char *path)
{
	FILE *fp;
	char *buf;

	fp = fopen(path, "r");
	if (fp == NULL)
		return -1;
	buf = get_line(fp);
	fclose(fp);
	if (buf == NULL)
		return -1;
	return atoi(buf);
}

static const char *prefix = "/proc/sys/net/ipv6/conf";

int
check_ipv6(const char *ifname)
{
	int r;
	char path[256];

	if (ifname == NULL)
		ifname = "all";

	snprintf(path, sizeof(path), "%s/%s/accept_ra", prefix, ifname);
	r = check_proc_int(path);
	if (r != 1 && r != 2) {
		dhcp_syslog(LOG_WARNING,
		    "%s: not configured to accept IPv6 RAs", ifname);
		return 0;
	}

	if (r != 2) {
		snprintf(path, sizeof(path), "%s/%s/forwarding",
		    prefix, ifname);
		if (check_proc_int(path) != 0) {
			dhcp_syslog(LOG_WARNING,
			    "%s: configured as a router, not a host", ifname);
			return 0;
		}
	}
	return 1;
}


int
closefrom(int fd)
{
	int max = getdtablesize();
	int i;
	int r = 0;

	for (i = fd; i < max; i++)
		r += close(i);
	return r;
}
