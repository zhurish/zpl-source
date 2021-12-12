/* tftpdLib.c - Trivial File Transfer Protocol server library */

/* Copyright 1992 - 2001 Wind River Systems, Inc. */
//#include "copyright_wrs.h"

/*
modification history
--------------------
01l,07may02,kbw  man page edits
01k,15oct01,rae  merge from truestack ver 01n, base 01i (SPR #69222 etc.)
01j,09jan01,dgr  Adding to comment-header-block of tftpdInit as per SPR #63337
01i,09oct97,nbs  modified tftpd to use filename from TFTP_DESC, spr # 9413
01h,01aug96,sgv  added trunc flag for open call SPR #6839
01g,21jul95,vin	 applied ntohs for the opcode field. SPR4124.
01f,11aug93,jmm  Changed ioctl.h and socket.h to sys/ioctl.h and sys/socket.h
01e,21sep92,jdi  documentation cleanup. 
01d,18jul92,smb  Changed errno.h to errnoLib.h.
01c,04jun92,ajm  shut up warnings on mips compiler
01b,26may92,rrr  the tree shuffle
		  -changed includes to have absolute path from h/
01a,29Jan92,jmm		written.
*/

/*
DESCRIPTION
This library implements the VxWorks Trivial File Transfer Protocol
(TFTP) server module.  The server can respond to both read and write
requests.  It is started by a call to tftpdInit().

The server has access to a list of directories that can either be
provided in the initial call to tftpdInit() or changed dynamically
using the tftpdDirectoryAdd() and tftpDirectoryRemove() calls.
Requests for files not in the directory trees specified in the access
list will be rejected, unless the list is empty, in which case all
requests will be allowed.  By default, the access list contains the
directory given in the global variable `tftpdDirectory'.  It is possible
to remove the default by calling tftpdDirectoryRemove().

For specific information about the TFTP protocol, see RFC 783, "TFTP
Protocol."

VXWORKS AE PROTECTION DOMAINS
Under VxWorks AE, you can run the tftp server in the kernel protection 
domain only.  This restriction does not apply under non-AE versions of 
VxWorks.  

INTERNAL

The server library uses the TFTP client routines tftpPut() and
tftpGet() to do the actual file transfer.  When the server receives a
request, it does one of three things:

    Read request (RRQ): spawns tftpFileRead task, which will call
                        tftpPut().

    Write request (WRQ): spawns tftpFileWrite task, which will call
                         tftpGet().

    All others: sends back error packet

To use this feature, include the following component:
INCLUDE_TFTP_SERVER

INCLUDE FILES: tftpdLib.h, tftpLib.h

SEE ALSO:
tftpLib, RFC 783 "TFTP Protocol"
*/
#include "systools.h"
#include "tftpLib.h"
#include "tftpdLib.h"


/* GLOBALS */

//zpl_bool tftpdDebug			= zpl_false;	/* zpl_true: debugging messages */
//static int tftpdErrorSendTries		= 3;
//int tftpdMaxConnections		= 10;

static TFTPD_CONFIG *tftpd_config = NULL;

static char	tftpdErrStr []	= "TFTP server";

/* PROTOTYPES */

static int tftpdRequestVerify (TFTP_DESC *pReplyDesc, int opCode,
				  char *fileName);
static int tftpdRequestDecode (TFTP_MSG *pTftpMsg, int *opCode,
				  char *fileName, char *mode);
static int tftpdFileRead (TFTP_DESC *pReplyDesc);
static int tftpdFileWrite (TFTP_DESC *pReplyDesc);

static TFTP_DESC *tftpdDescriptorCreate (TFTP_DESC *mode, zpl_bool connected,
					 int sock, zpl_ushort clientPort,
					 struct sockaddr_in *pClientAddr);
static int tftpdDescriptorDelete (TFTP_DESC *descriptor);
static int tftpdDirectoryValidate (char *fileName);
static int tftpdErrorSend (TFTP_DESC *pReplyDesc, int errorNum);

