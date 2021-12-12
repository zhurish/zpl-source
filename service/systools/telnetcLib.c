/* telnetcLib.c - telnet client library */

/*
 * Copyright (c) 2000, 2002, 2004-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement. No license to Wind River intellectual property rights
 * is granted herein. All rights not licensed by Wind River are reserved
 * by Wind River.
 */
 
/*
modification history
--------------------
01z,24apr07,tkf  Add IPv6-Only build support.
01y,05sep06,tkf  Fix telnet connection problem.
01x,30aug06,kch  Cleanup for IPNet integration.
01w,04feb06,dlk  Clean up user-side build warnings.
01v,23sep05,rp   fixed slot leak and issue found by static analysis tool
01u,15jun05,vvv  added support for IPv6
01t,16feb05,vvv  include netconf.h only in kernel
01s,03feb05,aeg  added inclusion of strings.h (SPR #105335)
01r,12jan05,vvv  osdep.h cleanup
01q,04oct04,kc   Fixed virtualization issues. Modified telnetExit() to close 
                 connection based on given slot number. Fixed telnet() to
                 prevent max connection exceeded.
01p,01oct04.zhr  SPR# 101978 telent hangs in windshell - removed spawning of
                 a child task and replaced taskDelay() with select().          
01o,02sep04,dlk  Replace LOG_ERROR() with log_err(), etc.. Replace
                 strerror() with strerror_r().
01n,14sep04,niq  virtual stack related backwards compatibility changes
01m,25aug04,ann  merged from COMP_WN_IPV6_BASE6_ITER5_TO_UNIFIED_PRE_MERGE
01l,13aug04,kc   Fixed virtualization issue.
01k,04aug04,jwl  removed redundant calls to vsMyStackNumSet()
01j,14jul04,jwl  implement virtualization
01i,08jul04,vvv  fixed warnings
01h,22par04,xli  fix gnu compiler warning
01g,08mar04,zhr  clean up left over printf and fix doc markups. 
01e,04mar04,zhr  fix the hang caused by telnetExit() 
01f,24feb04,zhr  call connectWithTimeout() instead of connect(),
                 remove 'goto', added LOG_ERR and updated comments for doc.
01e,21jan04,zhr  ported to the base6 
01d,18feb02,tjf  restore saved console options instead of OPT_TERMINAL
01c,04oct00,tjf  added comments saying telnet() on windsh is unsupported
01b,03oct00,tjf  fixed endian issues found by Anton Langebne
01a,25sep00,tjf  written by Ted Friedl, Wind River PS, Madison, WI
*/

/*
DESCRIPTION
This library provides a telnet client for VxWorks that implements
the basic NVT, or "Network Virtual Terminal," as described in RFC
854 and allows users to log in to remote systems via the network.

A VxWorks user may log in to any other remote VxWorks or UNIX system
via the network by calling telnet() from the shell.

INTERNAL
Because windsh does not pass characters to the target until <CR> is
pressed (see TSR145120), this application, like rlogin(), is not
supported on windsh.

INCLUDE FILES: telnetLib.h

SEE ALSO:
RFCs 854 (telnet), 855 (options), 857 (echo), 858 (suppress go ahead)
*/

/* includes */
#include "systools.h"
#include <telnetLib.h>

/* defines */

#define TELNET_ESC_CHAR     (char)29  /* escape character */
#define TELNET_ESC_STRING   "^]"      /* string for escape character */

#define TELNET_IAC_INDEX      0       /* offset of IAC control code */
#define TELNET_CMD_INDEX      1       /* offset of command byte */
#define TELNET_OPT_INDEX      2       /* offset of option byte */

#define TELNET_STATE_NORMAL  10       /* expect 'thru' character or IAC */
#define TELNET_STATE_CMD     11       /* expect command or IAC 'thru' character */
#define TELNET_STATE_OPT     12       /* expect option */

#define TELNET_OPT_LEN        3       /* length of option bytes */
#define TELNET_CMD_LEN        2       /* length of command bytes */
#define TELNET_PORT          23       /* default port number */
#define TELNET_CONN_TIMEOUT  900      /* default connection timeout */
#define TELNET_MAX_SCTD      3        /* default telnet session with reentrant  */

/* globals */


static int telnetExit(TELNETC_SESSION_DATA *);

/*******************************************************************************
*
* telnetCmdSend - send a telnet command to the host
*
* This routine sends a 2 or 3 byte command sequence to the host.
*
* RETURNS: 
* OK or ERROR if write to host fails.
*
* ERRNO:
* N/A
*
* \NOMANUAL
*/

