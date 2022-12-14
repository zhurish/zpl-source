
#define CONFIG_GOBINET
#define CONFIG_QMIWWAN
#define CONFIG_SIM
#define CONFIG_APN
#define CONFIG_VERSION
#define CONFIG_DEFAULT_PDP 1

#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stddef.h>

#include "MPQMI.h"
#include "MPQCTL.h"
#include "MPQMUX.h"

#pragma pack(push, 1)

typedef struct _QCQMIMSG {
    QCQMI_HDR QMIHdr;
    union {
        QMICTL_MSG CTLMsg;
        QMUX_MSG MUXMsg;
    };
} __attribute__ ((packed)) QCQMIMSG, *PQCQMIMSG;

#pragma pack(pop)

typedef struct __IPV4 {
    ULONG Address;
    ULONG Gateway;
    ULONG SubnetMask;
    ULONG DnsPrimary;
    ULONG DnsSecondary;
} IPV4_T;

typedef struct __PROFILE {
    const char *apn;
    const char *user;
    const char *password;
    const char *pincode;
    int auth;
    int pdp;
    int IPType;
    int rawIP;
} PROFILE_T;

typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
    SIM_NETWORK_PERSONALIZATION = 5,
    SIM_BAD = 6,
} SIM_Status;

#define WDM_DEFAULT_BUFSIZE	256
#define RIL_REQUEST_QUIT    0x1000
#define RIL_INDICATE_DEVICE_CONNECTED    0x1002
#define RIL_INDICATE_DEVICE_DISCONNECTED    0x1003
#define RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED    0x1004
#define RIL_UNSOL_DATA_CALL_LIST_CHANGED    0x1005

extern int pthread_cond_timeout_np(pthread_cond_t *cond, pthread_mutex_t * mutex, unsigned msecs);
extern int QmiThreadSendQMI(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse);
extern int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse, unsigned msecs);
extern void QmiThreadRecvQMI(PQCQMIMSG pResponse);
extern int QmiWwanInit(void);
extern int QmiWwanDeInit(void);
extern int QmiWwanSendQMI(PQCQMIMSG pRequest);
extern void * QmiWwanThread(void *pData);
extern int GobiNetSendQMI(PQCQMIMSG pRequest);
extern void * GobiNetThread(void *pData);
extern void udhcpc_start(const char *ifname, int IPType, int rawIP);
extern void udhcpc_stop(const char *ifname);
extern void dump_qmi(void *dataBuffer, int dataLen);
extern void qmidevice_send_event_to_main(int triger_event);
extern int requestSetEthMode(PROFILE_T *profile);
extern int requestGetSIMStatus(SIM_Status *pSIMStatus);
extern int requestEnterSimPin(const CHAR *pPinCode);
extern int requestRegistrationState(UCHAR *pPSAttachedState);
extern int requestQueryDataCall(UCHAR  *pConnectionStatus);
extern int requestSetupDataCall(PROFILE_T *profile);
extern int requestDeactivateDefaultPDP(void);
extern int requestSetProfile(PROFILE_T *profile);
extern int requestGetProfile(PROFILE_T *profile);
extern int requestBaseBandVersion(const char **pp_reversion);
extern int requestGetIPAddress(IPV4_T *pIpv4);

extern int requestGetPINStatus(SIM_Status *pSIMStatus);
extern int requestGetHomeNetwork(USHORT *p_mcc, USHORT *p_mnc, USHORT *p_sid, USHORT *p_nid);

PQMI_TLV_HDR GetTLV (PQCQMUX_MSG_HDR pQMUXMsgHdr, int TLVType);

unsigned long ifc_get_addr(const char *ifname);

extern FILE *logfilefp;
extern int debug_qmi;
extern char * qmichannel;
extern int qmidevice_control_fd[2];
extern int clientWDS;
extern int clientDMS;
extern int clientNAS;
extern int clientWDA;
extern void dbg_time (const char *fmt, ...);
extern USHORT le16_to_cpu(USHORT v16);
extern UINT  le32_to_cpu (UINT v32);
extern USHORT cpu_to_le16(USHORT v16);
extern UINT cpu_to_le32(UINT v32);
