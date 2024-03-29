/* syslogcLib.c - BSD syslog protocol client library */

/* Copyright 2004 Wind River Systems, Inc. */
// #include "copyright_wrs.h"
/*
 modification history
 --------------------
 01c,13jul04,myz  move common header files to fw.h
 01b,27feb04,myz  make syslogcBinToAscStrConvert() a local function
 01a,08sep03,myz  written
 */

/*
 DESCRIPTION

 This library implements the client side of the BSD syslog protocol RFC 3164.
 This protocol provides a transport to allow a machine to ipstack_send event
 notification messages across IP networks to event message collectors - also
 known as syslog servers.

 */

/*
 #include "fw.h"
 #include "ioLib.h"
 #include "hostLib.h"
 #include "sockLib.h"
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "zmemory.h"
#include "log.h"
#include "host.h"
#include "syslogcLib.h"

#ifdef ZPL_SERVICE_SYSLOG
/* typedefs */

/* locals */
static struct syslog_client *syslog_client = NULL;

/*
 static struct ipstack_in_addr client->address;
 static char  hostname[DFT_HOST_NAME_LEN];
 static int   client->sock = 0;
 static int  syslogcLibInitDone = zpl_false;
 */

static char *monthStr[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
						   "Aug", "Sep", "Oct", "Nov", "Dec"};

/* local function prototypes */

static int syslogMsgSend(struct syslog_client *, void *, zpl_uint32, char *, zpl_uint16, zpl_ulong,
						 zpl_uint32);
// static int mdataToSyslogBuf (M_BLK_ID,int, char *, int);
static int num100LessToAsc(zpl_uint32, char *);
static int timestampStrGet(char *);
static int syslogcBinToAscStrConvert(zpl_uint8 *, zpl_uint32, char *, zpl_uint32);
static char *syslogHostNameStrGet(struct syslog_client *);
/******************************************************************************
 *
 * syslogcLibInit - initialize the syslog client module
 *
 * This routine saves the default syslog server IP address if provided and
 * creates a client side UDP ipstack_socket binded with the default syslog port.
 * Both the UDP ipstack_socket and the server IP address will be used in the sending
 * routines.
 * default syslog server IP address, NULL terminated
 * RETURNS: ipstack_socket file descriptor or ERROR if fails
 *
 */
