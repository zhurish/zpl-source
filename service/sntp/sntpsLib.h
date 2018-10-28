/* sntpsLib.h - Simple Network Time Protocol (SNTP) server include file */

/* Copyright 1984-2001 Wind River Systems, Inc. */

/*
Modification history 
--------------------
01e,04nov03,rlm  Ran batch header path update for header re-org.
01d,03nov03,rlm  Removed wrn/coreip/ prefix from #includes for header re-org.
01c,30aug01,vvv  added extern "C" definition (SPR #21825)
01b,15jul97,spm  code cleanup, documentation, and integration; entered in
                 source code control
01a,20apr97,kyc  written

*/

#ifndef __INCsntpsh
#define __INCsntpsh

#ifdef __cplusplus
extern "C" {
#endif

/* includes */

#include <sntp.h>

/* defines */

#define S_sntpsLib_INVALID_PARAMETER         (M_sntpsLib | 1)
#define S_sntpsLib_INVALID_ADDRESS           (M_sntpsLib | 2)

/* Valid settings for SNTPS_MODE in configAll.h */

#define  SNTP_ACTIVE 		1
#define  SNTP_PASSIVE 		2

/* Values for "request" parameter to clock hook routine. */

#define SNTPS_ID 		1
#define SNTPS_RESOLUTION 	2
#define SNTPS_TIME 		3

/* Values for "setting" parameter to sntpsConfigSet() routine. */

#define SNTPS_ADDRESS 		1
#define SNTPS_DELAY 		2
#define SNTPS_MODE 		3

#define SNTPS_BROADCAST 		1
#define SNTPS_UNICAST 		2
#define SNTPS_MULTICAST 		3

struct sntp_server
{
	int enable;
	int sock;
	int mode;

	unsigned char leap;
	unsigned char version;
    unsigned char Mode;
    unsigned char stratum;
    unsigned char poll;

    int sntpsInterval;
    u_short sntpsPort;
    struct in_addr address;
    int sntps_ttl;
	unsigned char time_debug;

    ULONG 	sntpsClockId;         /* SNTP clock identifier. */
    ULONG   sntpsRefId;           /* Default reference identifier. ָʾʱ�Ӳο�Դ�ı�ǣ����ֶ�ֻ�ڷ���������Ч */
    ULONG   sntpsResolution;      /* Clock resolution, in nanoseconds. */
    INT8    sntpsPrecision;       /* Clock precision, in RFC format. */
    int		sntpsClockReady;
//    FUNCPTR sntpsClockHookRtn;
    BOOL (*sntpsClockHookRtn)(int, void *);

    void *master;
    void *obuf;
    struct thread *t_read;	/* read to output socket. */
    struct thread *t_write;	/* Write to output socket. */
    struct thread *t_time;	/* Write to output socket. */

    void *mutex;
};

typedef struct sntpsTimeData
    {
    ULONG seconds;
    ULONG fraction;
    } SNTP_TIMESTAMP;


extern int sntps_config(struct vty *);
extern int sntps_debug_config(struct vty *);
extern int sntpsInit(void *);

//#define SNTPS_CLI_ENABLE
#ifndef SNTPS_CLI_ENABLE
enum
{
	API_SNTPS_SET_ENABLE = 1,
	API_SNTPS_SET_LISTEN,
	API_SNTPS_SET_VERSION,
	API_SNTPS_SET_MODE,
	API_SNTPS_SET_INTERVAL,
	API_SNTPS_SET_TTL,
	API_SNTPS_SET_DEBUG,

	API_SNTPS_GET_ENABLE,
	API_SNTPS_GET_LISTEN,
	API_SNTPS_GET_VERSION,
	API_SNTPS_GET_MODE,
	API_SNTPS_GET_INTERVAL,
	API_SNTPS_GET_TTL,
	API_SNTPS_GET_DEBUG,
};


extern int vty_show_sntps_server(struct vty *vty);

extern int sntp_server_set_api(struct vty *, int cmd, const char *value);
extern int sntp_server_get_api(struct vty *, int cmd, const char *value);

#endif

/*IMPORT STATUS sntpsInit (char *, u_char, char *, short, u_short, FUNCPTR);
IMPORT STATUS sntpsClockSet (FUNCPTR);
IMPORT ULONG  sntpsNsecToFraction (ULONG);
IMPORT STATUS sntpsConfigSet (int, void *);*/

#ifdef __cplusplus
}
#endif

#endif /* __INCsntpsh */

