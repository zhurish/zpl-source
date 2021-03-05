/* tftpLib.c - Trivial File Transfer Protocol (TFTP) client library */

/* Copyright 1984 - 2001 Wind River Systems, Inc. */


#include "zebra.h"
#include "tftpLib.h"
/*
 modification history
 --------------------
 02o,10may02,kbw  making man page edits
 02n,15oct01,rae  merge from truestack ver 02v, base 02m (SPRs 65595,
 33975, 32821/31223, 23051, 5515, etc.)
 02m,15mar99,elg  fix errors in sample code in tftpXfer refman (SPR 8557).
 02l,30dec98,ead  Commented out the erroneous calls to unlink() in tftpGet()
 and tftpPut().  Included systools_printf() calls. (SPR #23051)
 02k,26aug97,spm  removed compiler warnings (SPR #7866)
 02j,23oct96,sgv  Added code for file truncation when file transfer is aborted
 SPR #6839
 02i,16oct96,dgp  doc: modify tftpCopy() example per SPR 7226
 02h,09sep94,jmm  removed include of xdr_nfs.h
 02g,11aug93,jmm  Changed ioctl.h and socket.h to sys/ioctl.h and sys/socket.h
 02f,21sep92,jdi  documentation cleanup.
 02f,21aug92,rrr  The 02e change nuke a previous 02e change, put that back in.
 It was changed systime.h to sys/times.h
 02e,14aug92,elh  documentation changes.
 01d,04jun92,ajm  tftpErrorCreate forward declared in tftpLib.h
 01c,26may92,rrr  the tree shuffle
 -changed includes to have absolute path from h/
 01b,01mar92,elh  ansified.
 incorperated server changes by jmm.
 01a,10jun91,elh  written.
 */

/*
 DESCRIPTION
 This library implements the VxWorks Trivial File Transfer Protocol (TFTP)
 client library.  TFTP is a simple file transfer protocol (hence the name
 "trivial") implemented over UDP.  TFTP was designed to be small and easy to
 implement. Therefore, it is limited in functionality in comparison with other
 file transfer protocols, such as FTP.  TFTP provides only the read/write
 capability to and from a remote server.

 TFTP provides no user authentication. Therefore, the remote files must have
 "loose" permissions before requests for file access will be granted by the
 remote TFTP server. This means that the files to be read must be publicly
 readable, and files to be written must exist and be publicly writable).
 Some TFTP servers offer a secure option (-s) that specifies a directory
 where the TFTP server is rooted.  Refer to the host manuals for more
 information about a particular TFTP server.

 HIGH-LEVEL INTERFACE
 The tftpLib library has two levels of interface.  The tasks tftpXfer()
 and tftpCopy() operate at the highest level and are the main call
 interfaces.  The tftpXfer() routine provides a stream interface to TFTP.
 That is, it spawns a task to perform the TFTP transfer and provides a
 descriptor from which data can be transferred interactively.  The
 tftpXfer() interface is similar to ftpXfer() in ftpLib.  The tftpCopy()
 routine transfers a remote file to or from a passed file (descriptor).

 LOW-LEVEL INTERFACE
 The lower-level interface is made up of various routines that act on a TFTP
 session.  Each TFTP session is defined by a TFTP descriptor.   These routines
 include:

 tftpInit() to initialize a session;
 tftpModeSet() to set the transfer mode;
 tftpPeerSet() to set a peer/server address;
 tftpPut() to put a file to the remote system;
 tftpGet() to get file from remote system;
 tftpInfoShow() to show status information; and
 tftpQuit() to quit a TFTP session.

 EXAMPLE
 The following code provides an example of how to use the lower-level
 routines.  It implements roughly the same function as tftpCopy().

 .CS
 char *         pHost;
 int            port;
 char *         pFilename;
 char *         pCommand;
 char *         pMode;
 int            fd;
 TFTP_DESC *    pTftpDesc;
 int            status;

 if ((pTftpDesc = tftpInit ()) == NULL)
 return (ERROR);

 if ((tftpPeerSet (pTftpDesc, pHost, port) == ERROR) ||
 (tftpModeSet (pTftpDesc, pMode) == ERROR))
 {
 (void) tftpQuit (pTftpDesc);
 return (ERROR);
 }

 if (strcmp (pCommand, "get") == 0)
 {
 status = tftpGet (pTftpDesc, pFilename, fd, TFTP_CLIENT);
 }
 else if (strcmp (pCommand, "put") == 0)
 {
 status =  tftpPut (pTftpDesc, pFilename, fd, TFTP_CLIENT);
 }

 else
 {
 errno = ECOMM;
 status = ERROR;
 }

 (void) tftpQuit (pTftpDesc);
 .CE

 To use this feature, include the following component:
 INCLUDE_TFTP_CLIENT

 INCLUDE FILES: tftpLib.h

 SEE ALSO: tftpdLib

 INTERNAL
 The diagram below outlines the structure of tftpLib.


 tftpXfer--> ) netJobQ ( -->tftpChildTaskSpawn
 |
 v
 tftpTask
 |   |
 v   v
 ______________________tftpCopy__________________
 /	         ________/ |     \ \                 \	    v
 |           /          |      | \_____           |	    |
 v          v           v      v       \          |	    v
 tftpInit tftpModeSet tftpPeerSet tftpQuit |          |  tftpInfoShow
 v          v
 ______tftpPut___  __tftpGet_
 |     /    __|___\/___|  |   |
 v    |    |  |   /\      |   v
 fileToAscii v    v  |   | \     | asciiToFile
 tftpErrorCreate|   |  v    v
 v   v tftpRequestCreate
 tftpSend
 v
 tftpPacketTrace
 */

/* includes */

#include "vty.h"

/*
#include "errno.h"
#include "sys/times.h"
#include "string.h"
#include "unistd.h"
#include "stdio.h"
#include "sys/socket.h"

#include "dirent.h"
*/
#include "systools.h"

/* globals */
//#define TFTPC_DEBUG

ospl_bool tftpDebug = ospl_true; /* debug mode		*/
static ospl_bool tftpVerbose = ospl_true; /* verbose mode		*/
static ospl_bool tftpTrace = ospl_false; /* packet tracing (debug detail) 	*/

static int tftpTimeout = TFTP_TIMEOUT * 5; /* total timeout (sec.)	*/
static int tftpReXmit = TFTP_TIMEOUT; /* rexmit value  (sec.) */


/* tftp task parameters */
typedef struct				/* TFTP parms for tftpTask */
{
    char *		pHost;		/* host name or address */
    int			port;		/* port number		*/
    char *		pFname;		/* remote filename 	*/
    char *		pCmd;		/* TFTP command 	*/
    char *		pMode;		/* TFTP transfer mode 	*/
    int			dSock;
    int			eSock;
    int			value;
} TFTPPARAM;
//static int 	tftpTaskPriority = TFTP_TASK_PRIORITY;	/* tftp task priority   */
//static int 	tftpTaskOptions	 = TFTP_TASK_OPTIONS;	/* tftp task options    */
//static int 	tftpTaskStackSize= TFTP_TASK_STACKSIZE; /* tftp task stack size */
/* forward declarations */

/* locals */
static int fileToAscii(int fd, /* file descriptor	*/
		char * pBuffer, /* buffer 		*/
		int bufLen, /* buffer length 	*/
		char * charConv /* char being converted */
);

static int asciiToFile(int fd, /* file descriptor 	*/
		char * pBuffer, /* buffer 		*/
		int bufLen, /* buffer length	*/
		char * charConv, /* char being converted */
		ospl_bool isLastBuff /* last buffer?		*/
);