static int syslogc_socket_init(struct syslog_client *client)
{
	struct ipstack_sockaddr_in sin;
	zpl_socket_t sFd;
	zpl_family_t family = IPSTACK_SOCK_DGRAM;
	if (!client)
		return ERROR;
	if (client && client->enable == zpl_true)
	{
		if (!ipstack_invalid(client->sock))
			ipstack_close(client->sock);
	}
	/* validate the destination parameter */

	if (client->address_string == NULL)
	{
		return ERROR;
	}
	else
	{
		// hostGetByName
		struct ipstack_hostent *hptr;
		hptr = ipstack_gethostbyname(client->address_string);
		if (hptr)
		{
			char **pptr;
			char str[32];
			/* ���ݵ�ַ���ͣ�����ַ����� */
			switch (hptr->h_addrtype)
			{
			case IPSTACK_AF_INET:
			case IPSTACK_AF_INET6:
				pptr = hptr->h_addr_list;
				/* ���ղŵõ������е�ַ������������е�����ipstack_inet_ntop()���� */
				for (; *pptr != NULL; pptr++)
				{
					/*					printf(" address:%s\n",
												ipstack_inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));*/
					ipstack_inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
					client->address.s_addr = ipstack_inet_addr(str);
					break;
				}
				break;
			default:
				fprintf(stdout, "\r\n%s:unknown address type:%s\r\n",
						__func__, os_strerror(ipstack_errno));
				break;
			}
		}
		else
		{
			if (((client->address.s_addr = ipstack_inet_addr(client->address_string)) == ERROR) /* &&
			  ((client->address.s_addr = ipstack_gethostbyname(pServer)) == ERROR)*/
			)
			{
				client->address.s_addr = 0;
				fprintf(stdout, "\r\n%s:syslogcLibInit,unknown default syslog server name:%s\r\n",
						__func__, os_strerror(ipstack_errno));
				fflush(stdout);
				return ERROR;
			}
		}
	}

	/* create the client ipstack_socket */

	bzero((char *)&sin, sizeof(sin));

	if (client->mode == SYSLOG_TCP_MODE)
		family = IPSTACK_SOCK_STREAM;
	else
		family = IPSTACK_SOCK_DGRAM;
	sFd = ipstack_socket(IPSTACK_IPCOM, IPSTACK_AF_INET, family, 0);
	if (ipstack_invalid(sFd))
	{
		fprintf(stdout, "\r\n%s: syslogcLibInit, syslog ipstack_socket creation fail:%s\r\n",
				__func__, os_strerror(ipstack_errno));
		fflush(stdout);
		return ERROR;
	}

	/* ipstack_bind the ipstack_socket to the default port as recommeded by RFC3164 */

	sin.sin_family = IPSTACK_AF_INET;
	//    sin.sin_len    = sizeof(sin);
	sin.sin_port = htons(SYSLOGC_DEFAULT_PORT);
	sin.sin_addr.s_addr = IPSTACK_INADDR_ANY;

	if (ipstack_bind(sFd, (struct ipstack_sockaddr *)&sin, sizeof(sin)) == ERROR)
	{
		//		perror("\r\nsyslogcLibInit, syslog sokcet ipstack_bind fail");
		fprintf(stdout, "%s: syslogcLibInit ipstack_bind fail:%s\r\n",
				__func__, os_strerror(ipstack_errno));
		ipstack_close(sFd);
		fflush(stdout);
		return ERROR;
	}

	/* get the client's host name used as ID in syslog message */

	/*
		bzero(client->hostname, DFT_HOST_NAME_LEN);

		if (gethostname(client->hostname, DFT_HOST_NAME_LEN - 1) == ERROR)
			strcpy(client->hostname, "Un-defined host name");
	*/
	sin.sin_family = IPSTACK_AF_INET;
	// serverSockAddr.sin_len         = sizeof(serverSockAddr);
	sin.sin_port = htons(client->port);
	sin.sin_addr.s_addr = client->address.s_addr;
	if (ipstack_connect(sFd, (struct ipstack_sockaddr *)&sin, sizeof(sin)) == ERROR)
	{
		fprintf(stdout, "%s: syslogcLibInit ipstack_connect fail:%s\r\n",
				__func__, os_strerror(ipstack_errno));
		ipstack_close(sFd);
		fflush(stdout);
		return ERROR;
	}

	client->sock = sFd;
	client->ipstack_connect = zpl_true;
	return 1;
}

static int syslogc_reconnect(struct syslog_client *client)
{
	return syslogc_socket_init(client);
}
/*****************************************************************************
 *
 * syslogcShutdown - close and clean up the syslog client module
 *
 * This routine disables the syslog client and reclaims the resource. The
 * syslogcLibInit() should be called again in order to restart the syslog client.
 *
 * RETURNS: NONE
 */

static int syslogcShutdown(struct syslog_client *client)
{
	if (!client)
		return ERROR;
	if (client->enable == zpl_true)
	{
		if (!ipstack_invalid(client->sock))
			ipstack_close(client->sock);
		client->ipstack_connect = zpl_false;
		// client->enable = zpl_false;
		return OK;
	}
	return OK;
}
/******************************************************************************
 *
 * syslogcMdataSend - ipstack_send the data in mbuf chain to the syslog server
 *
 * This routine sends the data in mbuf chain up to the maximum limit per syslog
 * message to the syslog server.
 *
 * RETURNS: OK or ERROR
 */
