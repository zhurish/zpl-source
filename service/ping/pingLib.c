/* pingLib.c - Packet InterNet Groper (PING) library */

/* Copyright 1994 - 2002 Wind River Systems, Inc. */
//#include "copyright_wrs.h"
/*
 modification history
 --------------------
 01r,22apr02,rae  Note that PING_OPT_DONTROUTE affects pinging localhost
 (SPR #72917), other minor changes
 01q,11mar02,rae  Print stats when task killed (SPR #73570)
 01p,08jan02,rae  Don't print error messages when PING_OPT_SILENT (SPR #69537)
 01o,15oct01,rae  merge from truestack ver 01q, base 01k (SPRs 67440,
 30151, 66062, etc.)
 01n,30oct00,gnn  Added PING_OPT_NOHOST flag to deal with SPR 22766
 01m,08nov99,pul  T2 cumulative patch 2
 01l,22sep99,cno  corrected ping error for pingRxPrint() (SPR22571)
 01k,14mar99,jdi  doc: removed refs to config.h and/or configAll.h (SPR 25663).
 01j,12mar99,p_m  Fixed SPR 8742 by documentating ping() configuration global
 variables.
 01i,05feb99,dgp  document ipstack_errno values
 01h,17mar98,jmb  merge jmb patch of 04apr97 from HPSIM: corrected
 creation/deletion of task delete hook.
 01g,30oct97,cth  changed stack size of tPingTxn from 3000 to 6000 (SPR 8222).
 01f,26aug97,spm  removed compiler warnings (SPR #7866)
 01e,30sep96,spm  corrected ping error for little-endian machines (SPR #4235)
 01d,13mar95,dzb  changed to use free() instead of cfree() (SPR #4113)
 01c,24jan95,jdi  doc tweaks
 01b,10nov94,rhp  minor edits to man pages
 01a,25oct94,dzb  written
 */

/*
 DESCRIPTION
 This library contains the ping() utility, which tests the reachability
 of a remote host.

 The routine ping() is typically called from the VxWorks shell to check the
 network connection to another VxWorks target or to a UNIX host.  ping()
 may also be used programmatically by applications that require such a test.
 The remote host must be running TCP/IP networking code that responds to
 ICMP echo request packets.  The ping() routine is re-entrant, thus may
 be called by many tasks concurrently.

 The routine pingLibInit() initializes the ping() utility and allocates
 resources used by this library.  It is called automatically when
 INCLUDE_PING is defined.
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
 #include "pingLib.h"
 #include "errnoLib.h"
 #include "kernelLib.h"
 */
#include "service.h"
#include "vty.h"
#include "zmemory.h"
#include "command.h"
#include "pingLib.h"
#include "checksum.h"
/* defines */

#define pingError(pPS)	{ pPS->flags |= PING_OPT_SILENT; goto release; }

/* globals */

/* static forward declarations */

static int pingRxPrint(PING_STAT *pPS, int len, struct ipstack_sockaddr_in *from,
		struct timeval now);
static void pingFinish (PING_STAT * pPS);

/*******************************************************************************
 *
 * pingLibInit - initialize the ping() utility
 *
 * This routine allocates resources used by the ping() utility.
 * It is called automatically when INCLUDE_PING is defined.
 *
 * RETURNS:
 * OK
 */



