/* ftpLib.h - arpa File Transfer Protocol library header */

/*
 * Copyright (c) 1986-1987, 1990-1992, 2002-2003, 
 *               2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01v,22oct08,spw  Fix for WIND00135072
01u,16feb06,mwv  Add ftpLibInit, and ftpLs (SPR 117425)
01t,20nov03,niq  Remove copyright_wrs.h file inclusion
01s,05nov03,cdw  Removal of unnecessary _KERNEL guards.
01r,04nov03,rlm  Ran batch header path update for header re-org.
01q,03nov03,rlm  Removed wrn/coreip/ prefix from #includes for header re-org.
01p,10jun03,vvv  include netVersion.h
01o,09may03,vvv  included vxWorks.h and vwModNum.h
01n,06jun02,elr  Change return for ftpTransientFatalInstall()
01m,23may02,elr  added temporary global flag to bypass PASSIVE mode
01l,22may02,elr  Changed API of ftpCommandEnhanced() and ftpReplyGetEnhanced()
                 for PASSIVE mode support
                 Added function ftpDataConnInitPassiveMode ()
                 Added ftpLibDebugOptionsSet()
01k,12mar02,elr  Added error return codes
                 Replaced ftpErrorSuppress with ftplDebug (SPR 71496)
01j,22sep92,rrr  added support for c++
01i,11sep92,jmm  added external definition for ftpErrorSupress (for spr #1257)
01h,04jul92,jcf  cleaned up.
01g,26may92,rrr  the tree shuffle
01f,04oct91,rrr  passed through the ansification filter
		  -fixed #else and #endif
		  -changed copyright notice
01e,19oct90,shl  changed ftpCommand() to use variable length argument list.
01d,05oct90,shl  added ANSI function prototypes.
                 made #endif ANSI style.
                 added copyright notice.
01c,07aug90,shl  added INCftpLibh to #endif.
01b,20mar87,dnw  prepended FTP_ to reply codes.
01a,07nov86,dnw  written
*/

#ifndef __INCftpLibh
#define __INCftpLibh

