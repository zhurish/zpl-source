/* tracerouteLib.c - Packet InterNet Groper (TRACEROUTE) library */

/* Copyright 1994 - 2002 Wind River Systems, Inc. */
//#include "copyright_wrs.h"
/*
 modification history
 --------------------
 01r,22apr02,rae  Note that TRACEROUTE_OPT_DONTROUTE affects tracerouteing localhost
 (SPR #72917), other minor changes
 01q,11mar02,rae  Print stats when task killed (SPR #73570)
 01p,08jan02,rae  Don't print error messages when TRACEROUTE_OPT_SILENT (SPR #69537)
 01o,15oct01,rae  merge from truestack ver 01q, base 01k (SPRs 67440,
 30151, 66062, etc.)
 01n,30oct00,gnn  Added TRACEROUTE_OPT_NOHOST flag to deal with SPR 22766
 01m,08nov99,pul  T2 cumulative patch 2
 01l,22sep99,cno  corrected traceroute error for tracerouteRxPrint() (SPR22571)
 01k,14mar99,jdi  doc: removed refs to config.h and/or configAll.h (SPR 25663).
 01j,12mar99,p_m  Fixed SPR 8742 by documentating traceroute() configuration global
 variables.
 01i,05feb99,dgp  document errno values
 01h,17mar98,jmb  merge jmb patch of 04apr97 from HPSIM: corrected
 creation/deletion of task delete hook.
 01g,30oct97,cth  changed stack size of tPingTxn from 3000 to 6000 (SPR 8222).
 01f,26aug97,spm  removed compiler warnings (SPR #7866)
 01e,30sep96,spm  corrected traceroute error for little-endian machines (SPR #4235)
 01d,13mar95,dzb  changed to use free() instead of cfree() (SPR #4113)
 01c,24jan95,jdi  doc tweaks
 01b,10nov94,rhp  minor edits to man pages
 01a,25oct94,dzb  written
 */

/*
 DESCRIPTION
 This library contains the traceroute() utility, which tests the reachability
 of a remote host.

 The routine traceroute() is typically called from the VxWorks shell to check the
 network connection to another VxWorks target or to a UNIX host.  traceroute()
 may also be used programmatically by applications that require such a test.
 The remote host must be running TCP/IP networking code that responds to
 ICMP echo request packets.  The traceroute() routine is re-entrant, thus may
 be called by many tasks concurrently.

 The routine tracerouteLibInit() initializes the traceroute() utility and allocates
 resources used by this library.  It is called automatically when
 INCLUDE_TRACEROUTE is defined.
 */

/* includes */

/*
 #include "vxWorks.h"
 #include "string.h"
 #include "stdioLib.h"
 #include "wdLib.h"
 #include "netLib.h"
 #include "sockLib.h"
 #include "inetLib.h"
 #include "semLib.h"
 #include "taskLib.h"
 #include "hostLib.h"
 #include "ioLib.h"
 #include "tickLib.h"
 #include "taskHookLib.h"
 #include "sysLib.h"
 #include "vxLib.h"
 #include "netinet/in_systm.h"
 #include "netinet/ip.h"
 #include "netinet/ip_icmp.h"
 #include "netinet/icmp_var.h"
 #include "tracerouteLib.h"
 #include "errnoLib.h"
 #include "kernelLib.h"
 */
#include "zebra.h"
#include "buffer.h"
#include "command.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "vty.h"
#include "tracerouteLib.h"
#include "systools.h"
/* defines */

#define tracerouteError(pPS)	{ pPS->flags |= TRACEROUTE_OPT_SILENT; goto release; }

/* globals */

/* static forward declarations */

static int tracerouteRxPrint(TRACEROUTE_STAT *pPS, int len, struct sockaddr_in *from,
		struct timeval now, int cnt);
static void tracerouteFinish (TRACEROUTE_STAT * pPS);

/*******************************************************************************
 *
 * tracerouteLibInit - initialize the traceroute() utility
 *
 * This routine allocates resources used by the traceroute() utility.
 * It is called automatically when INCLUDE_TRACEROUTE is defined.
 *
 * RETURNS:
 * OK
 */



