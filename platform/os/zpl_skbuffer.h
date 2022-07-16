/*
 * zpl_skbuffer.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_SKBUFFER_H__
#define __ZPL_SKBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"
#include "os_list.h"


#define ZPL_SKB_START_OFFSET    64
#define ZPL_SKBUF_ALIGN(n)      (((((n)+3)/4)*4))
#define ZPL_SKSIZE_ALIGN(n)     (((((n)+3)/4)*4) + ZPL_SKB_START_OFFSET)

enum zpl_netpkt_reason 
{
    NETPKT_REASON_NONE,
    NETPKT_REASON_INVALID = NETPKT_REASON_NONE,
    NETPKT_REASON_ARP, 
    NETPKT_REASON_BPDU, 
    NETPKT_REASON_BROADCAST, 
    NETPKT_REASON_CLASSBASEDMOVE, 
    NETPKT_REASON_CLASSTAGPACKETS, 
    NETPKT_REASON_CONTROL, 
    NETPKT_REASON_CPULEARN, 
    NETPKT_REASON_DESTLOOKUPFAIL,
    NETPKT_REASON_DHCP, 
    NETPKT_REASON_DOSATTACK, 
    NETPKT_REASON_E2EHOLIBP, 
    NETPKT_REASON_ENCAPHIGIGERROR, 
    NETPKT_REASON_FILTERMATCH, 
    NETPKT_REASON_GRECHECKSUM, 
    NETPKT_REASON_GRESOURCEROUTE, 
    NETPKT_REASON_HIGIGCONTROL, 
    NETPKT_REASON_HIGIGHDRERROR, 
    NETPKT_REASON_ICMPREDIRECT, 
    NETPKT_REASON_IGMP, 
    NETPKT_REASON_INGRESSFILTER, 
    NETPKT_REASON_IP, 
    NETPKT_REASON_IPFIXRATEVIOLATION, 
    NETPKT_REASON_IPMCASTMISS, 
    NETPKT_REASON_IPMCRESERVED, 
    NETPKT_REASON_IPOPTIONVERSION, 
    NETPKT_REASON_IPMC, 
    NETPKT_REASON_L2CPU, 
    NETPKT_REASON_L2DESTMISS, 
    NETPKT_REASON_L2LEARNLIMIT, 
    NETPKT_REASON_L2MOVE, 
    NETPKT_REASON_L2MTUFAIL, 
    NETPKT_REASON_L2NONUNICASTMISS, 
    NETPKT_REASON_L2SOURCEMISS, 
    NETPKT_REASON_L3ADDRBINDFAIL, 
    NETPKT_REASON_L3DESTMISS, 
    NETPKT_REASON_L3HEADERERROR, 
    NETPKT_REASON_L3MTUFAIL, 
    NETPKT_REASON_L3SLOWPATH, 
    NETPKT_REASON_L3SOURCEMISS, 
    NETPKT_REASON_L3SOURCEMOVE, 
    NETPKT_REASON_MARTIANADDR, 
    NETPKT_REASON_MCASTIDXERROR, 
    NETPKT_REASON_MCASTMISS, 
    NETPKT_REASON_MIMSERVICEERROR, 
    NETPKT_REASON_MPLSCTRLWORDERROR, 
    NETPKT_REASON_MPLSERROR, 
    NETPKT_REASON_MPLSINVALIDACTION, 
    NETPKT_REASON_MPLSINVALIDPAYLOAD, 
    NETPKT_REASON_MPLSLABELMISS, 
    NETPKT_REASON_MPLSSEQUENCENUMBER, 
    NETPKT_REASON_MPLSTTL, 
    NETPKT_REASON_MULTICAST, 
    NETPKT_REASON_NHOP, 
    NETPKT_REASON_OAMERROR, 
    NETPKT_REASON_OAMSLOWPATH, 
    NETPKT_REASON_OAMLMDM, 
    NETPKT_REASON_PARITYERROR, 
    NETPKT_REASON_PROTOCOL, 
    NETPKT_REASON_SAMPLEDEST, 
    NETPKT_REASON_SAMPLESOURCE, 
    NETPKT_REASON_SHAREDVLANMISMATCH, 
    NETPKT_REASON_SOURCEROUTE, 
    NETPKT_REASON_TIMESTAMP, 
    NETPKT_REASON_TTL, 
    NETPKT_REASON_TTL1, 
    NETPKT_REASON_TUNNELERROR, 
    NETPKT_REASON_UDPCHECKSUM, 
    NETPKT_REASON_UNKNOWNVLAN, 
    NETPKT_REASON_URPFFAIL, 
    NETPKT_REASON_VCLABELMISS, 
    NETPKT_REASON_VLANFILTERMATCH, 
    NETPKT_REASON_WLANCLIENTERROR, 
    NETPKT_REASON_WLANSLOWPATH, 
    NETPKT_REASON_WLANDOT1XDROP, 
    NETPKT_REASON_EXCEPTIONFLOOD, 
    NETPKT_REASON_TIMESYNC, 
    NETPKT_REASON_EAVDATA, 
    NETPKT_REASON_SAMEPORTBRIDGE, 
    NETPKT_REASON_SPLITHORIZON, 
    NETPKT_REASON_L4ERROR, 
    NETPKT_REASON_STP, 
    NETPKT_REASON_EGRESSFILTERREDIRECT, 
    NETPKT_REASON_FILTERREDIRECT, 
    NETPKT_REASON_LOOPBACK, 
    NETPKT_REASON_VLANTRANSLATE, 
    NETPKT_REASON_MMRP, 
    NETPKT_REASON_SRP, 
    NETPKT_REASON_TUNNELCONTROL, 
    NETPKT_REASON_L2MARKED, 
    NETPKT_REASON_WLANSLOWPATHKEEPALIVE, 
    NETPKT_REASON_STATION, 
    NETPKT_REASON_NIV, 
    NETPKT_REASON_NIVPRIODROP, 
    NETPKT_REASON_NIVINTERFACEMISS, 
    NETPKT_REASON_NIVRPFFAIL, 
    NETPKT_REASON_NIVTAGINVALID, 
    NETPKT_REASON_NIVTAGDROP, 
    NETPKT_REASON_NIVUNTAGDROP, 
    NETPKT_REASON_TRILL, 
    NETPKT_REASON_TRILLINVALID, 
    NETPKT_REASON_TRILLMISS, 
    NETPKT_REASON_TRILLRPFFAIL, 
    NETPKT_REASON_TRILLSLOWPATH, 
    NETPKT_REASON_TRILLCOREISIS, 
    NETPKT_REASON_TRILLTTL, 
    NETPKT_REASON_TRILLNAME, 
    NETPKT_REASON_COUNT,
    NETPKT_REASON_COPYTOCPU,
	NETPKT_REASON_TONETDEV,      
};

	
typedef struct
{
    zpl_uint8       unit;                         /* Unit number. */
    zpl_uint8       cos;                          /* The local COS queue to use. */
    zpl_uint8       prio_int;                     /* Internal priority of the packet. */
    vlan_t          tpid;
    vlan_t          vlan;                    /* 802.1q VID or VSI or VPN. */
    zpl_uint8       vlan_pri;                     /* Vlan tag priority . */
    zpl_uint8       vlan_cfi;                     /* Vlan tag CFI bit. */
    vlan_t          inner_tpid;
    vlan_t          inner_vlan;              /* Inner VID or VSI or VPN. */
    zpl_uint8       inner_vlan_pri;               /* Inner vlan tag priority . */
    zpl_uint8       inner_vlan_cfi;               /* Inner vlan tag CFI bit. */
    zpl_uint16      ethtype;

    zpl_uint8       color;                  /* Packet color. */
    zpl_uint32      reason;         /* Opcode from packet. */
    zpl_uint8       untagged;       /* The packet was untagged on ingress. */
    ifindex_t       ifindex;
    zpl_phyport_t   trunk;          /* Source trunk group ID used in header/tag, -1 if src_port set . */
    zpl_phyport_t   phyid;          /* Source port used in header/tag. */
    zpl_uint32      timestamp;
    volatile zpl_uint32      reference;

}zpl_netpkt_hdr_t __attribute__ ((aligned (1)));

