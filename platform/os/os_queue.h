/*
 * os_queue.h
 *
 *  Created on: Jun 9, 2018
 *      Author: zhurish
 */

#ifndef __OS_QUEUE_H__
#define __OS_QUEUE_H__


#define OS_QUEUE_NAME_MAX	32


typedef struct queue_s
{
	NODE	node;
	u_int	size;
	char	*data;
}queue_t;


typedef struct os_queue
{
	os_sem_t	*sem;
	os_mutex_t	*mutex;
	u_int	max;
	u_int	size;
	u_int	use;
	char	name[OS_QUEUE_NAME_MAX];
	LIST	list;
	LIST	ulist;

}os_queue_t;


extern int os_msgq_init();
extern int os_msgq_exit();


extern os_queue_t *os_queue_create(int max, int size);
extern int os_queue_name(os_queue_t *queue, char *name);
//int os_queue_info(os_queue_t *queue, char *name);
extern int os_queue_send(os_queue_t *queue, char *data, int len, int op);
extern int os_queue_recv(os_queue_t *queue, char *name, int len, int timeout);
extern int os_queue_delete(os_queue_t *queue);

#endif /* __OS_QUEUE_H__ */
