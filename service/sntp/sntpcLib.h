/* sntpcLib.h - Simple Network Time Protocol client include file */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
Modification history 
--------------------
01f,23aug04,rp   merged from COMP_WN_IPV6_BASE6_ITER5_TO_UNIFIED_PRE_MERGE
01e,04nov03,rlm  Ran batch header path update for header re-org.
01d,03nov03,rlm  Removed wrn/coreip/ prefix from #includes for header re-org.
01c,25aug99,cno  Add extern "C" definition (SPR21825)
01b,15jul97,spm  code cleanup, documentation, and integration; entered in
                 source code control
01a,20apr97,kyc  written

*/

#ifndef __INCsntpch
#define __INCsntpch
#ifdef __cplusplus
extern "C" {
#endif
/* includes */

#include <sntp.h>

/* defines */

#define S_sntpcLib_INVALID_PARAMETER         (M_sntpcLib | 1)
#define S_sntpcLib_INVALID_ADDRESS           (M_sntpcLib | 2)
#define S_sntpcLib_TIMEOUT                   (M_sntpcLib | 3)
#define S_sntpcLib_VERSION_UNSUPPORTED       (M_sntpcLib | 4)
#define S_sntpcLib_SERVER_UNSYNC             (M_sntpcLib | 5)

#define SNTP_CLIENT_REQUEST 0x0B             /* standard SNTP client request */


struct sntp_client
{
	zpl_uint8 enable;
	zpl_socket_t sock;
	zpl_uint8 type;
#define SNTP_PASSIVE	1
	zpl_uint8 leapVerMode;
//    zpl_uint32 timezone;
    zpl_uint32 sync_clock;/* local clock is sync flag */

	zpl_uint8 time_debug;

    zpl_ushort sntpcPort;
    zpl_ushort sntpc_interval;
    struct ipstack_in_addr address;
    zpl_bool		dynamics;

    struct timespec sntpTime;	/* storage for retrieved time value */

    void *master;
    struct thread *t_read;	/* read to output ipstack_socket. */
    struct thread *t_write;	/* Write to output ipstack_socket. */
    struct thread *t_time;	/* Write to output ipstack_socket. */
    void *mutex;
};

#ifndef INET6
/* SNTP data retrived from SNTP IPv4 protocol message in mSntpcTimeGet */
typedef struct sntpData
    {
    zpl_uint8     stratum;                 
    char              poll;
    char              precision;
    } SNTP_DATA;

IMPORT STATUS mSntpcTimeGet (char *, zpl_uint32, struct timespec *, char *, 
                             SNTP_DATA *);

#endif


extern int sntpcInit (void *);
extern int sntpcDisable(void);
extern int sntpc_config(struct vty *);
extern int sntpc_debug_config(struct vty *);

extern int sntpc_is_sync(void);
extern int sntpc_server_address(struct ipstack_in_addr *);

extern int sntpc_is_dynamics(void);
extern int sntpc_dynamics_enable(void);
extern int sntpc_dynamics_disable(void);

extern int cmd_sntpc_init();
//#define SNTPC_CLI_ENABLE
#ifndef SNTPC_CLI_ENABLE

enum
{
	API_SNTPC_SET_ENABLE = 1,
	API_SNTPC_SET_ADDRESS,
	API_SNTPC_SET_PORT,
	API_SNTPC_SET_INTERVAL,
	API_SNTPC_SET_VERSION,
	API_SNTPC_SET_TIMEZONE,
	API_SNTPC_SET_PASSIVE,
	API_SNTPC_SET_DEBUG,

	API_SNTPC_GET_ENABLE,
	API_SNTPC_GET_ADDRESS,
	API_SNTPC_GET_PORT,
	API_SNTPC_GET_INTERVAL,
	API_SNTPC_GET_VERSION,
	API_SNTPC_GET_TIMEZONE,
	API_SNTPC_GET_PASSIVE,
	API_SNTPC_GET_DEBUG,
};


extern int vty_show_sntpc_client(struct vty *vty);

extern int sntpc_client_set_api(struct vty *, zpl_uint32 cmd, const char *value);
extern int sntpc_client_dynamics_set_api(struct vty *, zpl_uint32 cmd, const char *value);
extern int sntpc_client_get_api(struct vty *, zpl_uint32 cmd, const char *value);
extern int sntpc_client_dynamics_get_api(struct vty *vty, zpl_uint32 cmd, const char *value);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __INCsntpch */