#if 0
int syslogcMdataSend
(
		M_BLK_ID pMblk, /* point to the mbuf */
		int pktLogLen, /* the number of bytes of the packet to be logged */
		char * pMsgId, /* The message ID string, optional */
		zpl_uint16 code, /* syslog facility and severity code, default used if 0 */
		zpl_ulong serverIpAddr /* binary server IP address in network byte  order,
		 * default used if value is 0 */
)
{
	if (syslogcLibInitDone == zpl_false)
	return ERROR;

	return syslogMsgSend((void *)pMblk,pktLogLen,pMsgId,code,
			serverIpAddr,MDATA_TYPE);
}
#endif
/******************************************************************************
 *
 * syslogcBinDataSend - ipstack_send a block of binary data to the syslog server
 *
 * This routine sends a block of binary data to the syslog server
 *
 * RETURNS: OK or ERROR
 */
#if 0
static int syslogcBinDataSend(struct syslog_client *client, zpl_uint8 * pData, /* point to the data stream */
int len, /* length of the data stream */
char * pMsgId, /* The message ID string, optional. */
zpl_uint16 code, /* syslog facility and severity code, default used if 0 */
zpl_ulong serverIpAddr /* binary server IP address in network byte order,
 * default used if value is 0 */
) {
	if(!client)
		return ERROR;
	if (client->enable == zpl_false)
		return ERROR;

	return syslogMsgSend(client, (void *) pData, len, pMsgId, code, serverIpAddr,
			BDATA_TYPE);
}
#endif
/******************************************************************************
 *
 * syslogcStringSend - ipstack_send a string of character data to the syslog server
 *
 * This routine sends a string of character data to the syslog server
 *
 * RETURNS: OK or ERROR
 */

static int syslogcStringSend(struct syslog_client *client, char *pStr, /* point to the NULL terminated string */
							 zpl_uint16 code,						   /* syslog facility and severity code, default used if 0 */
							 zpl_ulong serverIpAddr					   /* binary server IP address in network byte order,
																		* default used if value is 0 */
)
{
	if (!client)
		return ERROR;
	if (client->enable == zpl_false)
		return ERROR;

	return syslogMsgSend(client, (void *)pStr, 0, NULL, code, serverIpAddr, CDATA_TYPE);
}

/******************************************************************************
 *
 * syslogMsgSend - Send a message to the syslog server
 *
 * This routine composes a syslog format message and sends to the syslog server
 * specified in the init routine.
 *
 * RETURNS: OK or ERROR
 *
 */
