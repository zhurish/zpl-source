/* sntpsLib.c - Simple Network Time Protocol (SNTP) server library */

/* Copyright 1984 - 2000 Wind River Systems, Inc. */
//#include "copyright_wrs.h"

/*
modification history 
--------------------
01l,07may02,kbw  man page edits
01k,25oct00,ham  doc: cleanup for vxWorks AE 1.0.
01j,16mar99,spm  removed references to configAll.h (SPR #25663)
01i,16mar99,spm  recovered orphaned code from tor2_0_x branch (SPR #25770)
01h,01dec98,spm  corrected man page references for clock hook (SPR #22860)
01g,14dec97,jdi  doc: cleanup.
01f,10dec97,kbw  made minor man page changes
01e,04dec97,spm  added minor changes to man pages; changed parameter names to 
                 comply with coding standards
01d,02sep97,spm  corrected return value and typecast for sntpsConfigSet routine
01c,27aug97,spm  corrected header file name in library description
01b,15jul97,spm  code cleanup, documentation, and integration; entered in
                 source code control
01a,24may97,kyc  written
*/

/* 
DESCRIPTION
This library implements the server side of the Simple Network Time Protocol
(SNTP), a protocol that allows a system to maintain the accuracy of its 
internal clock based on time values reported by one or more remote sources. 
The library is included in the VxWorks image if INCLUDE_SNTPS is defined 
at the time the image is built.

USER INTERFACE
The routine sntpsInit() is called automatically during system startup when 
the SNTP server library is included in the VxWorks image. Depending on the 
value of SNTPS_MODE, the server executes in either a passive or an active 
mode.  When SNTPS_MODE is set to SNTP_PASSIVE (0x2), the server waits for
requests from clients, and sends replies containing an NTP timestamp. When
the mode is set to SNTP_ACTIVE (0x1), the server transmits NTP timestamp
information at fixed intervals. 

When executing in active mode, the SNTP server uses the SNTPS_DSTADDR and 
SNTPS_INTERVAL definitions to determine the target IP address and broadcast
interval.  By default, the server will transmit the timestamp information to
the local subnet broadcast address every 64 seconds.  These settings can be
changed with a call to the sntpsConfigSet() routine.  The SNTP server operating
in active mode will still respond to client requests.

The SNTP_PORT definition in assigns the source and destination UDP port.  The 
default port setting is 123 as specified by the relevant RFC.  Finally, the
SNTP server requires access to a reliable external time source.  The
SNTPS_TIME_HOOK constant specifies the name of a routine with the following
interface:

.CS
    STATUS sntpsTimeHook (int request, void *pBuffer);
.CE

This routine can be assigned directly by altering the value of SNTPS_TIME_HOOK
or can be installed by a call to the sntpsClockSet() routine. The manual pages
for sntpsClockSet() describe the parameters and required operation of the
timestamp retrieval routine.  Until this routine is specified, the SNTP server
will not provide timestamp information.

VXWORKS AE PROTECTION DOMAINS
Under VxWorks AE, the SNPT server can run in the kernel protection domain only. 
The SNTPS_TIME_HOOK MUST, if used, must reference a function in the kernel 
protection domain.  This restriction does not apply under non-AE versions of 
VxWorks.  

INCLUDE FILES: sntpsLib.h

SEE ALSO: sntpcLib, RFC 1769
*/

/* includes */

/*
#include "vxWorks.h"
#include "sysLib.h"
#include "inetLib.h"
#include "sockLib.h"
#include "netLib.h"
#include "ioLib.h"
#include "wdLib.h"
#include "usrLib.h"
#include "errnoLib.h"
*/
#include "zebra.h"
#include "log.h"

#include "command.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"

#include "vty.h"

#include "sntpsLib.h"


/* defines */

#define NSEC_BASE2 	30 	/* Integral log2 for nanosecond conversion. */

/* forward declarations */

static struct sntp_server *sntp_server = NULL;
static BOOL sntpsClockHookRtn(int , void *);
static STATUS sntpsClockSet (struct sntp_server *, BOOL (*)(int, void *));
/*******************************************************************************
*
* sntpsInit - set up the SNTP server
*
* This routine is called from usrNetwork.c to link the SNTP server module into
* the VxWorks image.  It creates all necessary internal data structures and
* initializes the SNTP server according to the assigned settings.
*
* RETURNS: OK or ERROR.
*
* ERRNO:
*  S_sntpsLib_INVALID_PARAMETER
*
* NOMANUAL
*/

static STATUS sntps_init
    (
    void *m
    )
{

	if(sntp_server == NULL)
		sntp_server = (struct sntp_server *)malloc(sizeof(struct sntp_server));
	os_memset(sntp_server, 0, sizeof(struct sntp_server));
	sntp_server->master = m;
	sntp_server->mutex = os_mutex_init();
//	sntp_server->version =
	return (OK);
}

/*******************************************************************************
*
* sntpsLog2Get - find approximate power of two
*
* This routine determines the nearest integral power of two for the given 
* value, without using floating point arithmetic.  It is used to convert 
* 32-bit nanosecond values into signed integers for assignment to the poll 
* and precision fields of NTP messages.
*
* RETURNS: Nearest integral log base two value, in host byte order.
*
* ERRNO: N/A
*
* INTERNAL
* Floating-point calculations can't be used because some boards (notably
* the SPARC architectures) disable software floating point by default to
* speed up context switching. These boards abort with an exception when
* floating point operations are encountered.
*
* NOMANUAL
*/

static INT8 sntpsLog2Get
    (
    ULONG inval 	/* input value for calculation */
    )
{
	u_int32 loop;
	u_int32 floor; /* Nearest power of two for smaller value */
	u_int32 limit; /* Nearest power of two for larger value */
	int result;
	ULONG mask; /* Bitmask for log2 calculation */

	if (inval == 0)
		result = 0;
	else
	{
		/*
		 * Set increasing numbers of the low-order bits of the input value
		 * to zero until all bits have been cleared. The current and previous
		 * values of the loop counter indicate the adjacent integral powers
		 * of two.
		 */
		for (loop = 0; loop < 32; loop++)
		{
			mask = ~0 << loop; /* Mask out the rightmost "loop" bits. */
			if ((inval & mask) == 0)
				break;
		}
		floor = 1 << (loop - 1);
		limit = 1 << loop;
		if (inval - floor < limit - inval)
			result = loop - 1;
		else
			result = loop;
	}
	return (result);
}

