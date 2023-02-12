/* sntpcLib.c - Simple Network Time Protocol (SNTP) client library */

/* Copyright 1984-2002 Wind River Systems, Inc. */
//#include "copyright_wrs.h"

/*
modification history 
--------------------
01k,07jan02,vvv  doc: added errnos for sntpcTimeGet and sntpcFetch (SPR #71557)
01j,16mar99,spm  doc: removed references to configAll.h (SPR #25663)
01e,14dec97,jdi  doc: cleanup.
01d,10dec97,kbw  making man page changes
01c,27aug97,spm  corrections for man page generation
01b,15jul97,spm  code cleanup, documentation, and integration; entered in
                 source code control
01a,24may97,kyc  written
*/

/* 
DESCRIPTION
This library implements the client side of the Simple Network Time 
Protocol (SNTP), a protocol that allows a system to maintain the 
accuracy of its internal clock based on time values reported by one 
or more remote sources.  The library is included in the VxWorks image 
if INCLUDE_SNTPC is defined at the time the image is built.

USER INTERFACE
The sntpcTimeGet() routine retrieves the time reported by a remote source and
converts that value for POSIX-compliant clocks.  The routine will either ipstack_send a 
request and extract the time from the reply, or it will wait until a message is
received from an SNTP/NTP server executing in broadcast mode.

INCLUDE FILES: sntpcLib.h

SEE ALSO: clockLib, RFC 1769
*/

/* includes */

/*
#include "vxWorks.h"
#include "sysLib.h"
#include "ioLib.h"
#include "inetLib.h"
#include "hostLib.h"
#include "sockLib.h"
#include "errnoLib.h"
*/
#include "auto_include.h"
#include <zplos_include.h>
#include "zmemory.h"
#include "vty.h"
#include "command.h"
#include "module.h"
#include "sntpcLib.h"

static struct sntp_client *sntp_client = NULL;

struct sntp_global_config sntp_global_config;


struct module_list module_list_sntpc = 
{ 
	.module=MODULE_SNTP, 
	.name="SNTP\0", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=cmd_sntpc_init, 
	.flags = 0,
	.taskid=0,
};
/* forward declarations */
/*******************************************************************************
*
* sntpcFractionToNsec - convert time from the NTP format to POSIX time format
*
* This routine converts the fractional part of the NTP timestamp format to a 
* value in nanoseconds compliant with the POSIX clock.  While the NTP time 
* format provides a precision of about 200 pico-seconds, rounding error in the 
* conversion routine reduces the precision to tenths of a micro-second.
* 
* RETURNS: Value for struct timespec corresponding to NTP fractional part
*
* ERRNO:   N/A
*
* INTERNAL
*
* Floating-point calculations can't be used because some boards (notably
* the SPARC architectures) disable software floating point by default to 
* speed up context switching. These boards abort with an exception when
* floating point operations are encountered.
*
* NOMANUAL
*/

LOCAL ULONG sntpcFractionToNsec
    (
    ULONG sntpFraction      /* base 2 fractional part of the NTP timestamp */
    )
    {
    ULONG factor = 0x8AC72305; /* Conversion factor from base 2 to base 10 */
    ULONG divisor = 10;        /* Initial exponent for mantissa. */
    ULONG mask = 1000000000;   /* Pulls digits of factor from left to right. */
    int loop;
    ULONG nsec = 0;
    zpl_bool shift = zpl_false;        /* Shifted to avoid overflow? */
    /* 
     * Adjust large values so that no intermediate calculation exceeds 
     * 32 bits. (This test is overkill, since the fourth MSB can be set 
     * sometimes, but it's fast).
     */
    if (sntpFraction & 0xF0000000)
    {
        sntpFraction /= 10;
        shift = zpl_true;
    }
    /* 
     * In order to increase portability, the following conversion avoids
     * floating point operations, so it is somewhat obscure.
     *
     * Incrementing the NTP fractional part increases the corresponding
     * decimal value by 2^(-32). By interpreting the fractional part as an
     * integer representing the number of increments, the equivalent decimal
     * value is equal to the product of the fractional part and 0.2328306437.
     * That value is the mantissa for 2^(-32). Multiplying by 2.328306437EERROR0
     * would convert the NTP fractional part into the equivalent in seconds.
     *
     * The mask variable selects each digit from the factor sequentially, and
     * the divisor shifts the digit the appropriate number of decimal places. 
     * The initial value of the divisor is 10 instead of 1E10 so that the 
     * conversion produces results in nanoseconds, as required by POSIX clocks.
     */
    for (loop = 0; loop < 10; loop++)    /* Ten digits in mantissa */
    {
		nsec += sntpFraction * (factor/mask)/divisor;  /* Use current digit. */
		factor %= mask;    /* Remove most significant digit from the factor. */
		mask /= 10;        /* Reduce length of mask by one. */
		divisor *= 10;     /* Increase preceding zeroes by one. */
    }
    /* Scale result upwards if value was adjusted before processing. */
    if (shift)
        nsec *= 10;

    return (nsec);
}
/*******************************************************************************/
static int sntp_socket_init(struct sntp_client *client)
{
    int result;
    struct ipstack_sockaddr_in srcAddr;

    client->sock = ipstack_socket (IPSTACK_IPCOM, IPSTACK_AF_INET, IPSTACK_SOCK_DGRAM, 0);
    if (ipstack_invalid(client->sock))
        return (ERROR);

    if(client->address.s_addr)
    {
    	int optval;
		optval = 1;
		result = ipstack_setsockopt (client->sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_BROADCAST,
							 (char *)&optval, sizeof (optval));
    }
    else
    {
		/* Initialize source address. */
		bzero ( (char *)&srcAddr, sizeof (srcAddr));
		srcAddr.sin_addr.s_addr = IPSTACK_INADDR_ANY;
		srcAddr.sin_family = IPSTACK_AF_INET;
		srcAddr.sin_port = client->sntpcPort;

		result = ipstack_bind (client->sock, (struct ipstack_sockaddr *)&srcAddr, sizeof (srcAddr));
		if (result == ERROR)
		{
			ipstack_close (client->sock);
			return (ERROR);
		}
    }
    return OK;
}