static int syslogMsgSend(struct syslog_client *client, void *pData, /* point to the data message */
						 zpl_uint32 dataLen,						/* length of the message */
						 char *pMsgIdStr,							/* message Id string */
						 zpl_uint16 code,							/* syslog facility and severity code, default used if 0 */
						 zpl_ulong serverIpAddr, zpl_uint32 dataType)
{
	char pLogMsg[SYSLOG_MSG_MAX_LEN];
	zpl_uint32 cnt = 0;
	zpl_uint32 remainder;
	zpl_uint32 digit;
	char *pTmp;
	zpl_uint32 len;
	//	struct ipstack_sockaddr_in serverSockAddr;
	zpl_uint8 fCode; /* syslog facility code */
	zpl_uint8 sCode; /* syslog severity code */

	/* safety check */

	if (pData == NULL)
		return ERROR;

	fCode = (code >> 8) & 0xff;
	sCode = (zpl_uint8)(code & 0xff);

	/* check facility code, use default if not specified or invalid */

	if (fCode > SYSLOGC_static7)
		fCode = SYSLOGC_USER_LEVEL_MSG;

	/* check severity code, use default if not specified or invalid */

	if (sCode > SYSLOGC_SEVERITY_DEBUG)
		sCode = SYSLOGC_SEVERITY_INFORMATIONAL;

	if (serverIpAddr == 0 && client->address.s_addr == 0)
	{
		/* syslog server IP address is not specified  */

		return ERROR;
	}

	if (!serverIpAddr)
		serverIpAddr = client->address.s_addr;

	/* initialize the server address structure */

	/*
		serverSockAddr.sin_family = IPSTACK_AF_INET;
		serverSockAddr.sin_len         = sizeof(serverSockAddr);
		serverSockAddr.sin_port = htons(SYSLOGC_DEFAULT_PORT);
		serverSockAddr.sin_addr.s_addr = serverIpAddr;
	*/

	/* malloc a buffer to compose the syslog message */

	/*	if ((pLogMsg = XMALLOC(MTYPE_ZLOG, SYSLOG_MSG_MAX_LEN)) == NULL) {
			fprintf (stdout, "\r\nERROR: slogcMsgSend: fail to allocate the memory\r\n");
			return ERROR;
		}*/
	os_memset(pLogMsg, 0, SYSLOG_MSG_MAX_LEN);

	/* compose the PRI part */

	pLogMsg[cnt++] = PRI_PART_LEADING_CHAR; /* leading character */

	/* priority value = facility number * 8 + severity number */

	remainder = fCode * 8 + sCode; /* priority value */

	/* extract the 100th digit and convert to ASCII value*/

	digit = remainder / 100;
	if (digit)
	{
		remainder = remainder - (digit * 100);
		pLogMsg[cnt++] = digit + ASCII_ZERO;
	}

	/* convert 100 less number */

	cnt += num100LessToAsc(remainder, &pLogMsg[cnt]);

	pLogMsg[cnt++] = PRI_PART_ENDING_CHAR; /* ending character for PRI part */

	/* compose the Header part, immediately follow the PRI part
	 * The Header consists of two field timerstamp and host name
	 * Each field should be followed by a space character
	 */

	/* first the timestamp */

	cnt += timestampStrGet(&pLogMsg[cnt]);
	pLogMsg[cnt++] = FIELD_DELIMITER;

	/* host name */
	client->hostname = syslogHostNameStrGet(client);
	if (client->hostname)
	{
		bcopy(client->hostname, &pLogMsg[cnt], os_strlen(client->hostname));
		cnt += strlen(client->hostname);
		pLogMsg[cnt++] = FIELD_DELIMITER;
	}

	/* The final part, the MSG part consists of two fields TAG and CONTENT.
	 * TAG, max 32 chars, CONTENT, the first char should be non-alphanumeric
	 * signifying the end of TAG. There is no ending delimiter to the whole
	 * MSG part
	 */

	/* Use task name as the  process Id */

	pTmp = client->processname; // taskName(taskIdSelf());

	if ((pTmp))
	{
		len = MIN(strlen(pTmp), DFT_TAGID_LEN);
		bcopy(pTmp, &pLogMsg[cnt], len);
		cnt += len;
		pLogMsg[cnt++] = ':';
		pLogMsg[cnt++] = ' ';
	}

	if (pMsgIdStr != NULL)
	{
		len = MIN(strlen(pMsgIdStr), SYSLOG_MSG_MAX_LEN - cnt - 1);
		bcopy(pMsgIdStr, &pLogMsg[cnt], len);
		cnt += len;
	}

	pLogMsg[cnt++] = ' ';

	switch (dataType)
	{
	case MDATA_TYPE:
		/*            cnt += mdataToSyslogBuf((M_BLK_ID)pData,dataLen,
		 &pLogMsg[cnt],
		 SYSLOG_MSG_MAX_LEN - cnt);*/
		break;

	case BDATA_TYPE:
		cnt += syslogcBinToAscStrConvert((zpl_uint8 *)pData, dataLen,
										 &pLogMsg[cnt],
										 SYSLOG_MSG_MAX_LEN - cnt);
		break;

	case CDATA_TYPE:
	{
		zpl_uint32 copyLen;

		copyLen = MIN(strlen((char *)pData), (SYSLOG_MSG_MAX_LEN - cnt));

		bcopy((char *)pData, &pLogMsg[cnt], copyLen);
		cnt += copyLen;
	}
	break;

	default:
		fprintf(stdout, "\r\nERROR type 0x%x not supported\r\n", dataType);
	}

	/* now ipstack_send the message to the server */

	/*	if (ipstack_sendto(client->sock, (caddr_t) pLogMsg, cnt, 0,
				(struct ipstack_sockaddr *) &serverSockAddr, sizeof(serverSockAddr))
				== ERROR) {
			XFREE(MTYPE_ZLOG,pLogMsg);
			fprintf (stdout, "\r\n%s: syslogMsgSend ipstack_sendto fail:%s\r\n",
					   __func__,os_strerror (ipstack_errno) );
			fflush(stdout);
			return (ERROR);
		}*/
	if (ipstack_write(client->sock, pLogMsg, cnt) == ERROR)
	{

		syslogcShutdown(client);
		return syslogc_reconnect(client);
		fprintf(stdout, "\r\n%s: syslogMsgSend ipstack_sendto fail:%s\r\n",
				__func__, os_strerror(ipstack_errno));
		fflush(stdout);
		return (ERROR);
	}

	//	XFREE(MTYPE_ZLOG,pLogMsg);
	return OK;
}

