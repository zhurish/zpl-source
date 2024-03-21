/*
 * os_queue.h
 *
 *  Created on: Jun 9, 2018
 *      Author: zhurish
 */

#ifndef __OS_QUEUE_H__
#define __OS_QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif



#define OS_QDATA_START_OFFSET    64
#define OS_QDATA_ALIGN(n)      (((((n)+3)/4)*4))
#define OS_QDATA_SIZE_ALIGN(n)     (((((n)+3)/4)*4) + OS_QDATA_START_OFFSET)

typedef struct os_queue_data_s
{
	NODE	node;
    zpl_uint32	skb_len;         //当前缓存帧的长度
    zpl_uint32	skb_maxsize;     //buffer 的长度
    zpl_char	*skb_data;       //buffer
    zpl_uint32	skb_start;
	zpl_uint32	skb_timetick;
    unsigned char *b_rptr;
	unsigned char *b_wptr;
	zpl_uint32	res;
	zpl_uint32	res1;
	zpl_uint32	res2;
}os_qdata_t;

typedef enum  
{
    OS_QDATA_FLAGS_NONE         = 0,
    OS_QDATA_FLAGS_ASYNC        = 1,   //asynchronous
	OS_QDATA_FLAGS_LIMIT_MAX    = 2,   //Maximum limit
	OS_QDATA_FLAGS_NEEDFREE     = 3,   //Maximum limit    
}os_queue_flags_t;

typedef struct os_queue
{
    char    *name;
	void	*sem;
	void	*mutex;

    zpl_uint32	max_num;
	LIST	    list;			//add queue
	LIST	    ulist;			//unuse queue
    zpl_int32   queue_flag;
    zpl_int32   sync_wait;
    zpl_void    *privatedata;
}os_queue_t;


extern int os_msgq_init(void);
extern int os_msgq_exit(void);

#define OS_QUEUE_MAXNUM(q)  ((q)->max_num)
#define OS_QUEUE_NUM(q)  (lstCount(&(q)->list))

#define OS_QUEUE_DATA(sk)        (((sk)->skb_data + ((sk)->skb_start)))
#define OS_QUEUE_DATA_LEN(sk)    (((sk)->skb_len))
#define OS_QUEUE_DATA_START(sk)    (((sk)->skb_start))
/* os_queue_t */
extern int os_queue_init(os_queue_t *q, char *name, zpl_uint32 max_num, zpl_bool sem);
extern os_queue_t *os_queue_create(char *name, zpl_uint32 max_num, zpl_bool sem);
extern int os_queue_destroy(os_queue_t *queue);
extern int os_queue_flush(os_queue_t *queue);
extern int os_queue_attribute_set(os_queue_t *queue, zpl_int32);
extern int os_queue_attribute_unset(os_queue_t *queue, zpl_int32);
extern int os_queue_attribute_get(os_queue_t *queue);
extern int os_queue_set_privatedata(os_queue_t *queue, zpl_void *privatedata);
extern zpl_void *os_queue_get_privatedata(os_queue_t *queue);

extern int os_queue_async_enqueue(os_queue_t *queue, os_qdata_t *skbuf);
extern os_qdata_t * os_queue_async_dequeue(os_queue_t *queue);
os_qdata_t *os_queue_async_wait_dequeue(os_queue_t *queue, int waitms);

extern int os_queue_finsh(os_queue_t *queue, os_qdata_t *skbuf);
extern int os_queue_add(os_queue_t *queue, os_qdata_t *skbuf);
extern int os_queue_insert(os_queue_t *queue, os_qdata_t *skbufp, os_qdata_t *skbuf);
extern int os_queue_insert_befor(os_queue_t *queue, os_qdata_t *skbufp, os_qdata_t *skbuf);
extern os_qdata_t *os_queue_get(os_queue_t *queue);

extern int os_queue_distribute(os_queue_t *queue, int(*func)(os_qdata_t*, void *), void *p);
extern int os_queue_async_wait_distribute(os_queue_t *queue, int sync_wait_ms, int(*func)(os_qdata_t*, void *), void *p);

/* os_qdata_t */
/* 在报文offset前面添加数据 */
extern int os_qdata_push(os_qdata_t *skbuf, uint32_t offset, uint8_t *data, uint32_t len);
/* 在报文offset处删除数据 */
extern int os_qdata_pull(os_qdata_t *skbuf, uint32_t offset, uint32_t len);

/* 报文后面数据 */
extern int os_qdata_put(os_qdata_t *skbuf, uint8_t *data, uint32_t len);

extern int os_qdata_init_default(os_qdata_t *skbuf, int maxlen);

extern os_qdata_t * os_qdata_create(os_queue_t *queue, zpl_uint32 len);
extern os_qdata_t * os_qdata_clone(os_queue_t *queue, os_qdata_t *skbuf);

extern int os_qdata_destroy(os_qdata_t *skbuf);


#ifdef __cplusplus
}
#endif

#endif /* __OS_QUEUE_H__ */
