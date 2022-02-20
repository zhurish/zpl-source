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

#define OS_QUEUE_NAME_MAX	32


typedef struct os_queue_data_s
{
	NODE	node;
	zpl_uint32	size;
	zpl_char	*data;
}os_queue_data_t;


typedef struct os_queue
{
	os_sem_t	*sem;
	os_mutex_t	*mutex;
	zpl_uint32	max;
	zpl_uint32	size;
	zpl_uint32	use;
	zpl_char	name[OS_QUEUE_NAME_MAX];
	LIST	list;
	LIST	ulist;

}os_queue_t;


extern int os_msgq_init(void);
extern int os_msgq_exit(void);


extern os_queue_t *os_queue_create(zpl_uint32 max, zpl_uint32 size);
extern int os_queue_name(os_queue_t *queue, zpl_char *name);
//int os_queue_info(os_queue_t *queue, zpl_char *name);
extern int os_queue_send(os_queue_t *queue, zpl_char *data, zpl_uint32 len, zpl_uint32 op);
extern int os_queue_recv(os_queue_t *queue, zpl_char *name, zpl_uint32 len, zpl_uint32 timeout);
extern int os_queue_delete(os_queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif /* __OS_QUEUE_H__ */