/*******************************************************************************
 *
 * ping - test that a remote host is reachable
 *
 * This routine tests that a remote host is reachable by sending ICMP
 * echo request packets, and waiting for replies.  It may called from
 * the VxWorks shell as follows:
 * .CS
 *    -> ping "remoteSystem", 1, 0
 * .CE
 * where <remoteSystem> is either a host name that has been previously added
 * to the remote host table by a call to hostAdd(), or an Internet address in
 * dot notation (for example, "90.0.0.2").
 *
 * The second parameter, <numPackets>, specifies the number of ICMP packets
 * to receive from the remote host.  If <numPackets> is 1, this routine waits
 * for a single echo reply packet, and then prints a zpl_int16 message
 * indicating whether the remote host is reachable.  For all other values
 * of <numPackets>, timing and sequence information is printed as echoed
 * packets are received.  If <numPackets> is 0, this routine runs continuously.
 *
 * If no replies are received within a 5-second timeout period, the
 * routine exits.  An ERROR status is returned if no echo replies
 * are received from the remote host.
 *
 * The following flags may be given through the <options> parameter:
 * .iP PING_OPT_SILENT
 * Suppress output.  This option is useful for applications that
 * use ping() programmatically to examine the return status.
 * .iP PING_OPT_DONTROUTE
 * Do not route packets past the local network.  This also prevents pinging
 * local addresses (i.e. the IP address of the host itself).  The 127.x.x.x
 * addresses will still work however.
 * .iP PING_OPT_NOHOST
 * Suppress host lookup.  This is useful when you have the DNS resolver
 * but the DNS server is down and not returning host names.
 * .iP PING_OPT_DEBUG
 * Enables debug output.
 * .RS 4 4
 * \&NOTE: The following global variables can be set from the target shell
 * or Windsh to configure the ping() parameters:
 * .iP _pingTxLen
 * Size of the ICMP echo packet (default 64).
 * .iP _pingTxInterval
 * Packet interval in seconds (default 1 second).
 * .iP _pingTxTmo
 * Packet timeout in seconds (default 5 seconds).
 *
 *.RE
 *
 * RETURNS:
 * OK, or ERROR if the remote host is not reachable.
 *
 * ERRNO: IPSTACK_ERRNO_EINVAL, S_pingLib_NOT_INITIALIZED, S_pingLib_TIMEOUT
 *
 */