static int tftpdTask( struct eloop *thread);
/******************************************************************************
*
* tftpdInit - initialize the TFTP server task
*
* This routine will spawn a new TFTP server task, if one does not already
* exist.  If a TFTP server task is running already, tftpdInit() will simply
* return an ERROR value without creating a new task.
*
* To change the default stack size for the TFTP server task, use the
* <stackSize> parameter.  The task stack size should be set to a large enough
* value for the needs of your application - use checkStack() to evaluate your
* stack usage.  The default size is set in the global variable
* `tftpdTaskStackSize'.  Setting <stackSize> to zero will result in the stack
* size being set to this default.
*
* To set the maximum number of simultaneous TFTP connections (each with its
* own transfer identifier or TID), set the <maxConnections> parameter.  More
* information on this is found in RFC 1350 ("The TFTP Protocol (Revision 2)").
* Setting <maxConnections> to zero will result in the maximum number of
* connections being set to the default, which is 10.
*
* If <noControl> is zpl_true, the server will be set up to transfer any
* file in any location.  Otherwise, it will only transfer files in the
* directories in '/tftpboot' or the <nDirectories> directories in the
* <directoryNames> list, and will send an
* access violation error to clients that attempt to access files outside of
* these directories.
*
* By default, <noControl> is zpl_false, <directoryNames> is empty, <nDirectories>
* is zero, and access is restricted to the '/tftpboot' directory.
*
* Directories can be added to the access list after initialization by using
* the tftpdDirectoryAdd() routine.
*
* VXWORKS AE PROTECTION DOMAINS
* Under VxWorks AE, you can call this function from within the kernel 
* protection domain only.  In addition, all arguments to this function can  
* reference only that data which is valid in the kernel protection domain. 
* This restriction does not apply under non-AE versions of VxWorks.  
*
* RETURNS:
* OK, or ERROR if a new TFTP task cannot be created.
*/
static int tftpd_socket_init(TFTPD_CONFIG *config)
{
	int serverSocket = 0;
    int	value;
	struct sockaddr_in	serverAddr;
    TFTP_MSG 		requestBuffer;
    serverSocket = socket (AF_INET, SOCK_DGRAM, 0);
    if(serverSocket <= 0)
    	return ERROR;
    bzero ((char *) &serverAddr, sizeof (struct sockaddr_in));

    serverAddr.sin_family	= AF_INET;
    if(config->port)
        serverAddr.sin_port		= htons((zpl_ushort) config->port);
    else
    	serverAddr.sin_port		= htons((zpl_ushort) TFTP_PORT);

    serverAddr.sin_addr.s_addr	= INADDR_ANY;

    if(strlen(config->address))
        serverAddr.sin_addr.s_addr	= ipstack_inet_addr(config->address);

    if (bind (serverSocket, (struct sockaddr *) &serverAddr,
	      sizeof (struct sockaddr_in)) == ERROR)
	{
    	close(serverSocket);
    	systools_error ("%s: could not bind to TFTP port\n", tftpdErrStr);
    	return (ERROR);
	}
    while(1)
	{
		if (ioctl (serverSocket, FIONREAD, (int) &value) == ERROR)
		{
			close(serverSocket);
			return (ERROR);
		}
		if (value == 0)                /* done - socket cleaned out */
			break;

		if(recvfrom (serverSocket, (caddr_t) &requestBuffer,
				sizeof (TFTP_MSG), 0, (struct sockaddr *) NULL, (int *) NULL) <= 0)
		{
			close(serverSocket);
			return (ERROR);
		}
	}
    return serverSocket;
}


int tftpdInit(void *master, char *basedir)
{
	if(tftpd_config && tftpd_config->enable)
		return OK;

	if(tftpd_config)
	{
		memset(tftpd_config, 0, sizeof(TFTPD_CONFIG));
		if(basedir)
			strcpy(tftpd_config->dirName, basedir);
		else
			strcpy(tftpd_config->dirName, TFTPD_BASEDIR_DEFAULT);

		tftpd_config->master = master;
		tftpd_config->tftpdDebug = zpl_false;	/* zpl_true: debugging messages */
		tftpd_config->tftpdErrorSendTries = 3;
		tftpd_config->tftpdMaxConnections = 10;
	}
	else
	{
		tftpd_config = malloc(sizeof(TFTPD_CONFIG));
		if(tftpd_config == NULL)
			return ERROR;
		memset(tftpd_config, 0, sizeof(TFTPD_CONFIG));
		if(basedir)
			strcpy(tftpd_config->dirName, basedir);
		else
			strcpy(tftpd_config->dirName, TFTPD_BASEDIR_DEFAULT);
		tftpd_config->master = master;

	    if(access(tftpd_config->dirName, 0) != 0)
	    {
	    	mkdir(tftpd_config->dirName, 0766);
	    }

		tftpd_config->tftpdDebug = zpl_false;	/* zpl_true: debugging messages */
		tftpd_config->tftpdErrorSendTries = 3;
		tftpd_config->tftpdMaxConnections = 10;
	}
	return ERROR;
}