/*******************************************************************************
*
* sntpsNsecToFraction - convert portions of a second to NTP format
*
* This routine is provided for convenience in fulfilling an SNTPS_TIME request
* to the clock hook.  It converts a value in nanoseconds to the fractional part 
* of the NTP timestamp format.  The routine is not designed to convert 
* non-normalized values greater than or equal to one second.  Although the NTP 
* time format provides a precision of about 200 pico-seconds, rounding errors 
* in the conversion process decrease the accuracy as the input value increases.
* In the worst case, only the 24 most significant bits are valid, which reduces
* the precision to tenths of a micro-second.
*
* RETURNS: Value for NTP fractional part in host-byte order.
*
* ERRNO: N/A
*
* INTERNAL
* Floating-point calculations can't be used because some boards (notably
* the SPARC architectures) disable software floating point by default to
* speed up context switching. These boards abort with an exception when
* floating point operations are encountered.
*/

static ULONG sntpsNsecToFraction
    (
    ULONG nsecs 	/* nanoseconds to convert to binary fraction */
    )
{
	ULONG factor = 294967296; /* Partial conversion factor from base 10 */
	ULONG divisor = 10; /* Initial exponent for mantissa. */
	ULONG mask = 100000000; /* Pulls digits of factor from left to right. */
	int loop;
	ULONG fraction = 0;
	BOOL shift = FALSE; /* Shifted to avoid overflow? */

	/*
	 * Adjust large values so that no intermediate calculation exceeds
	 * 32 bits. (This test is overkill, since the fourth MSB can be set
	 * sometimes, but it's fast).
	 */

	if (nsecs & 0xF0000000) {
		nsecs >>= 4; /* Exclude rightmost hex digit. */
		shift = TRUE;
	}

	/*
	 * In order to increase portability, the following conversion avoids
	 * floating point operations, so it is somewhat obscure.
	 *
	 * A one nanosecond increase corresponds to increasing the NTP fractional
	 * part by (2^32)/1E9. Multiplying the number of nanoseconds by that value
	 * (4.294967286) produces the NTP fractional part.
	 *
	 * The above constant is separated into integer and decimal parts to avoid
	 * overflow. The mask variable selects each digit from the decimal part
	 * sequentially, and the divisor shifts the digit the appropriate number
	 * of decimal places.
	 */

	fraction += nsecs * 4; /* Handle integer part of conversion */

	for (loop = 0; loop < 9; loop++) /* Nine digits in mantissa */
	{
		fraction += nsecs * (factor / mask) / divisor;
		factor %= mask; /* Remove most significant digit from the factor. */
		mask /= 10; /* Reduce length of mask by one. */
		divisor *= 10; /* Increase shift by one decimal place. */
	}

	/* Scale result upwards if value was adjusted before processing. */

	if (shift)
		fraction <<= 4;

	return (fraction);
}

/*******************************************************************************
*
* sntpsConfigSet - change SNTP server broadcast settings
*
* This routine alters the configuration of the SNTP server when operating
* in broadcast mode.  A <setting> value of SNTPS_DELAY interprets the contents
* of <pValue> as the new 16-bit broadcast interval.  When <setting> equals
* SNTPS_ADDRESS, <pValue> should provide the string representation of an
* IP broadcast or multicast address (for example, "224.0.1.1").  Any changed 
* settings will take effect after the current broadcast interval is 
* completed and the corresponding NTP message is sent.
*
* RETURNS: OK or ERROR.
*
* ERRNO:
*  S_sntpsLib_INVALID_PARAMETER
*/
#if 0
static STATUS sntpsConfigSet
    (
    int 	setting,     /* configuration option to change */
    void * 	pValue       /* new value for parameter */
    )
{
	struct in_addr target;
	short interval;
	int result = OK;

	/* Don't change settings if message transmission in progress. */

//    semTake (sntpsMutexSem, WAIT_FOREVER);

	if (setting == SNTPS_ADDRESS) {
		target.s_addr = inet_addr((char *) pValue);
		if (target.s_addr == ERROR) {
//            errnoSet (S_sntpsLib_INVALID_PARAMETER);
			result = ERROR;
		} else
			sntp_server->address.s_addr = target.s_addr;
	} else if (setting == SNTPS_DELAY) {
		interval = (short) (*((int *) pValue));
		sntp_server->sntpsInterval = interval;
	} else if (setting == SNTPS_MODE) {
		interval = (short) (*((int *) pValue));
		sntp_server->mode = interval;
	} else {
//        errnoSet (S_sntpsLib_INVALID_PARAMETER);
		result = ERROR;
	}

//    semGive (sntpsMutexSem);

	return (result);
}
#endif
/*******************************************************************************
*
* sntpsMsgSend - transmit an unsolicited NTP message
*
* This routine sends an NTP message to the assigned destination address when 
* the broadcast interval has elapsed.  It is called by watchdog timers set
* during initialization, and should not be used directly.
*
* RETURNS: N/A
*
* ERRNO: N/A
*
* NOMANUAL
*/