/******************************************************************************
 *
 * syslogcBinToAscStrConvert - Convert binary data to the ASCII format
 *
 * RETURN: number of ASCII bytes written to buffer
 */

static int syslogcBinToAscStrConvert(zpl_uint8 *pData,	 /* point to a block of data */
									 zpl_uint32 dataLen, /* data length */
									 char *pBuf,		 /* buffer to store converted ASCII value */
									 zpl_uint32 bufLen	 /* length of the buffer */
)
{
	zpl_uint32 i;
	zpl_uint32 cnt = 0;
	zpl_uint32 copyLen;

	/* make sure not write beyond the buffer limit */

	copyLen = MIN(dataLen, bufLen / 3);

	for (i = 0; i < copyLen; i++)
	{
		/* the most significant 4 bits */

		if (BYTE_MS4BITS(pData[i]) > 9)
			pBuf[cnt++] = BYTE_MS4BITS(pData[i]) - 9 + ASCII_a;
		else
			pBuf[cnt++] = BYTE_MS4BITS(pData[i]) + ASCII_ZERO;

		/* the least significant 4 bits */

		if (BYTE_LS4BITS(pData[i]) > 9)
			pBuf[cnt++] = BYTE_LS4BITS(pData[i]) - 9 + ASCII_a;
		else
			pBuf[cnt++] = BYTE_LS4BITS(pData[i]) + ASCII_ZERO;

		/* the space char */

		pBuf[cnt++] = ' ';
	}
	return cnt;
}

/******************************************************************************
 *
 * mdataToSyslogBuf - Convert data in mbuf into ASCII format in syslog buffer
 *
 * RETURNS: number of bytes of the buffer consumed
 */
#if 0
static int mdataToSyslogBuf
(
		M_BLK_ID pMblk, /* point to the mbuf chain */
		int pktLogLen, /* number of the bytes of the packet to be sent */
		char * pBuf, /* the buffer to take the converted ASCII data */
		int bufLen /* the maxium length of the buffer */
)
{
	int mdataLen;
	int copyLen;
	zpl_uint8 * pData;
	int cnt = 0;

	/* each 8 bit binary data consumes 3 bytes(data + space) of ASCII buffer */

	if (pktLogLen)
	copyLen = MIN(pktLogLen,pMblk->m_pkthdr.len);
	else
	copyLen = pMblk->m_pkthdr.len;

	copyLen = MIN(copyLen, (bufLen/3));

	/* now copy the packet data */

	while (pMblk != NULL)
	{
		mdataLen = MIN(pMblk->mBlkHdr.mLen, copyLen);

		pData = (zpl_uint8 *)pMblk->mBlkHdr.mData;

		cnt += syslogcBinToAscStrConvert(pData,mdataLen,&pBuf[cnt],bufLen-cnt);

		copyLen -= mdataLen;

		if (copyLen <= 0)
		break;

		pMblk = pMblk->mBlkHdr.mNext;
	}
	return cnt;
}
#endif
/******************************************************************************
 *
 * timestampStrGet - Get the syslog format timestamp string
 *
 * This routine retrieves the timestamp information from the local clock using
 * clockLib function clock_gettime. It assumes the clock has been setup and
 * running.
 *
 * RETURNS: the length of  the timestamp string
 */