int tftpdUnInit(void)
{
	if(tftpd_config)
	{
		if(tftpd_config->count)
		{
			systools_printf("%s is running, please waiting...\r\n", tftpdErrStr);
			return ERROR;
		}
		if(tftpd_config->t_read)
			eloop_cancel(tftpd_config->t_read);
		tftpd_config->t_read = NULL;
		if(tftpd_config->sock)
			close(tftpd_config->sock);
		tftpd_config->sock = 0;
		free(tftpd_config);
		tftpd_config = NULL;
	}
	return OK;
}

int tftpdEnable(zpl_bool enable, char *localipaddress, int port)
{
	if(tftpd_config && enable)
	{
		if(tftpd_config->enable)
		{
			if(tftpd_config->count)
			{
				systools_printf("%s is running, please waiting...\r\n", tftpdErrStr);
				return ERROR;
			}
			if(tftpd_config->t_read)
				eloop_cancel(tftpd_config->t_read);
			tftpd_config->t_read = NULL;
			if(tftpd_config->sock)
				close(tftpd_config->sock);
			tftpd_config->sock = 0;
		}
		tftpd_config->port = port;
		if(localipaddress)
			strcpy(tftpd_config->address, localipaddress);

		tftpd_config->sock = tftpd_socket_init(tftpd_config);
		if(tftpd_config->sock > 0)
		{
			tftpd_config->t_read = eloop_add_read(tftpd_config->master, tftpdTask, tftpd_config, tftpd_config->sock);
			tftpd_config->enable = zpl_true;
			return OK;
		}
		return OK;
	}
	if(tftpd_config && !enable)
	{
		if(tftpd_config->enable)
		{
			if(tftpd_config->count)
			{
				systools_printf("%s is running, please waiting...\r\n", tftpdErrStr);
				return ERROR;
			}
			if(tftpd_config->t_read)
				eloop_cancel(tftpd_config->t_read);
			tftpd_config->t_read = NULL;
			if(tftpd_config->sock)
				close(tftpd_config->sock);
			tftpd_config->sock = 0;
		}
		return OK;
	}
	return ERROR;
}


#if 0
int tftpdInit
    (
    int	 stackSize,		/* stack size for the tftpdTask		*/
    int  nDirectories,		/* number of directories allowed read	*/
    char **directoryNames,	/* array of dir names			*/
    zpl_bool noControl,		/* zpl_true if no access control required	*/
    int	 maxConnections
    )
    {

    /*
     * Make sure there isn't a TFTP server task running already
     */

    if (tftpdTaskId != NONE)
	{
	return (ERROR);
	}

    /*
     * Initialize the access list, add the default directory,
     * and give the semaphore that protects the list
     */

    lstInit (&tftpdDirectoryList);

    //tftpdDirectorySem = semMCreate(SEM_Q_FIFO);

    /*
     * If access control isn't turned off, add the default directory
     * to the list
     */

    if (noControl != zpl_true)
        tftpdDirectoryAdd (tftpdDirectoryDefault);

    /*
     * Add the first set of directories to the list
     */

    while (--nDirectories >= 0)
	{
	tftpdDirectoryAdd (directoryNames [nDirectories]);
	}

    /* create a TFTP server task */

/*
    tftpdTaskId = taskSpawn ("tTftpdTask", tftpdTaskPriority, 0,
			     stackSize == 0 ? tftpdTaskStackSize : stackSize,
			     tftpdTask, nDirectories, (int) directoryNames,
			     maxConnections, 0, 0, 0, 0, 0, 0, 0);

    if (tftpdTaskId == ERROR)
	{
        printErr ("%s: tftpdTask cannot be created\n", tftpdErrStr);
	return (ERROR);
	}
*/

    return (OK);
    }

