/* ospf.h */

#if !defined (_OSPF_H_)
#define _OSPF_H_

#include <zebra.h>
#include "memory.h"
#include "memtypes.h"
#include "md5.h"
#include "pqueue.h"
#include "prefix.h"
#include "if_manage.h"
#include "rib.h"

//#define OSPF_RTSOCK_ENABLE
//#define OSPF_CMDSOCK_ENABLE

/*bfd for ospf*/
//#define  HAVE_BFD
#define  OSPF_POLICY
#define  OSPF_REDISTRIBUTE
#define	 OSPF_FRR
#define  OSPF_VPN

#define  OSPF_PTP
//#define  OSPF_TE


#ifdef OSPF_POLICY
#define HAVE_ROUTEPOLICY
#endif



#include "bitmap.h"
#include "avl.h"
#include "../../platform/lib/list.h"
#include "ospf_memory.h"
#include "ospf_table.h"
#include "ospf_main.h"
#include "ospf_export.h"
#include "ospf_nm.h"
#include "ospf_area.h"
#include "ospf_te_interface.h"
#include "ospf_lsa.h"
#include "ospf_dcn.h"
#include "ospf_packet.h"
#include "ospf_interface.h"
#include "ospf_os.h"
#include "ospf_nbr.h"
#include "ospf_trap.h"
#include "ospf_spf.h"
#include "ospf_route.h"
#include "ospf_redistribute.h"
#include "ospf_restart.h"
#include "ospf_util.h"
#include "ospf_syn.h"
#include "ospf_frr.h"
#include "ospf_dcn.h"
#include "ospf_relation.h"
#include "ospf_pal.h"

#if HOST == OSIX_LITTLE_ENDIAN
#define OSIX_NTOHL(x) ((((x) & 0xFF000000)>>24) |  (((x) & 0x00FF0000)>>8) | \
                       (((x) & 0x0000FF00)<<8 ) |  (((x) & 0x000000FF)<<24))
#define OSIX_NTOHS(x)  ((((x) & 0xFF00)>>8) |  (((x) & 0x00FF)<<8))
#define OSIX_HTONL(x) OSIX_NTOHL(x)
#define OSIX_HTONS(x) OSIX_NTOHS(x)
#else
#define OSIX_NTOHL(x) (x)
#define OSIX_NTOHS(x) (x)
#define OSIX_HTONL(x) (x)
#define OSIX_HTONS(x) (x)
#endif /* HOST == OSIX_LITTLE_ENDIAN */


#ifdef USE_IPSTACK_KERNEL
#define USE_LINUX_OS
#endif

#define USP_MULTIINSTANCE_WANTED 1

#if 0
/*OSPF模块错误码定义*/	
#define EOSPF_NOAREA				((EBASE_OSPF<<16) | 0x1)	/*1.	Failed.Area doesn't exist. ���򲻴���*/
#define EOSPF_NULLAUTHTYPE			((EBASE_OSPF<<16) | 0x2)	/*2.	Failed.Authenticate type is null. ��֤����Ϊ��*/
#define EOSPF_TOOBIGTOS				((EBASE_OSPF<<16) | 0x3)	/*3.	Failed.Tos value exceeds the maximum. Tosֵ���������ֵ*/
#define EOSPF_NORANGE				((EBASE_OSPF<<16) | 0x4)	/*4.	Failed.Range doesn't exist. ��Χ������*/
#define EOSPF_NULLIFAREA			((EBASE_OSPF<<16) | 0x5)	/*5.	Failed.The interface's area is null. �ӿ���������Ϊnull*/
#define EOSPF_NONEIGH				((EBASE_OSPF<<16) | 0x6)	/*6.	Failed.Neighbor doesn't exist. �ھӲ�����*/
#define EOSPF_NONETORAREANOMATCH	((EBASE_OSPF<<16) | 0x7)	/*7.	Failed.Network doesn't exist or its area doesn't match.  Network�����ڻ������ڵ�����ƥ��*/
#define EOSPF_NOREDISTCFG			((EBASE_OSPF<<16) | 0x8)	/*8.	Failed.Redistribute policy doesn't exist. �ط�����Բ�����*/
#define EOSPF_NOFILTER				((EBASE_OSPF<<16) | 0x9)	/*9.	Failed.Filter policy doesn't exist. ���˲��Բ�����*/
#endif
/*定义ospf的操作状态*/
#define OSPF_IF_UP       1
#define OSPF_IF_DOWN     2
#define OSPF_IF_RUNNING  3
#define OSPF_IF_OPERATIVE  4
#define OSPF_IF_TEST  5

#ifndef OK
#define OK   0
#endif

#ifndef ERR
#define ERR  -1
#endif



#define IFINDEX_INTERNAL   0  /* 指示无效的接口 */  
#define IFINDEX_LOOPBACK   1  /* 设备环回接口lo  */
#define IFINDEX_LOCALETH   2  /* 本地网管口eth0 */

#define OSPF_QUEUE_LEN 100
//#define OSPF_MSG_SIZE  100
#define OSPF_VRF_NOMATCH   1

#define INVALID_VPN_ID    0xFFFFFFFF

/*declare of all struct*/
struct ospf_retransmit;
struct ospf_ackinfo;
struct ospf_route;
struct ospf_lsa;
struct ospf_request_node;
struct ospf_lshdr;
struct ospf_updateinfo;
struct ospf_nssa_lsa;
struct ospf_dd_msg;
struct ospf_nbr;
struct ospf_iproute;
struct ospf_if;

//#define OSIX_TICKS_PERSEC  100
//#define sysClkRateGet()   OSIX_TICKS_PERSEC

typedef struct
{
    u_int ulAreaIndx;
    u_int ulProIndx;
}OSPF_PRO_AREA_T;


u_short checksum(u_short *pAddr, int len);
//u_int8* inet_ntoa_1(u_int8 *ip_str,u_int ipaddress);
int errnoGet();
STATUS rtSockOptionSet(int sockfd,int protoType,int opType);
int log_time_print (u_int8 *buf);

void in_len2mask(struct in_addr *mask, int len);
int in_mask2len(struct in_addr *mask);
//int inet_maskLen(unsigned int netmask);
 
#endif /* _OSPF_H_ */

