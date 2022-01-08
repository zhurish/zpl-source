/* tftpdLib.h - vxWorks Trival File Transfer protocol header file */

/*
 * Copyright (c) 1992-2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
02k,27dec06,kch  Removed references to the obsolete coreip virtual stack.
02j,28may04,niq  Merging from base6 label POST_ITER5_FRZ16_REBASE (ver
                 /main/vdt/base6_itn5_networking-int/1)
02i,15mar04,rp   merged from orion
02h,20nov03,niq  Remove copyright_wrs.h file inclusion
02g,04nov03,rlm  Ran batch header path update for header re-org.
02f,03nov03,rlm  Removed wrn/coreip/ prefix from #includes for header re-org.
02e,24oct03,cdw  update include statements post header re-org.
02d,21apr03,ppp  vwModNum.h should be picked up from target/h
02c,11feb02,hgo  modifications for IPv6
02b,22sep92,rrr  added support for c++
02a,04jul92,jcf  cleaned up.
01b,26may92,rrr  the tree shuffle
		  -changed includes to have absolute path from h/
01a,06Feb92,jmm  written.
*/

#ifndef __INCtftpdLibh
#define __INCtftpdLibh

#ifdef __cplusplus
extern "C" {
#endif

#include <os_include.h>
#include <tftpLib.h>


#define TFTPD_BASEDIR_DEFAULT	BASE_DIR"/tftpboot"

typedef struct tftpd_config
{
	zpl_bool	enable;
    char	dirName [TFTP_FILENAME_SIZE];
    char    address[TFTP_FILENAME_SIZE];
    zpl_ushort	port;
    zpl_socket_t 	sock;
    void	*master;
    void	*t_read;
    int 	count;
    zpl_bool 	tftpdDebug;	/* zpl_true: debugging messages */
    int tftpdErrorSendTries;
    int tftpdMaxConnections;
} TFTPD_CONFIG;

extern int tftpdInit(void *master, char *basedir);
extern int tftpdUnInit(void);

extern int tftpdEnable(zpl_bool enable, char *localipaddress, int port);

/*extern int 	tftpdInit (int stackSize, int nDirectories,
			   char * directoryNames, zpl_bool noControl,
			   int maxConnections);*/


#ifdef __cplusplus
}
#endif

#endif /* __INCtftpdLibh */