static int sntpsMsgSend (struct sntp_server *server)
{
	SNTP_PACKET sntpReply;
	int result;
	//int optval;
	//short interval;
	SNTP_TIMESTAMP refTime;
//	int sntpSocket;
	struct sockaddr_in dstAddr;

	/* Retrieve the current clock ID, precision, and NTP timestamp. */
	//zlog_debug("%s:1111111111111111111111111",__func__);
//	sntpReply.precision = server->sntpsPrecision;
	sntp_global_precision_get(&sntpReply.precision);
	sntpReply.referenceIdentifier = server->sntpsClockId;

	if(server->sntpsClockHookRtn)
	{
		result = (server->sntpsClockHookRtn) (SNTPS_TIME, &refTime);
		if (result == ERROR) {
			/*        semGive (sntpsMutexSem);
			 wdStart (sntpsTimer, sntpsInterval * sysClkRateGet (),
			 (FUNCPTR)netJobAdd, (int)sntpsMsgSend);*/
			zlog_debug(ZLOG_SNTP, "%s:sntpsClockHookRtn",__func__);
			return ERROR;
		}
	}

	/* Assign target address for outgoing message. */

	bzero((char *) &dstAddr, sizeof(dstAddr));
	dstAddr.sin_addr.s_addr = server->address.s_addr;
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_port = htons(server->sntpsPort);

//    semGive (sntpsMutexSem);

	/* Enable broadcast option for socket. */
#if 0
	optval = 1;
	result = ip_setsockopt(server->sock, SOL_SOCKET, SO_BROADCAST, (char *) &optval,
			sizeof(optval));
	if (result == ERROR) {
		close(sntpSocket);
		/*        wdStart (sntpsTimer, interval * sysClkRateGet (),
		 (FUNCPTR)netJobAdd, (int)sntpsMsgSend);*/
		return ERROR;
	}
#endif
	if(server->mode == SNTPS_MULTICAST)
	{
		ip_setsockopt(server->sock, IPPROTO_IP, IP_MULTICAST_TTL,
				(void *)&server->sntps_ttl, sizeof(server->sntps_ttl));
	}
	/*
	 * Set the common values for outgoing NTP messages - root delay
	 * and root dispersion are 0.
	 */

	bzero((char *) &sntpReply, sizeof(sntpReply));
//	sntpReply.stratum = server->stratum;//SNTP_STRATUM_1;
	sntp_global_stratum_get(&sntpReply.stratum);
	/* Set the leap indicator, version number and mode. */

	sntpReply.leapVerMode |= (server->leap<<6);//SNTP_LI_0;
	//sntpReply.leapVerMode |= SNTP_VN_3;
	sntpReply.leapVerMode |= (server->version<<3);
	sntpReply.leapVerMode |= server->Mode;//SNTP_MODE_5;

	/* Set the poll field: find the nearest integral power of two. */

	sntpReply.poll = sntpsLog2Get(server->poll);

	/* Set the timestamp fields and send the message. */

	sntpReply.referenceTimestampSec = htonl(refTime.seconds);
	sntpReply.referenceTimestampFrac = htonl(refTime.fraction);

	sntpReply.receiveTimestampSec = sntpReply.referenceTimestampSec;
	sntpReply.receiveTimestampFrac = sntpReply.referenceTimestampFrac;

	sntpReply.transmitTimestampSec = sntpReply.referenceTimestampSec;
	sntpReply.transmitTimestampFrac = sntpReply.referenceTimestampFrac;

	sntpReply.originateTimestampSec = sntpReply.referenceTimestampSec;
	sntpReply.originateTimestampFrac = sntpReply.referenceTimestampFrac;

	result = ip_sendto(server->sock, (caddr_t) & sntpReply, sizeof(sntpReply), 0,
			(struct sockaddr *) &dstAddr, sizeof(dstAddr));


	zlog_debug(ZLOG_SNTP, "%s:send %d byte to %s:%d",__func__, result, inet_ntoa(dstAddr.sin_addr),server->sntpsPort);
	/* Schedule a new transmission after the broadcast interval. */

	/*    wdStart (sntpsTimer, interval * sysClkRateGet (),
	 (FUNCPTR)netJobAdd, (int)sntpsMsgSend);*/
	return result;
}

static int sntps_write(struct thread *thread)
{
	struct sntp_server *server = THREAD_ARG (thread);
	//sock = THREAD_FD (thread);
	return sntpsMsgSend (server);
}

static int sntps_time(struct thread *thread)
{
	struct sntp_server *server = THREAD_ARG (thread);

	if (!server->sntpsClockReady /*|| sntpsClockHookRtn == NULL*/) {
		sntpsClockSet(server, sntpsClockHookRtn);
		server->t_time = thread_add_timer (server->master, sntps_time, server, server->sntpsInterval);
		return 0;
	}
	zlog_debug(ZLOG_SNTP, "%s:saaaaaaaaaaaa",__func__);
	server->t_write = thread_add_write (server->master, sntps_write, server, server->sock);
	server->t_time = thread_add_timer (server->master, sntps_time, server, server->sntpsInterval);
	return 0;
}


