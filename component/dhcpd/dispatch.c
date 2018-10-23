/*	$OpenBSD: dispatch.c,v 1.38 2016/11/15 10:49:37 mestre Exp $ */

/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999
 * The Internet Software Consortium.   All rights reserved.
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
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <net/if.h>
//#include <net/if_dl.h>
//#include <net/if_media.h>

#include <netinet/in.h>

#include <errno.h>
#include <ifaddrs.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"
#include "sync.h"

extern int syncfd;

struct interface_info *dhcpd_interfaces_list = NULL;


#ifdef DHCPD_ANSYNC_ENABLE
void
dispatch(void)
{
	os_ansync_main(dhcpd_lstmaster, OS_ANSYNC_EXECUTE_ARGV);
}
#else
static struct protocol *protocols = NULL;
static struct dhcpd_timeout *timeouts = NULL;
static struct dhcpd_timeout *free_timeouts = NULL;
static int interfaces_invalidated;


/*
 * Wait for packets to come in using poll().  When a packet comes in,
 * call receive_packet to receive the packet and possibly strip hardware
 * addressing information from it, and then process it in do_packet.
 */
void
dispatch(void)
{
	int nfds, i, to_msec, num = 0;
	struct protocol *l = NULL;
	static struct pollfd *fds = NULL;
	static int nfds_max;
	time_t howlong;

	for (nfds = 0, l = protocols; l; l = l->next)
		nfds++;
	/*	if (syncfd != -1)
	 nfds++;*/
	if (nfds > nfds_max)
	{
		fds = reallocarray(fds, nfds, sizeof(struct pollfd));
		if (fds == NULL)
		{
			dhcpd_error("Can't allocate poll structures.");
			return;
		}
		nfds_max = nfds;
	}

	for (;;)
	{
		/*
		 * Call any expired timeouts, and then if there's
		 * still a timeout registered, time out the poll
		 * call then.
		 */
		time(&root_group.cur_time);
		another: if (timeouts)
		{
			if (timeouts->when <= root_group.cur_time)
			{
				struct dhcpd_timeout *t = timeouts;
				timeouts = timeouts->next;
				(*(t->func))(t->what);
				t->next = free_timeouts;
				free_timeouts = t;
				goto another;
			}

			/*
			 * Figure timeout in milliseconds, and check for
			 * potential overflow, so we can cram into an int
			 * for poll, while not polling with a negative
			 * timeout and blocking indefinitely.
			 */
			howlong = timeouts->when - root_group.cur_time;
			if (howlong > INT_MAX / 1000)
				howlong = INT_MAX / 1000;
			to_msec = howlong * 1000;
		}
		else
			to_msec = 50000;

		/* Set up the descriptors to be polled. */
		for (i = 0, l = protocols; l; l = l->next)
		{
			if (l->local)
			{
				fds[i].fd = l->fd;
				fds[i].events = POLLIN;
				++i;
			}
		}

		if (i == 0)
			dhcpd_error("No live interfaces to poll on - exiting.");
#if 0
		if (syncfd != -1)
		{
			/* add syncer */
			fds[i].fd = syncfd;
			fds[i].events = POLLIN;
		}
#endif
		/* Wait for a packet or a timeout... */
		num = poll(fds, nfds, to_msec);
		switch (num)
		{
		case -1:
			if (errno != EAGAIN && errno != EINTR)
			{
				dhcpd_error("poll: %m");
				return;
			}
			/* FALLTHROUGH */
		case 0:
			continue; /* no packets */
		default:
			break;
		}

		time(&root_group.cur_time);

		for (i = 0, l = protocols; l; l = l->next)
		{
			if ((fds[i].revents & (POLLIN | POLLHUP)) && fds[i].fd == l->fd)
			{
				if (l->local)
				{
					(*(l->handler))(l);
					if (interfaces_invalidated)
						break;
				}
			}
			++i;
		}
		/*if ((syncfd != -1) && (fds[i].revents & (POLLIN | POLLHUP)))
		 sync_recv();*/
		interfaces_invalidated = 0;
	}
}
#endif

