/* syslogcLib.h - syslog client interface header */

/* Copyright 2003 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,27feb04,myz  removed syslogcBinToAscStrConvert()
01b,21nov03,svk  Fix copyright notice
01a,03sep03,myz  written.
*/

#ifndef __INCslogClientLibh
#define __INCslogClientLibh

#ifdef __cplusplus
extern "C" {
#endif

//#include "vxWorks.h"
#ifdef PL_SERVICE_SYSLOG
/* syslog port */

#define SYSLOGC_DEFAULT_PORT   514

/* syslog facility code */
#if 0
#define SYSLOGC_KERNEL_MSG             0
#define SYSLOGC_USER_LEVEL_MSG         1
#define SYSLOGC_MAIL_SYSTEM            2
#define SYSLOGC_SYSTEM_DAEMONS         3     
#define SYSLOGC_SECURITY_MSG           4     
#define SYSLOGC_MSG_FROM_SYSLOGD       5     
#define SYSLOGC_LINE_PRINTER           6     
#define SYSLOGC_NETWORK_NEWS           7     
#define SYSLOGC_UUCP_SUBSYS            8     
#define SYSLOGC_CLOCK_DAEMON           9     
#define SYSLOGC_FTP_DAEMON             11     
#define SYSLOGC_NTP_SUBSYSTEM          12     
#define SYSLOGC_LOG_AUDIT              13     
#define SYSLOGC_LOG_ALERT              14     
#define SYSLOGC_LOCAL0                 16     
#define SYSLOGC_LOCAL1                 17 
#define SYSLOGC_LOCAL2                 18 
#define SYSLOGC_LOCAL3                 19 
#define SYSLOGC_LOCAL4                 20 
#define SYSLOGC_LOCAL5                 21 
#define SYSLOGC_LOCAL6                 22 
#define SYSLOGC_LOCAL7                 23 

#define SYSLOGC_static7 	SYSLOGC_LOCAL7
/* syslog severity code */

#define SYSLOGC_SEVERITY_EMERGENCY         0
#define SYSLOGC_SEVERITY_ALERT             1
#define SYSLOGC_SEVERITY_CRITICAL          2
#define SYSLOGC_SEVERITY_ERROR             3
#define SYSLOGC_SEVERITY_WARNING           4
#define SYSLOGC_SEVERITY_NOTICE            5
#define SYSLOGC_SEVERITY_INFORMATIONAL     6
#define SYSLOGC_SEVERITY_DEBUG             7
#else
#define SYSLOGC_KERNEL_MSG             LOG_KERN
#define SYSLOGC_USER_LEVEL_MSG         LOG_USER
#define SYSLOGC_MAIL_SYSTEM            LOG_MAIL
#define SYSLOGC_SYSTEM_DAEMONS         LOG_DEAMON
#define SYSLOGC_SECURITY_MSG           LOG_AUTH
#define SYSLOGC_MSG_FROM_SYSLOGD       LOG_SYSLOG
#define SYSLOGC_LINE_PRINTER           LOG_LPR
#define SYSLOGC_NETWORK_NEWS           LOG_NEWS
#define SYSLOGC_UUCP_SUBSYS            LOG_UUCP
#define SYSLOGC_CLOCK_DAEMON           LOG_CRON
#define SYSLOGC_FTP_DAEMON             LOG_FTP
#define SYSLOGC_NTP_SUBSYSTEM          (12<<3)
#define SYSLOGC_LOG_AUDIT              (13<<3)
#define SYSLOGC_LOG_ALERT              (14<<3)
#define SYSLOGC_LOCAL0                 LOG_LOCAL0
#define SYSLOGC_LOCAL1                 LOG_LOCAL1
#define SYSLOGC_LOCAL2                 LOG_LOCAL2
#define SYSLOGC_LOCAL3                 LOG_LOCAL3
#define SYSLOGC_LOCAL4                 LOG_LOCAL4
#define SYSLOGC_LOCAL5                 LOG_LOCAL5
#define SYSLOGC_LOCAL6                 LOG_LOCAL6
#define SYSLOGC_LOCAL7                 LOG_LOCAL7

#define SYSLOGC_static7 	SYSLOGC_LOCAL7
/* syslog severity code */

#define SYSLOGC_SEVERITY_EMERGENCY         LOG_EMERG
#define SYSLOGC_SEVERITY_ALERT             LOG_ALERT
#define SYSLOGC_SEVERITY_CRITICAL          LOG_CRIT
#define SYSLOGC_SEVERITY_ERROR             LOG_ERR
#define SYSLOGC_SEVERITY_WARNING           LOG_WARNING
#define SYSLOGC_SEVERITY_NOTICE            LOG_NOTICE
#define SYSLOGC_SEVERITY_INFORMATIONAL     LOG_INFO
#define SYSLOGC_SEVERITY_DEBUG             LOG_DEBUG
#endif

/* misc. */

#define SYSLOGC_CODE_DEFAULT      (0xFFFF)

/* local macros */

/* constant and default value definitions */

#define TEMP_HOST_NAME_LEN    100
#define DFT_TAGID_LEN         32
#define SYSLOG_MSG_MAX_LEN    1024
#define DFT_HOST_NAME_LEN     40

/* various special values related to the syslog message format */

#define FIELD_DELIMITER        ' '
#define PRI_PART_LEADING_CHAR  '<'
#define PRI_PART_ENDING_CHAR   '>'
#define ASCII_ZERO  0x30
#define ASCII_a     0x60

/* indication of the type of the input data */

#define MDATA_TYPE   0x10    /* input data contained in mbuf */
#define BDATA_TYPE   0x20    /* input data in a binray buffer */
#define CDATA_TYPE   0x30    /* input data as NULL terminated string */

/* Macros for generic calculation */

#define BYTE_MS4BITS(x) (((x) >> 4) & 0x0f)
#define BYTE_LS4BITS(x) ((x) & 0x0f)
#ifndef  MIN
#define MIN(x,y)   ((x) > (y)? (y): (x))
#endif

struct syslog_client
{
	BOOL enable;
	enum { SYSLOG_UDP_MODE, SYSLOG_TCP_MODE } mode;
	int sock;
	int port;
	struct in_addr address;
	char  address_string[DFT_HOST_NAME_LEN];
	BOOL		dynamics;
	char  *hostname;//[DFT_HOST_NAME_LEN];
	char  *processname;//[DFT_HOST_NAME_LEN];

	int facility;

	BOOL connect;

	void *master;

	void *mutx;
};

/* function declarations */

#if defined(__STDC__) || defined(__cplusplus)

extern int syslogc_lib_init(void *, char *);
extern int syslogc_lib_uninit(void);
extern int syslogc_host_config_set(char *, int, int );
extern int syslogc_host_config_get(char *, int *, int *);

extern int syslogc_enable(char *);
extern BOOL syslogc_is_enable(void);
extern int syslogc_disable(void);
extern int syslogc_is_dynamics(void);
extern int syslogc_dynamics_enable(void);
extern int syslogc_dynamics_disable(void);
extern int syslogc_mode_set(int );
extern int syslogc_mode_get(int *);
extern int syslogc_facility_set(int );
extern int syslogc_facility_get(int *);

extern int vsysclog (int , int, char *, va_list );
extern int syslogc_out(int , int, char * , int );
/*
extern int syslogcLibInit (char *);
//extern STATUS syslogcMdataSend(M_BLK_ID,int,char *, UINT16,ULONG);
extern int syslogcBinDataSend (unsigned char *, int, char *,unsigned short, unsigned long);
extern int syslogcStringSend (char *, unsigned short, unsigned long);
*/

#else	/* __STDC__ */

extern int syslogcLibInit ();
extern void syslogcShutdown ();
extern STATUS syslogcMdataSend();
extern STATUS syslogcBinDataSend ();
extern STATUS syslogcStringSend ();

#endif	/* __STDC__ */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __INCslogClientLibh */