static int tftpRequestCreate(TFTP_MSG * pTftpMsg, /* TFTP message pointer	*/
		int opCode, /* request opCode 	*/
		char * pFilename, /* remote filename 	*/
		char * pMode /* TFTP transfer mode	*/
);

static void tftpPacketTrace(char * pMsg, /* diagnostic message 	*/
		TFTP_MSG * pTftpMsg, /* TFTP packet 		*/
		int size /* packet size 		*/
);

static int tftpTask(TFTPPARAM *		pParam			/* param need to cleanup*/
);

static int 	tftpModeSet (TFTP_DESC * pTftpDesc, char * pMode);
static int 	tftpPeerSet (TFTP_DESC * pTftpDesc, char * pHostname,
			     int port);

static int 	tftpInfoShow (TFTP_DESC * pTftpDesc);


static void tftpParamCleanUp(TFTPPARAM * pParam);
/*static void tftpChildTaskSpawn (int priority, int options, int stackSize,
 TFTPPARAM * pParam);*/

/*******************************************************************************
 *
 * tftpXfer - transfer a file via TFTP using a stream interface
 *
 * This routine initiates a transfer to or from a remote file via TFTP.  It
 * spawns a task to perform the TFTP transfer and returns a descriptor from
 * which the data can be read (for "get") or to which it can be written (for
 * "put") interactively.  The interface for this routine is similar to ftpXfer()
 * in ftpLib.
 *
 * <pHost> is the server name or Internet address.  A non-zero value for <port>
 * specifies an alternate TFTP server port number (zero means use default TFTP
 * port number (69)).  <pFilename> is the remote filename. <pCommand> specifies
 * the TFTP command.  The command can be either "put" or "get".
 *
 * The tftpXfer() routine returns a data descriptor, in <pDataDesc>, from
 * which the TFTP data is read (for "get") or to which is it is written (for
 * "put").  An error status descriptor is returned in the variable
 * <pErrorDesc>.  If an error occurs during the TFTP transfer, an error
 * string can be read from this descriptor.  After returning successfully
 * from tftpXfer(), the calling application is responsible for closing both
 * descriptors.
 *
 * If there are delays in reading or writing the data descriptor, it is
 * possible for the TFTP transfer to time out.
 *
 * INTERNAL
 * tftpXfer() uses stream sockets connected over the loopback address for
 * communication between the calling application and the task performing
 * the tftpCopy.  It might be desirable to use some alternate method so TCP
 * is no longer necessary for this module.
 *
 * EXAMPLE
 * The following code demonstrates how tftpXfer() may be used:
 *
 * .CS
 *     #include "tftpLib.h"
 *
 *     #define BUFFERSIZE	512
 *
 *     int  dataFd;
 *     int  errorFd;
 *     int  num;
 *     char buf [BUFFERSIZE + 1];
 *
 *     if (tftpXfer ("congo", 0, "/usr/fred", "get", "ascii", &dataFd,
 *                   &errorFd) == ERROR)
 *         return (ERROR);
 *
 *     while ((num = read (dataFd, buf, sizeof (buf))) > 0)
 *         {
 *         ....
 *         }
 *
 *     close (dataFd);
 *
 *     num = read (errorFd, buf, BUFFERSIZE);
 *     if (num > 0)
 *         {
 *         buf [num] = '\0';
 *         printf ("YIKES! An error occurred!:%s\en", buf);
 *         .....
 *         }
 *
 *     close (errorFd);
 * .CE
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * ERRNO
 * EINVAL
 *
 * SEE ALSO: ftpLib
 */

int tftpXfer(char * pHost, /* host name or address */
		int port, /* port number		*/
		char * pFilename, /* remote filename 	*/
		char * pCommand, /* TFTP command 	*/
		char * pMode, /* TFTP transfer mode 	*/
		int pDataDesc, /* data desc.	*/
		int value /* value desc.	*/
)
{
	TFTPPARAM * pParam = NULL;
	if (pHost == NULL || pFilename == NULL || pCommand == NULL
			|| pMode == NULL || pDataDesc == 0 /*|| pErrorDesc == NULL*/)
	{
		errno = EINVAL;
		return (ERROR);
	}

	/* setup TFTP parameters */
	if ((pParam = malloc(sizeof(TFTPPARAM))) == NULL)
	{
		return (ERROR);
	}

	bzero((char *) pParam, sizeof(TFTPPARAM));

	pParam->dSock = pDataDesc;
	pParam->value = value;
	pParam->port = port;

	if ((pParam->pHost = strdup(pHost)) == NULL)
	{
		tftpParamCleanUp(pParam);
		return (ERROR);
	}
	if ((pParam->pFname = strdup(pFilename)) == NULL)
	{
		tftpParamCleanUp(pParam);
		return (ERROR);
	}
	if ((pParam->pCmd = strdup(pCommand)) == NULL)
	{
		tftpParamCleanUp(pParam);
		return (ERROR);
	}
	if ((pParam->pMode = strdup(pMode)) == NULL)
	{
		tftpParamCleanUp(pParam);
		return (ERROR);
	}
/*	bzero(pParam->pMode, strlen(pMode) + 1);
	strcpy(pParam->pHost, pHost);
	strcpy(pParam->pFname, pFilename);
	strcpy(pParam->pCmd, pCommand);
	strcpy(pParam->pMode, pMode);*/

	/* spawn tftpTask via netJobQueue to be callable in user PD      */
	/* closing sockets/freeing TFTPPARM are done by tftpParamCleanUp */
	/*    if (netJobAdd ((FUNCPTR)tftpChildTaskSpawn, tftpTaskPriority,
	 tftpTaskOptions, tftpTaskStackSize, (int)pParam,0) == ERROR)*/
	//if (os_job_add(tftpTask, pParam) == ERROR)
	if(tftpTask(pParam) == ERROR)
	{
		//systools_error("tftp transfer failed: %s", strerror(errno));
		tftpParamCleanUp(pParam);
		pParam = NULL;
		return (ERROR);
	}
	tftpParamCleanUp (pParam);
	pParam = NULL;
	return (OK);
}

/*******************************************************************************
 *
 * tftpTask - perform a TFTP file transfer
 *
 * This task calls tftpCopy() to perform a TFTP file transfer.  If the transfer
 * fails, it writes an error status message to <errorFd>.  tftpTask closes
 * both <errorFd> and <fd> on completion.
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * NOMANUAL
 */

static int tftpTask(TFTPPARAM * pParam /* param need to cleanup*/
)

{
	int status;
	if ((status = tftpCopy(pParam->pHost, pParam->port, pParam->pFname,
			pParam->pCmd, pParam->pMode, pParam->dSock)) == ERROR)
	{
		//systools_error("tftp transfer failed: %s", strerror(errno));
	}
	//(pParam); /* clean tftp params up here */
	return (status);
}

/*******************************************************************************
 *
 * tftpCopy - transfer a file via TFTP
 *
 * This routine transfers a file using the TFTP protocol to or from a remote
 * system.
 * <pHost> is the remote server name or Internet address.  A non-zero value
 * for <port> specifies an alternate TFTP server port (zero means use default
 * TFTP port number (69)).  <pFilename> is the remote filename. <pCommand>
 * specifies the TFTP command, which can be either "put" or "get".
 * <pMode> specifies the mode of transfer, which can be "ascii",
 * "netascii",  "binary", "image", or "octet".
 *
 * <fd> is a file descriptor from which to read/write the data from or to
 * the remote system.  For example, if the command is "get", the remote data
 * will be written to <fd>.  If the command is "put", the data to be sent is
 * read from <fd>.  The caller is responsible for managing <fd>.  That is, <fd>
 * must be opened prior to calling tftpCopy() and closed up on completion.
 *
 * EXAMPLE
 * The following sequence gets an ASCII file "/folk/vw/xx.yy" on host
 * "congo" and stores it to a local file called "localfile":
 *
 * .CS
 *     -> fd = open ("localfile", 0x201, 0644)
 *     -> tftpCopy ("congo", 0, "/folk/vw/xx.yy", "get", "ascii", fd)
 *     -> close (fd)
 * .CE
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * ERRNO
 * ECOMM
 *
 * SEE ALSO: ftpLib
 */