static int sntp_socket_close(struct sntp_client *client)
{
	if(!ipstack_invalid(client->sock))
		ipstack_close (client->sock);
    return OK;
}

static int sntp_socket_write(struct sntp_client *client)
{
    struct ipstack_sockaddr_in dstAddr;
    SNTP_PACKET sntpRequest;     /* sntp request packet for */
    //int servAddrLen = 0;
  
    /* Set destination for request. */
  
    bzero ( (char *)&dstAddr, sizeof (dstAddr));
    dstAddr.sin_addr.s_addr = client->address.s_addr;
    dstAddr.sin_family = IPSTACK_AF_INET;
    dstAddr.sin_port = htons(client->sntpcPort);

    /* Initialize SNTP message buffers. */
  
    bzero ( (char *)&sntpRequest, sizeof (sntpRequest));

    sntpRequest.leapVerMode = client->leapVerMode;//SNTP_CLIENT_REQUEST;
  
    bzero ( (char *) &dstAddr, sizeof (dstAddr));
    //servAddrLen = sizeof (dstAddr);
  
    /* Transmit SNTP request. */
    if (ipstack_sendto (client->sock, (caddr_t)&sntpRequest, sizeof(sntpRequest), 0,
                (struct ipstack_sockaddr *)&dstAddr, sizeof (dstAddr)) == ERROR)
    {
        //close (client->sock);
        if(client->time_debug)
        	zlog_err(MODULE_SNTP, "Transmit SNTP equest from %s:%d(%s)",ipstack_inet_ntoa(client->address),client->sntpcPort,ipstack_strerror(ipstack_errno));
        return (ERROR);
    }
    if(client->time_debug)
    	zlog_debug(MODULE_SNTP, "Transmit SNTP request from %s:%d",ipstack_inet_ntoa(client->address),client->sntpcPort);
    return OK;
}
static int sntp_socket_read(struct sntp_client *client)
{
    SNTP_PACKET sntpMessage;    /* buffer for message from server */
    struct ipstack_sockaddr_in srcAddr;
    int result;
    int srcAddrLen;
    char ver[32];
    /* Wait for broadcast message from server. */
    /* Wait for reply at the ephemeral port selected by the ipstack_sendto () call. */
    result = ipstack_recvfrom (client->sock, (caddr_t) &sntpMessage, sizeof(sntpMessage),
                       0, (struct ipstack_sockaddr *) &srcAddr, (socklen_t *)&srcAddrLen);
    if (result == ERROR)
    {
        //close (client->sock);
    	if(client->time_debug)
    		zlog_err(MODULE_SNTP, "SNTP receive from %s:%d(%s)",ipstack_inet_ntoa(srcAddr.sin_addr),ntohs(srcAddr.sin_port),ipstack_strerror(ipstack_errno));
        return (ERROR);
    }

    //close (client->sock);
    switch(sntpMessage.leapVerMode & SNTP_VN_MASK)
    {
    case SNTP_VN_0:           /* not supported */
    	sprintf(ver,"%s","version:0");
    	break;
    case SNTP_VN_1:           /* the earliest version */
    	sprintf(ver,"%s","version:1");
    	break;
    case SNTP_VN_2:
    	sprintf(ver,"%s","version:2");
    	break;
    case SNTP_VN_3:           /* VxWorks implements this version */
    	sprintf(ver,"%s","version:3");
    	break;
    case SNTP_VN_4:           /* the latest version, implemented if INET6 is defined */
    	sprintf(ver,"%s","version:4");
    	break;
    case SNTP_VN_5:          /* reserved */
    	sprintf(ver,"%s","version:5");
    	break;
    case SNTP_VN_6:           /* reserved */
    	sprintf(ver,"%s","version:6");
    	break;
    case SNTP_VN_7:           /* reserved */
    	sprintf(ver,"%s","version:7");
    	break;
    default:
    	sprintf(ver,"%s","unknow version");
    	break;
    }
    if(client->time_debug)
    {
    	zlog_debug(MODULE_SNTP, "SNTP receive SNTP %s from %s:%d",ver,ipstack_inet_ntoa(srcAddr.sin_addr),ntohs(srcAddr.sin_port));
    }
    /*
     * Return error if the server clock is unsynchronized, or the version is 
     * not supported.
     */

    if ( (sntpMessage.leapVerMode & SNTP_LI_MASK) == SNTP_LI_3 ||
        sntpMessage.transmitTimestampSec == 0)
    {
        if(client->time_debug)
        	zlog_warn(MODULE_SNTP, "SNTP server clock unsynchronized");
        //errnoSet (S_sntpcLib_SERVER_UNSYNC);
        return (ERROR);
    }

    if ( (sntpMessage.leapVerMode & SNTP_VN_MASK) == SNTP_VN_0 ||
        (sntpMessage.leapVerMode & SNTP_VN_MASK) > SNTP_VN_3)
    {
        if(client->time_debug)
        	zlog_warn(MODULE_SNTP, "SNTP version (%s) unsupported",ver);
        //errnoSet (S_sntpcLib_VERSION_UNSUPPORTED);
        return (ERROR);
    }
    /* Convert the NTP timestamp to the correct format and store in clock. */
    /* Add test for 2036 base value here! */

    sntpMessage.transmitTimestampSec =
                                     ntohl (sntpMessage.transmitTimestampSec) -
                                     SNTP_UNIX_OFFSET;
    /*
     * Adjust returned value if leap seconds are present.
     * This needs work!
     */
    /* if ( (sntpReply.leapVerMode & SNTP_LI_MASK) == SNTP_LI_1)
            sntpReply.transmitTimestampSec += 1;
     else if ((sntpReply.leapVerMode & SNTP_LI_MASK) == SNTP_LI_2)
              sntpReply.transmitTimestampSec -= 1;
    */
    sntp_global_stratum_set(sntpMessage.stratum);
    sntp_global_precision_set(sntpMessage.precision);

    sntpMessage.transmitTimestampFrac = ntohl (sntpMessage.transmitTimestampFrac);
    client->sntpTime.tv_sec = sntpMessage.transmitTimestampSec;
    client->sntpTime.tv_nsec = sntpcFractionToNsec (sntpMessage.transmitTimestampFrac);
    return (OK);
}