/*******************************************************************************
 *
 * traceroute - test that a remote host is reachable
 *
 * This routine tests that a remote host is reachable by sending ICMP
 * echo request packets, and waiting for replies.  It may called from
 * the VxWorks shell as follows:
 * .CS
 *    -> traceroute "remoteSystem", 1, 0
 * .CE
 * where <remoteSystem> is either a host name that has been previously added
 * to the remote host table by a call to hostAdd(), or an Internet address in
 * dot notation (for example, "90.0.0.2").
 *
 * The second parameter, <numPackets>, specifies the number of ICMP packets
 * to receive from the remote host.  If <numPackets> is 1, this routine waits
 * for a single echo reply packet, and then prints a short message
 * indicating whether the remote host is reachable.  For all other values
 * of <numPackets>, timing and sequence information is printed as echoed
 * packets are received.  If <numPackets> is 0, this routine runs continuously.
 *
 * If no replies are received within a 5-second timeout period, the
 * routine exits.  An ERROR status is returned if no echo replies
 * are received from the remote host.
 *
 * The following flags may be given through the <options> parameter:
 * .iP TRACEROUTE_OPT_SILENT
 * Suppress output.  This option is useful for applications that
 * use traceroute() programmatically to examine the return status.
 * .iP TRACEROUTE_OPT_DONTROUTE
 * Do not route packets past the local network.  This also prevents tracerouteing
 * local addresses (i.e. the IP address of the host itself).  The 127.x.x.x
 * addresses will still work however.
 * .iP TRACEROUTE_OPT_NOHOST
 * Suppress host lookup.  This is useful when you have the DNS resolver
 * but the DNS server is down and not returning host names.
 * .iP TRACEROUTE_OPT_DEBUG
 * Enables debug output.
 * .RS 4 4
 * \&NOTE: The following global variables can be set from the target shell
 * or Windsh to configure the traceroute() parameters:
 * .iP _tracerouteTxLen
 * Size of the ICMP echo packet (default 64).
 * .iP _tracerouteTxInterval
 * Packet interval in seconds (default 1 second).
 * .iP _tracerouteTxTmo
 * Packet timeout in seconds (default 5 seconds).
 *
 *.RE
 *
 * RETURNS:
 * OK, or ERROR if the remote host is not reachable.
 *
 * ERRNO: EINVAL, S_tracerouteLib_NOT_INITIALIZED, S_tracerouteLib_TIMEOUT
 *
 */