/*******************************************************************************
*
* sntpsClockSet - assign a routine to access the reference clock
*
* This routine installs a hook routine that is called to access the
* reference clock used by the SNTP server. This hook routine must use the
* following interface:
* .CS
*     STATUS sntpsClockHook (int request, void *pBuffer);
* .CE
* The hook routine should copy one of three settings used by the server to
* construct outgoing NTP messages into <pBuffer> according to the value of
* the <request> parameter.  If the requested setting is available, the
* installed routine should return OK (or ERROR otherwise).
*
* This routine calls the given hook routine with the <request> parameter
* set to SNTPS_ID to get the 32-bit reference identifier in the format
* specified in RFC 1769.  It also calls the hook routine with <request>
* set to SNTPS_RESOLUTION to retrieve a 32-bit value containing the clock
* resolution in nanoseconds.  That value will be used to determine the 8-bit
* signed integer indicating the clock precision (according to the format
* specified in RFC 1769).  Other library routines will set the <request>
* parameter to SNTPS_TIME to retrieve the current 64-bit NTP timestamp
* from <pBuffer> in host byte order.  The routine sntpsNsecToFraction() will
* convert a value in nanoseconds to the format required for the NTP
* fractional part.
*
* VXWORKS AE PROTECTION DOMAINS
* Under VxWorks AE, you can call this function from within the kernel
* protection domain only.  In addition, all arguments to this function can
* reference only that data which is valid in the kernel protection domain.
* This restriction does not apply under non-AE versions of VxWorks.
*
* RETURNS: OK or ERROR.
*
* ERRNO: N/A
*/
static STATUS sntpsClockSet
    (
    struct sntp_server *server,
	BOOL (*pClockHookRtn)(int, void *) 	 	/* new interface to reference clock */
    )
{
	STATUS result;
	INT8 basetwo;
//	FUNCPTR	sntpsClockHookRtn;
	if (pClockHookRtn == NULL)
		return (ERROR);

	/* Don't change clock setting if current routine is in use. */

//    semTake (sntpsMutexSem, WAIT_FOREVER);
	server->sntpsClockHookRtn = pClockHookRtn;
	/* Get clock precision and clock identifier. */

    result = (* server->sntpsClockHookRtn) (SNTPS_ID, &server->sntpsClockId);
	if (result == ERROR) /* Clock ID not available. Use default value. */
		server->sntpsClockId = server->sntpsRefId;

    result = (* server->sntpsClockHookRtn) (SNTPS_RESOLUTION, &server->sntpsResolution);
	if (result == ERROR)
		server->sntpsPrecision = 0;
	else {
		/* Find nearest power of two to clock resolution. */

		basetwo = sntpsLog2Get(server->sntpsResolution);

		/*
		 * Convert to seconds required for NTP message. Subtract nearest
		 * integer to log base two of 1E9 (corresponds to division of clock
		 * resolution by 1E9).
		 */

		server->sntpsPrecision = basetwo - NSEC_BASE2;
	}

	if (!server->sntpsClockReady)
		server->sntpsClockReady = TRUE; /* Enable transmission of messages. */

//    semGive (sntpsMutexSem);

	return (OK);
}
/*******************************************************************************
*
* sntpsStart - execute the SNTP server
*
* This routine monitors the specified SNTP/NTP port for incoming requests from
* clients and transmits replies containing the NTP timestamp obtained from
* the hook provided by the user. If the server executes in broadcast mode,
* this routine also schedules the transmission of NTP messages at the assigned
* broadcast interval. It is the entry point for the SNTP server task and should
* only be called internally.
*
* RETURNS: N/A
*
* ERRNO: N/A
*
* NOMANUAL
*/
LOCAL int sntpsRead (struct sntp_server *server)
{
	SNTP_PACKET sntpRequest; /* SNTP request received from client */
	SNTP_PACKET sntpReply; /* buffer for server reply */

//	struct sockaddr_in srcAddr; /* address of requesting SNTP/NTP client */
	struct sockaddr_in dstAddr; /* target address of transmission */
//	int sntpSocket;
	int result;
	int addrLen;
	SNTP_TIMESTAMP refTime;
	BOOL unsync;

	addrLen = sizeof(dstAddr);

	/* Set address information. */
#if 0
	bzero((char *) &srcAddr, sizeof(srcAddr));
	bzero((char *) &dstAddr, sizeof(dstAddr));
	srcAddr.sin_addr.s_addr = INADDR_ANY;
	srcAddr.sin_family = AF_INET;
	srcAddr.sin_port = server->sntpsPort;

	/* Create UDP socket and bind to the SNTP port. */

	server->sock = ip_socket(AF_INET, SOCK_DGRAM, 0);
	if (server->sock == -1)
		return;

	result = ip_bind(server->sock, (struct sockaddr *) &srcAddr, sizeof(srcAddr));
	if (result == -1) {
		ip_close(server->sock);
		return;
	}
#endif
	/*
	 * The use of sntpsInterval below doesn't need to be guarded from a call to
	 * sntpsConfigSet() because this routine is called during system startup.
	 */

	/*    if (sntpsMode == SNTP_ACTIVE)
	 wdStart (sntpsTimer, sntpsInterval * sysClkRateGet (),
	 (FUNCPTR)netJobAdd, (int)sntpsMsgSend);*/
	//for (;;) {
		result = ip_recvfrom(server->sock, (caddr_t) & sntpRequest,
				sizeof(sntpRequest), 0, (struct sockaddr *) &dstAddr, &addrLen);
		if (result == -1)
			return -1;
			//continue;

//        semTake (sntpsMutexSem, WAIT_FOREVER);   /* Lock out clock changes. */

		/* Can't transmit messages if no access to clock is provided. */

		if (!server->sntpsClockReady /*|| sntpsClockHookRtn == NULL*/) {
//            semGive (sntpsMutexSem);
			//continue;
			sntpsClockSet(server, sntpsClockHookRtn);
			return -1;
		}

		/* All timestamp fields are zero by default. */

		bzero((char *) &sntpReply, sizeof(sntpReply));

		/* Retrieve the current clock ID, precision, and NTP timestamp. */

		sntpReply.precision = server->sntpsPrecision;
		sntpReply.referenceIdentifier = server->sntpsClockId;

		unsync = FALSE;
		/* confirm if local time is sync */
        result = (* server->sntpsClockHookRtn) (SNTPS_TIME, &refTime);
		if (result == ERROR)
			unsync = TRUE;

//        semGive (sntpsMutexSem);

		/* Set the leap indicator and version number. */

		sntpReply.leapVerMode = 0;

		if (unsync) {
			/* local time is not sync */
			sntpReply.stratum = SNTP_STRATUM_0;
			sntpReply.leapVerMode |= SNTP_LI_3;
		} else {
//			sntpReply.stratum = SNTP_STRATUM_1;
			sntp_global_stratum_get(&sntpReply.stratum);
			sntpReply.leapVerMode |= SNTP_LI_0;
		}

		sntpReply.leapVerMode |= (sntpRequest.leapVerMode & SNTP_VN_MASK);

		/* Set mode to server for client response, or to symmetric passive. */

		if ((sntpRequest.leapVerMode & SNTP_MODE_MASK) == SNTP_MODE_3)
			sntpReply.leapVerMode |= SNTP_MODE_4;
		else
			sntpReply.leapVerMode |= SNTP_MODE_2;

		/* Copy the poll field from the request. */

		sntpReply.poll = sntpRequest.poll;

		/*
		 * Leave the root delay and root dispersion fields at zero.
		 * Set the timestamp fields and send the message.
		 */

		if (!unsync) {
			sntpReply.referenceTimestampSec = htonl(refTime.seconds);
			sntpReply.referenceTimestampFrac = htonl(refTime.fraction);

			sntpReply.receiveTimestampSec = sntpReply.referenceTimestampSec;
			sntpReply.receiveTimestampFrac = sntpReply.referenceTimestampFrac;

			sntpReply.transmitTimestampSec = sntpReply.referenceTimestampSec;
			sntpReply.transmitTimestampFrac = sntpReply.referenceTimestampFrac;

			/* The originate timestamp contains the request transmit time. */

			sntpReply.originateTimestampSec = sntpRequest.transmitTimestampSec;
			sntpReply.originateTimestampFrac =
					sntpRequest.transmitTimestampFrac;
		}

		result = ip_sendto(server->sock, (caddr_t) & sntpReply, sizeof(sntpReply), 0,
				(struct sockaddr *) &dstAddr, sizeof(dstAddr));
	//}
	/* Not reached. */
	//close(server->sock);
	return result;
}