static int sntp_read(struct thread *thread)
{
	int ret = 0;
	struct sntp_client *client;
	client = THREAD_ARG (thread);
	//sock = THREAD_FD (thread);
	client->t_read = eloop_add_read (sntp_client->master, sntp_read, client, client->sock);
	ret = sntp_socket_read(client);
	if(ret == OK)
	{
		zpl_time_t	time_sec = 0;
		int timezone = 0;
		clock_settime(CLOCK_REALTIME, &client->sntpTime);//SET SYSTEM LOCAL TIME
		time_sec = time(NULL);
		if(client->time_debug)
			zlog_debug(MODULE_SNTP, "SNTP receive and set sys time:%s",ctime(&time_sec));
		sntp_global_timezone_get(&timezone);
		time_sec += timezone;//LOCAL_GMT_OFSET;
		if(client->time_debug)
			zlog_debug(MODULE_SNTP, "SNTP receive and set sys time:%s",ctime(&time_sec));
	}
	else
	{
		if(client->time_debug)
			zlog_warn(MODULE_SNTP, "SNTP protocol can't receive sys time");
	}
	return OK;
}

static int sntp_write(struct thread *thread)
{
	struct sntp_client *client;
	client = THREAD_ARG (thread);
	sntp_socket_write(client);
	return OK;
}

