/*
 * nsm_queue.c
 *
 *  Created on: May 12, 2018
 *      Author: zhurish
 */
#include "zebra.h"
#include "stream.h"
#include "log.h"
#include "memory.h"
#include "network.h"
#include "prefix.h"
#include "vty.h"
#include "os_sem.h"

#include "nsm_pqueue.h"


static int nsm_pqueue_add_stream (struct nsm_pqueue *queue, struct stream *s)
{
	struct nsm_stream *stream = NULL;
	if(lstCount(&queue->stream_list) < queue->array_max - 1)
	{
		stream = XMALLOC(MTYPE_STREAM, sizeof(struct nsm_stream));
		if(stream)
		{
			stream->stream = stream_dup(s);
			lstAdd(&queue->stream_list, (NODE *)stream);
			return OK;
		}
	}
	return ERROR;
}



int nsm_pqueue_stream (struct nsm_pqueue *queue, struct stream *s)
{
	int ret = ERROR;
	if(queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	ret = nsm_pqueue_add_stream (queue, s);
	if(queue->mutex)
		os_mutex_unlock(queue->mutex);
	return ret;
}

int nsm_pqueue_fetch (struct nsm_pqueue *queue)
{
	int ret = ERROR;
	NODE index;
	struct nsm_stream *pstNode = NULL;
	if(queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	for(pstNode = (struct nsm_stream *)lstFirst(&queue->stream_list);
			pstNode != NULL;  pstNode = (struct nsm_stream *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->stream)
		{
			lstDelete(&queue->stream_list, (NODE *)pstNode);
			if(queue->fetch)
			{
				ret = (queue->fetch)(pstNode->stream);
			}
			stream_free(pstNode->stream);
			XFREE(MTYPE_STREAM, pstNode);
		}
	}
	if(queue->mutex)
		os_mutex_unlock(queue->mutex);
	return pstNode;
}

struct nsm_pqueue * nsm_pqueue_create (u_int num)
{
  struct nsm_pqueue *queue;

  queue = XCALLOC (MTYPE_PQUEUE, sizeof (struct nsm_pqueue));
  queue->mutex = os_mutex_init();

  lstInit(&queue->stream_list);
  queue->array_max = num;
  /* By default we want nothing to happen when a node changes. */
  //queue->update = NULL;
  return queue;
}