static BOOL sntpsClockHookRtn(int type, void *pVoid)
{
	extern int sntpc_is_sync(void);
	BOOL ret = FALSE;
	SNTP_TIMESTAMP *tp = (SNTP_TIMESTAMP *)pVoid;
	ULONG *value = (ULONG *)pVoid;
	struct timespec sntpTime;
	//	if(sntpc_is_sync() != ERROR)

	switch(type)
	{
	case SNTPS_TIME:
		if(clock_gettime(CLOCK_REALTIME, &sntpTime))
		{
			tp->seconds = sntpTime.tv_sec;
			tp->fraction = sntpsNsecToFraction(sntpTime.tv_nsec);
			ret = TRUE;
		}
		break;
	case SNTPS_ID:
		if(value)
			*value= 1;
		ret = TRUE;
		break;
	case SNTPS_RESOLUTION:
		if(value)
			*value= rand();
		ret = TRUE;
		break;
	}
	//extern struct in_addr sntpc_server_address(void);
	return ret;
}

static int sntps_read(struct thread *thread)
{
	struct sntp_server *server = THREAD_ARG (thread);
	sntpsRead (server);
	server->t_read = thread_add_read (server->master, sntps_read, server, server->sock);
	//server->t_time = thread_add_time (server->master, sntps_write, server, server->sntpsInterval);
	return 0;
}


static int sntps_socket_init(struct sntp_server *server)
{
		struct sockaddr_in srcAddr; /* address of requesting SNTP/NTP client */
	//	int sntpSocket;
		int result;
//		int addrLen;
//		addrLen = sizeof(srcAddr);
		/* Set address information. */
		bzero((char *) &srcAddr, sizeof(srcAddr));
		srcAddr.sin_addr.s_addr = INADDR_ANY;
		srcAddr.sin_family = AF_INET;
		srcAddr.sin_port = server->sntpsPort;
		/* Create UDP socket and bind to the SNTP port. */
		server->sock = ip_socket(AF_INET, SOCK_DGRAM, 0);
		if (server->sock == -1)
			return ERROR;

		if(server->mode == SNTPS_UNICAST)
		{
			result = ip_bind(server->sock, (struct sockaddr *) &srcAddr, sizeof(srcAddr));
			if (result == -1) {
				ip_close(server->sock);
				return ERROR;
			}
		}
		else if(sntp_server->mode == SNTPS_BROADCAST)
		{
			int optval = 1;
			result = ip_setsockopt(server->sock, SOL_SOCKET, SO_BROADCAST, (char *) &optval,
					sizeof(optval));
			if (result == ERROR) {
				ip_close(server->sock);
				/*        wdStart (sntpsTimer, interval * sysClkRateGet (),
				 (FUNCPTR)netJobAdd, (int)sntpsMsgSend);*/
				return ERROR;
			}
		}
		else if(sntp_server->mode == SNTPS_MULTICAST)
		{
			/*
			 //��ʼ��socket,����Ϊ�ಥ���IP��ַ�Ͷ˿ں�
			    send_socket = ip_socket(PF_INET,SOCK_DGRAM,0);
			    memset(&multicast_addr,0,sizeof(multicast_addr));
			    multicast_addr.sin_family = AF_INET;
			    //��֮ǰ��UDP���÷�ʽһ����ֻ��������Ƕಥ���IP��ַ�Ͷ˿ں�
			    multicast_addr.sin_addr.s_addr = inet_addr(argv[1]);
			    multicast_addr.sin_port = htons(atoi(argv[2]));

			    //Ϊ�ಥ���ݱ������������ʱ��
			    ip_setsockopt(send_socket,IPPROTO_IP,IP_MULTICAST_TTL,(void *)&live_time,sizeof(live_time));
		*/
		}
		/*
		 * The use of sntpsInterval below doesn't need to be guarded from a call to
		 * sntpsConfigSet() because this routine is called during system startup.
		 */

		/*    if (sntpsMode == SNTP_ACTIVE)
		 wdStart (sntpsTimer, sntpsInterval * sysClkRateGet (),
		 (FUNCPTR)netJobAdd, (int)sntpsMsgSend);*/
		//server->t_time = thread_add_time (server->master, sntps_write, server, server->sntpsInterval);
		return OK;
}

static int sntpsEnable(u_short port, int time_interval, int version, int mode)
{
	int ret = 0;
	if(sntp_server)
	{
		if(port)
			sntp_server->sntpsPort =  (port);
		else
			sntp_server->sntpsPort =  (SNTPC_SERVER_PORT);
		sntp_server->enable = 1;

		if(time_interval)
			sntp_server->sntpsInterval = time_interval;
		else
			sntp_server->sntpsInterval = LOCAL_UPDATE_GMT;

		if(mode)
			sntp_server->mode = mode;
		else
			sntp_server->mode = SNTPS_UNICAST;

		if(version)
			sntp_server->version = version;
		else
			sntp_server->version = (SNTP_VN_3>>3);

		ret = sntps_socket_init(sntp_server);
		if(ret == OK)
		{
			if(sntp_server->mode != SNTPS_UNICAST)
			{
				sntp_server->t_time = thread_add_timer (sntp_server->master, sntps_time,
						sntp_server, sntp_server->sntpsInterval);
				//sntp_server->t_write = thread_add_write (sntp_server->master, sntp_write, sntp_server, sntp_server->sock);
			}
			else
			{
				sntp_server->t_read = thread_add_read (sntp_server->master, sntps_read,
					sntp_server, sntp_server->sock);
			}
			sntpsClockSet(sntp_server, sntpsClockHookRtn);
			return (OK);
		}
	}
    return (ERROR);
}

