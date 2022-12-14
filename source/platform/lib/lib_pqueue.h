/*
 * lib_pqueue.h
 *
 *  Created on: May 12, 2018
 *      Author: zhurish
 */

#ifndef __LIB_PQUEUE_H__
#define __LIB_PQUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_list.h"

struct lib_stream
{
	NODE			node;
	struct stream 	*stream;
};

struct lib_pqueue
{
	LIST	stream_list;
	zpl_uint32 	array_max;			//one stream size
	void	*mutex;
	int		(*fetch)(struct stream *);
};



struct lib_pqueue * lib_pqueue_create (zpl_uint32 num);

int lib_pqueue_fetch (struct lib_pqueue *queue);

int lib_pqueue_stream (struct lib_pqueue *queue, struct stream *s);



 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_PQUEUE_H__ */