int tftpCopy(char * pHost, /* host name or address	*/
		int port, /* optional port number	*/
		char * pFilename, /* remote filename 	*/
		char * pCommand, /* TFTP command 	*/
		char * pMode, /* TFTP transfer mode 	*/
		int fd /* fd to put/get data   */
)

{
	TFTP_DESC * pTftpDesc = NULL; /* TFTP descriptor	*/
	int status = 0; /* return status 	*/

	if ((pTftpDesc = tftpInit()) == NULL) /* get tftp descriptor  */
		return (ERROR);
	/* set peer and mode */

	if ((tftpPeerSet(pTftpDesc, pHost, port) == ERROR)
			|| ((pMode != NULL) && (tftpModeSet(pTftpDesc, pMode) == ERROR)))
	{
		(void) tftpQuit(pTftpDesc);
		return (ERROR);
	}

	if (strcmp(pCommand, "get") == 0) /* get or put the file */
	{
		pTftpDesc->tftp_start_time = os_time(NULL);
		status = tftpGet(pTftpDesc, pFilename, fd, TFTP_CLIENT);
	}

	else if (strcmp(pCommand, "put") == 0)
	{
		pTftpDesc->tftp_start_time = os_time(NULL);
		status = tftpPut(pTftpDesc, pFilename, fd, TFTP_CLIENT);
	}

	else
	{
		errno = ECOMM;
		status = ERROR;
	}
	if(status == ERROR)
	{
		systools_printf("TFTP CMD is not define.");
	}
	else
		tftpInfoShow(pTftpDesc);
	(void) tftpQuit(pTftpDesc); /* close tftp session */
	return (status);
}

/*******************************************************************************
 *
 * tftpInit - initialize a TFTP session
 *
 * This routine initializes a TFTP session by allocating and initializing a TFTP
 * descriptor.  It sets the default transfer mode to "netascii".
 *
 * RETURNS: A pointer to a TFTP descriptor if successful, otherwise NULL.
 */

TFTP_DESC * tftpInit(void)
{
	TFTP_DESC * pTftpDesc; /* TFTP descriptor     */
	struct sockaddr_in localAddr; /* local address       */

	if ((pTftpDesc = (TFTP_DESC *) calloc(1, sizeof(TFTP_DESC))) == NULL)
		return (NULL);
	/* default mode is netascii */

	strcpy(pTftpDesc->mode, "netascii");
	pTftpDesc->connected = ospl_false;
	/* set up a datagram socket */

	if ((pTftpDesc->sock = socket(AF_INET, SOCK_DGRAM, 0)) == ERROR)
	{
		free((char *) pTftpDesc);
		systools_error("TFTP  socket create failed: %s", strerror(errno));
		return (NULL);
	}

	bzero((char *) &localAddr, sizeof(struct sockaddr_in));
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons((ospl_ushort) 0);

	if (bind(pTftpDesc->sock, (const struct sockaddr *) &localAddr,
			sizeof(struct sockaddr_in)) == ERROR)
	{
		systools_error("TFTP  socket bind failed: %s", strerror(errno));
		free((char *) pTftpDesc);
		close(pTftpDesc->sock);
		return (NULL);
	}

	return (pTftpDesc); /* return descriptor */
}

/*******************************************************************************
 *
 * tftpModeSet - set the TFTP transfer mode
 *
 * This routine sets the transfer mode associated with the TFTP descriptor
 * <pTftpDesc>.  <pMode> specifies the transfer mode, which can be
 * "netascii", "binary", "image", or "octet".  Although recognized, these
 * modes actually translate into either octet or netascii.
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * ERRNO
 *  EINVAL
 *  EINVAL
 *  S_tftpLib_INVALID_MODE
 */

static int tftpModeSet(TFTP_DESC * pTftpDesc, /* TFTP descriptor 	*/
		char * pMode /* TFTP transfer mode  	*/
)

{
#ifdef TFTPC_DEBUG
	char * sep = " "; /* separator character	*/
#endif

	TFTP_MODE * pTftpModes; /* pointer to modes	*/
	static TFTP_MODE tftpModes[] = /* available modes	*/
	{
	{ "ascii", "netascii" },
	{ "netascii", "netascii" },
	{ "binary", "octet" },
	{ "image", "octet" },
	{ "octet", "octet" },
	{ NULL, NULL } };

	if (pTftpDesc == NULL) /* validate arguments */
	{
		errno = EINVAL;
		return (ERROR);
	}

	if (pMode == NULL)
	{
		errno = EINVAL;
		return (ERROR);
	}

	/* look for actual mode type from the passed mode */

	for (pTftpModes = tftpModes; pTftpModes->m_name != NULL; pTftpModes++)
	{
		if (strcmp(pMode, pTftpModes->m_name) == 0)
		{
			bzero(pTftpDesc->mode, sizeof(pTftpDesc->mode));
			strcpy(pTftpDesc->mode, pTftpModes->m_mode);
			if (tftpDebug)
				systools_debug("TFTP mode set to %s.\r\n", pTftpDesc->mode);
			return (OK);
		}
	}
	/* invalid mode, print error */
	//errno = S_tftpLib_INVALID_MODE;
#ifdef TFTPC_DEBUG
	systools_printf("%s: unknown mode\r\n", pMode);
	systools_printf("valid modes: [");

	for (pTftpModes = tftpModes; pTftpModes->m_name != NULL; pTftpModes++)
	{
		systools_printf("%s%s", sep, pTftpModes->m_name);
		if (*sep == ' ')
			sep = " | ";
	}
	systools_printf(" ]\r\n");
#endif

	return (ERROR);
}

/*******************************************************************************
 *
 * tftpPeerSet - set the TFTP server address
 *
 * This routine sets the TFTP server (peer) address associated with the TFTP
 * descriptor <pTftpDesc>.  <pHostname> is either the TFTP server name
 * (e.g., "congo") or the server Internet address (e.g., "90.3").  A non-zero
 * value for <port> specifies the server port number (zero means use
 * the default TFTP server port number (69)).
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * ERRNO
 *  EINVAL
 *  EINVAL
 *  EHOSTUNREACH
 */