static int traceroute_thread(TRACEROUTE_STAT * pPS)
{
	struct sockaddr_in to; /* addr of Tx packet */
	struct sockaddr_in from; /* addr of Rx packet */
	int fromlen = sizeof(from);/* size of Rx addr */
	int ix = 1, ttl = 1; /* bytes read */
	int status = ERROR; /* return status */
	fd_set readFd;
	struct timeval tracerouteTmo;
	struct timeval now;
	int sel = 0, num = 0, seq = rand();
	struct vty *vty = pPS->vty;

	tracerouteTmo.tv_sec = pPS->tracerouteTxTmo;
	tracerouteTmo.tv_usec = 0;

	pPS->pBufIcmp = (struct icmp *) pPS->bufTx; /* pointer to icmp header out */
	pPS->pBufTime = (struct timeval *) (pPS->bufTx + 8);/* pointer to time out */

	pPS->idRx = os_task_gettid(); //os_task_pthread_self(); /* get own task Id  */

	/* initialize the socket address struct */

	to.sin_family = AF_INET;
	to.sin_addr.s_addr = inet_addr(pPS->toInetName);

	//strcpy(pPS->toHostName, host); /* save host name */

	pPS->dataLen = pPS->tracerouteTxLen - 8; /* compute size of data */

	/* open raw socket for ICMP communication */

	if ((pPS->tracerouteFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		tracerouteError(pPS);

	if (pPS->flags & TRACEROUTE_OPT_DONTROUTE) /* disallow packet routing ? */
		if (setsockopt(pPS->tracerouteFd, SOL_SOCKET, SO_DONTROUTE,
				(char *) &ix, sizeof(ix)) == ERROR)
			tracerouteError(pPS);

	pPS->pBufIcmp->icmp_type = ICMP_ECHO; /* set up Tx buffer */
	pPS->pBufIcmp->icmp_code = 0;
	pPS->pBufIcmp->icmp_id = htons(pPS->idRx & 0xffff);

	for (ix = sizeof(struct timeval); ix < pPS->dataLen; ix++) /* skip 4 bytes for time */
		pPS->bufTx[8 + ix] = ix;

/*
	vty_out(vty, " dataLen           = %d %s", pPS->dataLen, VTY_NEWLINE);
	vty_out(vty, " rxmaxlen          = %d %s", pPS->rxmaxlen, VTY_NEWLINE);
	vty_out(vty, " tracerouteTxLen   = %d %s", pPS->tracerouteTxLen, VTY_NEWLINE);
	vty_out(vty, " tracerouteTxTmo   = %d %s", pPS->tracerouteTxTmo, VTY_NEWLINE);
	vty_out(vty, " maxttl            = %d %s", pPS->maxttl, VTY_NEWLINE);
*/

	vty_out(vty, " traceroute to %s (%s), %d hops max, %d byte packets%s",
			pPS->toHostName, pPS->toInetName, pPS->maxttl, pPS->dataLen, VTY_NEWLINE);
	/*
	* traceroute to 14.215.177.38 (14.215.177.38), 30 hops max, 60 byte packets
	*/
	while (ttl <= pPS->maxttl)
	{
send_next:

		if(from.sin_addr.s_addr == to.sin_addr.s_addr)
			break;
		if(ttl > pPS->maxttl)
			break;

		//vty_out(vty, " ttl = %d maxttl = %d %s", ttl, pPS->maxttl, VTY_NEWLINE);

		setsockopt(pPS->tracerouteFd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
		vty_out(vty, " %d ", ttl);

		ttl++;
		os_sleep(TRACEROUTE_INTERVAL);
		for (num = 0; num < 3; num++)
		{
			os_gettime(OS_CLK_MONOTONIC, pPS->pBufTime); /* load current tick count */
			memcpy(&now, pPS->pBufTime, sizeof(struct timeval));
			pPS->pBufIcmp->icmp_seq = seq++; /* increment seq number */
			pPS->pBufIcmp->icmp_cksum = 0;
			pPS->pBufIcmp->icmp_cksum = in_cksum((u_short *) pPS->pBufIcmp,
					pPS->tracerouteTxLen);

			if ((ix = sendto(pPS->tracerouteFd, (char *) pPS->pBufIcmp,
					pPS->tracerouteTxLen, 0, (struct sockaddr *) &to,
					sizeof(struct sockaddr))) != pPS->tracerouteTxLen)
			{
				if (pPS->flags & TRACEROUTE_OPT_DEBUG)
					vty_out(vty, "%s traceroute: wrote %s %d chars, ret=%d%s",
							VTY_NEWLINE, pPS->toHostName, pPS->tracerouteTxLen, ix,
							VTY_NEWLINE);
				if (ix < 0)
				{
					if (errno == ENETUNREACH)
						vty_out(vty, "%s traceroute %s Network is unreachable%s",
								VTY_NEWLINE, pPS->toHostName, VTY_NEWLINE);
					else if (errno == EHOSTUNREACH)
						vty_out(vty, "%s traceroute %s No route to host%s",
								VTY_NEWLINE, pPS->toHostName, VTY_NEWLINE);
					goto release;
				}
			}

check_fd_again: /* Wait for ICMP reply */
			FD_ZERO(&readFd);
			FD_SET(pPS->tracerouteFd, &readFd);
			FD_SET(vty->fd, &readFd);
			sel = select(max(pPS->tracerouteFd, vty->fd) + 1, &readFd, NULL,
					NULL, &tracerouteTmo);
			if (sel == ERROR)
			{
				if (errno == EINTR)
					goto check_fd_again;

				if (!(pPS->flags & TRACEROUTE_OPT_SILENT))
					vty_out(vty, "%s traceroute: ERROR%s", VTY_NEWLINE, VTY_NEWLINE);
				goto release;
			}
			else if (sel == 0)
			{
				if (!(pPS->flags & TRACEROUTE_OPT_SILENT))
					vty_out(vty, " * ");
				if( num == 2)
				{
					vty_out(vty, "%s", VTY_NEWLINE);
					goto send_next;
				}
				continue;
			}

			if (FD_ISSET(vty->fd, &readFd))
			{
#ifndef CONTROL
#define CONTROL(X)  ((X) - '@')
#endif
				int c = vty_getc_input(vty);
				if (c == '\r' || c == '\n')
					vty_out(vty, "%s", VTY_NEWLINE);
				else if (c == CONTROL('C') || c == CONTROL('Z'))
					break;
				else
					vty_out(vty, "%s", VTY_NEWLINE);
			}
			if (!FD_ISSET(pPS->tracerouteFd, &readFd))
				goto check_fd_again;

			/* the fd is ready - FD_ISSET isn't needed */
			if ((ix = recvfrom(pPS->tracerouteFd, (char *) pPS->bufRx,
					pPS->rxmaxlen, 0, (struct sockaddr *) &from, &fromlen))
					== ERROR)
			{
				if (errno == EINTR)
					goto check_fd_again;
				break; /* goto release */
			}
			//vty_out(vty, " recvfrom %s%s", inet_ntoa(from.sin_addr), VTY_NEWLINE);

			if (tracerouteRxPrint(pPS, ix, &from, now, num) == ERROR)
				goto check_fd_again;
			//else
			//	goto send_next;
		}
	}
release:
	vty_out(vty, "%s", VTY_NEWLINE);
	tracerouteFinish(pPS);
	return (status);
}

int traceroute(struct vty *vty, char * host, int maxttl, int len, u_int32 options)
{
	TRACEROUTE_STAT * pPS = NULL;
	struct in_addr addr;

	pPS = (TRACEROUTE_STAT *)XMALLOC(MTYPE_DATA, sizeof(TRACEROUTE_STAT));
	if(!pPS)
		return ERROR;

	memset(pPS, 0, sizeof(TRACEROUTE_STAT));

	pPS->rxmaxlen = len + TRACEROUTE_MINPACKET;
	pPS->bufRx = (char *)XMALLOC(MTYPE_DATA, pPS->rxmaxlen);
	if(!pPS->bufRx)
	{
		free(pPS);
		return ERROR;
	}
	memset(pPS->bufRx, 0, pPS->rxmaxlen);
	pPS->bufTx = (char *)XMALLOC(MTYPE_DATA, pPS->rxmaxlen);
	if(!pPS->bufTx)
	{
		free(pPS->bufRx);
		free(pPS);
		return ERROR;
	}
	memset(pPS->bufTx, 0, pPS->rxmaxlen);
	pPS->flags = options;
	pPS->tracerouteTxTmo = TRACEROUTE_TMO; /* packet timeout in seconds */
	pPS->tracerouteTxLen = len;
	pPS->tracerouteTxLen = max(pPS->tracerouteTxLen, TRACEROUTE_MINPACKET); /* sanity check global */
	pPS->tracerouteTxLen = min(pPS->tracerouteTxLen, TRACEROUTE_MAXPACKET); /* sanity check global */
    pPS->ifindex = 0;
    pPS->maxttl = maxttl;
    if(maxttl == 0)
    	 pPS->maxttl = TRACEROUTE_MAXTTL;
    pPS->vty = vty;
    if(inet_aton (host, &addr) == 0)
	{
    	struct hostent * hoste = NULL;
    	hoste = gethostbyname(host);
    	if (hoste && hoste->h_addr_list[0])
    	{
    		//addr.s_addr = hoste->h_addr_list[0];
    		addr = *(struct in_addr*)hoste->h_addr_list[0];
    		sprintf(pPS->toInetName, "%s", inet_ntoa(addr));
			strcpy(pPS->toHostName, host);
    	}
	}
    else
    {
    	strcpy(pPS->toInetName, host);
    	strcpy(pPS->toHostName, host);
    }

    vty_ansync_enable(vty, TRUE);
    vty_out(vty,"%s", VTY_NEWLINE);
    traceroute_thread(pPS);

    vty_ansync_enable(vty, FALSE);

	if(pPS->bufRx != NULL)
		XFREE(MTYPE_DATA, pPS->bufRx);
	if(pPS->bufTx != NULL)
		XFREE(MTYPE_DATA, pPS->bufTx);
	if(pPS != NULL)
		XFREE(MTYPE_DATA, pPS);
	pPS = NULL;
	return OK;
}
/*******************************************************************************
 *
 * tracerouteRxPrint - print out information about a received packet
 *
 * This routine prints out information about a received ICMP echo reply
 * packet.  First, the packet is checked for minimum length and
 * correct message type and destination.
 *
 * RETURNS:
 * N/A.
 */

static int tracerouteRxPrint(TRACEROUTE_STAT * pPS, /* traceroute stats structure */
		int len, /* Rx message length */
		struct sockaddr_in *from, /* Rx message address */
		struct timeval send, int cnt)
{
	struct ip * ip = (struct ip *) pPS->bufRx;
	struct icmp * icp = NULL;
	int hlen = 0;
	int triptime = 0;
	struct timeval now;
	struct vty *vty = pPS->vty;

	os_gettime(OS_CLK_MONOTONIC, &now);

	hlen = ip->ip_hl << 2;

	if (len < hlen + ICMP_MINLEN) /* at least min length ? */
	{
		if (pPS->flags & TRACEROUTE_OPT_DEBUG)
			vty_out(vty, "packet too short (%d bytes) from %s%s", len,
					inet_ntoa(from->sin_addr), VTY_NEWLINE);
		return (ERROR);
	}

	len -= hlen; /* strip IP header */
	icp = (struct icmp *) (pPS->bufRx + hlen);
	if (icp->icmp_type == ICMP_TIME_EXCEEDED || icp->icmp_type == ICMP_TIMXCEED_INTRANS)
	{
		ip = &icp->icmp_ip;
		hlen = ip->ip_hl << 2;
		if(ip->ip_p == IPPROTO_ICMP)
		{
			//struct icmp *hicmp;
			icp = (struct icmp *)((unsigned char *)ip + hlen);
			if (ntohs(icp->icmp_id) != (pPS->idRx & 0xffff))
			{
				return ERROR;
			}
			if (cnt == 0)
			{
				vty_out(vty, " %s", inet_ntoa(from->sin_addr));
				vty_out(vty, " (%s) ", inet_ntoa(from->sin_addr));
			}
			triptime = os_timeval_elapsed(now, send) / 1000;
			vty_out(vty, " %d ms", triptime);
			if (cnt == 2)
				vty_out(vty, " %s", VTY_NEWLINE);
			return (OK);
		}
/*		if(ip->ip_p == IPPROTO_UDP)
		{
			icp = (struct icmp *)((unsigned char *)ip + hlen);
			if (ntohs(icp->icmp_id) != (pPS->idRx & 0xffff))
			{
				return ERROR;
			}
			vty_out(vty, " %s", inet_ntoa(from->sin_addr));
			vty_out(vty, " (%s) ", inet_ntoa(from->sin_addr));
			triptime = os_timeval_elapsed(now, *ti) / 1000;
			vty_out(vty, " %d ms", triptime);
			if (cnt == 2)
				vty_out(vty, " %s", VTY_NEWLINE);
		}*/
	}
	return (ERROR);
}

/*******************************************************************************
 *
 * tracerouteFinish - return all allocated resources and print out final statistics
 *
 * This routine returns all resources allocated for the traceroute session, and
 * prints out a stats summary.
 *
 * The traceroute session is located in the session list (tracerouteHead) by searching
 * the traceroute stats structure for the receiver task ID.  This is necessary
 * because this routine is passed a pointer to the task control block, and does
 * not have ready access to the traceroute stats structure itself.  This accomodates
 * the use of task delete hooks as a means of calling this routine.
 *
 * RETURNS:
 * N/A.
 */

static void tracerouteFinish(TRACEROUTE_STAT * pPS /* pointer to task control block */
)
{
	if (pPS->tracerouteFd)
		(void) close(pPS->tracerouteFd);
}