#if 1
static int timestampStrGet(char *pBuf)
{
	zpl_uint32 cnt = 0;
	zpl_time_t timeSec = 0;
	struct tm *tm;
	char buf[50];
	timeSec = time(NULL);
	if (!timeSec)
		return cnt;
	os_memset(buf, 0, sizeof(buf));
	tm = localtime((zpl_time_t *)&timeSec);
	strftime(buf, 50, "%d %H:%M:%S", tm);
	strcpy(pBuf, monthStr[tm->tm_mon]);
	cnt += strlen(monthStr[tm->tm_mon]);
	pBuf[cnt++] = FIELD_DELIMITER;
	strcpy(&pBuf[cnt], buf);
	cnt += strlen(buf);
	return cnt;
}

#else
static int timestampStrGet(char *pBuf /* buffer to store the timestamp string */
)
{
	int cnt = 0;
	zpl_uint32 timeSec = 0;
	int digit;
	struct timespec ts;
	struct tm timeFields;
	// struct timespec _clockRealtime;

	// if (_clockRealtime.timeBase.tv_sec)
	{
		clock_gettime(CLOCK_REALTIME, &ts);
		timeSec = ts.tv_sec;
	}

	if (!timeSec)
		return cnt;

	localtime_r((zpl_time_t *)&timeSec, &timeFields);

	/* month field */

	strcpy(pBuf, monthStr[timeFields.tm_mon]);
	cnt += strlen(monthStr[timeFields.tm_mon]);

	pBuf[cnt++] = FIELD_DELIMITER;

	/* day field */

	if (timeFields.tm_mday < 10)
		pBuf[cnt++] = ' ';
	else
	{
		digit = timeFields.tm_mday / 10;
		timeFields.tm_mday = timeFields.tm_mday - (digit * 10);
		pBuf[cnt++] = digit + ASCII_ZERO;
	}

	pBuf[cnt++] = timeFields.tm_mday + ASCII_ZERO;
	pBuf[cnt++] = FIELD_DELIMITER;

	/* hour field */

	cnt += num100LessToAsc(timeFields.tm_hour, &pBuf[cnt]);
	pBuf[cnt++] = ':';

	/* minute field */

	cnt += num100LessToAsc(timeFields.tm_min, &pBuf[cnt]);
	pBuf[cnt++] = ':';

	/* second field */

	cnt += num100LessToAsc(timeFields.tm_sec, &pBuf[cnt]);

	return cnt;
}
#endif
/******************************************************************************
 *
 * num100LessToAsc - Convert numeric values less than 100 to the ASCII value
 *
 * RETURN: number of bytes of the ASCII data
 */

static int num100LessToAsc(zpl_uint32 number, /* numeric value */
						   char *pBuf		  /* buffer to store the converted ASCII data */
)
{
	zpl_uint32 digit;
	zpl_uint32 cnt = 0;

	digit = number / 10;
	pBuf[cnt++] = digit + ASCII_ZERO;
	number = number - digit * 10;
	pBuf[cnt++] = number + ASCII_ZERO;

	return cnt;
}

/******************************************************************************/
static char *syslogHostNameStrGet(struct syslog_client *client)
{
	if (client && client->hostname)
		return client->hostname;
	else
		return _global_host.name;
}
/*
 * <199>Sep .7 09:38:46 localhost.localdomain  aaa bbb
 *
 */

int syslogc_lib_init(void *m, char *processname)
{
	if (!syslog_client)
		syslog_client = XMALLOC(MTYPE_ZLOG, sizeof(struct syslog_client));
	if (syslog_client)
	{
		syslog_client->master = m;
		if (processname)
		{
			if (syslog_client->processname)
				XFREE(MTYPE_ZLOG, syslog_client->processname);
			syslog_client->processname = XSTRDUP(MTYPE_ZLOG, processname);
			// os_memset(syslog_client->processname, 0, sizeof(syslog_client->processname));
			// os_memcpy(syslog_client->processname, processname, MIN(os_strlen(processname),DFT_HOST_NAME_LEN));
		}
		if (!syslog_client->mutx)
			syslog_client->mutx = os_mutex_name_create("syslog-mutex");
		//		OS_SERVICE_DEBUG("%s:processname=%s\r\n", __func__,syslog_client->processname);
		return OK;
	}
	return ERROR;
}