static int telnetCmdSend
    (
    char cmd,   /* command byte */
    char opt,    /* option (when command is DO, DONT, WILL or WONT) */
	TELNETC_SESSION_DATA *session
    )
    {
    int        bytesToWrite;
    int        bytesWritten;
    char pCmdBuf[] = {(char)IAC, 0, 0};
    pCmdBuf[TELNET_CMD_INDEX] = cmd;

    if ((cmd == (char)DO) || (cmd == (char)DONT) ||
        (cmd == (char)WILL) || (cmd == (char)WONT))
        {
        pCmdBuf[TELNET_OPT_INDEX] = opt;
        bytesToWrite = TELNET_OPT_LEN;
        }
    else
        bytesToWrite = TELNET_CMD_LEN;

    bytesWritten = write (session->hostFd, pCmdBuf, bytesToWrite);

    if (bytesWritten != bytesToWrite)
        {
        return (ERROR);
        }

    return (OK);
    }

/*******************************************************************************
*
* telnetHostInputParse - parse character from host
*
* This routine takes a character (from the host) and keeps the state
* of the input stream.  Telnet commands are filtered out and responded
* to appropriately.  All other characters are passed to stdout.
*
* RETURNS: 
* OK or ERROR if writes to host or stdout fail.
*
* ERRNO:
* N/A
*
* \NOMANUAL
*/

static int telnetHostInputParse
    (
    char ch,   /* character to parse */
	TELNETC_SESSION_DATA *session
    )
    {
    int        bytesWritten;

    switch (session->hostStreamState)
        {
        case TELNET_STATE_NORMAL:
            if (ch != (char)IAC)
                {
                /* pass character to stdout */

                bytesWritten = write (session->loutfd, &ch, 1);

                if (bytesWritten != 1)
                    {
                    //log_err (TELNET_LOG | LOG_ERRNO, "stdout write");
                    return (ERROR);
                    }
                }
            else
                {
                /* prepare for command byte */

            	session->hostStreamState = TELNET_STATE_CMD;
                }
            break;

        case TELNET_STATE_CMD:
            if (ch == (char)IAC)
                {
                /* pass character to stdout */

                bytesWritten = write (session->loutfd, &ch, 1);

                if (bytesWritten != 1)
                    {
                    //log_err (TELNET_LOG | LOG_ERRNO, "stdout write");
                    return (ERROR);
                    }

                session->hostStreamState = TELNET_STATE_NORMAL;
                }
            else if ((ch == (char)DO) || (ch == (char)DONT) ||
                     (ch == (char)WILL) || (ch == (char)WONT))
                {
                /* save command and prepare for option byte */

            	session->cmd = ch;

            	session->hostStreamState = TELNET_STATE_OPT;
                }
            else
                {
                /* discard command (i.e., do nothing) */

                session->hostStreamState = TELNET_STATE_NORMAL;
                }
            break;

       case TELNET_STATE_OPT:
            if ((session->cmd == (char)DO) || (session->cmd == (char)DONT))
                {
                /* we WON'T do anything host asks for */

                if (telnetCmdSend ((char)WONT, ch, session) != OK)
                    return (ERROR);
                }
            else if (ch == (char)TELOPT_ECHO)
                {
                session->echoIsDone = zpl_true;
                
                if (session->cmd == (char)WONT)
                    {
                    /* host WON'T echo - echo characters locally */

/*                    if(ioctl (STD_IN, FIOSETOPTIONS, OPT_ECHO) == ERROR)
                         log_err (TELNET_LOG | LOG_ERRNO, "ioctl() failure.");*/
                    }
                }
            else if (ch == (char)TELOPT_SGA)
                {
                session->sgaIsDone = zpl_true;

                /* no action necessary */
                }
            else
                {
                /* we DON'T want anything else! */

                if (telnetCmdSend ((char)DONT, ch, session) != OK)
                    return (ERROR);
                }

            session->hostStreamState = TELNET_STATE_NORMAL;
            break;

       default:
/*            fprintf (stderr,"%s: telnetHostInputParse() hostStreamState ERROR\n\r",
                    pAppName);*/
            return (ERROR);
       }

   return (OK);
   }