static int tftpPeerSet(TFTP_DESC * pTftpDesc, /* TFTP descriptor	*/
		char * pHostname, /* server name/address	*/
		int port /* port number		*/
)
{
	struct hostent * hoste = NULL;
	if (pTftpDesc == NULL) /* validate arguments */
	{
		errno = EINVAL;
		return (ERROR);
	}

	if ((port < 0) || (pHostname == NULL))
	{
		errno = EINVAL;
		return (ERROR);
	}

	bzero((char *) &pTftpDesc->serverAddr, sizeof(struct sockaddr_in));
	pTftpDesc->serverAddr.sin_family = AF_INET;

    if(inet_aton (pHostname, &pTftpDesc->serverAddr.sin_addr) == 0)
    {
    	/* get server address */
    	hoste = gethostbyname(pHostname);
    	if (hoste && hoste->h_addr_list[0])
    	{
    		pTftpDesc->serverAddr.sin_addr.s_addr = hoste->h_addr_list[0];
    	}
    	else
    	{
			pTftpDesc->connected = ospl_false;

			systools_error("TFTP %s: unknown host.", pHostname);
#ifdef TFTPC_DEBUG
			systools_printf("tftpPeerSet %s: unknown host.\r\n", pHostname);
#endif
			errno = EHOSTUNREACH;
			return (ERROR);
    	}
    }

	/* set port and hostname */
	bzero(pTftpDesc->serverName, sizeof(pTftpDesc->serverName));
	strcpy(pTftpDesc->serverName, pHostname);
	pTftpDesc->serverPort =
			(port != 0) ? htons((ospl_ushort) port) : htons((ospl_ushort) TFTP_PORT);
	pTftpDesc->connected = ospl_true;

	if (tftpDebug)
		systools_debug("TFTP Connected to %s [%d]", pHostname,
				ntohs(pTftpDesc->serverPort));
	return (OK);
}

/*******************************************************************************
 *
 * tftpPut - put a file to a remote system
 *
 * This routine puts data from a local file (descriptor) to a file on the remote
 * system.  <pTftpDesc> is a pointer to the TFTP descriptor.  <pFilename> is
 * the remote filename.  <fd> is the file descriptor from which it gets the
 * data.  A call to tftpPeerSet() must be made prior to calling this routine.
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * ERRNO
 *  EINVAL
 *  EINVAL
 *  ENOTCONN
 */

int tftpPut(TFTP_DESC * pTftpDesc, /* TFTP descriptor       */
		char * pFilename, /* remote filename       */
		int fd, /* file descriptor       */
		int clientOrServer /* which side is calling */
)

{
	int size = 0; /* size of data to send */
	TFTP_MSG tftpMsg; /* TFTP message         */
	TFTP_MSG tftpAck; /* TFTP ack message     */
	int port = 0; /* return port number   */
	int block = 0; /* data block number    */
	ospl_bool convert = ospl_false; /* convert to ascii	*/
	char charTemp = 0; /* temp char holder	*/
	int tftpwli = 0, wlicnt = 0;
	if (pTftpDesc == NULL) /* validate arguments */
	{
		errno = EINVAL;
		return (ERROR);
	}

	if ((fd < 0) || (pFilename == NULL))
	{
		errno = EINVAL;
		return (ERROR);
	}
	pTftpDesc->tftp_size = 0;
	if (!pTftpDesc->connected) /* must be connected */
	{
		systools_error("TFTP  PUT No target machine specified.");
#ifdef TFTPC_DEBUG
		systools_printf("tftpPut: No target machine specified.\r\n");
#endif
		errno = ENOTCONN;
		return (ERROR);
	}
	/* initialize variables */
	convert = (strcmp(pTftpDesc->mode, "netascii") == 0) ? ospl_true : ospl_false;
	charTemp = '\0';
	bzero((char *) &tftpMsg, sizeof(TFTP_MSG));
	bzero((char *) &tftpAck, sizeof(TFTP_MSG));
	pTftpDesc->serverAddr.sin_port = pTftpDesc->serverPort;
	block = 0;

	if (tftpDebug)
		systools_debug("TFTP putting to %s:%s [%s]", pTftpDesc->serverName,
				pFilename, pTftpDesc->mode);

	/*
	 *  If we're a client, then
	 *  create and send write request message.  Get ACK back with
	 *  block 0, and a new port number (TID).  If we're a server,
	 *  this isn't necessary.
	 */

	if (clientOrServer == TFTP_CLIENT)
	{
		size = tftpRequestCreate(&tftpMsg, TFTP_WRQ, pFilename,
				pTftpDesc->mode);
		if (tftpDebug)
			systools_debug("TFTP Sending WRQ to port %d", pTftpDesc->serverPort);

		if ((tftpSend(pTftpDesc, &tftpMsg, size, &tftpAck, TFTP_ACK, block,
				&port) == ERROR))
			return (ERROR);

		pTftpDesc->serverAddr.sin_port = port;
	}

	do
	{
		/*
		 *  Read data from file - converting to ascii if necessary.
		 *  All data packets, except the last, must have 512 bytes of data.
		 *  The TFTP server sees packets with less than 512 bytes of
		 *  data as end of transfer.
		 */
		if (convert)
			size = fileToAscii(fd, tftpMsg.th_data, TFTP_SEGSIZE, &charTemp);
		else
			size = read/*fioRead*/(fd, tftpMsg.th_data, TFTP_SEGSIZE);

		if (size == ERROR) /* if error, send message and bail */
		{
			size = tftpErrorCreate(&tftpMsg, EUNDEF);
			(void) tftpSend(pTftpDesc, &tftpMsg, size, (TFTP_MSG *) NULL, 0, 0,
					(int *) NULL);
			close(fd);
			systools_error("TFTP PUT: Error occurred while reading the file: %s", strerror(errno));
#ifdef TFTPC_DEBUG
			systools_printf("tftpPut: Error occurred while reading the file.\r\n");
#endif

			return (ERROR);
		}

		/* send data message and get an ACK back with same block no. */

		block++;
		tftpMsg.th_opcode = htons((ospl_ushort) TFTP_DATA);
		tftpMsg.th_block = htons((ospl_ushort) block);

		if (tftpSend(pTftpDesc, &tftpMsg, size + TFTP_DATA_HDR_SIZE, &tftpAck,
				TFTP_ACK, block, (int *) NULL) == ERROR)
			return (ERROR);

		tftpwli++;
		pTftpDesc->tftp_size += size;
		wlicnt ++;

		if(tftpVerbose && (wlicnt & 0xff) == 0)
		{
			tftpwli++;
			if(tftpwli%80 == 0)
				systools_printf("#\r\n");
			else
				systools_printf("#");
		}

	} while (size == TFTP_SEGSIZE);
	if(tftpVerbose)
		systools_printf("\r\n");
	return (OK);
}

/*******************************************************************************
 *
 * tftpGet - get a file from a remote system
 *
 * This routine gets a file from a remote system via TFTP.  <pFilename> is the
 * filename.  <fd> is the file descriptor to which the data is written.
 * <pTftpDesc> is a pointer to the TFTP descriptor.  The tftpPeerSet() routine
 * must be called prior to calling this routine.
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * ERRNO
 *  EINVAL
 *  EINVAL
 *  ENOTCONN
 */

int tftpGet(TFTP_DESC * pTftpDesc, /* TFTP descriptor       */
		char * pFilename, /* remote filename       */
		int fd, /* file descriptor       */
		int clientOrServer /* which side is calling */
)