int syslogc_lib_uninit(void)
{
	if (syslog_client)
	{
		//		OS_SERVICE_DEBUG("%s:\r\n", __func__);
		syslogcShutdown(syslog_client);
		if (syslog_client->processname)
			XFREE(MTYPE_ZLOG, syslog_client->processname);
		// if(syslog_client->hostname)
		if (syslog_client->mutx)
			os_mutex_destroy(syslog_client->mutx);
		//	XFREE(MTYPE_ZLOG, syslog_client->hostname);
		XFREE(MTYPE_ZLOG, syslog_client);
		syslog_client = NULL;
	}
	return OK;
}

int syslogc_enable(char *hostname)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->mutx)
		os_mutex_lock(syslog_client->mutx, OS_WAIT_FOREVER);
	if (syslog_client->enable == zpl_false)
		syslog_client->enable = zpl_true;
	if (hostname)
	{
		/*		if(syslog_client->hostname)
					XFREE(MTYPE_ZLOG, syslog_client->hostname);
				syslog_client->processname = XSTRDUP(MTYPE_ZLOG, processname);
				*/
		syslog_client->hostname = hostname;
		// os_memset(syslog_client->hostname, 0, sizeof(syslog_client->hostname));
		// os_memcpy(syslog_client->hostname, hostname, MIN(os_strlen(hostname),DFT_HOST_NAME_LEN));
	}
	else
	{
		syslog_client->hostname = syslogHostNameStrGet(syslog_client);
		// syslog_client->hostname = zpl_host.name;
	}
	if (syslog_client->mutx)
		os_mutex_unlock(syslog_client->mutx);
	return OK;
}

zpl_bool syslogc_is_enable(void)
{
	if (!syslog_client)
		return zpl_false;
	return syslog_client->enable;
}

int syslogc_disable(void)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->enable == zpl_false)
		return OK;
	if (syslog_client->mutx)
		os_mutex_lock(syslog_client->mutx, OS_WAIT_FOREVER);
	syslogcShutdown(syslog_client);
	syslog_client->enable = zpl_false;
	if (syslog_client->mutx)
		os_mutex_unlock(syslog_client->mutx);
	return OK;
}

int syslogc_is_dynamics(void)
{
	if (syslog_client == NULL)
		return ERROR;
	return syslog_client->dynamics;
}

int syslogc_dynamics_enable(void)
{
	if (syslog_client == NULL)
		return ERROR;
	if (!syslog_client->dynamics)
		os_memset(syslog_client->address_string, 0, sizeof(syslog_client->address_string));
	syslog_client->dynamics = zpl_true;
	return OK;
}

int syslogc_dynamics_disable(void)
{
	if (syslog_client == NULL)
		return ERROR;
	if (syslog_client->dynamics)
		os_memset(syslog_client->address_string, 0, sizeof(syslog_client->address_string));
	syslog_client->dynamics = zpl_false;
	return OK;
}

int syslogc_host_config_set(char *address_string, zpl_uint16 port, zpl_uint32 facility)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->enable == zpl_false)
		return ERROR;
	if (syslog_client->mutx)
		os_mutex_lock(syslog_client->mutx, OS_WAIT_FOREVER);
	syslog_client->port = port;
	syslog_client->facility = facility;
	if (address_string)
	{
		os_memset(syslog_client->address_string, 0, sizeof(syslog_client->address_string));
		os_memcpy(syslog_client->address_string, address_string, MIN(os_strlen(address_string), DFT_HOST_NAME_LEN));
	}
	if (syslogcShutdown(syslog_client) == OK)
	{
		if (syslogc_socket_init(syslog_client) != ERROR)
		{
			fflush(stdout);
			if (syslog_client->mutx)
				os_mutex_unlock(syslog_client->mutx);
			return OK;
		}
		fflush(stdout);
	}
	if (syslog_client->mutx)
		os_mutex_unlock(syslog_client->mutx);
	return OK;
}