/*******************************************************************************
*
* telnet - telnet to a remote host
*
* This routine allows users to telnet to a remote host. In the kernel, it
* should be called from the VxWorks shell as follows:
*
*   -> telnet "hostName"
*
* telnet() API can be invoked as follows from an application in user space:
*
* telnet ("hostName");
*
* where <hostName> is either:
* \ml
* \m -
* a host name, which has been previously added to the remote host table by
* a call to hostAdd() or can be resolved using a DNS server, or
* \m -
* an Internet address (e.g., "90.0.0.2", "3ffe::1")
* \me
*
* If a port other than 23 (that designated for telnet logins) is required,
* this routine should be called as follows:
*
* \IFSET_START KERNEL
*   -> telnet "hostName", port
* \IFSET_END
*
* \IFSET_START USER
* telnet ("hostName", port);
* \IFSET_END
*
* where <port> is the port number.
*
* The user disconnects from the remote system with the key '^]', or by
* simply logging out from the remote system using logout().
*
* Parameters:
* \is
* \i hostName
* The remote host to connect to
* \i port
* The port number to connect to
* \ie
*
* NOTE: Windsh does not send keyboard input to the target until <CR> is
* typed.  For this reason, telnet() is NOT supported on windsh.
*
* \IFSET_START USER
* The telnet client is incompatible with the C interpreter for the kernel
* shell when running in an RTP. For this reason, a telnet client RTP
* should only be executed using the command-line interpreter for the 
* kernel shell. This functionality can be included in the image by
* including the component INCLUDE_SHELL_INTERP_CMD. The command interpreter
* can be invoked by calling "cmd" at the kernel shell prompt.
*
* \cs
* -> cmd
* [vxWorks *]# telnetc.vxe <host>
* \ce
* \IFSET_END
*
* RETURNS: OK or ERROR if remote system is unknown, cannot be connected
* to, or the connection breaks unexpectedly.
*
* ERRNO: N/A
*/

static int telnetTask(TELNETC_SESSION_DATA *session)
{
	int width = 0;
	int bytesRead = 0;
	int bytesWritten = 0;
	char ch = 0;
	zpl_uint32 i = 0;
	fd_set readFds;
	struct vty *vty = session->vty;
	while(session->connect != zpl_true)
	{
		if(session->state == SCTD_EMPTY)
			return telnetExit(session);
		os_msleep(10);
	}
	vty_out(vty, "Connected to %s.%s", session->hostname, VTY_NEWLINE);
	vty_out(vty, "Exit character is '%s'.%s", TELNET_ESC_STRING, VTY_NEWLINE);
	/* send our DO commands to host */
	session->echoIsDone = zpl_false;
	if (telnetCmdSend((char) DO, (char) TELOPT_ECHO, session) == ERROR)
	{
		return telnetExit(session);
	}
	session->sgaIsDone = zpl_false;
	if (telnetCmdSend((char) DO, (char) TELOPT_SGA, session) == ERROR)
	{
		return telnetExit(session);
	}
	session->hostStreamState = TELNET_STATE_NORMAL;

	while (1)
	{
		if(session->state == SCTD_EMPTY)
			return telnetExit(session);
		FD_ZERO(&readFds);
		//if (session->echoIsDone && session->sgaIsDone)
		FD_SET(session->linfd, &readFds);
		FD_SET(session->hostFd, &readFds);
		width = max(session->linfd, session->hostFd) + 1;
		/* wait for input */
		//fprintf(stdout, "%s wait for input\r\n", __func__);
		if (select(width, &readFds, NULL, NULL, NULL) == ERROR)
		{
			//log_err (TELNET_LOG | LOG_ERRNO, "select() failure.");
			return telnetExit(session);
		}

		/* process stdin stream */

		if (FD_ISSET(session->linfd, &readFds))
		{
			/* process stdin stream and get bytes from stdin */
			if (session->echoIsDone && session->sgaIsDone)
			{
				memset(session->pBuf, 0, sizeof(session->pBuf));
				bytesRead = read(session->linfd, session->pBuf,
						sizeof(session->pBuf));
			}
			else
				continue;

			if ((bytesRead == ERROR) || (bytesRead == 0))
			{
				//log_err (TELNET_LOG | LOG_ERRNO, "stdin read() failure.");
				return telnetExit(session);
			}

			/* write bytes to host watching for escape key */
/*			if(strstr(session->pBuf, "exit") ||
					strstr(session->pBuf, "quit"))
				return telnetExit(session);*/

		    //fprintf(stdout, "%s input=%s\r\n", __func__, session->pBuf);
			for (i = 0; i < bytesRead; i++)
			{
				ch = session->pBuf[i];

				if (ch == TELNET_ESC_CHAR)
				{
					/* user wants to break connection */
					return telnetExit(session);
				}

				bytesWritten = write(session->hostFd, &ch, 1);

				if (bytesWritten != 1)
				{
					//log_err (TELNET_LOG | LOG_ERRNO, "write() failure.");
					return telnetExit(session);
				}
			}
		}

		/* process host stream */

		if (FD_ISSET(session->hostFd, &readFds))
		{
			/* get bytes from the host */
			memset(session->pBuf, 0, sizeof(session->pBuf));
			bytesRead = read(session->hostFd, session->pBuf,
					sizeof(session->pBuf));

			if (bytesRead == 0)
			{
				/* host broke connection (e.g., user types "logout") */
				//fprintf(stdout, "%s host broke connection (e.g., user types \"logout\")\r\n", __func__);
				return telnetExit(session);
			}

			if (bytesRead == ERROR)
			{
				//log_err (TELNET_LOG | LOG_ERRNO, "host stram read() failure.");
				return telnetExit(session);
			}
			//fprintf(stdout, "%s output=%s\r\n", __func__, session->pBuf);
			/* write bytes to stdout watching for commands */

			for (i = 0; i < bytesRead; i++)
			{
				if (telnetHostInputParse(session->pBuf[i], session) == ERROR)
					return telnetExit(session);
			}
		}
	}
	/* NOT REACHED */
}


