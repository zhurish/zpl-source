/* tftpLib.h - vxWorks Trival File Transfer protocol header file */

/* Copyright 1991 - 2004 Wind River Systems, Inc. */

/*
modification history
--------------------
01s,17aug09,ggz  Defect00176203. Fix block number wraparound to support large files.
01r,23aug04,rp   merged from COMP_WN_IPV6_BASE6_ITER5_TO_UNIFIED_PRE_MERGE
01q,28may04,niq  Merging from base6 label POST_ITER5_FRZ16_REBASE (ver
                 /main/vdt/base6_itn5_networking-int/2)
01p,20nov03,niq  Remove copyright_wrs.h file inclusion
01o,04nov03,rlm  Ran batch header path update for header re-org.
01n,03nov03,rlm  Removed wrn/coreip/ prefix from #includes for header re-org.
01m,24oct03,cdw  update include statements post header re-org.
01l,01may03,spm  Tornado 2.2 CP1 merge (from ver 01k,13jan03,rae:
                 TOR2_2-CP1-F label, tor2_2-patch branch, /wind/river VOB)
01k,21apr03,ppp  vwModNum.h should be picked up from target/h
01j,24oct01,hgo  extension for IPv6
01i,29nov00,ham  dealt with ARM alignment issue (SPR 62202).
01h,09oct97,nbs  modified TFTP_DESC to include the file name
		 in the request, spr # 9413
01g,22sep92,rrr  added support for c++
01f,18sep92,jcf  added include of netinet/in.h.
01e,04jul92,jcf  cleaned up.
01d,15jun92,elh	 changed retransmission time.
01c,26may92,rrr  the tree shuffle
01b,29feb92,elh  ansified.
		 incorperated changes done by jmm for server.
01a,19Jul91,elh  written.
*/

#ifndef __INCtftpLibh
#define __INCtftpLibh