/******************************************************************************
*
* tftpdTask - TFTP server daemon task
*
* This routine processes incoming TFTP client requests by spawning a new
* task for each connection that is set up.  This routine is called by 
* tftpdInit().
*
* VXWORKS AE PROTECTION DOMAINS
* Under VxWorks AE, you can call this function from within the kernel 
* protection domain only.  In addition, all arguments to this function can  
* reference only that data which is valid in the kernel protection domain. 
* This restriction does not apply under non-AE versions of VxWorks.  
*
* RETURNS:
* OK, or ERROR if the task returns unexpectedly.
*/

int tftpdTask
    (
    int		nDirectories,		/* number of dirs allowed access    */
    char	**directoryNames,	/* array of directory names         */
    int		maxConnections		/* max number of simultan. connects */
    )
    {
    int			serverSocket;	/* socket to use to communicate with
					 * the remote process */
    struct sockaddr_in	clientAddr;	/* process requesting TFTP
					 * connection */
    struct sockaddr_in	serverAddr;
    int			clientAddrLength = sizeof (struct sockaddr_in);
    TFTP_MSG 		requestBuffer;
    int			value;
    int			opCode;
    char		*fileName;
    char		mode [TFTP_SEGSIZE];
    TFTP_DESC		*pReplyDesc;
    int			replySocket;

    serverSocket = socket (AF_INET, SOCK_DGRAM, 0);

    bzero ((char *) &serverAddr, sizeof (struct sockaddr_in));
    bzero ((char *) &clientAddr, sizeof (struct sockaddr_in));

    serverAddr.sin_family	= AF_INET;
    serverAddr.sin_port		= htons((zpl_ushort) TFTP_PORT);


    serverAddr.sin_addr.s_addr	= INADDR_ANY;

    if (bind (serverSocket, (SOCKADDR *) &serverAddr,
	      sizeof (struct sockaddr_in)) == ERROR)
	{
	printErr ("%s: could not bind to TFTP port\n", tftpdErrStr);
	return (ERROR);
	}

    if (tftpdDescriptorQueueInit (maxConnections) == ERROR)
	{
	printErr ("%s: could not create descriptor queue\n", tftpdErrStr);
	return (ERROR);
	}

    /*
     * Clean out any outstanding data on the TFTP port.
     */

    while(1)
	{
	if (ioctl (serverSocket, FIONREAD, (int) &value) == ERROR)
	    return (ERROR);

	if (value == 0)                /* done - socket cleaned out */
	    break;

	recvfrom (serverSocket, (caddr_t) &requestBuffer,
		  sizeof (TFTP_MSG), 0, (SOCKADDR *) NULL,
		  (int *) NULL);
	}

    /*
     * The main loop.  Receive requests on the TFTP port, parse the request,
     * and spawn tasks to handle it.
     */

    while(1)
	{

	/*
	 * Read a message from the TFTP port
	 */

	value = recvfrom (serverSocket, (char *) &requestBuffer, TFTP_SEGSIZE,
			  0, (struct sockaddr *) &clientAddr,
			  &clientAddrLength);

	/*
	 * If there's an error reading on the port, abort the server.
	 */

	if (value == ERROR)
	    {
	    printErr ("%s:  could not read on TFTP port\n", tftpdErrStr);
	    close (serverSocket);
	    tftpdDescriptorQueueDelete ();
	    break;
	    }

	/*
	 * Set up a socket to use for a reply, and get a port number for it.
	 */

	replySocket = socket (AF_INET, SOCK_DGRAM, 0);
	if (replySocket == ERROR)
	    {

	    /*
	     * XXX How should we deal with an error here?
	     */

	    continue;
	    }

	serverAddr.sin_port = htons((zpl_ushort) 0);
	if (bind (replySocket, (SOCKADDR *) &serverAddr,
		  sizeof (struct sockaddr_in)) == ERROR)
	    {

	    /*
	     * XXX How should we deal with an error here?
	     */

	    continue;
	    }

	if (tftpdRequestDecode (&requestBuffer, &opCode, NULL,
				(char *) mode) == ERROR)
	    {

	    /*
	     * We received something that doesn't look like a TFTP request.
	     * Ignore it.
	     */

	    close (replySocket);
	    continue;
	    }

	/*
	 * Get a reply descriptor.  This will pend until one is available.
	 */

	pReplyDesc = tftpdDescriptorCreate (mode, zpl_true, replySocket,
					    clientAddr.sin_port, &clientAddr);
	if (pReplyDesc == NULL)
	    {

	    /*
	     * Couldn't create a reply descriptor.
	     */

	    close (replySocket);
	    continue;
	    }

	/*
	 * Copy the name of the requested file into the TFTP_DESC
	 */

	fileName = pReplyDesc->fileName;
        if (tftpdRequestDecode (&requestBuffer, NULL, (char *) fileName,
                                NULL) == ERROR)
            {

            /*
             * We received something that doesn't look like a TFTP request.
             * Ignore it.
             */

            close (replySocket);
            continue;
            }

	if (tftpdRequestVerify (pReplyDesc, opCode, fileName) != OK)
	    {

	    /*
	     * Invalid request, error packet already sent by tftpdRequestVerify
	     */

	    tftpdDescriptorDelete (pReplyDesc);
	    close (replySocket);
	    continue;
	    }

	if (tftpd_config && tftpd_config->tftpdDebug)
	    {
	    printf ("%s: Request: Opcode = %d, file = %s, client = %s\n",
		    tftpdErrStr, opCode, fileName, pReplyDesc->serverName);
	    }

	switch (opCode)
	    {
	    case TFTP_RRQ:

		/*
		 * We've received a read request.  Spawn a tftpdFileRead
		 * task to process it.
		 */

	        taskSpawn ("tTftpRRQ", tftpdResponsePriority, 0, 10000,
			   tftpdFileRead, (int) fileName, (int) pReplyDesc,
			   0, 0, 0, 0, 0, 0, 0, 0);

		break;

	    case TFTP_WRQ:

		/*
		 * We've received a write request.  Spawn a tftpdFileWrite
		 * task to process it.
		 */

	        taskSpawn ("tTftpWRQ", tftpdResponsePriority, 0, 10000,
			   tftpdFileWrite, (int) fileName, (int) pReplyDesc,
			   0, 0, 0, 0, 0, 0, 0, 0);
		break;
	    }
	} /* end while(1) */

    printErr ("%s:  aborting TFTP server\n", tftpdErrStr);
    tftpdDescriptorQueueDelete ();
    close (serverSocket);
    return (ERROR);
    }