{
	TFTP_MSG tftpMsg; /* TFTP message		*/
	TFTP_MSG tftpReply; /* TFTP DATA		*/
	int sizeMsg; /* size to send		*/
	int port; /* return port		*/
	int * pPort; /* port pointer		*/
	int block; /* block expected	*/
	char * pBuffer; /* pointer to buffer	*/
	ospl_bool convert; /* convert from ascii 	*/
	int numBytes; /* number of bytes 	*/
	int sizeReply; /* number of data bytes */
	char charTemp; /* temp char holder 	*/
	int errorVal; /* error value 		*/
	int tftpwli = 0, wlicnt= 0;
	if (pTftpDesc == NULL) /* validate arguments */
	{
		errno = EINVAL;
		return (ERROR);
	}

	if ((fd < 0) || (pFilename == NULL))
	{
		errno = EINVAL;
		return (ERROR);
	}

	if (!pTftpDesc->connected) /* return if not connected  */
	{
#ifdef TFTPC_DEBUG
		systools_printf("tftpGet: No target machine specified.\r\n");
#endif

		errno = ENOTCONN;
		return (ERROR);
	}
	pTftpDesc->tftp_size = 0;
	/* initialize variables */
	bzero((char *) &tftpMsg, sizeof(TFTP_MSG));
	bzero((char *) &tftpReply, sizeof(TFTP_MSG));
	pPort = &port;
	convert = (strcmp(pTftpDesc->mode, "netascii") == 0) ? ospl_true : ospl_false;
	charTemp = '\0';
	pTftpDesc->serverAddr.sin_port = pTftpDesc->serverPort;
	block = 1;

	if (tftpDebug)
		systools_debug("TFTP getting from %s:%s [%s]", pTftpDesc->serverName,
				pFilename, pTftpDesc->mode);

	/*
	 * If we're a server, then the first message is an ACK with block = 0.
	 * If we're a client, then it's an RRQ.
	 */

	if (clientOrServer == TFTP_SERVER)
	{
		tftpMsg.th_opcode = htons((ospl_ushort) TFTP_ACK);
		tftpMsg.th_block = htons((ospl_ushort) (0));
		sizeMsg = TFTP_ACK_SIZE;
	}
	else
	{

		/* formulate a RRQ message */

		sizeMsg = tftpRequestCreate(&tftpMsg, TFTP_RRQ, pFilename,
				pTftpDesc->mode);
	}

	while (1)

	{

		/* send the RRQ or ACK - expect back DATA */

		sizeReply = tftpSend(pTftpDesc, &tftpMsg, sizeMsg, &tftpReply,
				TFTP_DATA, block, pPort);

		if (sizeReply == ERROR) /* if error then bail */
		{
			systools_error("TFTP GET Error occurred while transferring the file: %s", strerror(errno));
#ifdef TFTPC_DEBUG
			systools_printf(
					"tftpGet: Error occurred while transferring the file.\r\n");
#endif
			return (ERROR);
		}

		sizeReply -= TFTP_DATA_HDR_SIZE; /* calculate data size */

		/*
		 *  The first block returns a new server port number (TID) to use.
		 *  From now on, we use this port and tell tftpSend to validate it.
		 */

		if (block == 1)
		{
			pTftpDesc->serverAddr.sin_port = port;
			pPort = NULL;
		}

		/* write data to file, converting to ascii if necessary */

		if (convert)
			errorVal = asciiToFile(fd, tftpReply.th_data, sizeReply, &charTemp,
					sizeReply < TFTP_SEGSIZE);
		else
		{
			int bytesWritten;

			for (bytesWritten = 0, pBuffer = tftpReply.th_data, errorVal = OK;
					bytesWritten < sizeReply;
					bytesWritten += numBytes, pBuffer += numBytes)

			{
				if ((numBytes = write(fd, pBuffer, sizeReply - bytesWritten))
						<= 0)
				{
					systools_error("TFTP GET Error occurred while writing to the file: %s", strerror(errno));
					errorVal = ERROR;
					break;
				}
			}
		}

		if (errorVal == ERROR) /* if error send message and bail */
		{
			sizeMsg = tftpErrorCreate(&tftpMsg, EUNDEF);
			(void) tftpSend(pTftpDesc, &tftpMsg, sizeMsg, (TFTP_MSG *) NULL, 0,
					0, (int *) NULL);
			systools_error("TFTP GET Error occurred while writing the file: %s", strerror(errno));
#ifdef TFTPC_DEBUG
			systools_printf("tftpGet: Error occurred while writing the file.\r\n");
#endif
			return (ERROR);
		}

		pTftpDesc->tftp_size += sizeReply;
		wlicnt ++;

		if(tftpVerbose && (wlicnt & 0xff) == 0)
		{
			tftpwli++;
			if(tftpwli%80 == 0)
				systools_printf("#\r\n");
			else
				systools_printf("#");
		}
		/* create ACK message */

		tftpMsg.th_opcode = htons((ospl_ushort) TFTP_ACK);
		tftpMsg.th_block = htons((ospl_ushort) (block));
		sizeMsg = TFTP_ACK_SIZE;

		/*
		 * if last packet received was less than 512 bytes then end
		 * of transfer. Send final ACK, then we're outta here.
		 */

		if (sizeReply < TFTP_SEGSIZE)
		{
			(void) tftpSend(pTftpDesc, &tftpMsg, sizeMsg, (TFTP_MSG *) NULL, 0,
					0, (int *) NULL);

			if(tftpVerbose)
			{
				systools_printf("\r\n");
			}
			return (OK);
		}

		block++;
	} /* end forever loop */
	return ERROR;
}

/*******************************************************************************
 *
 * tftpInfoShow - get TFTP status information
 *
 * This routine prints information associated with TFTP descriptor <pTftpDesc>.
 *
 * EXAMPLE
 * A call to tftpInfoShow() might look like:
 *
 * .CS
 *     -> tftpInfoShow (tftpDesc)
 *            Connected to yuba [69]
 *            Mode: netascii  Verbose: off  Tracing: off
 *            Rexmt-interval: 5 seconds, Max-timeout: 25 seconds
 *     value = 0 = 0x0
 *     ->
 * .CE
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * ERRNO
 * EINVAL
 */

static int tftpInfoShow(TFTP_DESC * pTftpDesc /* TFTP descriptor */
)
{
	int tftp_end_time = os_time(NULL);
	if (pTftpDesc == NULL)
	{
		errno = EINVAL;
		return (ERROR);
	}

	systools_printf("TFTP Transfer size : %s in %d seconds to %s\r\n",
			os_file_size_string(pTftpDesc->tftp_size),
			tftp_end_time - pTftpDesc->tftp_start_time,
			pTftpDesc->serverName);

/*    int	tftp_start_time;

	if (pTftpDesc->connected == ospl_true)
		systools_printf("\tConnected to %s [%d]\r\n", pTftpDesc->serverName,
				ntohs(pTftpDesc->serverPort));
	else
		systools_printf("\tNot connected\r\n");

	systools_printf("\tMode: %s  Verbose: %s  Tracing: %s\r\n", pTftpDesc->mode,
			tftpDebug ? "on" : "off", tftpTrace ? "on" : "off");
	systools_printf("\tRexmt-interval: %d seconds, Max-timeout: %d seconds\r\n",
			tftpReXmit, tftpTimeout);*/

	return (OK);
}

/*******************************************************************************
 *
 * tftpQuit - quit a TFTP session
 *
 * This routine closes a TFTP session associated with the TFTP descriptor
 * <pTftpDesc>.
 *
 * RETURNS: OK, or ERROR if unsuccessful.
 *
 * ERRNO
 * EINVAL
 */

int tftpQuit(TFTP_DESC * pTftpDesc /* TFTP descriptor */
)

{
	if (pTftpDesc == NULL)
	{
		errno = EINVAL;
		return (ERROR);
	}
	if(pTftpDesc->sock)
	{
		close(pTftpDesc->sock); /* close up shop */
		pTftpDesc->sock = -1;
	}
	free((char *) pTftpDesc);
	return (OK);
}

