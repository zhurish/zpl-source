/*
 * nsm_queue.c
 *
 *  Created on: May 12, 2018
 *      Author: zhurish
 */
#include "os_include.h"
#include "zpl_include.h"
#include "zmemory.h"
#include "lib_pqueue.h"

static int lib_pqueue_add_stream (struct lib_pqueue *queue, struct stream *s)
{
	struct lib_stream *stream = NULL;
	if(lstCount(&queue->stream_list) < queue->array_max - 1)
	{
		stream = XMALLOC(MTYPE_STREAM, sizeof(struct lib_stream));
		if(stream)
		{
			stream->stream = stream_dup(s);
			lstAdd(&queue->stream_list, (NODE *)stream);
			return OK;
		}
	}
	return ERROR;
}



int lib_pqueue_stream (struct lib_pqueue *queue, struct stream *s)
{
	int ret = ERROR;
	if(queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	ret = lib_pqueue_add_stream (queue, s);
	if(queue->mutex)
		os_mutex_unlock(queue->mutex);
	return ret;
}

int lib_pqueue_fetch (struct lib_pqueue *queue)
{
	int ret = ERROR;
	NODE index;
	struct lib_stream *pstNode = NULL;
	if(queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	for(pstNode = (struct lib_stream *)lstFirst(&queue->stream_list);
			pstNode != NULL;  pstNode = (struct lib_stream *)lstNext((NODE*)&index))
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

struct lib_pqueue * lib_pqueue_create (zpl_uint32 num)
{
  struct lib_pqueue *queue;

  queue = XCALLOC (MTYPE_PQUEUE, sizeof (struct lib_pqueue));
  queue->mutex = os_mutex_init();

  lstInit(&queue->stream_list);
  queue->array_max = num;
  /* By default we want nothing to happen when a node changes. */
  //queue->update = NULL;
  return queue;
}
