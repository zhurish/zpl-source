/*
 * zpl_skbuffer.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"




zpl_uint32 zpl_skb_timerstamp(void)
{
	zpl_uint32 pts = 0u;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	pts = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return pts;
}


int zpl_skbuffer_destroy(zpl_skbuffer_t *skbuf)
{
	assert(skbuf);
	if (skbuf)
	{
		if (skbuf->skb_data)
		{
			os_free(skbuf->skb_data);
		}	
		os_free(skbuf);
	}
	return OK;
}

zpl_skbqueue_t *zpl_skbqueue_create(zpl_uint32 maxsize, zpl_bool sem)
{
	zpl_skbqueue_t *queue = os_malloc(sizeof(zpl_skbqueue_t));
	if (queue)
	{
		os_memset(queue, 0, sizeof(zpl_skbqueue_t));
		queue->maxsize = maxsize;
		queue->mutex = os_mutex_init();
		if (sem)
			queue->sem = os_sem_init();
		else
			queue->sem = NULL;

		lstInitFree(&queue->ulist, zpl_skbuffer_destroy);
		lstInitFree(&queue->list, zpl_skbuffer_destroy);
		return queue;
	}
	return NULL;
}

int zpl_skbqueue_destroy(zpl_skbqueue_t *queue)
{
	assert(queue);
	if (!queue)
		return ERROR;
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	lstFree(&queue->ulist);
	lstFree(&queue->list);
	if (queue->mutex)
		os_mutex_exit(queue->mutex);
	if (queue->sem)
		os_sem_exit(queue->sem);
	os_free(queue);
	return OK;
}

int zpl_skbqueue_finsh(zpl_skbqueue_t *queue, zpl_skbuffer_t *skbuf)
{
	assert(queue);
	assert(skbuf);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	lstAdd(&queue->ulist, skbuf);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int zpl_skbqueue_enqueue(zpl_skbqueue_t *queue, zpl_skbuffer_t *skbuf)
{
	assert(queue);
	assert(skbuf);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	lstAdd(&queue->list, skbuf);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	if (queue->sem)
		os_sem_give(queue->sem);
	return OK;
}

zpl_skbuffer_t *zpl_skbqueue_dequeue(zpl_skbqueue_t *queue)
{
	zpl_skbuffer_t *skbuf = NULL;
	assert(queue);
	if (queue->sem)
		os_sem_take(queue->sem, OS_WAIT_FOREVER);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (lstCount(&queue->list))
	{
		skbuf = lstFirst(&queue->list);
		if (skbuf)
			lstDelete(&queue->list, (NODE *)skbuf);
	}
	//if(skbuf)
	//__sync_fetch_and_add(skbuf->atomic, 1);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return skbuf;
}

int zpl_skbqueue_add(zpl_skbqueue_t *queue, zpl_skbuffer_t *skbuf)
{
	assert(queue);
	assert(skbuf);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	lstAdd(&queue->list, skbuf);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int zpl_skbqueue_put(zpl_skbqueue_t *queue, zpl_skbuffer_t *skbuf)
{
	return zpl_skbqueue_add(queue, skbuf);
}

zpl_skbuffer_t *zpl_skbqueue_get(zpl_skbqueue_t *queue)
{
	zpl_skbuffer_t *skbuf = NULL;
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (lstCount(&queue->list))
	{
		skbuf = lstFirst(&queue->list);
		if (skbuf)
			lstDelete(&queue->list, (NODE *)skbuf);
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return skbuf;
}

int zpl_skbqueue_distribute(zpl_skbqueue_t *queue, int(*func)(zpl_skbuffer_t*, void *), void *p)
{
	NODE node;
	zpl_skbuffer_t *skbuf = NULL;
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	for (skbuf = (zpl_skbuffer_t *)lstFirst(&queue->list); skbuf != NULL;
		 skbuf = (zpl_skbuffer_t *)lstNext(&node))
	{
		node = skbuf->node;
		if (skbuf && skbuf->skb_len)
		{
			lstDelete(&queue->list, (NODE *)skbuf);
			(func)(skbuf, p);
			lstAdd(&queue->ulist, skbuf);
		}
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}



static zpl_skbuffer_t *zpl_skbuffer_create_raw(zpl_skbqueue_t *queue, zpl_uint32 len)
{
	NODE node;
	zpl_skbuffer_t *skbuf = NULL;
	if(queue)
	{
		assert(queue);
		if (queue->mutex)
			os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
		if (lstCount(&queue->list) >= queue->maxsize)
		{
			if (queue->mutex)
				os_mutex_unlock(queue->mutex);
			return NULL;
		}
		for (skbuf = (zpl_skbuffer_t *)lstFirst(&queue->ulist); skbuf != NULL;
			skbuf = (zpl_skbuffer_t *)lstNext(&node))
		{
			node = skbuf->node;
			if (skbuf && skbuf->skb_maxsize >= (len))
			{
				lstDelete(&queue->ulist, (NODE *)skbuf);
				break;
			}
		}
	}
	if (skbuf == NULL)
	{
		skbuf = os_malloc(sizeof(zpl_skbuffer_t));
		if (skbuf)
		{
			memset(skbuf, 0, sizeof(zpl_skbuffer_t));

			skbuf->skb_maxsize = (len); //buffer 的长度
			skbuf->skb_data = os_malloc(skbuf->skb_maxsize);				//buffer
			skbuf->skb_start = ZPL_SKB_START_OFFSET;
			if (skbuf->skb_data == NULL)
			{
				skbuf->skb_maxsize = 0;
				free(skbuf);
				skbuf = NULL;
			}
		}
	}
	if (queue && queue->mutex)
		os_mutex_unlock(queue->mutex);
	return skbuf;
}

zpl_skbuffer_t *zpl_skbuffer_create(zpl_skbqueue_t *queue, zpl_uint32 len)
{
	zpl_skbuffer_t *skbuf = zpl_skbuffer_create_raw(queue, ZPL_SKSIZE_ALIGN(len));
	return skbuf;
}

zpl_skbuffer_t *zpl_skbuffer_clone(zpl_skbqueue_t *queue, zpl_skbuffer_t *skbuf)
{
	zpl_skbuffer_t *skbuftmp = NULL;
	skbuftmp = zpl_skbuffer_create_raw(queue, skbuf->skb_maxsize);
	if (skbuftmp)
	{
		memcpy(&skbuftmp->skb_header, &skbuf->skb_header, sizeof(skbuf->skb_header));
		skbuftmp->skb_timetick = skbuf->skb_timetick; //时间戳 毫秒
		skbuftmp->skb_len = skbuf->skb_len;			//当前缓存帧的长度
		skbuftmp->skb_maxsize = skbuf->skb_maxsize; //buffer 的长度
		memset(skbuftmp->skb_data, 0, skbuf->skb_maxsize);
		memcpy(skbuftmp->skb_data, skbuf->skb_data, skbuf->skb_len);
		return skbuftmp;
	}
	return NULL;
}

/* 报文前面添加数据 */
int zpl_skbuffer_push(zpl_skbuffer_t *skbuf, uint8_t *data, uint32_t len)
{
	if(skbuf->skb_start > 0 && skbuf->skb_start >= len)
	{
		skbuf->skb_start -= len;
		skbuf->skb_len += len;
		if(data)
			memcpy(skbuf->skb_data + skbuf->skb_start, data, len);
		return  len;
	}
	return ERROR;
}