/*******************************************************************************
 *
 * tftpSend - send a TFTP message to the remote system
 *
 * This routine sends <sizeMsg> bytes of the passed message <pTftpMsg> to the
 * remote system associated with the TFTP descriptor <pTftpDesc>.  If
 * <pTftpReply> is not NULL, tftpSend() tries to get a reply message with a
 * block number <blockReply> and an opcode <opReply>.  If <pPort> is NULL,
 * the reply message must come from the same port to which the message
 * was sent.  If <pPort> is not NULL, the port number from which the reply
 * message comes is copied to this variable.
 *
 * INTERNAL
 * This routine does the lock step acknowledgement and protocol processing for
 * TFTP.  Basically, it sends a message to the server and (waits for and)
 * receives the next message.  This is implemented via two main loops.  The
 * outer loop sends the message to the TFTP server, the inner loop tries to
 * get the next message.  If the reply message does not arrive in a specified
 * amount of time, the inner loop is broken out to in order to retransmit
 * the send message.
 *
 * RETURNS: The size of the reply message, or ERROR.
 *
 * ERRNO
 *  ETIMEDOUT
 *  S_tftpLib_TFTP_ERROR
 */

int tftpSend(TFTP_DESC * pTftpDesc, /* TFTP descriptor	*/
		TFTP_MSG * pTftpMsg, /* TFTP send message	*/
		int sizeMsg, /* send message size  	*/
		TFTP_MSG * pTftpReply, /* TFTP reply message	*/
		int opReply, /* reply opcode   	*/
		int blockReply, /* reply block number	*/
		int * pPort /* return port number	*/
)

{
	int timeWait = 0; /* time waited		*/
	int maxWait; /* max time to wait	*/
	struct timeval reXmitTimer; /* retransmission time	*/
	fd_set readFds; /* select read fds	*/
	struct sockaddr_in peer; /* peer			*/
	int peerlen; /* peer len		*/
	int num; /* temp variable	*/
	int amount; /* amount in socket 	*/
	int retryCount = 0; /* sendto retry count (for DHCP completion) */
	/* set up the timeout values */

	bzero((char *) &reXmitTimer, sizeof(struct timeval));
	maxWait = (tftpTimeout < 0) ? (TFTP_TIMEOUT * 5) : tftpTimeout;
	reXmitTimer.tv_sec = min((tftpReXmit < 0) ? (TFTP_TIMEOUT) : tftpReXmit,
			maxWait);

	while (1) /* send loop */
	{
		if (tftpTrace)
			tftpPacketTrace("sent", pTftpMsg, sizeMsg);
#ifdef OLD_STYLE_TFTP
		/* send message to server */
		if (sendto (pTftpDesc->sock, (caddr_t) pTftpMsg, sizeMsg, 0,
						(const struct sockaddr *) &pTftpDesc->serverAddr,
						sizeof (struct sockaddr_in)) != sizeMsg)
		{
			systools_error("TFTP Send: Error occurred while sendto %s:%d: %s",
					inet_ntoa(pTftpDesc->serverAddr.sin_addr),
					ntohs(pTftpDesc->serverAddr.sin_port), strerror(errno));
			return (ERROR);
		}
#else
		for (;;)
		{
			num = sendto(pTftpDesc->sock, (caddr_t) pTftpMsg, sizeMsg, 0,
					(struct sockaddr *) &(pTftpDesc->serverAddr),
					sizeof(struct sockaddr_in));
			if (num < 0)
			{
				systools_error("TFTP Send: Error occurred while sendto %s:%d: %s",
						inet_ntoa(pTftpDesc->serverAddr.sin_addr),
						ntohs(pTftpDesc->serverAddr.sin_port), strerror(errno));
				switch (errno)
				{
				case EHOSTUNREACH:
				case ENETUNREACH:
				case ENETDOWN:
					/* could be due to DHCP negotiation not complete, retry */
					if (retryCount++ < maxWait)
					{
						/* Delay for 1s */
						//taskDelay (sysClkRateGet());
						continue;
					}
					/* else FALLTHROUGH */
					break;
				default:
					systools_error("TFTP message transmit failed.");
					return (ERROR);
				}
			}
			else if (num != sizeMsg)
			{
				systools_error("TFTP incomplete message transmit.");
				return (ERROR);
			}
			break;
		}
#endif
		if (pTftpReply == NULL) /* return if no reply desired */
			return (0);

		while (1) /* receive loop */
		{
			FD_ZERO(&readFds);
			FD_SET(pTftpDesc->sock, &readFds);
			/* wait for reply message */

			if ((num = select(pTftpDesc->sock + 1, &readFds, (fd_set *) NULL,
					(fd_set *) NULL, &reXmitTimer)) == ERROR)
				return (ERROR);

			if (num == 0) /* select timed out */
			{
				timeWait += reXmitTimer.tv_sec;
				if (timeWait >= maxWait) /* return error, if waited */
				/* for too long 	   */
				{
					systools_error("Tftp send Transfer Timed Out.");
#ifdef TFTPC_DEBUG
					systools_printf("Transfer Timed Out.\r\n");
#endif
					errno = ETIMEDOUT;
					return (ERROR);
				}

				/* break out to send loop in order to retransmit */
				break;
			}

			if (ioctl(pTftpDesc->sock, FIONREAD, (int) &amount) == ERROR)
				return (ERROR);

			if (amount == 0) /* woke up but no data	*/
				continue; /* just go back to sleep*/

			peerlen = sizeof(struct sockaddr_in);
			if ((num = recvfrom(pTftpDesc->sock, (caddr_t) pTftpReply,
					sizeof(TFTP_MSG), 0, (const struct sockaddr *) &peer,
					&peerlen)) == ERROR)
			{
				systools_error("Tftp send occurred while recvfrom %s:%d: %s",
										inet_ntoa(pTftpDesc->serverAddr.sin_addr),
										ntohs(pTftpDesc->serverAddr.sin_port), strerror(errno));
				return (ERROR);
			}
			if (tftpTrace)
				tftpPacketTrace("received", pTftpReply, num);

			/* just return if we received an error message */

			if (ntohs(pTftpReply->th_opcode) == TFTP_ERROR)
			{
				systools_error("Tftp send: Error code %d: %s",
						ntohs(pTftpReply->th_error), pTftpReply->th_errMsg);
#ifdef TFTPC_DEBUG
				systools_printf("Error code %d: %s\r\n",
						ntohs(pTftpReply->th_error), pTftpReply->th_errMsg);
#endif
				//errno = S_tftpLib_TFTP_ERROR;
				return (ERROR);
			}
			/*
			 *  ignore message if it does not have the correct opcode
			 *  or it is not from the port we expect.
			 */

			if (((pPort == NULL)
					&& (peer.sin_port != pTftpDesc->serverAddr.sin_port))
					|| (opReply != ntohs(pTftpReply->th_opcode)))
				continue;

			/* if got right block number, then we got the right packet! */

			if (ntohs(pTftpReply->th_block) == blockReply)
			{
				if (pPort != NULL)
					*pPort = peer.sin_port;
				return (num);
			}
			/*
			 * somehow we didn't get the right block, lets flush out the
			 * the socket, and try resynching if things look salvageable.
			 */

			while (1)
			{
				TFTP_MSG trash;

				if (ioctl(pTftpDesc->sock, FIONREAD, (int) &amount) == ERROR)
					return (ERROR);

				if (amount == 0) /* done - socket cleaned out */
					break;

				recvfrom(pTftpDesc->sock, (caddr_t) & trash, sizeof(TFTP_MSG),
						0, (const struct sockaddr *) NULL, (int *) NULL);
			}

			/* break out to send loop in order to resynch by reresending */

			if (ntohs(pTftpReply->th_block) == (blockReply - 1))
				break;

			return (ERROR); /* things look bad - return */
		}
	}
	return ERROR;
}