typedef struct
{
    zpl_uint16 	ID;                 //ID 通道号
    zpl_uint8 	buffer_type;        //音频视频
    zpl_uint8 	buffer_codec;       //编码类型
    zpl_uint8 	buffer_key;         //帧类型
    zpl_uint8 	buffer_rev;         //
    zpl_uint16 	buffer_flags;       //ZPL_BUFFER_DATA_E
    zpl_uint32 	buffer_timetick;    //时间戳毫秒
    zpl_uint32 	buffer_seq;         //序列号底层序列号
    zpl_uint32	buffer_len;         //帧长度
}zpl_media_hdr_t __attribute__ ((aligned (1)));


typedef struct
{
	NODE	node;
    union 
    {
        zpl_netpkt_hdr_t    net_header;
        zpl_media_hdr_t     media_header;
    }skb_header;

    zpl_void    *device;

    zpl_uint32 	skb_timetick;    //时间戳 毫秒
    zpl_uint32	skb_len;         //当前缓存帧的长度
    zpl_uint32	skb_maxsize;     //buffer 的长度
    zpl_char	*skb_data;       //buffer
    zpl_uint32	skb_start;
    volatile zpl_uint8   reference;       //引用计数
}zpl_skb_data_t __attribute__ ((aligned (1)));

typedef struct 
{
	void	*sem;
	void	*mutex;
    zpl_uint32	maxsize;
	LIST	    list;			//add queue
	LIST	    rlist;			//ready queue
	LIST	    ulist;			//unuse queue
}zpl_skb_queue_t;