/* 报文前面删除数据 */
int zpl_skbuffer_pull(zpl_skbuffer_t *skbuf, uint8_t *data, uint32_t len)
{
	if(skbuf->skb_start > 0 && skbuf->skb_len >= len)
	{
		if(data)
		{
			memcpy(data, skbuf->skb_data + skbuf->skb_start, len);
		}
		skbuf->skb_start += len;
		skbuf->skb_len -= len;
		return  len;
	}
	return ERROR;
}



int zpl_skbuffer_put(zpl_skbuffer_t *skbuf, uint8_t *data, uint32_t len)
{
	if (len <= 0)
		return OK;
	if (skbuf->skb_data == NULL)
	{
		skbuf->skb_maxsize = ZPL_SKSIZE_ALIGN(len);
		skbuf->skb_data = malloc(skbuf->skb_maxsize);
		skbuf->skb_start = ZPL_SKB_START_OFFSET;
	}
	else
	{
		if (skbuf->skb_maxsize < (skbuf->skb_len + ZPL_SKBUF_ALIGN(len)))
		{
			zpl_uint32 skb_maxsize = skbuf->skb_maxsize + ZPL_SKBUF_ALIGN(len);
			skbuf->skb_data = realloc(skbuf->skb_data, skb_maxsize);
			if (skbuf->skb_data)
				skbuf->skb_maxsize = skb_maxsize;
		}
	}

	if (skbuf->skb_data != NULL && skbuf->skb_len > 0)
	{
		memcpy(skbuf->skb_data + skbuf->skb_len, data, len);
		skbuf->skb_len += len;
		return skbuf->skb_len;
	}
	return OK;
}

int zpl_skbuffer_unref(zpl_skbuffer_t *skbuf)
{
	if(skbuf->reference)
		skbuf->reference--;
	return OK;
}

int zpl_skbuffer_addref(zpl_skbuffer_t *skbuf)
{
	skbuf->reference++;
	return OK;
}

/* net pkt */
int zpl_skbuffer_netpkt_hdrset(zpl_skbuffer_t *skbuf, zpl_netpkt_hdr_t *hdr)
{
	memcpy(&skbuf->skb_header.net_header, hdr, sizeof(zpl_netpkt_hdr_t));
	return sizeof(zpl_netpkt_hdr_t);
}