static int sntpsDisable(void)
{
	if(sntp_server)
	{
		sntp_server->enable = 0;
		if(sntp_server->t_time)
			thread_cancel(sntp_server->t_time);
		if(sntp_server->t_read)
			thread_cancel(sntp_server->t_read);
		if(sntp_server->t_write)
			thread_cancel(sntp_server->t_write);
		if(sntp_server->sock)
			ip_close (sntp_server->sock);
		//os_memset(sntp_server, 0, sizeof(struct sntp_client));
		return (OK);
	}
    return (ERROR);
}

static int sntpsIsEnable(void)
{
	if(sntp_server == NULL)
		return ERROR;
	return sntp_server->enable;
}

#ifdef SNTPS_CLI_ENABLE
DEFUN (sntps_server_enable,
		sntps_server_enable_cmd,
	    "sntp server enable",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"enable sntp server\n")
#else
static int sntps_server_enable_cmd(void)
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	if(sntpsIsEnable())
		return CMD_SUCCESS;
	sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
			sntp_server->version, sntp_server->mode);
	sntp_server->enable = 1;
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (no_sntps_server_enable,
		no_sntps_server_enable_cmd,
	    "no sntp server enable",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"enable sntp server\n")
#else
static int no_sntps_server_enable_cmd(void)
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	if(sntpsIsEnable())
	{
		sntpsDisable();
	}
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (sntps_server_mode,
		sntps_server_mode_cmd,
	    "sntp server mode (broadcast|unicast|multicast)",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server mode configure\n"
		"sntp server broadcast mode\n"
		"sntp server unicast mode\n"
		"sntp server multicast mode\n")
#else
static int sntps_server_mode_cmd(char *argv[])
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	if(os_memcmp(argv[0], "broadcast", 1) == 0)
	{
		sntp_server->address.s_addr = inet_addr("192.168.198.1");
		sntp_server->mode = SNTPS_BROADCAST;
	}
	else if(os_memcmp(argv[0], "unicast", 1) == 0)
	{
		sntp_server->mode = SNTPS_UNICAST;
	}
	else if(os_memcmp(argv[0], "multicast", 1) == 0)
	{
		sntp_server->address.s_addr = inet_addr(SNTP_MUTILCAST_ADDRESS);
		sntp_server->mode = SNTPS_MULTICAST;
	}
	if(sntpsIsEnable())
	{
		sntpsDisable();
		sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
				sntp_server->version, sntp_server->mode);
	}
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (no_sntps_server_mode,
		no_sntps_server_mode_cmd,
	    "no sntp server mode",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"enable sntp server\n")
#else
static int no_sntps_server_mode_cmd()
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	sntp_server->mode = SNTPS_UNICAST;
	if(sntpsIsEnable())
	{
		sntpsDisable();
		sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
				sntp_server->version, sntp_server->mode);
	}
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (sntps_server_interval,
		sntps_server_interval_cmd,
	    "sntp server interval <60-3600>",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server poll-interval configure\n"
		"poll-interval value in sec\n")
#else
static int sntps_server_interval_cmd(struct vty *vty, char *argv[])
#endif
{
	int time_interval = LOCAL_UPDATE_GMT;
	if(sntp_server == NULL)
		return CMD_WARNING;
	time_interval = atoi(argv[0]);
	if( (time_interval > LOCAL_UPDATE_GMT_MAX)||(time_interval < LOCAL_UPDATE_GMT_MIN) )
	{

		vty_out(vty,"%% Interval port, may be %d-%d %s",LOCAL_UPDATE_GMT_MIN,
				LOCAL_UPDATE_GMT_MAX, VTY_NEWLINE);
		return CMD_WARNING;
	}
	sntp_server->sntpsInterval = time_interval;
	if(sntpsIsEnable())
	{
		sntpsDisable();
		sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
				sntp_server->version, sntp_server->mode);
	}
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (no_sntps_server_interval,
		no_sntps_server_interval_cmd,
	    "no sntp server interval",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"poll-interval\n")
#else
static int no_sntps_server_interval_cmd()
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	sntp_server->sntpsInterval = LOCAL_UPDATE_GMT;
	if(sntpsIsEnable())
	{
		sntpsDisable();
		sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
				sntp_server->version, sntp_server->mode);
	}
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (sntps_server_listen_port,
		sntps_server_listen_port_cmd,
	    "sntp server listen-port <100-65536>",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server listen port configure\n"
		"sntp server local UDP port\n")
#else
static int sntps_server_listen_port_cmd(struct vty *vty, char *argv[])
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	int port = SNTPC_SERVER_PORT;
	if( (port > SNTPC_SERVER_PORT_MAX)||(port < SNTPC_SERVER_PORT_MIN) )
	{

		vty_out(vty,"Interval port, may be %d-%d %s",SNTPC_SERVER_PORT_MIN,
				SNTPC_SERVER_PORT_MAX, VTY_NEWLINE);
		return CMD_WARNING;
	}
	sntp_server->sntpsPort = port;
	if(sntpsIsEnable())
	{
		sntpsDisable();
		sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
				sntp_server->version, sntp_server->mode);
	}
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (no_sntps_server_listen_port,
		no_sntps_server_listen_port_cmd,
	    "no sntp server listen-port",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server listen port configure\n")
#else
static int no_sntps_server_listen_port_cmd()
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	sntp_server->sntpsPort = SNTPC_SERVER_PORT;
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (sntps_version,
		sntps_version_cmd,
	    "sntp server version (1|2|3|4)",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp protocol version information\n"
		"sntp protocol version 1\n"
		"sntp protocol version 2\n"
		"sntp protocol version 3\n"
		"sntp protocol version 4\n")
#else
static int sntps_version_cmd(struct vty *vty, char *argv[])
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	int ver = atoi(argv[0]);
	if(ver > SNTP_VN_0 && ver < SNTP_VN_7)
	{
		if(ver == sntp_server->version)
			return CMD_SUCCESS;
		sntp_server->version = ver;
	}
	if(sntpsIsEnable())
	{
		sntpsDisable();
		sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
				sntp_server->version, sntp_server->mode);
	}
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (no_sntps_version,
		no_sntps_version_cmd,
	    "no sntp server version",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp protocol version information\n")
#else
static int no_sntps_version_cmd()
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	sntp_server->version = (SNTP_VN_3>>3);
	if(sntpsIsEnable())
	{
		sntpsDisable();
		sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
				sntp_server->version, sntp_server->mode);
	}
	return CMD_SUCCESS;
}

