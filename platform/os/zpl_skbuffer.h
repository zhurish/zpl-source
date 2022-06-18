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
    NETPKT_REASON_COPYTOCPU,
	NETPKT_REASON_TONETDEV,
};

	
typedef struct
{
    zpl_uint8       unit;                         /* Unit number. */
    zpl_uint8       cos;                          /* The local COS queue to use. */
    zpl_uint8       prio_int;                     /* Internal priority of the packet. */
    vlan_t          vlan;                    /* 802.1q VID or VSI or VPN. */
    zpl_uint8       vlan_pri;                     /* Vlan tag priority . */
    zpl_uint8       vlan_cfi;                     /* Vlan tag CFI bit. */
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
