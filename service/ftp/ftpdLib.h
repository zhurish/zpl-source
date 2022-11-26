/* ftpdLib.h - header file for ftpdLib.c */

/* Copyright 1990-2004 Wind River Systems, Inc. */

/*
modification history
--------------------
01q,20aug04,rp   merged from COMP_WN_IPV6_BASE6_ITER5_TO_UNIFIED_PRE_MERGE
01p,21jul04,spm  removed extra initialization routine
01o,28may04,niq  Merging from base6 label POST_ITER5_FRZ16_REBASE (ver
                 /main/vdt/base6_itn5_networking-int/1)
01n,15mar04,rp   merged from orion
01m,20nov03,niq  Remove copyright_wrs.h file inclusion
01l,05nov03,cdw  Removal of unnecessary _KERNEL guards.
01k,04nov03,rlm  Ran batch header path update for header re-org.
01j,03nov03,rlm  Removed wrn/coreip/ prefix from #includes for header re-org.
01i,10jun03,vvv  include netVersion.h
01h,22feb99,spm  recovered version replaced by merge from vxw5_3_x branch; 
                 removed duplicate history entry 01g describing existing change
01g,10dec97,spm  changed prototype for ftpdInit to support configurable
                 password authentication (SPR #8602); removed unused
                 prototype for ftpdTask routine
01f,22sep92,rrr  added support for c++
01e,04jul92,jcf  cleaned up.
01d,26may92,rrr  the tree shuffle
01c,04oct91,rrr  passed through the ansification filter
		  -fixed #else and #endif
		  -changed VOID to void
		  -changed copyright notice
01b,05oct90,dnw deleted ftpdWorkTask().
01a,05oct90,shl created.
*/


#ifndef __INCftpdLibh
#define __INCftpdLibh

#ifdef __cplusplus
extern "C" {
#endif

#include <auto_include.h>

#define FTPD_BASEDIR_DEFAULT	BASE_DIR"/ftpboot"


/*
#define FTPD_DEBUG_LOGIN	1
#define FTPD_DEBUG_DATA		2
*/
#define FTPD_DEBUG_CMD		4

#define FTPD_DEBUG_EVENT	8
//#define FTPD_DEBUG_DEFAIL	0X10

#define FTPD_IS_DEBUG(n)		(ftpd_config.ftpdDebug & FTPD_DEBUG_ ## n)


/* function declarations */

extern int 	ftpdInit (void	*, void *);
extern int 	ftpdDelete (void);
extern int   ftpdAnonymousAllow (const char * rootDir, const char * uploadDir);
extern void     ftpdEnableSecurity (void);
extern void     ftpdDisableSecurity (void);
extern void     ftpdLingerSecondsSet (zpl_ulong seconds);

extern int ftpdEnable(char *address, int port);
extern int ftpdDisable(void);


#ifdef __cplusplus
}
#endif

#endif /* __INCftpdLibh */