int syslogc_host_config_get(char *hostname, zpl_uint16 *port, zpl_uint32 *facility)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->mutx)
		os_mutex_lock(syslog_client->mutx, OS_WAIT_FOREVER);
	if (port)
		*port = syslog_client->port;
	if (hostname && os_strlen(syslog_client->address_string) > 1)
		os_strncpy(hostname, syslog_client->address_string,
				   MIN(os_strlen(syslog_client->address_string), DFT_HOST_NAME_LEN));
	if (facility)
		*facility = syslog_client->facility;
	if (syslog_client->mutx)
		os_mutex_unlock(syslog_client->mutx);
	return OK;
}

int syslogc_mode_set(zpl_uint32 mode)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->enable == zpl_false)
		return ERROR;

	if (syslog_client->mode == mode)
		return OK;
	if (syslog_client->mutx)
		os_mutex_lock(syslog_client->mutx, OS_WAIT_FOREVER);
	syslog_client->mode = mode;
	if (syslogcShutdown(syslog_client) == OK)
	{
		if (syslogc_socket_init(syslog_client) != ERROR)
		{
			if (syslog_client->mutx)
				os_mutex_unlock(syslog_client->mutx);
			fflush(stdout);
			return OK;
		}
		fflush(stdout);
	}
	if (syslog_client->mutx)
		os_mutex_unlock(syslog_client->mutx);
	return OK;
}

int syslogc_mode_get(zpl_uint32 *mode)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->enable == zpl_false)
		return ERROR;
	if (syslog_client->mutx)
		os_mutex_lock(syslog_client->mutx, OS_WAIT_FOREVER);
	if (mode)
		*mode = syslog_client->mode;
	if (syslog_client->mutx)
		os_mutex_unlock(syslog_client->mutx);
	return OK;
}

int syslogc_facility_set(zpl_uint32 facility)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->enable == zpl_false)
		return ERROR;
	if (syslog_client->mutx)
		os_mutex_lock(syslog_client->mutx, OS_WAIT_FOREVER);
	syslog_client->facility = facility;
	if (syslog_client->mutx)
		os_mutex_unlock(syslog_client->mutx);
	return OK;
}

int syslogc_facility_get(zpl_uint32 *facility)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->enable == zpl_false)
		return ERROR;
	if (syslog_client->mutx)
		os_mutex_lock(syslog_client->mutx, OS_WAIT_FOREVER);
	if (facility)
		*facility = syslog_client->facility;
	if (syslog_client->mutx)
		os_mutex_unlock(syslog_client->mutx);
	return OK;
}

static int syslogc_format(struct syslog_client *client, zpl_uint32 pri, char *pStr, zpl_uint32 len)
{
	zpl_uint16 code = 0;
	if (!client)
		return ERROR;
	code = ((client->facility & 0xff) << 5);
	code |= (pri & 0xff);

	return syslogcStringSend(client, pStr,
							 code, client->address.s_addr);
}

int vsysclog(zpl_uint32 priority, zpl_uint32 facility, char *format, va_list arglist)
{
	zpl_uint32 len = 0;
	char log_msg[SYSLOG_MSG_MAX_LEN];
	va_list ac;
	if (!syslog_client)
		return ERROR;
	if (syslog_client->enable == zpl_false)
		return ERROR;
	syslog_client->facility = facility;
	va_copy(ac, arglist);
	len = vsnprintf(log_msg, SYSLOG_MSG_MAX_LEN, format, ac);
	// vfprintf (stdout, format, ac);
	syslogc_format(syslog_client, priority, log_msg, len);
	va_end(ac);
	return OK;
}

int syslogc_out(zpl_uint32 priority, zpl_uint32 facility, char *pStr, zpl_uint32 len)
{
	if (!syslog_client)
		return ERROR;
	if (syslog_client->enable == zpl_false)
		return ERROR;
	if (pStr == NULL)
		return ERROR;
	if (syslog_client->ipstack_connect == zpl_false)
		return ERROR;
	syslog_client->facility = facility;
	return syslogc_format(syslog_client, priority, pStr, len);
}
#endif