/*******************************************************************************
 *
 * fileToAscii - read data from file and convert to netascii.
 *
 * This routine reads data from the file descriptor <fd>, into the buffer
 * specified by <pBuffer> whose width is <bufLen>, converting it to netascii
 * format.  <charConv>, is a holding space that must be passed to each call
 * to fileToAscii().  The first time fileToAscii() is called, the contents of
 * <charConv> should contain the character (\0).
 *
 * The netascii conversions include:
 *	lf (\n) -> cr,lf  (\r\n)
 *	cr (\r) -> cr, nul (\r\0).
 *
 * RETURNS: The number of bytes inserted into buffer, or ERROR.
 */

static int fileToAscii(int fd, /* file descriptor	*/
		char * pBuffer, /* buffer 		*/
		int bufLen, /* buffer length 	*/
		char * charConv /* char being converted */
)

{
	char * ptr; /* pointer to buffer	*/
	char currentChar; /* current char 	*/
	ospl_uint32 index; /* count variable 	*/
	int numBytes; /* num bytes read 	*/

	for (index = 0, ptr = pBuffer; index < bufLen; index++)
	{

		/* if previous char was a \n or \r, then expand it */

		if ((*charConv == '\n') || (*charConv == '\r'))
		{
			currentChar = ((*charConv == '\n') ? '\n' : '\0');
			*charConv = '\0';
		}
		else
		{
			if ((numBytes = read(fd, &currentChar, sizeof(char))) == ERROR)
				return (ERROR);

			if (numBytes == 0) /* end of file 	*/
				break;

			if ((currentChar == '\n') || (currentChar == '\r'))
			{
				*charConv = currentChar;
				currentChar = '\r'; /* put in \r first */
			}
		}
		*ptr++ = currentChar;
	}

	return ((int) (ptr - pBuffer));
}

/*******************************************************************************
 *
 * asciiToFile - write data to file and convert from netascii
 *
 * Write <bufLen> bytes of data from buffer passed in <pBuffer> to file
 * descriptor <fd>, converting from netascii format.  <charConv>, is a holding
 * space that must be passed to each call to asciiToFile.  The first time
 * asciiToFile is called the contents of <charConv> should contain the
 * character (\0).  <isLastBuff> specifies if this is the last buffer to be
 * converted.
 *
 * The conversions include:
 *    cr, nul (\r\0) -> cr (\r)
 *    cr, lf ( (\r\n) -> lf (\n)
 *
 * RETURNS: OK if successful, ERROR otherwise.
 */

static int asciiToFile(int fd, /* file descriptor 	*/
	char * pBuffer, /* buffer 		*/
	int bufLen, /* buffer length	*/
	char * charConv, /* char being converted */
	ospl_bool isLastBuff /* last buffer?		*/
)

{
	int count = bufLen; /* counter		*/
	char * ptr = pBuffer; /* buffer pointer	*/
	char currentChar; /* current character	*/
	ospl_bool skipWrite; /* skip writing		*/

	while (count--)
	{
		currentChar = *ptr++;
		skipWrite = ospl_false;

		if (*charConv == '\r')

		{
			/* write out previous (\r) for anything but (\n). */

			if (currentChar != '\n')
			{
				if (write(fd, charConv, sizeof(char)) <= 0)
					return (ERROR);
			}

			if (currentChar == '\0') /* skip writing (\0) for \r\0 */
				skipWrite = ospl_true;
		}

		if ((!skipWrite) && (currentChar != '\r'))
		{
			if (write(fd, &currentChar, sizeof(char)) <= 0)
				return (ERROR);
		}

		*charConv = currentChar;
	}

	/*
	 *  Since we wait to write the (\r), if this is the last
	 *  buffer and the last character is a (\r), flush it now.
	 */
	if (isLastBuff && (*charConv == '\r'))
	{
		if (write(fd, charConv, sizeof(char)) <= 0)
			return (ERROR);
		*charConv = '\0';
	}

	return (OK);
}

/*******************************************************************************
 *
 * tftpRequestCreate - create a TFTP read/write request message
 *
 * This routine creates a TFTP read/write request message. <pTftpMsg> is the
 * message to be filled in.  <opcode> specifies the request (either TFTP_RRQ
 * or TFTP_WRQ), <pFilename> specifies the filename and <pMode> specifies the
 * mode.  The format of a TFTP read/write request message is:
 *
 *	2 bytes |   string | 1 byte | string | 1 byte
 *	----------------------------------------------
 *	OpCode  | filename |   0    | Mode   |   0
 *
 * RETURNS: size of message
 */

static int tftpRequestCreate(TFTP_MSG * pTftpMsg, /* TFTP message pointer	*/
		int opCode, /* request opCode 	*/
		char * pFilename, /* remote filename 	*/
		char * pMode /* TFTP transfer mode	*/
)

{
	register char * cp; /* character pointer 	*/

	pTftpMsg->th_opcode = htons((ospl_ushort) opCode);

	cp = pTftpMsg->th_request; /* fill in file name	*/
	strcpy(cp, pFilename);
	cp += strlen(pFilename);
	*cp++ = '\0';

	strcpy(cp, pMode); /* fill in mode		*/
	cp += strlen(pMode);
	*cp++ = '\0';

	return (cp - (char *) pTftpMsg); /* return size of message */
}

/*******************************************************************************
 *
 * tftpErrorCreate - create a TFTP error message
 *
 * This routine creates a TFTP error message.  <pTftpMsg> is the TFTP message
 * to be filled in.  <errorNum> is a TFTP defined error number.  The format of
 * a TFTP error message is:
 *
 *	2 bytes | 2 bytes   | string | 1 byte
 *	-------------------------------------
 *	OpCode  | ErrorCode | ErrMsg |   0
 *
 * RETURNS: Size of message.
 *
 * NOMANUAL
 */

int tftpErrorCreate(TFTP_MSG * pTftpMsg, /* TFTP error message	*/
		int errorNum /* error number 	*/
)

{
	TFTP_ERRMSG * pError; /* pointer to error	*/
	static TFTP_ERRMSG tftpErrors[] = /* TFTP	defined errors	*/
	{
	{ EUNDEF, "Undefined error code" },
	{ ENOTFOUND, "File not found" },
	{ EACCESS, "Access violation" },
	{ ENOSPACE, "Disk full or allocation exceeded" },
	{ EBADOP, "Illegal TFTP operation" },
	{ EBADID, "Unknown transfer ID" },
	{ EEXISTS, "File already exists" },
	{ ENOUSER, "No such user" },
	{ -1, NULL } };

	switch (errorNum)
	{
	//case S_nfsLib_NFSERR_NOENT:
	case EEXIST:
	case ENOENT:
		errorNum = ENOTFOUND;
		break;
	}
	/* locate the passed error */
	for (pError = tftpErrors; pError->e_code >= 0; pError++)
	{
		if (pError->e_code == errorNum)
		{
			break;
		}
	}

	if (pError->e_code < 0) /* if not found, use EUNDEF */
		pError = tftpErrors;
	/* fill in error message */
	pTftpMsg->th_opcode = htons((ospl_ushort) TFTP_ERROR);
	pTftpMsg->th_error = htons((ospl_ushort) pError->e_code);
	strcpy(pTftpMsg->th_errMsg, pError->e_msg);

	systools_printf("ERROR : Tftp module e_code : %d, emsg = %s\r\n", pError->e_code, pError->e_msg);

	return (strlen(pError->e_msg) + 5); /* return size of message */
}

/*******************************************************************************
 *
 * tftpPacketTrace - trace a TFTP packet
 *
 * tftpPacketTrace prints out diagnostic information about a TFTP packet.
 * Tracing is enabled when the global variable tftpTrace is ospl_true.  <pMsg> is a
 * pointer to a diagnostic message that gets printed with each trace.
 * <pTftpMsg> is the TFTP packet of <size> bytes.
 *
 * RETURNS: N/A
 */