static int ping_thread(PING_STAT * pPS)
{
	struct ipstack_sockaddr_in to; /* addr of Tx packet */
	struct ipstack_sockaddr_in from; /* addr of Rx packet */
	int fromlen = sizeof(from);/* size of Rx addr */
	int ix = 1, ttl = 0; /* bytes read */
	int status = ERROR; /* return status */
	ipstack_fd_set readFd;
	struct timeval pingTmo;
	struct timeval now;
	int sel = 0;
	struct vty *vty = pPS->vty;

//	os_sleep(1);

	if (pPS->numPacket == 0)
		pPS->numPacket = 3; /* don't do infinite by default */

	pPS->tMin = 999999999; /* init min rt time */
	//pPS->flags = options; /* save flags field */

	pingTmo.tv_sec = pPS->pingTxTmo;
	pingTmo.tv_usec = 0;

	pPS->pBufIcmp = (struct icmp *) pPS->bufTx; /* pointer to icmp header out */
	pPS->pBufTime = (struct timeval *) (pPS->bufTx + 8);/* pointer to time out */

	pPS->idRx = os_task_gettid();//os_task_pthread_self(); /* get own task Id  */

	/* initialize the ipstack_socket address struct */

	to.sin_family = IPSTACK_AF_INET;
	to.sin_addr.s_addr = ipstack_inet_addr(pPS->toInetName);

	//strcpy(pPS->toHostName, host); /* save host name */

	pPS->dataLen = pPS->pingTxLen - 8; /* compute size of data */

	/* open raw ipstack_socket for ICMP communication */
	pPS->pingFd = ipstack_socket(IPCOM_STACK, IPSTACK_AF_INET, IPSTACK_SOCK_RAW, IPSTACK_IPPROTO_ICMP);
	if (ipstack_invalid(pPS->pingFd))
		pingError(pPS);

	if (pPS->flags & PING_OPT_DONTROUTE) /* disallow packet routing ? */
		if (ipstack_setsockopt(pPS->pingFd, IPSTACK_SOL_SOCKET, IPSTACK_SO_DONTROUTE, (char *) &ix,
				sizeof(ix)) == ERROR)
			pingError(pPS);

	if (!(pPS->flags & PING_OPT_SILENT) && pPS->numPacket != 1)
	{
		vty_out(vty,"PING %s (%s)", pPS->toHostName, pPS->toInetName); /* print out dest info */
		vty_out(vty,": %d data bytes%s", pPS->dataLen, VTY_NEWLINE);
	}
	if(pPS->maxttl)
	{
		ttl = pPS->maxttl;
		ipstack_setsockopt(pPS->pingFd, IPSTACK_IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
	}
	pPS->pBufIcmp->icmp_type = ICMP_ECHO; /* set up Tx buffer */
	pPS->pBufIcmp->icmp_code = 0;
	pPS->pBufIcmp->icmp_id = htons(pPS->idRx & 0xffff);

	for (ix = sizeof(struct timeval); ix < pPS->dataLen; ix++) /* skip 4 bytes for time */
		pPS->bufTx[8 + ix] = ix;

	/* receive echo reply packets from remote host */
	while (!pPS->numPacket || (pPS->numRx != pPS->numPacket))
	{
		if(pPS->quit)
			break;
		if(pPS->numRx == pPS->numPacket)
			break;
		os_gettime(OS_CLK_MONOTONIC, pPS->pBufTime); /* load current tick count */
		pPS->pBufIcmp->icmp_seq = pPS->numTx++; /* increment seq number */
		pPS->pBufIcmp->icmp_cksum = 0;
		pPS->pBufIcmp->icmp_cksum = in_cksum((zpl_ushort *) pPS->pBufIcmp,
				pPS->pingTxLen);
		/* transmit ICMP packet */

		if ((ix = ipstack_sendto(pPS->pingFd, (char *) pPS->pBufIcmp, pPS->pingTxLen, 0,
				(struct ipstack_sockaddr *) &to, sizeof(struct ipstack_sockaddr)))
				!= pPS->pingTxLen)
		{
			if (pPS->flags & PING_OPT_DEBUG)
				vty_out(vty,"ping: wrote %s %d chars, ret=%d%s", pPS->toInetName,
						pPS->pingTxLen, ix, VTY_NEWLINE);
			if(ix < 0)
			{
				if(ipstack_errno == IPSTACK_ERRNO_ENETUNREACH)
					vty_out(vty,"ping %s Network is unreachable%s", pPS->toInetName,VTY_NEWLINE);
				else if(ipstack_errno == IPSTACK_ERRNO_EHOSTUNREACH)
					vty_out(vty,"ping %s No route to host%s", pPS->toInetName,VTY_NEWLINE);
				break;
			}
		}
		/* Update ICMP statistics for ECHO messages - this is not the best
		 * place to put it since this shifts responsibility of updating ECHO
		 * statistics to anyone writing an application that sends out ECHO
		 * messages. However, this seems to be the only place to put it.
		 */

check_fd_again: /* Wait for ICMP reply */
		IPSTACK_FD_ZERO(&readFd);
		IPSTACK_FD_SET(ipstack_fd(pPS->pingFd), &readFd);
		IPSTACK_FD_SET(ipstack_fd(vty->fd), &readFd);
		sel = ipstack_select(IPCOM_STACK, max(ipstack_fd(pPS->pingFd), ipstack_fd(vty->fd)) + 1, &readFd, NULL, NULL, &pingTmo);
		if (sel == ERROR)
		{
			if (!(pPS->flags & PING_OPT_SILENT))
				vty_out(vty,"ping: ERROR%s", VTY_NEWLINE);
			break; /* goto release */
		}
		else if (sel == 0)
		{
			if (!(pPS->flags & PING_OPT_SILENT))
				vty_out(vty,"ping: timeout%s",VTY_NEWLINE);
			break; /* goto release */
		}
		if (IPSTACK_FD_ISSET(ipstack_fd(vty->fd), &readFd))
		{
#ifndef CONTROL
#define CONTROL(X)  ((X) - '@')
#endif
			int c = vty_getc_input(vty);
			if(c == '\r' || c == '\n')
				vty_out(vty,"%s",VTY_NEWLINE);
			else if(c == CONTROL('C') || c == CONTROL('Z'))
				break;
			else
				vty_out(vty,"%s",VTY_NEWLINE);
		}
		if (!IPSTACK_FD_ISSET(ipstack_fd(pPS->pingFd), &readFd))
			goto check_fd_again;

		/* the fd is ready - FD_ISSET isn't needed */
		if ((ix = ipstack_recvfrom(pPS->pingFd, (char *) pPS->bufRx, pPS->rxmaxlen, 0,
				(struct ipstack_sockaddr *) &from, &fromlen)) == ERROR)
		{
			if (ipstack_errno == IPSTACK_ERRNO_EINTR)
				goto check_fd_again;
			break; /* goto release */
		}
		os_gettime(OS_CLK_MONOTONIC, &now);
		if (pingRxPrint(pPS, ix, &from, now) == ERROR)
			goto check_fd_again;

		//vty_out(vty, "=========> %d %d %s", pPS->numRx, pPS->numPacket, VTY_NEWLINE);
		os_sleep(pPS->pingTxInterval);
	}

	if (pPS->numRx > 0)
		status = OK; /* host is reachable */

release:
	pingFinish(pPS);
	return (status);
}

/*
static int ping_ctrl_quit(struct vty *vty, int ctrl, void *p)
{
	PING_STAT * pPS = (PING_STAT *)p;
	if(pPS)
	{
		pPS->quit = zpl_true;
	    //vty->shell_ctrl_cmd = NULL;
	    //vty->ctrl = NULL;
	}
	return OK;
}
*/

int ping(struct vty *vty, char * host, int numPackets, int len, zpl_uint32 options)
{
	PING_STAT * pPS = NULL;
	struct ipstack_in_addr addr;

	pPS = (PING_STAT *)XMALLOC(MTYPE_DATA, sizeof(PING_STAT));
	if(!pPS)
		return ERROR;

	memset(pPS, 0, sizeof(PING_STAT));

	pPS->rxmaxlen = len + PING_MINPACKET;
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
	pPS->pingTxLen = len;
	pPS->pingTxLen = max(pPS->pingTxLen, PING_MINPACKET); /* sanity check global */
	pPS->pingTxLen = min(pPS->pingTxLen, PING_MAXPACKET); /* sanity check global */
	pPS->pingTxInterval = PING_INTERVAL; /* packet interval in seconds */
	pPS->pingTxTmo = PING_TMO + 1; /* packet timeout in seconds */
	pPS->numPacket = numPackets;
    pPS->ifindex = 0;
    pPS->numRx = 0;
    pPS->numTx = 0;
    pPS->vty = vty;
    if(ipstack_inet_aton (host, &addr) == 0)
	{
    	struct ipstack_hostent * hoste = NULL;
    	hoste = ipstack_gethostbyname(host);
    	if (hoste && hoste->h_addr_list[0])
    	{
    		addr = *(struct ipstack_in_addr*)hoste->h_addr_list[0];//hoste->h_addr_list[0];
    		sprintf(pPS->toInetName, "%s", ipstack_inet_ntoa(addr));
			strcpy(pPS->toHostName, host);
			//vty_out(vty, "PING -----> %s (%s)%s", pPS->toHostName, pPS->toInetName, VTY_NEWLINE);
    	}
	}
    else
    {
    	strcpy(pPS->toInetName, host);
		strcpy(pPS->toHostName, host);
    }

    vty_ansync_enable(vty, zpl_true);
    vty_out(vty,"%s", VTY_NEWLINE);
    ping_thread(pPS);
    //os_thread_once(ping_thread, pPS);
    //vty->shell_ctrl_cmd = ping_ctrl_quit;
    //vty->ctrl = pPS;
    //pingFinish(pPS);
    vty_ansync_enable(vty, zpl_false);

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
 * pingRxPrint - print out information about a received packet
 *
 * This routine prints out information about a received ICMP echo reply
 * packet.  First, the packet is checked for minimum length and
 * correct message type and destination.
 *
 * RETURNS:
 * N/A.
 */

static int pingRxPrint(PING_STAT * pPS, /* ping stats structure */
		int len, /* Rx message length */
		struct ipstack_sockaddr_in *from, /* Rx message address */
		struct timeval now)
{
	struct ip * ip = (struct ip *) pPS->bufRx;
	long * lp = (long *) pPS->bufRx;
	struct icmp * icp = NULL;
	int ix = 0;
	int hlen = 0;
	int triptime = 0;
	struct timeval *ti = NULL;
	struct vty *vty = pPS->vty;

	hlen = ip->ip_hl << 2;

	if (len < hlen + ICMP_MINLEN) /* at least min length ? */
	{
		if (pPS->flags & PING_OPT_DEBUG)
			vty_out(vty,"packet too zpl_int16 (%d bytes) from %s%s", len, ipstack_inet_ntoa(from->sin_addr), VTY_NEWLINE);
		return (ERROR);
	}

	len -= hlen; /* strip IP header */
	icp = (struct icmp *) (pPS->bufRx + hlen);
	ti = (struct timeval *) (pPS->bufRx + hlen + 8);
	if (icp->icmp_type != ICMP_ECHOREPLY) /* right message ? */
	{
		if (pPS->flags & PING_OPT_DEBUG) /* debug odd message */
		{
			vty_out(vty,"%d bytes from %s: ", len, ipstack_inet_ntoa(from->sin_addr));
			//icp->icmp_type = min(icp->icmp_type, ICMP_TYPENUM);
			vty_out(vty,"icmp_type=%d%s", icp->icmp_type, VTY_NEWLINE);
			for (ix = 0; ix < 12; ix++)
				vty_out(vty,"x%2.2x: x%8.8lx%s",  (ix * sizeof(long)),
						 *lp++, VTY_NEWLINE);

			vty_out(vty,"icmp_code=%d%s", icp->icmp_code,VTY_NEWLINE);
		}

		return (ERROR);
	}

	/* check if the received reply is ours. */
	if (ntohs(icp->icmp_id) != (pPS->idRx & 0xffff))
	{
		return (ERROR); /* wasn't our ECHO */
	}

	/* print out Rx packet stats */
	if (!(pPS->flags & PING_OPT_SILENT) && pPS->numPacket != 1)
	{
		vty_out(vty,"%d bytes from %s: ", len, ipstack_inet_ntoa(from->sin_addr));
		vty_out(vty,"icmp_seq=%d. ", icp->icmp_seq);
		triptime = os_timeval_elapsed(now, *ti)/1000;
		vty_out(vty,"time=%d. ms%s", triptime, VTY_NEWLINE);
		pPS->tSum += triptime;
		pPS->tMin = min(pPS->tMin, triptime);
		pPS->tMax = max(pPS->tMax, triptime);
	}

	pPS->numRx++;
	return (OK);
}

/*******************************************************************************
 *
 * pingFinish - return all allocated resources and print out final statistics
 *
 * This routine returns all resources allocated for the ping session, and
 * prints out a stats summary.
 *
 * The ping session is located in the session list (pingHead) by searching
 * the ping stats structure for the receiver task ID.  This is necessary
 * because this routine is passed a pointer to the task control block, and does
 * not have ready access to the ping stats structure itself.  This accomodates
 * the use of task delete hooks as a means of calling this routine.
 *
 * RETURNS:
 * N/A.
 */

static void pingFinish(PING_STAT * pPS /* pointer to task control block */
)
{
	/* return all allocated/created resources */
	struct vty *vty = pPS->vty;
	if (!ipstack_invalid(pPS->pingFd))
		(void) ipstack_close(pPS->pingFd);

	if (!(pPS->flags & PING_OPT_SILENT)) /* print final report ? */
	{
		if (pPS->numRx) /* received at least one ? */
		{
			if (pPS->numPacket != 1) /* full report */
			{
				vty_out(vty,"----%s PING Statistics----%s", pPS->toInetName, VTY_NEWLINE);
				vty_out(vty,"%d packets transmitted, ", pPS->numTx);
				vty_out(vty,"%d packets received, ", pPS->numRx);

				if (pPS->numTx)
					vty_out(vty,"%d%% packet loss",
							((pPS->numTx - pPS->numRx) * 100) / pPS->numTx);
				vty_out(vty,"%s", VTY_NEWLINE);

				if (pPS->numRx)
					vty_out(vty,"round-trip (ms)  min/avg/max = %d/%d/%d%s",
							pPS->tMin, pPS->tSum / pPS->numRx, pPS->tMax, VTY_NEWLINE);
			}
			else
				/* zpl_int16 report */
				vty_out(vty,"%s is alive%s", pPS->toInetName, VTY_NEWLINE);
		}
		else
			vty_out(vty,"no answer from %s%s", pPS->toInetName, VTY_NEWLINE);
	}
}