#ifdef SNTPS_CLI_ENABLE
DEFUN (sntps_debug,
		sntps_debug_cmd,
	    "debug sntp server",
		DEBUG_STR
		"sntp protocol configure\n"
		"SNTP Server timespec infomation\n")
#else
static int sntps_debug_cmd()
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	sntp_server->time_debug = 1;
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (no_sntps_debug,
		no_sntps_debug_cmd,
	    "no debug sntp server",
		NO_STR
		DEBUG_STR
		"sntp protocol configure\n"
		"SNTP Server timespec infomation\n")
#else
static int no_sntps_debug_cmd()
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	sntp_server->time_debug = 0;
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (sntps_mutilcast_ttl,
		sntps_mutilcast_ttl_cmd,
	    "sntp server mutilcast ttl <2-255>",
		DEBUG_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp mutilcast information\n"
		"mutilcast TTL configure\n"
		"mutilcast TTL value\n")
#else
static int sntps_mutilcast_ttl_cmd(struct vty *vty, char *argv[])
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	sntp_server->sntps_ttl = atoi(argv[0]);
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (no_sntps_mutilcast_ttl,
		no_sntps_mutilcast_ttl_cmd,
	    "sntp server mutilcast ttl",
		DEBUG_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp mutilcast information\n"
		"mutilcast TTL configure\n")
#else
static int no_sntps_mutilcast_ttl_cmd()
#endif
{
	if(sntp_server == NULL)
		return CMD_WARNING;
	sntp_server->sntps_ttl = 16;
	return CMD_SUCCESS;
}
#ifdef SNTPS_CLI_ENABLE
DEFUN (show_sntps_server,
		show_sntps_server_cmd,
	    "show sntp server",
		SHOW_STR
		"sntp protocol configure\n"
		"sntp server configure\n")
#else
int vty_show_sntps_server(struct vty *vty)
#endif
{
	char *mode_str[] = {"broadcast","unicast","multicast"};
	if(sntp_server == NULL)
		return CMD_WARNING;
	if(sntp_server->mutex)
		os_mutex_lock(sntp_server->mutex, OS_WAIT_FOREVER);
	if(sntp_server->enable == 0)
	{
		vty_out(vty, "SNTP Server		: Disable%s",VTY_NEWLINE);
		if(sntp_server->mutex)
			os_mutex_unlock(sntp_server->mutex);
		return CMD_SUCCESS;
	}
	else
		vty_out(vty, "SNTP Server		: Enable%s",VTY_NEWLINE);

	vty_out(vty, "SNTP Server Version	: %d%s",sntp_server->version,VTY_NEWLINE);
	vty_out(vty, "SNTP Server Mode	: %s%s",mode_str[sntp_server->mode-1],VTY_NEWLINE);

	vty_out(vty, "SNTP Server Address	: %s%s",inet_ntoa(sntp_server->address),VTY_NEWLINE);
	vty_out(vty, "SNTP Server TTL		: %d%s",sntp_server->sntps_ttl,VTY_NEWLINE);

	if(sntp_server->mode == SNTPS_UNICAST && sntp_server->sntpsPort)
	{
		vty_out(vty, "SNTP Server Port	: %d%s",sntp_server->sntpsPort,VTY_NEWLINE);
	}
	if(sntp_server->mode != SNTPS_UNICAST && sntp_server->sntpsInterval)
	{
		vty_out(vty, "SNTP Sync Interval	: %d (sec)%s",sntp_server->sntpsInterval,VTY_NEWLINE);
	}
	if(sntp_server->mutex)
		os_mutex_unlock(sntp_server->mutex);
	return CMD_SUCCESS;
}
#ifndef SNTPS_CLI_ENABLE
int sntp_server_set_api(struct vty *vty, int cmd, const char *value)
{
	int ret = CMD_WARNING;
	char *argv[2] = {NULL, NULL};
	int *intValue = (int *)value;
	argv[0] = (char *)value;
	if(sntp_server->mutex)
		os_mutex_lock(sntp_server->mutex, OS_WAIT_FOREVER);
	switch(cmd)
	{
	case API_SNTPS_SET_ENABLE:
		if(intValue && *intValue)
			ret = sntps_server_enable_cmd();
		else
			ret = no_sntps_server_enable_cmd();
		break;
	case API_SNTPS_SET_LISTEN:
		if(argv[0])
			ret = sntps_server_listen_port_cmd(vty, argv);
		else
			ret = no_sntps_server_listen_port_cmd();
		break;
	case API_SNTPS_SET_VERSION:
		if(argv[0])
			ret = sntps_version_cmd(vty, argv);
		else
			ret = no_sntps_version_cmd();
		break;
	case API_SNTPS_SET_MODE:
		if(argv[0])
			ret = sntps_server_mode_cmd(argv);
		else
			ret = no_sntps_server_mode_cmd();
		break;
	case API_SNTPS_SET_INTERVAL:
		if(argv[0])
			ret = sntps_server_interval_cmd(vty, argv);
		else
			ret = no_sntps_server_interval_cmd();
		break;
	case API_SNTPS_SET_TTL:
		if(argv[0])
			ret = sntps_mutilcast_ttl_cmd(vty, argv);
		else
			ret = no_sntps_mutilcast_ttl_cmd();
		break;
	case API_SNTPS_SET_DEBUG:
		if(intValue && *intValue)
			ret = sntps_debug_cmd();
		else
			ret = no_sntps_debug_cmd();
		break;
	default:
		break;
	}
	if(sntp_server->mutex)
		os_mutex_unlock(sntp_server->mutex);
	return ret;
}
int sntp_server_get_api(struct vty *vty, int cmd, const char *value)
{
	int ret = ERROR;
	int *intValue = (int *)value;
	if(sntp_server->mutex)
		os_mutex_lock(sntp_server->mutex, OS_WAIT_FOREVER);
	switch(cmd)
	{
	case API_SNTPS_GET_ENABLE:
		if(intValue)
			*intValue = sntp_server->enable;
		ret = OK;
		break;
	case API_SNTPS_GET_LISTEN:
		if(intValue)
			*intValue = sntp_server->sntpsPort;
		ret = OK;
		break;
	case API_SNTPS_GET_VERSION:
		if(intValue)
			*intValue = sntp_server->version;
		ret = OK;
		break;
	case API_SNTPS_GET_MODE:
		if(intValue)
			*intValue = sntp_server->mode;
		ret = OK;
		break;
	case API_SNTPS_GET_INTERVAL:
		if(intValue)
			*intValue = sntp_server->sntpsInterval;
		ret = OK;
		break;

	case API_SNTPS_GET_TTL:
		if(intValue)
			*intValue = sntp_server->sntps_ttl;
		ret = OK;
		break;
	case API_SNTPS_GET_DEBUG:
		if(intValue)
			*intValue = sntp_server->time_debug;
		ret = OK;
		break;
	default:
		break;
	}
	if(sntp_server->mutex)
		os_mutex_unlock(sntp_server->mutex);
	return ret;
}
#endif
int sntps_config(struct vty *vty)
{
	//"sntp-server A.B.C.D port <162-65536> interval <30-3600>",
	char *mode_str[] = {"broadcast","unicast","multicast"};
	if(sntp_server == NULL)
		return CMD_SUCCESS;
	if(sntp_server->enable == 0)
		return CMD_SUCCESS;
	if(sntp_server->mutex)
		os_mutex_lock(sntp_server->mutex, OS_WAIT_FOREVER);
	vty_out(vty, "sntp server enable%s",VTY_NEWLINE);

	if(sntp_server->sntpsPort != SNTPC_SERVER_PORT)
		vty_out(vty, "sntp server listen-port %d%s", sntp_server->sntpsPort,VTY_NEWLINE);

	if(sntp_server->mode != SNTPS_UNICAST)
		vty_out(vty, "sntp server mode %s%s",mode_str[sntp_server->mode],VTY_NEWLINE);

	if(sntp_server->sntpsInterval != LOCAL_UPDATE_GMT)
		vty_out(vty, "sntp server interval %d%s",sntp_server->sntpsInterval,VTY_NEWLINE);

	if((sntp_server->version<<3) != SNTP_VN_3)
		vty_out(vty, "sntp server version %d%s", sntp_server->version,VTY_NEWLINE);
	if(sntp_server->mutex)
		os_mutex_unlock(sntp_server->mutex);
	return 1;
}