static void tftpPacketTrace(char * pMsg, /* diagnostic message 	*/
		TFTP_MSG * pTftpMsg, /* TFTP packet 		*/
		int size /* packet size 		*/
)

{
	ospl_ushort op; /* message op code 	*/
	char * cp; /* temp char pointer 	*/
	static char * tftpOpCodes[] = /* ascii op codes 	*/
	{ "#0", "RRQ", /* read request		*/
	"WRQ", /* write request	*/
	"DATA", /* data message		*/
	"ACK", /* ack message		*/
	"ERROR" /* error message	*/
	};

	op = ntohs(pTftpMsg->th_opcode);

	if (op < TFTP_RRQ || op > TFTP_ERROR) /* unknown op code */
	{
		systools_printf("%s opcode=%x\r\n", pMsg, op);
		return;
	}
	systools_printf("%s %s ", pMsg, tftpOpCodes[op]);

	switch (op)
	{
	/* for request messages, display filename and mode */

	case TFTP_RRQ:
	case TFTP_WRQ:
		cp = index(pTftpMsg->th_request, '\0');
		systools_printf("<file=%s, mode=%s>\r\n", pTftpMsg->th_request, cp + 1);
		break;

		/* for the data messages, display the block number and bytes */

	case TFTP_DATA:
		systools_printf("<block=%d, %d bytes>\r\n", ntohs(pTftpMsg->th_block),
				size - TFTP_DATA_HDR_SIZE);
		break;

		/* for the ack messages, display the block number */

	case TFTP_ACK:
		systools_printf("<block=%d>\r\n", ntohs(pTftpMsg->th_block));
		break;

		/* for error messages display the error code and message */

	case TFTP_ERROR:
		systools_printf("<code=%d, msg=%s>\r\n", ntohs(pTftpMsg->th_error),
				pTftpMsg->th_errMsg);
		break;
	}
}

/*******************************************************************************
 *
 * connectOverLoopback - connect two stream sockets over the loopback address
 *
 * This routine creates two stream sockets and connects them over the loopback
 * address.  It places the two socket descriptors in <pSock1> and <pSock2>.
 *
 * RETURNS: OK if successful, otherwise ERROR.
 */
#if 0
static int connectOverLoopback(int * pSock1, /* socket 1 		*/
		int * pSock2 /* socket 2 		*/
)
{
	struct sockaddr_in serverAddr; /* server address 	*/
	char loopAddr[24]; /* loop back address 	*/
	int serverSock; /* server socket 	*/
	struct sockaddr_in clientAddr; /* client address	*/
	int addrlen; /* address length	*/
	int peerlen; /* peer address length	*/
	struct timeval timeOut; /* connect timeout 	*/

	/* set up server address over loopback. */

	bzero((char *) &serverAddr, sizeof(struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = ntohs((ospl_ushort) 0);

	if (/*(ifAddrGet ("lo0", loopAddr) == ERROR) ||*/
	((serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1")) == ERROR)
			|| (serverSock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
		return (ERROR);

	if ((*pSock1 = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
	{
		close(serverSock);
		return (ERROR);
	}

	bzero((char *) &timeOut, sizeof(struct timeval));
	timeOut.tv_sec = 5;
	timeOut.tv_usec = 0;

	peerlen = addrlen = sizeof(struct sockaddr_in);

	/* make connection */

	if ((bind(serverSock, (const struct sockaddr *) &serverAddr,
			sizeof(struct sockaddr_in)) == ERROR)
			|| (getsockname(serverSock, (const struct sockaddr *) &serverAddr,
					&addrlen) == ERROR) || (listen(serverSock, 5) == ERROR)
			|| (/*connectWithTimeout*/connect(*pSock1,
					(const struct sockaddr *) &serverAddr,
					sizeof(struct sockaddr_in)) == ERROR)
			|| ((*pSock2 = accept(serverSock,
					(const struct sockaddr *) &clientAddr, &peerlen)) == ERROR))
	{
		close(serverSock);
		close(*pSock1);
		return (ERROR);
	}

	close(serverSock);
	return (OK);
}
#endif
/*******************************************************************************
 *
 * tftpParamCleanUp - clean up given TFTP parameters
 *
 * This routine closes given sockets and memeories used for TFTP parameters.
 *
 * RETURNS:
 */

static void tftpParamCleanUp(TFTPPARAM * pParam)
{
	if (pParam == NULL)
		return;
	if (pParam->dSock)
		close(pParam->dSock);

	if (pParam->pHost != NULL)
		free(pParam->pHost);

	if (pParam->pFname != NULL)
		free(pParam->pFname);

	if (pParam->pCmd != NULL)
		free(pParam->pCmd);

	if (pParam->pMode != NULL)
		free(pParam->pMode);

	free((char *) pParam);
}

/*******************************************************************************
 *
 * tftpChildTaskSpawn - wrapper for taskSpawn via netJob Queue
 *
 * This routine calls taskSpawn to create tftpTask via netJob Queue.
 * This is needed in order to make tftpLib callable in application PD.
 *
 * RETURNS:
 */
#if 0
static void tftpChildTaskSpawn
(
		int priority,
		int options,
		int stackSize,
		TFTPPARAM * pParam
)
{

	/* clean tftp params up here if taskSpawn failed */

	/*    if (taskSpawn ("tTftpTask", priority, options, stackSize,
	 tftpTask, (int)pParam, 1,2,3,4,5,6,7,8,9) == ERROR)
	 tftpParamCleanUp (pParam);*/
}
#endif

/*******************************************************************************
 *
 * bootTftpLoad - Load a file from a remote host via TFTP.
 *
 * This routine is the TFTP file load handler that is registered with the boot
 * loader application to load files from a remote host machine via the TFTP
 * protocol.  This routine uses the TFTP protocol to load the file specified
 * by the boot parameters from the remote host machine specified by the boot
 * parameters.
 *
 * RETURNS:
 *   OK if the file is loaded via TFTP
 *   ERROR if the file does not exist on the remote host, or the transfer failed
 *
 * ERRNO: N/A
 *
 * \NOMANUAL
 */

int tftp_download(void *v, char *hostName, int port, char *fileName, char *usr,
		char *passwd, char *localfileName)
{
	int fd = 0;

	systools_set(v);

	fd = open(localfileName, O_RDWR|O_CREAT, CONFIGFILE_MASK);
	if (fd <= 0)
	{
		systools_error("TFTP transfer failed: %s", strerror(errno));
		systools_set(NULL);
		return (ERROR);
	}

	if (tftpXfer(hostName, port, fileName, "get", "binary", fd, 0) == ERROR)
	{
		systools_error("TFTP transfer failed.");
		systools_set(NULL);
		//close(fd);
		remove(localfileName);
		return (ERROR);
	}
	systools_set(NULL);
	//close(fd);
	return (OK);
}


int tftp_upload(void *v, char *hostName, int port, char *fileName, char *usr,
		char *passwd, char *localfileName)
{
	int fd = 0;
	systools_set(v);
	fd = open(localfileName, O_RDONLY);
	if (fd <= 0)
	{
		systools_printf("TFTP transfer failed: %s", strerror(errno));
		systools_set(NULL);
		return (ERROR);
	}

	if (tftpXfer(hostName, port, fileName, "put", "binary", fd, 0) == ERROR)
	{
		systools_error("TFTP transfer failed.");
		systools_set(NULL);
		//close(fd);
		return (ERROR);
	}
	systools_set(NULL);
	//close(fd);
	return (OK);
}