static int sntp_time(struct thread *thread)
{
	struct sntp_client *client;
	client = THREAD_ARG (thread);
	client->t_time = eloop_add_timer (sntp_client->master, sntp_time, client, client->sntpc_interval);
	client->t_write = eloop_add_write (sntp_client->master, sntp_write, client, client->sock);
	return OK;
}
/*******************************************************************************/
static int sntpcEnable(zpl_ushort port, char *ip, int time_interval, zpl_uint32 type, int mode)
{
	int ret = 0;
	if(sntp_client)
	{
		if(port)
			sntp_client->sntpcPort =  (port);
		else
			sntp_client->sntpcPort =  (SNTPC_SERVER_PORT);
		sntp_client->enable = 1;
		if(ip)
			sntp_client->address.s_addr = ipstack_inet_addr(ip);
		if(time_interval)
			sntp_client->sntpc_interval = time_interval;
		else
			sntp_client->sntpc_interval = LOCAL_UPDATE_GMT;
		if(mode)
			sntp_client->leapVerMode = mode;
		else
			sntp_client->leapVerMode = SNTP_CLIENT_REQUEST;
		sntp_client->type = type;

		ret = sntp_socket_init(sntp_client);
		if(ret > 0)
		{
			if(sntp_client->type != SNTP_PASSIVE)
			{
				sntp_client->t_time = eloop_add_timer (sntp_client->master, sntp_time, sntp_client, sntp_client->sntpc_interval);
				sntp_client->t_write = eloop_add_write (sntp_client->master, sntp_write, sntp_client, sntp_client->sock);
			}
			sntp_client->t_read = eloop_add_read (sntp_client->master, sntp_read, sntp_client, sntp_client->sock);
		}
		return (OK);
	}
    return (ERROR);
}

int sntpcDisable(void)
{
	if(sntp_client)
	{
		sntp_client->enable = 0;
		if(sntp_client->t_time)
			eloop_cancel(sntp_client->t_time);
		if(sntp_client->t_read)
			eloop_cancel(sntp_client->t_read);
		if(sntp_client->t_write)
			eloop_cancel(sntp_client->t_write);
		sntp_socket_close(sntp_client);
		//os_memset(sntp_client, 0, sizeof(struct sntp_client));
		return (OK);
	}
    return (ERROR);
}

static int sntpcIsEnable(void)
{
	if(sntp_client == NULL)
		return ERROR;
	return sntp_client->enable;
}


int sntpc_is_sync(void)
{
	if(sntp_client == NULL)
		return ERROR;
	return sntp_client->sync_clock;
}

int sntpc_server_address(struct ipstack_in_addr *address)
{
	if(sntp_client == NULL)
		return ERROR;
	if(address)
		*address = sntp_client->address;
	return OK;
}

int sntpc_is_dynamics(void)
{
	if(sntp_client == NULL)
		return ERROR;
	return sntp_client->dynamics;
}

int sntpc_dynamics_enable(void)
{
	if(sntp_client == NULL)
		return ERROR;
	if(!sntp_client->dynamics)
		sntp_client->address.s_addr = 0;
	sntp_client->dynamics = zpl_true;
	return OK;
}

int sntpc_dynamics_disable(void)
{
	if(sntp_client == NULL)
		return ERROR;
	if(sntp_client->dynamics)
		sntp_client->address.s_addr = 0;
	sntp_client->dynamics = zpl_false;
	return OK;
}

