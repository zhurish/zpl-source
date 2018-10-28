/* tracerouteLib.h - Packet InterNet Grouper (TRACEROUTE) library header */

/*
 * Copyright (c) 1994-2006  Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01n,26oct06,tkf  Restore removed structures (needed by tracerouteLib.c for user side
                 build).
01m,20sep06,tlu  Restore some header file, constant and type definitions
01l,03aug06,kch  Cleanup for IPNet stack. Changed TRACEROUTE_TMO from 5 to 1 sec.
01k,07feb05,vvv  _KERNEL cleanup
01j,20nov03,niq  Remove copyright_wrs.h file inclusion
01j,T14ov03,asr  Changes for porting traceroute to RTP
01i,05nov03,cdw  Removal of unnecessary _KERNEL guards.
01h,04nov03,rlm  Ran batch header path update for header re-org.
01g,03nov03,rlm  Removed wrn/coreip/ prefix from #includes for header re-org.
01f,15may03,spm  Tornado 2.2 FCS merge (SPR #73570; ver 01d,12mar02,rae:
                 TOR2_2-FCS-COPY label, tor2 branch, /wind/river VOB)
01e,09may03,vvv  included ip_icmp.h
01d,25jul02,ant  definitions ICMP_PROTO and ICMP_TYPENUM removed 
01c,03may02,ant  u_char	bufTx [TRACEROUTE_MAXPACKET] in the struct TRACEROUTE_STAT changed
		 to u_char *bufTx. It is allocated dynamically now. Receive
		 buffer u_char bufRx[TRACEROUTE_MAXPACKET] changed in the same way.
		 New member int timing defined in the struct TRACEROUTE_STAT.
		 TRACEROUTE_MAXPACKET	increased to 65536.
01b,14jan00,ham  changed TRACEROUTE_STAT for PD support.
01a,25oct94,dzb  written.
*/

#ifndef __INCtracerouteLibh
#define __INCtracerouteLibh

#ifdef __cplusplus
extern "C" {
#endif

/* includes */
#include <zebra.h>
#include <vty.h>
#include <netinet/ip_icmp.h>

/* defines */
#define TRACEROUTE_MINPACKET		64	/* min packet size */
#define TRACEROUTE_MAXPACKET		65536	/* max packet size */
#define TRACEROUTE_INTERVAL           1       /* default packet interval in seconds */
#define TRACEROUTE_TMO		1	/* default packet timeout in seconds */
#define TRACEROUTE_MAXTTL	30


#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 	64    
#endif

#ifndef INET_ADDR_LEN
#define	INET_ADDR_LEN	18
#endif

/* status codes */
/*

#define S_tracerouteLib_NOT_INITIALIZED               (M_tracerouteLib | 1)
#define S_tracerouteLib_TIMEOUT                       (M_tracerouteLib | 2)
*/


/* flags */

#define	TRACEROUTE_OPT_SILENT		0x1	/* work silently */
#define	TRACEROUTE_OPT_DONTROUTE	0x2	/* dont route option */
#define	TRACEROUTE_OPT_DEBUG		0x4	/* print debugging messages */
#define	TRACEROUTE_OPT_NOHOST		0x8     /* suppress host lookup    [ANT] */

/* typedefs */

typedef struct tracerouteStat                         /* TRACEROUTE_STAT */
    {
    int                 tracerouteFd;                 /* socket file descriptor */
    char                toInetName [INET_ADDR_LEN];/* IP addr to traceroute */
    ifindex_t			ifindex;
    char                *bufTx;                 /* transmit buffer */
    char                *bufRx;                 /* receive buffer */
    struct icmp *       pBufIcmp;               /* ptr to icmp */
    struct timeval		*pBufTime;               /* ptr to time */
    u_int16             dataLen;                /* size of data portion */
    u_int16             rxmaxlen;
    u_int16				tracerouteTxLen;
    u_int32             idRx;                   /* id of Rx task */
    u_int32             flags;                  /* option flags */
    u_int8 				tracerouteTxTmo; /* packet timeout in seconds */
    u_int8              maxttl;
    struct vty			*vty;
    } TRACEROUTE_STAT;


/* forward declarations */
 
extern int traceroute(struct vty *vty, char * host, int maxttl, int len, u_int32 options);


#ifdef __cplusplus
}
#endif
 
#endif /* __INCtracerouteLibh */
