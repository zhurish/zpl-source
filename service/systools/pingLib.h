/* pingLib.h - Packet InterNet Grouper (PING) library header */

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
01n,26oct06,tkf  Restore removed structures (needed by pingLib.c for user side
                 build).
01m,20sep06,tlu  Restore some header file, constant and type definitions
01l,03aug06,kch  Cleanup for IPNet stack. Changed PING_TMO from 5 to 1 sec.
01k,07feb05,vvv  _KERNEL cleanup
01j,20nov03,niq  Remove copyright_wrs.h file inclusion
01j,T14ov03,asr  Changes for porting ping to RTP
01i,05nov03,cdw  Removal of unnecessary _KERNEL guards.
01h,04nov03,rlm  Ran batch header path update for header re-org.
01g,03nov03,rlm  Removed wrn/coreip/ prefix from #includes for header re-org.
01f,15may03,spm  Tornado 2.2 FCS merge (SPR #73570; ver 01d,12mar02,rae:
                 TOR2_2-FCS-COPY label, tor2 branch, /wind/river VOB)
01e,09may03,vvv  included ip_icmp.h
01d,25jul02,ant  definitions ICMP_PROTO and ICMP_TYPENUM removed 
01c,03may02,ant  ospl_uchar	bufTx [PING_MAXPACKET] in the struct PING_STAT changed
		 to ospl_uchar *bufTx. It is allocated dynamically now. Receive
		 buffer ospl_uchar bufRx[PING_MAXPACKET] changed in the same way.
		 New member int timing defined in the struct PING_STAT. 
		 PING_MAXPACKET	increased to 65536.
01b,14jan00,ham  changed PING_STAT for PD support.
01a,25oct94,dzb  written.
*/

#ifndef __INCpingLibh
#define __INCpingLibh

#ifdef __cplusplus
extern "C" {
#endif

/* includes */
#include <zebra.h>
#include <vty.h>
#include <netinet/ip_icmp.h>

/* defines */
#define PING_MINPACKET		64	/* min packet size */
#define PING_MAXPACKET		65536	/* max packet size */
#define PING_INTERVAL           1       /* default packet interval in seconds */
#define PING_TMO		1	/* default packet timeout in seconds */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 	64    
#endif

#ifndef INET_ADDR_LEN
#define	INET_ADDR_LEN	18
#endif

/* status codes */
/*

#define S_pingLib_NOT_INITIALIZED               (M_pingLib | 1)
#define S_pingLib_TIMEOUT                       (M_pingLib | 2)
*/


/* flags */

#define	PING_OPT_SILENT		0x1	/* work silently */
#define	PING_OPT_DONTROUTE	0x2	/* dont route option */
#define	PING_OPT_DEBUG		0x4	/* print debugging messages */
#define	PING_OPT_NOHOST		0x8     /* suppress host lookup    [ANT] */

/* typedefs */

typedef struct pingStat                         /* PING_STAT */
    {
    int                 pingFd;                 /* socket file descriptor */
    char                toHostName [MAXHOSTNAMELEN + 2];/* name to ping */
    char                toInetName [INET_ADDR_LEN];/* IP addr to ping */
    ifindex_t			ifindex;
    char                *bufTx;                 /* transmit buffer */
    char                *bufRx;                 /* receive buffer */
    struct icmp         *pBufIcmp;               /* ptr to icmp */
    struct timeval		*pBufTime;               /* ptr to time */
    ospl_uint16				rxmaxlen;
    ospl_uint16             dataLen;                /* size of data portion */
    int                 numPacket;              /* total # of packets to send */
    ospl_uint32             numTx;                  /* number of packets sent */
    ospl_uint32             numRx;                  /* number of packets received */
    ospl_uint32             idRx;                   /* id of Rx task */
    ospl_uint32             tMin;                   /* min RT time (ms) */
    ospl_uint32             tMax;                   /* max RT time (ms) */
    ospl_uint32             tSum;                   /* sum of all times */
    ospl_uint32             flags;                  /* option flags */
    ospl_uint32             timing;                 /* flag to do timing */
    ospl_uint16 			pingTxLen; /* size of icmp echo packet */
    ospl_uint8 				pingTxInterval; /* packet interval in seconds */
    ospl_uint8 				pingTxTmo; /* packet timeout in seconds */
    ospl_uint8              maxttl;
    struct vty			*vty;
    ospl_bool				quit;
    } PING_STAT;


/* forward declarations */
 
extern int ping(struct vty *vty, char * host, int numPackets, int len, ospl_uint32 options);


#ifdef __cplusplus
}
#endif
 
#endif /* __INCpingLibh */