#define ZPL_SKB_DATA(sk)        (((sk)->skb_data + ((sk)->skb_start)))
#define ZPL_SKB_DATA_LEN(sk)    (((sk)->skb_len))


extern zpl_uint32 zpl_skb_timerstamp(void);


extern zpl_skb_queue_t *zpl_skb_queue_create(zpl_uint32 maxsize, zpl_bool sem);
extern int zpl_skb_queue_enqueue(zpl_skb_queue_t *queue, zpl_skb_data_t *data);
extern zpl_skb_data_t * zpl_skb_queue_dequeue(zpl_skb_queue_t *queue);
extern int zpl_skb_queue_finsh(zpl_skb_queue_t *queue, zpl_skb_data_t *data);
extern int zpl_skb_queue_destroy(zpl_skb_queue_t *queue);
extern int zpl_skb_queue_add(zpl_skb_queue_t *queue, zpl_skb_data_t *data);
extern int zpl_skb_queue_put(zpl_skb_queue_t *queue, zpl_skb_data_t *data);
extern zpl_skb_data_t *zpl_skb_queue_get(zpl_skb_queue_t *queue);
extern int zpl_skb_queue_distribute(zpl_skb_queue_t *queue, int(*func)(zpl_skb_data_t*, void *), void *p);

extern int zpl_skb_data_push(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len);
extern int zpl_skb_data_pull(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len);

extern int zpl_skb_data_append(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len);
extern zpl_skb_data_t * zpl_skb_data_malloc(zpl_skb_queue_t *queue, zpl_uint32 len);
extern zpl_skb_data_t * zpl_skb_data_clone(zpl_skb_queue_t *queue, zpl_skb_data_t *data);

extern int zpl_skb_data_free(zpl_skb_data_t *data);

/* net pkt */
extern int zpl_skb_data_net_header(zpl_skb_data_t *bufdata, zpl_netpkt_hdr_t *data);
extern zpl_netpkt_hdr_t * zpl_skb_netpkt_hdrget(zpl_skb_data_t *src);



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_SKBUFFER_H__ */
