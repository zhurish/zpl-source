/*
 * os_queue.h
 *
 *  Created on: Jun 9, 2018
 *      Author: zhurish
 */

#ifndef __OS_QUEUE_H_
#define __OS_QUEUE_H_


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

os_queue_t *os_queue_create(int max, int size);
int os_queue_name(os_queue_t *queue, char *name);
//int os_queue_info(os_queue_t *queue, char *name);
int os_queue_send(os_queue_t *queue, char *data, int len, int op);
int os_queue_recv(os_queue_t *queue, char *name, int len, int timeout);
int os_queue_delete(os_queue_t *queue);

#endif /* __OS_QUEUE_H_ */
