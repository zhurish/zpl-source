/*
 * nsm_queue.h
 *
 *  Created on: May 12, 2018
 *      Author: zhurish
 */

#ifndef __NSM_PQUEUE_H__
#define __NSM_PQUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_list.h"

struct nsm_stream
{
	NODE			node;
	struct stream 	*stream;
};

struct nsm_pqueue
{
	LIST	stream_list;
	zpl_uint32 	array_max;			//one stream size
	void	*mutex;
	int		(*fetch)(struct stream *);
};



struct nsm_pqueue * nsm_pqueue_create (zpl_uint32 num);

int nsm_pqueue_fetch (struct nsm_pqueue *queue);

int nsm_pqueue_stream (struct nsm_pqueue *queue, struct stream *s);



 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_PQUEUE_H__ */
