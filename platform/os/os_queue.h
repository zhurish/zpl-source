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


typedef struct queue_s
{
	NODE	node;
	ospl_uint32	size;
	ospl_char	*data;
}queue_t;


typedef struct os_queue
{
	os_sem_t	*sem;
	os_mutex_t	*mutex;
	ospl_uint32	max;
	ospl_uint32	size;
	ospl_uint32	use;
	ospl_char	name[OS_QUEUE_NAME_MAX];
	LIST	list;
	LIST	ulist;

}os_queue_t;


extern int os_msgq_init();
extern int os_msgq_exit();


extern os_queue_t *os_queue_create(ospl_uint32 max, ospl_uint32 size);
extern int os_queue_name(os_queue_t *queue, ospl_char *name);
//int os_queue_info(os_queue_t *queue, ospl_char *name);
extern int os_queue_send(os_queue_t *queue, ospl_char *data, ospl_uint32 len, ospl_uint32 op);
extern int os_queue_recv(os_queue_t *queue, ospl_char *name, ospl_uint32 len, ospl_uint32 timeout);
extern int os_queue_delete(os_queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif /* __OS_QUEUE_H__ */