#ifdef __cplusplus
extern "C" {
#endif

#include <os_include.h>
#include <log.h>

/* defines */

/* TFTP constant values.  */

#define	TFTP_SEGSIZE        512                 /* data segment size    */
#define	TFTP_REQSIZE        TFTP_SEGSIZE + 2
#define TFTP_FILENAME_SIZE  128		        /* max length of        */
                                                /* filename             */
#define TFTP_ACK_SIZE       4                   /* size of ack message  */
#define TFTP_DATA_HDR_SIZE  4                   /* header size          */
#define	TFTP_TIMEOUT        2                   /* secs between rexmt's	*/
#define TFTP_PORT           69                  /* tftp default port    */

#define TFTP_CLIENT		0
#define TFTP_SERVER		1

/* TFTP Op Codes/packet types */

#define	TFTP_RRQ	01                      /* read request         */
#define	TFTP_WRQ	02                      /* write request        */
#define	TFTP_DATA	03                      /* data packet          */
#define	TFTP_ACK	04                      /* acknowledgement      */
#define	TFTP_ERROR 	05                      /* error packet         */

/*
  TFTP message formats are:


Type        Op #     Format without header

             2 bytes   string   1 byte  string  1 byte
            ------------------------------------------
TFTP_RRQ/  | [01|02]| filename |  0   | mode   |   0 |
TFTP_WRQ    ------------------------------------------

             2 bytes  2 bytes   n bytes
            ---------------------------
TFTP_DATA  |   03   | block # | data  |
            ---------------------------

             2 bytes   2 bytes
            ------------------
TFTP_ACK   |   04   | block #|
            ------------------

             2 bytes  2 bytes     string  1 byte
            -------------------------------------
TFTP_ERROR |  05    | ErrorCode | ErrMsg |   0  |
            -------------------------------------
*/


#ifdef _WRS_VXWORKS_5_X
#if (defined __arm__)
#define TFTP_PACKED _WRS_PACK_ALIGN(1)
#else
#define TFTP_PACKED
#endif /* __arm__ */
#else
#if ((defined __arm__) && (defined __GNUC__))
#define TFTP_PACKED __attribute__((packed))
#else
#define TFTP_PACKED
#endif /* __arm__ */
#endif /* _WRS_VXWORKS_5_X */

/* TFTP packet structure.  */

struct	tftpFormat
    {
    zpl_ushort blockOrError;
    char data [TFTP_SEGSIZE];
    } TFTP_PACKED;

typedef struct tftp_msg
    {
    zpl_ushort	th_opcode;                      /* packet op code       */
    union
        {
        char    request [TFTP_REQSIZE];         /* request string 	*/
        struct  tftpFormat  TFTP_PACKED misc;
     	} TFTP_PACKED th;
    } TFTP_MSG;



/* useful defines to access the TFTP packet */

#define	th_request	th.request              /* request string       */
#define th_block	th.misc.blockOrError    /* block number         */
#define th_error	th.misc.blockOrError    /* error code           */
#define th_errMsg	th.misc.data            /* error message        */
#define th_data		th.misc.data            /* data	                */


/* TFTP defined error codes.  */

#define	EUNDEF		0                       /* not defined          */
#define	ENOTFOUND	1                       /* file not found       */
#define	EACCESS		2                       /* access violation     */
#define	ENOSPACE	3                       /* disk full            */
#define	EBADOP		4                       /* illegal operation 	*/
#define	EBADID		5                       /* unknown transfer ID	*/
#define	EEXISTS		6                       /* file already exists	*/
#define	ENOUSER		7                       /* no such user		*/

/* mode structure */

typedef struct tftp_mode
    {
    char *m_name;                           /* mode name            */
    char *m_mode;                           /* mode type            */
    } TFTP_MODE;

/* error structure */

typedef struct tftp_errmsg
    {
    int   e_code;                           /* error code           */
    char  *e_msg;                           /* error message        */
    } TFTP_ERRMSG;

/* TFTP descriptor */

typedef struct tftp_desc
    {
    char 	mode [32];                      /* mode                 */
    zpl_bool	connected;                      /* state                */
    char    serverName [128];               /* server ascii name    */
#ifndef INET6
    struct ipstack_sockaddr_in serverAddr;          /* server address 	    */
#else
    struct ipstack_sockaddr_in6 serverAddr;
#endif
    zpl_socket_t     sock;                           /* ipstack_socket               */
    int     sockFamily;                     /* IPSTACK_AF_INET(6)           */
    zpl_ushort	serverPort;                 /* server port number   */
    char	fileName [TFTP_FILENAME_SIZE];	/* requested file name  */
    int tftp_size;
    int	tftp_start_time;
    } TFTP_DESC;


/* TFTP task defaults */
#if 0
#define	TFTP_TASK_PRIORITY	70          /* TFTP task priority   */
#define TFTP_TASK_OPTIONS	0	    /* TFTP task options    */
#define TFTP_TASK_STACKSIZE	8*BUFSIZ    /* TFTP stack size      */
#endif
/* tftpLib error values */

/*
#define S_tftpLib_INVALID_ARGUMENT	(M_tftpLib | 1)
#define S_tftpLib_INVALID_DESCRIPTOR	(M_tftpLib | 2)
#define S_tftpLib_INVALID_COMMAND	(M_tftpLib | 3)
#define S_tftpLib_INVALID_MODE		(M_tftpLib | 4)
#define S_tftpLib_UNKNOWN_HOST		(M_tftpLib | 5)
#define S_tftpLib_NOT_CONNECTED		(M_tftpLib | 6)
#define S_tftpLib_TIMED_OUT		(M_tftpLib | 7)
#define S_tftpLib_TFTP_ERROR		(M_tftpLib | 8)

*/

extern int 	tftpXfer (char * pHost, int port, char * pFilename,
			  char * pCommand, char * pMode, int pDataDesc,
    			  int value);
extern int 	tftpCopy (char * pHost,	int port, char * pFilename,
			  char * pCommand, char * pMode, int fd);
extern TFTP_DESC *tftpInit (void);
extern int 	tftpQuit (TFTP_DESC * pTftpDesc);


/*extern int 	tftpModeSet (TFTP_DESC * pTftpDesc, char * pMode);
extern int 	tftpPeerSet (TFTP_DESC * pTftpDesc, char * pHostname,
			     int port);*/
extern int 	tftpPut (TFTP_DESC *pTftpDesc, char *pFilename, int fd,
		         int clientOrServer);
extern int 	tftpGet (TFTP_DESC *pTftpDesc, char *pFilename, int fd,
		         int clientOrServer);

extern int 	tftpSend (TFTP_DESC *, TFTP_MSG *, int ,
		          TFTP_MSG *, int , int ,
		          int *);

extern int 	tftpErrorCreate (TFTP_MSG * pTftpMsg, int errorNum);




extern zpl_bool tftpDebug;

extern int tftp_download(void *v, char *hostName, int port, char *fileName, char *usr,
		char *passwd, char *localfileName);

extern int tftp_upload(void *v, char *hostName, int port, char *fileName, char *usr,
		char *passwd, char *localfileName);


//extern int cmd_tftp_init();




#ifdef __cplusplus
}
#endif

#endif /* __INCtftpLibh */