int
locate_network(struct packet *packet)
{
	struct iaddr ia;

	/* If this came through a gateway, find the corresponding subnet... */
	if (packet->raw->giaddr.s_addr) {
		struct subnet *subnet = NULL;

		ia.len = 4;
		memcpy(ia.iabuf, &packet->raw->giaddr, 4);
		zlog_debug(6, "locate_network : %d.%d.%d.%d",
				ia.iabuf[0],ia.iabuf[1],ia.iabuf[2],ia.iabuf[3]);

		subnet = find_subnet(ia);
		if (subnet)
			packet->shared_network = subnet->shared_network;
		else
			packet->shared_network = NULL;
	} else {
		packet->shared_network = packet->interface->shared_network;
	}
	if (packet->shared_network)
		return 1;
	return 0;
}

#ifndef DHCPD_ANSYNC_ENABLE
void
add_timeout(time_t when, void (*where)(void *), void *what)
{
	struct dhcpd_timeout *t = NULL, *q = NULL;

	/* See if this timeout supersedes an existing timeout. */
	t = NULL;
	for (q = timeouts; q; q = q->next) {
		if (q->func == where && q->what == what) {
			if (t)
				t->next = q->next;
			else
				timeouts = q->next;
			break;
		}
		t = q;
	}

	/* If we didn't supersede a timeout, allocate a timeout
	   structure now. */
	if (!q) {
		if (free_timeouts) {
			q = free_timeouts;
			free_timeouts = q->next;
			q->func = where;
			q->what = what;
		} else {
			q = malloc(sizeof (struct dhcpd_timeout));
			if (!q)
				dhcpd_error("Can't allocate timeout structure!");
			memset(q, 0, sizeof(struct dhcpd_timeout));
			q->func = where;
			q->what = what;
		}
	}

	q->when = when;

	/* Now sort this timeout into the timeout list. */

	/* Beginning of list? */
	if (!timeouts || timeouts->when > q->when) {
		q->next = timeouts;
		timeouts = q;
		return;
	}

	/* Middle of list? */
	for (t = timeouts; t->next; t = t->next) {
		if (t->next->when > q->when) {
			q->next = t->next;
			t->next = q;
			return;
		}
	}

	/* End of list. */
	t->next = q;
	q->next = NULL;
}

void
cancel_timeout(void (*where)(void *), void *what)
{
	struct dhcpd_timeout *t = NULL, *q = NULL;

	/* Look for this timeout on the list, and unlink it if we find it. */
	t = NULL;
	for (q = timeouts; q; q = q->next) {
		if (q->func == where && q->what == what) {
			if (t)
				t->next = q->next;
			else
				timeouts = q->next;
			break;
		}
		t = q;
	}

	/* If we found the timeout, put it on the free list. */
	if (q) {
		q->next = free_timeouts;
		free_timeouts = q;
	}
}

/* Add a protocol to the list of protocols... */
void
add_protocol(char *name, int fd, void (*handler)(struct protocol *),
    void *local)
{
	struct protocol *p = NULL;

	p = malloc(sizeof *p);
	if (!p)
	{
		dhcpd_error("can't allocate protocol struct for %s", name);
		return;
	}
	memset(p, 0, sizeof(struct protocol));
	p->fd = fd;
	p->handler = handler;
	p->local = local;
	strcpy(p->name, name);
	p->next = protocols;
	protocols = p;
}

void
remove_protocol(struct protocol *proto)
{
	struct protocol *p = NULL, *next = NULL, *prev = NULL;

	for (p = protocols; p; p = next) {
		next = p->next;
		if (p == proto) {
			if (prev)
				prev->next = p->next;
			else
				protocols = p->next;
			free(p);
		}
	}
}
#endif