#ifdef __cplusplus
extern "C" {
#endif




/* For debugging options */

#define FTPL_DEBUG_OFF          0x00 /* No debugging messages */
#define FTPL_DEBUG_INCOMING     0x01 /* Show all incoming responses */
#define FTPL_DEBUG_OUTGOING     0x02 /* Show all outgoing commands */
#define FTPL_DEBUG_ERRORS       0x04 /* Display all errors and warnings that occur */

/* For FTP specification see RFC-765 */

/* Reply codes for ftpReplyGet(). (Major numbers 1xx-5xx) */

#define FTP_PRELIM              1 /* positive preliminary */
#define FTP_COMPLETE            2 /* positive completion */
#define FTP_CONTINUE            3 /* positive intermediate */
#define FTP_TRANSIENT           4 /* transient negative completion */
#define FTP_ERROR               5 /* permanent negative completion */

/* Detailed reply codes for ftpReplyGetEnhanced() */

#define FTP_SERVICE_CLOSING          221
#define FTP_PASSIVE_REPLY            227 /* Command not supported */
#define FTP_COMMAND_NOT_SUPPORTED    502 /* Command not supported */
#define FTP_NOACTION    550       /* requested action not taken */

/* Type codes */

#define	TYPE_A		1	/* ASCII */
#define	TYPE_E		2	/* EBCDIC */
#define	TYPE_I		3	/* image */
#define	TYPE_L		4	/* local byte size */

/* Form codes */

#define	FORM_N		1	/* non-print */
#define	FORM_T		2	/* telnet format effectors */
#define	FORM_C		3	/* carriage control (ASA) */

/* Structure codes */

#define	STRU_F		1	/* file (no record structure) */
#define	STRU_R		2	/* record structure */
#define	STRU_P		3	/* page structure */

/* Mode types */

#define	MODE_S		1	/* stream */
#define	MODE_B		2	/* block */
#define	MODE_C		3	/* compressed */

/* Record Tokens */

#define	REC_ESC		'\377'	/* Record-mode Escape */
#define	REC_EOR		'\001'	/* Record-mode End-of-Record */
#define REC_EOF		'\002'	/* Record-mode End-of-File */

/* Block Header */

#define	BLK_EOR		0x80	/* Block is End-of-Record */
#define	BLK_EOF		0x40	/* Block is End-of-File */
#define BLK_ERRORS	0x20	/* Block is suspected of containing errors */
#define	BLK_RESTART	0x10	/* Block is Restart Marker */

#define	BLK_BYTECOUNT	2	/* Bytes in this block */

/* error values */

/* 
 * Note that for unexpected responses, the low-order byte of the eror code 
 * contains the reponse code.
 */
#define M_ftpLib	0x100000
#define S_ftpLib_ILLEGAL_VALUE                  (M_ftpLib | 1)
#define S_ftpLib_TRANSIENT_RETRY_LIMIT_EXCEEDED (M_ftpLib | 2)
#define S_ftpLib_FATAL_TRANSIENT_RESPONSE       (M_ftpLib | 3)

#define S_ftpLib_REMOTE_SERVER_STATUS_221       (M_ftpLib | 221)
#define S_ftpLib_REMOTE_SERVER_STATUS_226       (M_ftpLib | 226)
#define S_ftpLib_REMOTE_SERVER_STATUS_257       (M_ftpLib | 257)
#define S_ftpLib_REMOTE_SERVER_ERROR_422        (M_ftpLib | 422)
#define S_ftpLib_REMOTE_SERVER_ERROR_425        (M_ftpLib | 425)
#define S_ftpLib_REMOTE_SERVER_ERROR_450        (M_ftpLib | 450)
#define S_ftpLib_REMOTE_SERVER_ERROR_451        (M_ftpLib | 451)
#define S_ftpLib_REMOTE_SERVER_ERROR_452        (M_ftpLib | 452)
#define S_ftpLib_REMOTE_SERVER_ERROR_500        (M_ftpLib | 500)
#define S_ftpLib_REMOTE_SERVER_ERROR_501        (M_ftpLib | 501)
#define S_ftpLib_REMOTE_SERVER_ERROR_502        (M_ftpLib | 502)
#define S_ftpLib_REMOTE_SERVER_ERROR_503        (M_ftpLib | 503)
#define S_ftpLib_REMOTE_SERVER_ERROR_504        (M_ftpLib | 504)
#define S_ftpLib_REMOTE_SERVER_ERROR_520        (M_ftpLib | 520)
#define S_ftpLib_REMOTE_SERVER_ERROR_521        (M_ftpLib | 521)
#define S_ftpLib_REMOTE_SERVER_ERROR_530        (M_ftpLib | 530)
#define S_ftpLib_REMOTE_SERVER_ERROR_550        (M_ftpLib | 550)
#define S_ftpLib_REMOTE_SERVER_ERROR_551        (M_ftpLib | 551)
#define S_ftpLib_REMOTE_SERVER_ERROR_552        (M_ftpLib | 552)
#define S_ftpLib_REMOTE_SERVER_ERROR_553        (M_ftpLib | 553)
#define S_ftpLib_REMOTE_SERVER_ERROR_554        (M_ftpLib | 554)

/* externals */

#define MAX_REPLY_MAX_LEN 1024

/* function declarations */


extern int ftpLibInit (long timeout);
extern int ftpLogin (zpl_socket_t ctrlSock, char *user, char *passwd, char *account);
extern int ftpLs (char *dirName);
extern int ftpFileSize(zpl_socket_t ctrlSock, const char *filename, int *rfilesize);

extern int ftpXfer (char *host, char *user, char *passwd, char *acct,
                       char *cmd, char *dirname, char *filename,
                       zpl_socket_t *pCtrlSock, zpl_socket_t *pDataSock, int *rfilesize);

extern int ftpCommand (zpl_socket_t ctrlSock, const char *format, ...);
extern int ftpCommandEnhanced (zpl_socket_t ctrlSock, char *replyString,
        				int replyStringLength, const char *format, ...);
extern zpl_socket_t ftpDataConnGet (zpl_socket_t dataSock);
extern zpl_socket_t ftpDataConnInit (zpl_socket_t ctrlSock);
extern zpl_socket_t ftpDataConnInitPassiveMode (zpl_socket_t ctrlSock);
extern zpl_socket_t ftpHookup (char *host);
extern void ftpLibDebugOptionsSet (zpl_uint32 options);
extern int ftpReplyGet (zpl_socket_t ctrlSock, zpl_bool expecteof);
extern int ftpReplyGetEnhanced (zpl_socket_t ctrlSock, zpl_bool expecteof, char *replyString,
                                int replyStringLength);
extern int ftpTransientConfigSet (zpl_uint32 maxRetryCount, zpl_uint32 retryInterval);
extern int ftpTransientConfigGet (zpl_uint32 *maxRetryCount, zpl_uint32 *retryInterval);
extern int ftpTransientFatalInstall ( zpl_bool (*configlette)(zpl_uint32));





extern int ftp_download(void *v, char *hostName, int port, char *path, char *fileName, char *usr,
		char *passwd, char *localfileName);
extern int ftp_upload(void *v, char *hostName, int port, char *path, char *fileName, char *usr,
		char *passwd, char *localfileName);

#ifdef __cplusplus
}
#endif

#endif /* __INCftpLibh */