#ifdef SNTPC_CLI_ENABLE
DEFUN (sntp_enable,
		sntp_enable_cmd,
	    "sntp server A.B.C.D",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server ip address format A.B.C.D \n")
{
	int ret;
	int port = SNTPC_SERVER_PORT;
	int time_interval = LOCAL_UPDATE_GMT;
	struct ipstack_in_addr host_id;
	if(sntp_client == NULL)
		return CMD_WARNING;

	if(argc >= 2 && argv[1])
		port = atoi(argv[1]);

	if(argc == 3 && argv[2])
		time_interval = atoi(argv[2]);

	ret = ipstack_inet_aton (argv[0], &host_id);
	if (!ret)
	{
	      vty_out (vty, "Please specify Ip Address by A.B.C.D%s", VTY_NEWLINE);
	      return CMD_WARNING;
	}
	if( (port > SNTPC_SERVER_PORT_MAX)||(port < SNTPC_SERVER_PORT_MIN) )
	{

		vty_out(vty,"%% Interval port, may be 64-65535 %s",SNTPC_SERVER_PORT_MIN,
				SNTPC_SERVER_PORT_MAX, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if( (time_interval > LOCAL_UPDATE_GMT_MAX)||(time_interval < LOCAL_UPDATE_GMT_MIN) )
	{

		vty_out(vty,"%% Interval port, may be %d-%d %s",LOCAL_UPDATE_GMT_MIN,
				LOCAL_UPDATE_GMT_MAX, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(sntpcIsEnable())
	{
		vty_out(vty,"sntp server is already enable, please disable  first %s",VTY_NEWLINE);
		return CMD_WARNING;
	}
//	vty_out(vty,"sntp server %s %d %d %s",argv[0],port,time_interval,VTY_NEWLINE);
	sntpcEnable((zpl_ushort)port, argv[0], time_interval, sntp_client->type, sntp_client->leapVerMode);
	return CMD_SUCCESS;
}

ALIAS(sntp_enable,
		sntp_enable_port_cmd,
		"sntp server A.B.C.D port <100-65536>",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server ip address format A.B.C.D \n"
		"sntp server port configure\n"
		"udp port of sntp server\n")

ALIAS(sntp_enable,
		sntp_enable_port_interval_cmd,
	    "sntp server A.B.C.D port <100-65536> interval <30-3600>",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server ip address format A.B.C.D \n"
		"sntp server port configure\n"
		"udp port of sntp server\n"
		"sntp server ipstack_send request interval\n"
		"time interval of sec\n")

DEFUN (no_sntp_enable,
		no_sntp_enable_cmd,
		"no sntp server",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n")
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	if(sntpcIsEnable())
		sntpcDisable();
	return CMD_SUCCESS;
}
#endif
#ifdef SNTPC_CLI_ENABLE
DEFUN (sntp_version,
		sntp_version_cmd,
	    "sntp client version (1|2|3|4)",
		"sntp protocol configure\n"
		"sntp client configure\n"
		"sntp protocol version information\n"
		"sntp protocol version 1\n"
		"sntp protocol version 2\n"
		"sntp protocol version 3\n"
		"sntp protocol version 4\n")
#else
static int sntp_version_cmd(struct vty *vty, char *argv[])
#endif
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	int ver = atoi(argv[0]);
	if(ver > SNTP_VN_0 && ver < SNTP_VN_7)
	{
		int vm = 0;
		int mode = ((sntp_client->leapVerMode & SNTP_MODE_MASK));
		vm = (ver << 3) | mode;
		if(vm == sntp_client->leapVerMode)
			return CMD_SUCCESS;
		sntp_client->leapVerMode = vm;
	}
	if(sntpcIsEnable())
	{
		sntpcDisable();
		sntpcEnable(sntp_client->sntpcPort, NULL, sntp_client->sntpc_interval,
				sntp_client->type, sntp_client->leapVerMode);
	}
	return CMD_SUCCESS;
}
#ifdef SNTPC_CLI_ENABLE
DEFUN (no_sntp_version,
		no_sntp_version_cmd,
	    "no sntp client version",
		NO_STR
		"sntp protocol configure\n"
		"sntp client configure\n"
		"sntp protocol version information\n")
#else
static int no_sntp_version_cmd(void)
#endif
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	if(sntp_client->leapVerMode == SNTP_CLIENT_REQUEST)
		return CMD_SUCCESS;
	sntp_client->leapVerMode = SNTP_CLIENT_REQUEST;
	if(sntpcIsEnable())
	{
		sntpcDisable();
		sntpcEnable(sntp_client->sntpcPort, NULL, sntp_client->sntpc_interval,
			sntp_client->type, sntp_client->leapVerMode);
	}
	return CMD_SUCCESS;
}
#ifdef SNTPC_CLI_ENABLE
DEFUN (clock_timezone,
		clock_timezone_cmd,
	    "clock timezone <0-24>",
		"clock timezone configure\n"
		"local clock timezone configure\n"
		"local clock timezone value(default 8).\n")
{
	int timezone = 0;
	if(sntp_client == NULL)
		return CMD_WARNING;
	timezone = atoi(argv[0]);
	sntp_global_timezone_set(timezone * LOCAL_UPDATE_GMT_MAX);
//	sntp_client->timezone = ;
	return CMD_SUCCESS;
}

DEFUN (no_clock_timezone,
		no_clock_timezone_cmd,
	    "no clock timezone",
		NO_STR
		"clock timezone configure\n"
		"local clock timezone configure(default 8).\n")
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	sntp_global_timezone_set(LOCAL_GMT_OFSET);
//	sntp_client->timezone = LOCAL_GMT_OFSET;
	return CMD_SUCCESS;
}
#endif
#ifdef SNTPC_CLI_ENABLE
DEFUN (sntp_passive,
		sntp_passive_cmd,
	    "sntp client passive",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server passive type\n")
#else
static int sntp_passive_cmd(void)
#endif
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	sntp_client->type = SNTP_PASSIVE;
	if(sntpcIsEnable())
		sntpcDisable();
	sntpcEnable(sntp_client->sntpcPort, NULL, sntp_client->sntpc_interval, sntp_client->type, sntp_client->leapVerMode);
	return CMD_SUCCESS;
}
#ifdef SNTPC_CLI_ENABLE
DEFUN (no_sntp_passive,
		no_sntp_passive_cmd,
	    "no sntp client passive",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server passive type\n")
#else
static int no_sntp_passive_cmd(void)
#endif
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	sntp_client->type = SNTP_PASSIVE;
	if(sntpcIsEnable())
		sntpcDisable();
	sntpcEnable(sntp_client->sntpcPort, NULL, sntp_client->sntpc_interval, sntp_client->type, sntp_client->leapVerMode);
	return CMD_SUCCESS;
}

#ifdef SNTPC_CLI_ENABLE
DEFUN (sntp_debug,
		sntp_debug_cmd,
	    "debug sntp client",
		DEBUG_STR
		"sntp protocol configure\n"
		"SNTP Client timespec infomation\n")
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	sntp_client->time_debug = 1;
	return CMD_SUCCESS;
}

DEFUN (no_sntp_debug,
		no_sntp_debug_cmd,
	    "no debug sntp client",
		NO_STR
		DEBUG_STR
		"sntp protocol configure\n"
		"SNTP Client timespec infomation\n")
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	sntp_client->time_debug = 0;
	return CMD_SUCCESS;
}
#endif
#ifdef SNTPC_CLI_ENABLE
DEFUN (show_sntp_client,
		show_sntp_client_cmd,
	    "show sntp client",
		SHOW_STR
		"sntp protocol configure\n"
		"sntp server configure\n")