#endif

static int tftpdTask( struct eloop *thread)
{
	int serverSocket = 0, clientSocket = 0; /* socket to use to communicate with the remote process */
	struct sockaddr_in clientAddr; /* process requesting TFTP connection */
	int clientAddrLength = sizeof(struct sockaddr_in);
	TFTP_MSG requestBuffer;
	int value = 0;
	int opCode = 0;
	TFTP_DESC *pReplyDesc = NULL;
	TFTP_DESC tftpdDesc;
	TFTPD_CONFIG *config = 	ELOOP_ARG(thread);

	serverSocket = 	ELOOP_FD(thread);
	config->t_read = NULL;
	/*
	 * Read a message from the TFTP port
	 */
	memset(&requestBuffer, 0, sizeof(requestBuffer));
	value = recvfrom(serverSocket, (char *) &requestBuffer, TFTP_SEGSIZE, 0,
			(struct sockaddr *) &clientAddr, &clientAddrLength);

	config->t_read = eloop_add_read(config->master, tftpdTask, config, serverSocket);
	/*
	 * If there's an error reading on the port, abort the server.
	 */

	if (value == ERROR)
	{
		systools_error("%s:  could not read on TFTP port\n", tftpdErrStr);
	}
	memset(&tftpdDesc, 0, sizeof(tftpdDesc));

	if (tftpdRequestDecode(&requestBuffer, &opCode, NULL, (char *) tftpdDesc.mode)
			== ERROR)
	{

		/*
		 * We received something that doesn't look like a TFTP request.
		 * Ignore it.
		 */
		return ERROR;
	}

	/*
	 * Copy the name of the requested file into the TFTP_DESC
	 */
	if (tftpdRequestDecode(&requestBuffer, NULL, (char *) tftpdDesc.fileName, NULL)
			== ERROR)
	{

		/*
		 * We received something that doesn't look like a TFTP request.
		 * Ignore it.
		 */

		return ERROR;
	}

	if (tftpdRequestVerify(&tftpdDesc, opCode, tftpdDesc.fileName) != OK)
	{

		/*
		 * Invalid request, error packet already sent by tftpdRequestVerify
		 */
		return ERROR;
	}

	if (tftpd_config && tftpd_config->tftpdDebug)
	{
		systools_debug("%s: Request: Opcode = %d, file = %s, client = %s\n",
				tftpdErrStr, opCode, tftpdDesc.fileName, tftpdDesc.serverName);
	}
	/*
	 * Get a reply descriptor.  This will pend until one is available.
	 */
	clientSocket = os_sock_create(zpl_false);
	if(clientSocket <= 0)
		return ERROR;
	os_sock_bind(clientSocket, NULL, 0);
	pReplyDesc = tftpdDescriptorCreate(&tftpdDesc, zpl_true, clientSocket, clientAddr.sin_port, &clientAddr);
	if (pReplyDesc == NULL)
	{
		/*
		 * Couldn't create a reply descriptor.
		 */
		close(clientSocket);
		return ERROR;
	}

	switch (opCode)
	{
	case TFTP_RRQ:

		/*
		 * We've received a read request.  Spawn a tftpdFileRead
		 * task to process it.
		 */
		os_job_add(tftpdFileRead, pReplyDesc);

		break;

	case TFTP_WRQ:

		/*
		 * We've received a write request.  Spawn a tftpdFileWrite
		 * task to process it.
		 */
		os_job_add(tftpdFileWrite, pReplyDesc);
		break;
	default:
		break;
	}
	return (ERROR);
}
/******************************************************************************
*
* tftpdRequestVerify - ensure that an incoming TFTP request is valid
*
* Checks a TFTP request to make sure that the opcode is either
* a read or write request, and then checks to see if the file requested
* is in the access list.
*
* If there is an error, it sends an error packet to the offending client.
*
* RETURNS: OK, or ERROR if any of the conditions aren't met.
*/