int telnet(struct vty *vty, char * pHostName, int port)
{
	int status = ERROR;
	//char pHostIpDottedName[128];
	struct sockaddr_in hostSockAddr;

	TELNETC_SESSION_DATA *session = NULL;

	session = malloc(sizeof(TELNETC_SESSION_DATA));
	if(!session)
		return ERROR;
	memset(session, 0, sizeof(TELNETC_SESSION_DATA));
	/* make sure the maximum allowed connection is not reached */

	session->state = SCTD_USED;
	session->linfd = vty->fd;
	session->loutfd = vty->wfd;
	session->vty = vty;
	strcpy(session->hostname, pHostName);
	/* get the 32-bit version of the host IP address */

	if (pHostName == NULL)
	{
		//fprintf (stderr,"usage: %s \"hostName\"[, port]\n\r", pAppName);
		return telnetExit(session);
	}

	/* set port to default (23) if zero */
	bzero ((char *) &hostSockAddr, sizeof (struct sockaddr_in));

	/* establish TCP/IP connection to host */
	vty_ansync_enable(vty, zpl_true);

	vty_out(vty, "%sTrying %s...%s", VTY_NEWLINE, session->hostname, VTY_NEWLINE);

	session->hostFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (session->hostFd == ERROR)
	{
		vty_out(vty, "Error creating socket%s", VTY_NEWLINE);
		vty_ansync_enable(vty, zpl_false);
		return telnetExit(session);
	}

/*
	tv.tv_sec = TELNET_CONN_TIMEOUT;
	tv.tv_usec = 0;
*/
	hostSockAddr.sin_family = AF_INET;
	hostSockAddr.sin_addr.s_addr = ipstack_inet_addr(session->hostname);
	if(port)
		hostSockAddr.sin_port = htons(port);
	else
		hostSockAddr.sin_port = htons(TELNET_PORT);

	if(os_task_create("telnet-client", OS_TASK_DEFAULT_PRIORITY,
               0, telnetTask, session, OS_TASK_DEFAULT_STACK) <= 0)
	{
		vty_ansync_enable(vty, zpl_false);
		telnetExit(session);
		return ERROR;
	}


	status = connect(session->hostFd, (struct sockaddr *) &hostSockAddr,
			sizeof(struct sockaddr));

	if (status != OK)
	{
		vty_ansync_enable(vty, zpl_false);
		session->state = SCTD_EMPTY;
		return ERROR;
	}
	vty_cancel(vty);
/*	vty_out(vty, "Connected to %s.%s", pHostName, VTY_NEWLINE);
	vty_out(vty, "Exit character is '%s'.%s", TELNET_ESC_STRING, VTY_NEWLINE);*/
	session->connect = zpl_true;
	return OK;
	/* NOT REACHED */
}
/*****************************************************************************
*
* telnetExit - exit from telnet
*
* Exit telnet performing graceful clean up. 
* 
* RETURNS: 
* OK or ERROR if exit improperly 
*
* ERRNO:
* N/A
*
* \NOMANUAL 
*/

static int telnetExit
    (
    	TELNETC_SESSION_DATA *session
    )
    {
    int retVal = OK;
    if(session->loutfd)
    	write(session->loutfd, "Connection closed by foreign host.\r\n",
			strlen("Connection closed by foreign host.\r\n"));
    if (session->hostFd != ERROR)
	{
    	shutdown (session->hostFd, 2);
    	close (session->hostFd);
	}
    if(session->vty)
    {
    	vty_ansync_enable(session->vty, zpl_false);
    	vty_resume(session->vty);
    	if(session->loutfd)
    		write(session->loutfd, "\r\n",strlen("\r\n"));
    }
    free(session);
    return (retVal);
    }

    