#else
int vty_show_sntpc_client(struct vty *vty)
#endif
{
	if(sntp_client == NULL)
		return CMD_WARNING;
	if(sntp_client->mutex)
		os_mutex_lock(sntp_client->mutex, OS_WAIT_FOREVER);
	if(sntp_client->enable == 0)
	{
		if(sntp_client->mutex)
			os_mutex_unlock(sntp_client->mutex);
		vty_out(vty, "SNTP Client		: Disable%s",VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	else
		vty_out(vty, "SNTP Client		: Enable%s",VTY_NEWLINE);
	if(sntp_client->leapVerMode)
	{
		int ver = ((sntp_client->leapVerMode & SNTP_VN_MASK) >> 3);
		int mode = ((sntp_client->leapVerMode & SNTP_MODE_MASK));

		char *mode_str[] = {"Reserve","Symmetric Active","Symmetric Passive",
				"Client","Server","Broadcast","NTP control message","Private use"};

		vty_out(vty, "SNTP Client Version	: %d%s",ver,VTY_NEWLINE);
		vty_out(vty, "SNTP Client Mode	: %s%s",mode_str[mode],VTY_NEWLINE);
	}
	if(sntp_client->type == SNTP_PASSIVE)
	{
		vty_out(vty, "SNTP Client Type	: Passive%s",VTY_NEWLINE);
	}
	if(sntp_client->type != SNTP_PASSIVE && sntp_client->address.s_addr)
	{
		vty_out(vty, "SNTP Server Address	: %s%s",ipstack_inet_ntoa(sntp_client->address),VTY_NEWLINE);
	}
	if(sntp_client->sntpcPort)
	{
		vty_out(vty, "SNTP Server Port	: %d%s",sntp_client->sntpcPort,VTY_NEWLINE);
	}
	if(sntp_client->type != SNTP_PASSIVE && sntp_client->sntpc_interval)
	{
		vty_out(vty, "SNTP Sync Interval	: %d (sec)%s",sntp_client->sntpc_interval,VTY_NEWLINE);
	}
	if(sntp_client->sntpTime.tv_sec)
	{
		struct tm t;
		char date_time[64];
		vty_out(vty, "SNTP Synchronized Status: Synchronized%s",VTY_NEWLINE);
		strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", localtime_r(&sntp_client->sntpTime.tv_sec, &t));
		vty_out(vty, "SNTP Last Time Sync	: %s%s",date_time,VTY_NEWLINE);
	}
	else
	{
		vty_out(vty, "SNTP Synchronized Status: Unsynchronized%s",VTY_NEWLINE);
		vty_out(vty, "SNTP Last Time Sync	: %s%s","0-0-0 0:0:0",VTY_NEWLINE);
	}
	if(sntp_client->mutex)
		os_mutex_unlock(sntp_client->mutex);
	return CMD_SUCCESS;
}

#ifndef SNTPC_CLI_ENABLE
static int sntpc_client_set_api_hw(struct vty *vty, zpl_uint32 cmd, const char *value, zpl_bool dynamics)
{

	int ret = CMD_WARNING;
	char *argv[2] = {NULL, NULL};
	int *intValue = (int *)value;
	argv[0] = (char*)value;
	if(sntp_client->mutex)
		os_mutex_lock(sntp_client->mutex, OS_WAIT_FOREVER);

	switch(cmd)
	{
	case API_SNTPC_SET_ENABLE:
		if(intValue && *intValue)
			ret = sntpcEnable(sntp_client->sntpcPort, NULL, sntp_client->sntpc_interval,
					sntp_client->type, sntp_client->leapVerMode);
		else
		{
			if(sntpcIsEnable())
				ret = sntpcDisable();
		}
		break;
	case API_SNTPC_SET_ADDRESS:
		if(argv[0])
		{
			if(strstr(argv[0], "."))
				sntp_client->address.s_addr = ipstack_inet_addr(argv[0]);
			sntp_client->dynamics = dynamics;
			ret = CMD_SUCCESS;
		}
		else
			ret = CMD_WARNING;
		break;
	case API_SNTPC_SET_PORT:
		if(intValue && (*intValue))
			sntp_client->sntpcPort =  (*intValue);
		else
			sntp_client->sntpcPort =  (SNTPC_SERVER_PORT);
		ret = CMD_SUCCESS;
		break;
	case API_SNTPC_SET_INTERVAL:
		if(intValue && (*intValue))
			sntp_client->sntpc_interval = (*intValue);
		else
			sntp_client->sntpc_interval = LOCAL_UPDATE_GMT;
		ret = CMD_SUCCESS;
		break;
	case API_SNTPC_SET_VERSION:
		if(argv[0])
			ret = sntp_version_cmd(vty, argv);
		else
			ret = no_sntp_version_cmd();
		break;
	case API_SNTPC_SET_TIMEZONE:
		if(argv[0])
			ret = sntp_global_timezone_set((*intValue)* LOCAL_UPDATE_GMT_MAX);
		else
			ret = sntp_global_timezone_set(LOCAL_GMT_OFSET);
		break;
	case API_SNTPC_SET_PASSIVE:
		if(intValue && *intValue)
			ret = sntp_passive_cmd();
		else
			ret = no_sntp_passive_cmd();
		break;

	case API_SNTPC_SET_DEBUG:
		if(intValue && *intValue)
			sntp_client->time_debug = 1;
		else
			sntp_client->time_debug = 0;
		ret = CMD_SUCCESS;
		break;
	default:
		break;
	}
	if(sntp_client->mutex)
		os_mutex_unlock(sntp_client->mutex);
	return ret;
}
int sntpc_client_set_api(struct vty *vty, zpl_uint32 cmd, const char *value)
{
	return sntpc_client_set_api_hw(vty,  cmd, value,  zpl_false);
}

int sntpc_client_dynamics_set_api(struct vty *vty, zpl_uint32 cmd, const char *value)
{
	return sntpc_client_set_api_hw(vty,  cmd, value,  zpl_true);
}

static int sntpc_client_get_api_hw(struct vty *vty, zpl_uint32 cmd, const char *value, zpl_bool dynamics)
{
	int ret = ERROR;
	int *intValue = (int *)value;
	if(sntp_client->dynamics != dynamics)
		return ret;
	if(sntp_client->mutex)
		os_mutex_lock(sntp_client->mutex, OS_WAIT_FOREVER);
	switch(cmd)
	{
	case API_SNTPC_GET_ENABLE:
		if(intValue)
			*intValue = sntp_client->enable;
		ret = OK;
		break;
	case API_SNTPC_SET_ADDRESS:
		if(intValue)
		{
			*intValue = ntohl(sntp_client->address.s_addr);
		}
		ret = OK;
		break;
	case API_SNTPC_SET_PORT:
		if(intValue)
			*intValue = sntp_client->sntpcPort;
		ret = OK;
		break;
	case API_SNTPC_SET_INTERVAL:
		if(intValue)
			*intValue = sntp_client->sntpc_interval;
		ret = OK;
		break;
	case API_SNTPC_SET_VERSION:
		if(intValue)
			*intValue = (sntp_client->leapVerMode & SNTP_VN_MASK)>>3;
		ret = OK;
		break;
	case API_SNTPC_SET_TIMEZONE:
		if(intValue)
			sntp_global_timezone_get(intValue);
		ret = OK;
		break;
	case API_SNTPC_SET_PASSIVE:
		if(intValue)
			*intValue = sntp_client->type;
		ret = OK;
		break;

	case API_SNTPC_SET_DEBUG:
		if(intValue)
			*intValue = sntp_client->time_debug;
		ret = OK;
		break;
	default:
		break;
	}
	if(sntp_client->mutex)
		os_mutex_unlock(sntp_client->mutex);
	return ret;
}

int sntpc_client_get_api(struct vty *vty, zpl_uint32 cmd, const char *value)
{
	return sntpc_client_get_api_hw(vty,  cmd, value, zpl_false);
}

int sntpc_client_dynamics_get_api(struct vty *vty, zpl_uint32 cmd, const char *value)
{
	return sntpc_client_get_api_hw(vty,  cmd, value, zpl_true);
}

#endif

int sntpc_config(struct vty *vty)
{
	//"sntp-server A.B.C.D port <162-65536> interval <30-3600>",
	int timezone = 0;
	if(sntp_client == NULL)
		return CMD_SUCCESS;
	if(sntp_client->mutex)
		os_mutex_lock(sntp_client->mutex, OS_WAIT_FOREVER);
	sntp_global_timezone_get(&timezone);
	vty_out(vty, "clock timezone %d%s",timezone,VTY_NEWLINE);

	if(sntp_client->enable == 0)
	{
		if(sntp_client->mutex)
			os_mutex_unlock(sntp_client->mutex);
		return CMD_SUCCESS;
	}
	if(sntp_client->dynamics == zpl_false)
	{
		vty_out(vty, "sntp server %s",ipstack_inet_ntoa(sntp_client->address));

		if(sntp_client->sntpcPort != SNTPC_SERVER_PORT)
			vty_out(vty, " port %d", sntp_client->sntpcPort);
	}
	else
	{
		vty_out(vty, "sntp server dynamics");

		if(sntp_client->sntpcPort != SNTPC_SERVER_PORT)
			vty_out(vty, " port %d", sntp_client->sntpcPort);
	}
	if(sntp_client->sntpc_interval != LOCAL_UPDATE_GMT)
		vty_out(vty, " interval %d", sntp_client->sntpc_interval);

	vty_out(vty, "%s",VTY_NEWLINE);

	if(sntp_client->type == SNTP_PASSIVE)
	{
		vty_out(vty, "sntp client passive%s",VTY_NEWLINE);
	}
	if(sntp_client->leapVerMode != SNTP_CLIENT_REQUEST)
	{
		int ver = ((sntp_client->leapVerMode & SNTP_VN_MASK) >> 3);
		vty_out(vty, "sntp client version %d%s", ver,VTY_NEWLINE);
	}
	if(sntp_client->mutex)
		os_mutex_unlock(sntp_client->mutex);
	return 1;
}

int sntpc_debug_config(struct vty *vty)
{
	//"sntp-server A.B.C.D port <162-65536> interval <30-3600>",
	if(sntp_client == NULL)
		return CMD_SUCCESS;
	if(sntp_client->enable == 0)
		return CMD_SUCCESS;
	if(sntp_client->mutex)
		os_mutex_lock(sntp_client->mutex, OS_WAIT_FOREVER);
	if(sntp_client->time_debug)
		vty_out(vty, "debug sntp client%s",VTY_NEWLINE);
	if(sntp_client->mutex)
		os_mutex_unlock(sntp_client->mutex);
	return 1;
}


int sntpcInit(void *m)
{
	if(sntp_client == NULL)
		sntp_client = (struct sntp_client *)malloc(sizeof(struct sntp_client));
	os_memset(sntp_client, 0, sizeof(struct sntp_client));
	sntp_client->master = m;
	sntp_client->leapVerMode = SNTP_CLIENT_REQUEST;
	sntp_client->mutex = os_mutex_name_create("sntpc");
#ifdef SNTPC_CLI_ENABLE
	install_node (&service_node, sntpc_config);
	install_default (ALL_SERVICE_NODE);

	install_element (ENABLE_NODE, CMD_VIEW_LEVEL, &show_sntp_client_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_enable_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_enable_port_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_enable_port_interval_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntp_enable_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_version_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntp_version_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &clock_timezone_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_clock_timezone_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_passive_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntp_passive_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_debug_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntp_debug_cmd);
#endif
	return 0;
}
/*
Ruijie(config)# sntp server 192.43.244.18
2������SNTPͬ��ʱ�ӵļ��
Ruijie(config)# sntp interval 1800        //����ÿ���Сʱ�����SNTP������ͬ����ʱ��
3�����ñ���ʱ��
Ruijie(config)# clock timezone 8        //Ĭ����GMTʱ�䣬���й��û�������Ϊ������������ʱ������8Сʱ
4����SNTP����
Ruijie(config)# sntp enable              //����ʹ��SNTP����



*/