static int tftpdRequestVerify
    (
    TFTP_DESC	*pReplyDesc,
    int		opCode,
    char	*fileName
    )
    {
    int		dirIsValid;

    /*
     * Need to check two things:
     *
     * 1.  The request itself needs to be valid, either a write request (WRQ)
     *     or a read request (RRQ).
     *
     * 2.  It needs to be to a valid directory.
     */

    if ((opCode != TFTP_RRQ) && (opCode != TFTP_WRQ))
	{

		/*
		 * Bad opCode sent to the server.
		 */

		tftpdErrorSend (pReplyDesc, EBADOP);
		return (ERROR);
	}

    dirIsValid = tftpdDirectoryValidate (fileName);
    if (dirIsValid != OK)
	{

		/*
		 * Access was denied to the file that the client
		 * requested.
		 */

		tftpdErrorSend (pReplyDesc, errno);
		return (ERROR);
	}

    return (OK);
    }

/******************************************************************************
*
* tftpdRequestDecode - break down a TFTP request
*
* Given a pointer to a TFTP message, this routine decodes the message
* and returns the message's opcode, file name, and mode.
*
* RETURNS:
* OK or ERROR.
*/


static int tftpdRequestDecode
    (
    TFTP_MSG	*pTftpMsg,
    int	*	opCode,		/* pointer to the opCode to return */
    char	*fileName,	/* where to return filename */
    char	*mode		/* where to return mode */
    )
    {
    char	*strIndex;	/* index into pTftpMsg to get mode string */

    if (pTftpMsg == NULL)
    	return (ERROR);

    if (opCode != NULL)
    	*opCode = ntohs(pTftpMsg->th_opcode);

    if (fileName != NULL)
	{
    	strncpy (fileName, pTftpMsg->th.request, 128);
    	fileName [127] = '\0';
	}

    if (mode != NULL)
	{

		/*
		 * Need to get the next string in the struct. Use the for loop to
		 * find the end of the first string.
		 */

		for (strIndex = pTftpMsg->th.request;
			 *strIndex != '\0';
			 strIndex++)
			;

		strncpy(mode, ++strIndex, 32);
		mode [31] = '\0';
	}

    return (OK);
    }