zpl_netpkt_hdr_t * zpl_skbuffer_netpkt_hdrget(zpl_skbuffer_t *src)
{
	zpl_netpkt_hdr_t *netpkt = &src->skb_header.net_header;
	return netpkt;
}

int zpl_skbuffer_netpkt_build_source(ifindex_t ifindex, zpl_phyport_t trunk, zpl_phyport_t phyid, zpl_skbuffer_t *skbuf)
{
    skbuf->skb_header.net_header.ifindex = ifindex;
    skbuf->skb_header.net_header.trunk = trunk;          /* Source trunk group ID used in header/tag, -1 if src_port set . */
    skbuf->skb_header.net_header.phyid = phyid;          /* Source port used in header/tag. */
	return OK;
}

int zpl_skbuffer_netpkt_build_reason(zpl_uint32 reason, zpl_skbuffer_t *skbuf)
{
	skbuf->skb_header.net_header.reason = reason;         /* Opcode from packet. */
	return OK;
}

int zpl_skbuffer_netpkt_build_timestamp(zpl_uint32 timestamp, zpl_skbuffer_t *skbuf)
{
	skbuf->skb_header.net_header.timestamp = timestamp;         /* Opcode from packet. */
	return OK;
}

int zpl_skbuffer_netpkt_build_header(zpl_uint8 unit, zpl_skbuffer_t *skbuf, uint8_t *data)
{
	zpl_skb_ethvlan_t *hdr = (zpl_skb_ethvlan_t *)data;
    skbuf->skb_header.net_header.unit = unit;                         /* Unit number. */
    skbuf->skb_header.net_header.cos = 0;                          /* The local COS queue to use. */
	skbuf->skb_header.net_header.prio_int = 0;                     /* Internal priority of the packet. */
	if(ntohs(hdr->ethhdr.ethtype) == 0x8100 || ntohs(hdr->ethhdr.ethtype) == 0x9100 || ntohs(hdr->ethhdr.ethtype) == 0x9200)
	{
    	skbuf->skb_header.net_header.tpid = ntohs(hdr->ethhdr.ethtype);
    	skbuf->skb_header.net_header.vlan = hdr->ethhdr.vlanhdr.vid;                    /* 802.1q VID or VSI or VPN. */
    	skbuf->skb_header.net_header.vlan_pri = hdr->ethhdr.vlanhdr.pri;                     /* Vlan tag priority . */
    	skbuf->skb_header.net_header.vlan_cfi = hdr->ethhdr.vlanhdr.cfi;                     /* Vlan tag CFI bit. */
		skbuf->skb_header.net_header.ethtype = ntohs(hdr->ethhdr.vlanhdr.ethtype);
		if(ntohs(hdr->ethhdr.vlanhdr.ethtype) == 0x8100)
		{
			hdr = (zpl_skb_ethvlan_t *)(data + 4);
			skbuf->skb_header.net_header.inner_tpid = ntohs(hdr->ethhdr.ethtype);
			skbuf->skb_header.net_header.inner_vlan = hdr->ethhdr.vlanhdr.vid;                    /* 802.1q VID or VSI or VPN. */
			skbuf->skb_header.net_header.inner_vlan_pri = hdr->ethhdr.vlanhdr.pri;                     /* Vlan tag priority . */
			skbuf->skb_header.net_header.inner_vlan_cfi = hdr->ethhdr.vlanhdr.cfi;                     /* Vlan tag CFI bit. */
			skbuf->skb_header.net_header.ethtype = ntohs(hdr->ethhdr.vlanhdr.ethtype);
		}
		skbuf->skb_header.net_header.untagged = 0;
	}
	else
	{
		skbuf->skb_header.net_header.ethtype = ntohs(hdr->ethhdr.ethtype);
		skbuf->skb_header.net_header.untagged = 1;       /* The packet was untagged on ingress. */
	}
    skbuf->skb_header.net_header.color = 0;                  /* Packet color. */
    skbuf->skb_header.net_header.reference = 0;	
	return OK;
}

zpl_uint16 zpl_skbuf_ethtype(char *src)
{
	zpl_skb_ethvlan_t *hdr = (zpl_skb_ethvlan_t *)src;
	if(ntohs(hdr->ethhdr.ethtype) == 0x8100)
	{
		return ntohs(hdr->ethhdr.vlanhdr.ethtype);
	}
	return ntohs(hdr->ethhdr.ethtype);
}

zpl_uint16 zpl_skbuf_vlan(char *src)
{
	zpl_skb_ethvlan_t *hdr = (zpl_skb_ethvlan_t *)src;
	if(ntohs(hdr->ethhdr.ethtype) == 0x8100)
	{
		return (hdr->ethhdr.vlanhdr.vid);
	}
	return 0;
}


