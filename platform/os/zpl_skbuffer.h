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


typedef struct
{
	NODE	node;
    zpl_uint32 	skb_timetick;    //时间戳 毫秒
    zpl_uint32	skb_len;         //当前缓存帧的长度
    zpl_uint32	skb_maxsize;     //buffer 的长度
    zpl_char	*skb_data;       //buffer
    zpl_char	skb_hdr_len;     //buffer
}zpl_skb_data_t __attribute__ ((aligned (1)));

typedef struct 
{
	os_sem_t	*sem;
	os_mutex_t	*mutex;
    zpl_uint32	maxsize;
	LIST	list;			//add queue
	LIST	rlist;			//ready queue
	LIST	ulist;			//unuse queue
}zpl_skb_queue_t;

#define ZPL_SKB_HEAD(sk)        (((sk)->skb_hdr_len)?((sk)->skb_data):NULL)
#define ZPL_SKB_HEAD_LEN(sk)    ((sk)->skb_hdr_len)

#define ZPL_SKB_DATA(sk)        (((sk)->skb_data + ((sk)->skb_hdr_len)))
#define ZPL_SKB_DATA_LEN(sk)    (((sk)->skb_len) - ((sk)->skb_hdr_len))


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


extern int zpl_skb_data_append(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len);
extern int zpl_skb_data_head(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len);
extern zpl_skb_data_t * zpl_skb_data_malloc(zpl_skb_queue_t *queue, zpl_uint32 len);
extern zpl_skb_data_t * zpl_skb_data_clone(zpl_skb_queue_t *queue, zpl_skb_data_t *data);
extern int zpl_skb_data_copy(zpl_skb_data_t *dst, zpl_skb_data_t *src);
extern int zpl_skb_data_free(zpl_skb_data_t *data);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_SKBUFFER_H__ */