/******************************************************************************
*
* tftpdFileRead - handle a read request
*
* This routine constructs and executes the tftpPut() command to put the file
* to the remote system.  Normally this routine is the entry point for a task
* created by tftpdTask() in response to a read request.
*
* RETURNS: OK, or ERROR if the file requested could not be opened.
*/

static int tftpdFileRead
    (
    TFTP_DESC	*pReplyDesc 	/* where to send the file */
    )
    {
	char filepath[TFTP_FILENAME_SIZE * 2];
    int		requestFd;
    int		returnValue = OK;
    if(tftpd_config)
    	tftpd_config->count++;

    memset(filepath, 0, sizeof(filepath));
    if(tftpd_config && strlen(tftpd_config->dirName))
    	snprintf(filepath, sizeof(filepath), "%s/%s", tftpd_config->dirName, pReplyDesc->fileName);
    else
    	snprintf(filepath, sizeof(filepath), "%s/%s", TFTPD_BASEDIR_DEFAULT, pReplyDesc->fileName);
    requestFd = open (filepath, O_RDONLY, 0);

    if (requestFd == ERROR)
	{

		systools_error ("%s: Could not open file %s\n", tftpdErrStr, pReplyDesc->fileName);
		tftpdErrorSend (pReplyDesc, errno);
	}
    else
	{
		/*
		 * We call tftpPut from the server on a read request because the
		 * server is putting the file to the client
		 */

		returnValue = tftpPut (pReplyDesc, pReplyDesc->fileName, requestFd,
					   TFTP_SERVER);
		close (requestFd);
	}

    /*
     * Close the socket, and delete the
     * tftpd descriptor.
     */

    if (returnValue == ERROR)
	{
    	systools_error ("%s:  could not send client file \"%s\"\n", tftpdErrStr,pReplyDesc->fileName);
	}


    tftpdDescriptorDelete (pReplyDesc);
    if(tftpd_config)
    	tftpd_config->count--;
    return (returnValue);
    }

/******************************************************************************
*
* tftpdFileWrite - handle a write request
*
* This routine constructs and executes the tftpGet() command to get the file
* from the remote system.  Normally this routine is the entry point for a
* task created by tftpdTask() in response to a write request.
*
* RETURNS: OK, or ERROR if the file requested could not be opened.
*/

static int tftpdFileWrite
    (
    TFTP_DESC	*pReplyDesc 	/* where to send the file */
    )
    {
	char filepath[TFTP_FILENAME_SIZE * 2];
    int		requestFd;
    int		returnValue = OK;
    if(tftpd_config)
    	tftpd_config->count++;

    memset(filepath, 0, sizeof(filepath));
    if(tftpd_config && strlen(tftpd_config->dirName))
    	snprintf(filepath, sizeof(filepath), "%s/%s", tftpd_config->dirName, pReplyDesc->fileName);
    else
    	snprintf(filepath, sizeof(filepath), "%s/%s", TFTPD_BASEDIR_DEFAULT, pReplyDesc->fileName);

    requestFd = open (pReplyDesc->fileName, O_WRONLY | O_CREAT | O_TRUNC, CONFIGFILE_MASK);

    if (requestFd == ERROR)
	{
		systools_error ("%s: Could not open file %s\n", tftpdErrStr, pReplyDesc->fileName);
		tftpdErrorSend (pReplyDesc, errno);
	}
    else
	{

		/*
		 * We call tftpGet from the server on a read request because the
		 * server is putting the file to the client
		 */

		returnValue = tftpGet (pReplyDesc, pReplyDesc->fileName, requestFd,
					   TFTP_SERVER);
		close (requestFd);
	}

    /*
     * Close the socket, and delete the
     * tftpd descriptor.
     */

    if (returnValue == ERROR)
	{
    	systools_error ("%s:  could not send \"%s\" to client\n", tftpdErrStr, pReplyDesc->fileName);
	}

    tftpdDescriptorDelete (pReplyDesc);
    if(tftpd_config)
    	tftpd_config->count--;
    return (returnValue);
    }
/******************************************************************************
*
* tftpdDescriptorCreate - build a tftp descriptor to use with tftpLib
*
* The routines in tftpLib, tftpPut() and tftpGet() in particular, expect to
* be passed a pointer to a struct of type TFTP_DESC that contains the
* information about the connection to the host.  This is a convenience
* routine to allocate space for a TFTP_DESC struct, fill in the elements,
* and return a pointer to it.
*
* This routine pends until a descriptor is available.
*
* RETURNS:
* A pointer to a newly allocated TFTP_DESC struct, or NULL on failure.
*/