int sntps_debug_config(struct vty *vty)
{
	//"sntp-server A.B.C.D port <162-65536> interval <30-3600>",
	if(sntp_server == NULL)
		return CMD_SUCCESS;
	if(sntp_server->enable == 0)
		return CMD_SUCCESS;
	if(sntp_server->mutex)
		os_mutex_lock(sntp_server->mutex, OS_WAIT_FOREVER);
	if(sntp_server->time_debug)
		vty_out(vty, "debug sntp server%s",VTY_NEWLINE);
	if(sntp_server->mutex)
		os_mutex_unlock(sntp_server->mutex);
	return 1;
}

int sntpsInit(void *m)
{
	sntps_init(m);
	if(sntp_server == NULL)
	{
		sntp_server->address.s_addr = inet_addr(SNTP_MUTILCAST_ADDRESS);
		sntp_server->mode = SNTPS_MULTICAST;
		sntp_server->sntpsInterval = 180;
		sntp_server->sntps_ttl = 2;
		sntp_server->version = (SNTP_VN_3>>3);

		if(sntpsIsEnable())
		{
			sntpsDisable();
		}
		sntpsEnable(sntp_server->sntpsPort, sntp_server->sntpsInterval,
				sntp_server->version, sntp_server->mode);
	}
	//sntp_server->
	//install_default (SERVICE_NODE);
#ifdef SNTPS_CLI_ENABLE
	install_element (ENABLE_NODE, &show_sntps_server_cmd);

	install_element (CONFIG_NODE, &sntps_server_enable_cmd);
	install_element (CONFIG_NODE, &no_sntps_server_enable_cmd);

	install_element (CONFIG_NODE, &sntps_server_listen_port_cmd);
	install_element (CONFIG_NODE, &no_sntps_server_listen_port_cmd);


	install_element (CONFIG_NODE, &sntps_version_cmd);
	install_element (CONFIG_NODE, &no_sntps_version_cmd);

	install_element (CONFIG_NODE, &sntps_server_interval_cmd);
	install_element (CONFIG_NODE, &no_sntps_server_interval_cmd);

	install_element (CONFIG_NODE, &sntps_server_mode_cmd);
	install_element (CONFIG_NODE, &no_sntps_server_mode_cmd);

	install_element (CONFIG_NODE, &sntps_mutilcast_ttl_cmd);
	install_element (CONFIG_NODE, &no_sntps_mutilcast_ttl_cmd);

	install_element (CONFIG_NODE, &sntps_debug_cmd);
	install_element (CONFIG_NODE, &no_sntps_debug_cmd);
#endif
	return 0;
}
/*
 * sntp server enable
 * sntp server listen-port <100-65536>
 * sntp server mode (broadcast|unicast|multicast)
 * sntp server interval
 */