static TFTP_DESC *tftpdDescriptorCreate
    (
    TFTP_DESC	*desc,			/* mode 		*/
    zpl_bool	connected,		/* state		*/
    int		sock,			/* socket number	*/
    zpl_ushort	clientPort,		/* client port number	*/
    struct sockaddr_in *pClientAddr 	/* client address	*/
    )
    {
    TFTP_DESC	*pTftpDesc = NULL;		/* pointer to the struct to return */
    pTftpDesc = malloc(sizeof(TFTP_DESC));
    if(pTftpDesc == NULL)
    	return NULL;

    memset(pTftpDesc, 0, sizeof(TFTP_DESC));
    /*
     * Copy the arguments into the appropriate elements of the struct
     */

    strncpy (pTftpDesc->mode, desc->mode, 31);
    pTftpDesc->mode [31] = '\0';
    pTftpDesc->connected = connected;

    strcpy (pTftpDesc->fileName, desc->fileName);

    /*
     * clientName and serverName are reversed, because the routines
     * that use this descriptor are expecting to look at the universe
     * from the client side.
     */

    sprintf(pTftpDesc->serverName, "%s", ipstack_inet_ntoa (pClientAddr->sin_addr));

    bcopy ((char *) pClientAddr, (char *) &pTftpDesc->serverAddr, sizeof (struct sockaddr_in));
    pTftpDesc->sock = sock;
    pTftpDesc->serverPort = clientPort;

    /*
     * We've filled the struct, now return a pointer to it
     */

    return (pTftpDesc);
    }

/******************************************************************************
*
* tftpdDescriptorDelete - delete a reply descriptor
*
* This routine returns the space for a reply descriptor back to the
* descriptor pool.
*
* RETURNS: OK or ERROR.
*/

static int tftpdDescriptorDelete
    (
    TFTP_DESC *descriptor
    )
    {
	if(descriptor)
	{
		if(descriptor->sock)
			close (descriptor->sock);
		free(descriptor);
	}
    return OK;
    }

/******************************************************************************
*
* tftpdDirectoryValidate - confirm that file requested is in valid directory
*
* This routine checks that the file requested is in a directory that
* matches an entry in the directory access list.  The validation
* procedure is:
*
*     1.  If the requested file matches the directory exactly,
*         deny access.  This prevents opening devices rather
* 	  than files.
*
*     2.  If the directory and the first part of the file
*         are equal, permission is granted.
*
*     Examples:
*
* 		File		Directory	Result
* 		/mem		/mem		rejected by #1
* 		/mem/foo	/mem		granted
* 		/stuff/foo	/mem		rejected by #2
* 						(first four chars of file
* 						don't match "/mem")
*
* RETURNS: OK, or ERROR if access is not allowed.
*/

static int tftpdDirectoryValidate
    (
    char *fileName
    )
    {
		return OK;
    }

/******************************************************************************
*
* tftpdErrorSend - send an error to a client
*
* Given a client connection and an error number, this routine builds an error
* packet and sends it to the client.
*
* RETURNS:
* OK, or ERROR if tftpSend() fails.  Note that no further action is required if
* tftpSend() fails.
*/

static int tftpdErrorSend
    (
    TFTP_DESC	*pReplyDesc,	/* client to send to */
    int		errorNum		/* error to send */
    )
    {
    TFTP_MSG	errMsg;
    int		errSize;
    int		repeatCount = tftpd_config ? tftpd_config->tftpdErrorSendTries:3;
    int	returnValue = OK;

    errSize = tftpErrorCreate (&errMsg, errorNum);

    /*
     * Try to send the error message a few times.
     */

    while (repeatCount--)
	{
		returnValue = tftpSend (pReplyDesc, &errMsg, errSize,
					(TFTP_MSG *) 0, 0, 0, (int *) 0);
		/*
		 * If we didn't get an error, break out of the loop.  Otherwise,
		 * wait one second and retry
		 */

		if (returnValue != ERROR)
			{
			break;
			}
		else
			{
			os_sleep (1);
			}
	}
    return (returnValue);
    }


