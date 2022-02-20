/*
 * nsm_mirror.c
 *
 *  Created on: May 11, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"

static Gmirror_t gMirror;


static int mirror_cleanup(ifindex_t ifindex, zpl_bool all);



int nsm_mirror_init(void)
{
	os_memset(&gMirror, 0, sizeof(Gmirror_t));
	gMirror.mirrorList = malloc(sizeof(LIST));
	gMirror.mutex = os_mutex_init();
	lstInit(gMirror.mirrorList);
	return OK;
}


int nsm_mirror_exit(void)
{
	if(lstCount(gMirror.mirrorList))
	{
		mirror_cleanup(0, zpl_true);
		lstFree(gMirror.mirrorList);
		free(gMirror.mirrorList);
		gMirror.mirrorList = NULL;
	}
	if(gMirror.mutex)
		os_mutex_exit(gMirror.mutex);
	return OK;
}

static int mirror_cleanup(ifindex_t ifindex, zpl_bool all)
{
	nsm_mirror_t *pstNode = NULL;
	NODE index;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	for(pstNode = (nsm_mirror_t *)lstFirst(gMirror.mirrorList);
			pstNode != NULL;  pstNode = (nsm_mirror_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && ifindex && pstNode->ifindex == ifindex)
		{
			lstDelete(gMirror.mirrorList, (NODE*)pstNode);
			XFREE(MTYPE_MIRROR, pstNode);
		}
		else if(pstNode && all)
		{
			lstDelete(gMirror.mirrorList, (NODE*)pstNode);
			XFREE(MTYPE_MIRROR, pstNode);
		}
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return OK;
}

static int nsm_mirror_client_setup(ifindex_t ifindex, void *p)
{
/*	struct interface *ifp = if_lookup_by_index(ifindex);
	if(ifp)
	{
		struct nsm_interface *nsm = ifp->info[MODULE_NSM];
		if(nsm)
			nsm->nsm_client[NSM_INTF_MIRROR] = p;
	}*/
	return OK;
}


static int mirror_add_node(nsm_mirror_t *value)
{
	nsm_mirror_t *node = XMALLOC(MTYPE_MIRROR, sizeof(nsm_mirror_t));
	if(node)
	{
		os_memset(node, 0, sizeof(nsm_mirror_t));
		os_memcpy(node, value, sizeof(nsm_mirror_t));
		if(node->enable && !node->mirror_dst)
			node->global = &gMirror;
		lstAdd(gMirror.mirrorList, (NODE *)node);
		nsm_mirror_client_setup(node->ifindex, node);
		return OK;
	}
	return ERROR;
}

static int mirror_del_node(nsm_mirror_t *node)
{
	if(node)
	{
		nsm_mirror_client_setup(node->ifindex, NULL);
		lstDelete(gMirror.mirrorList, (NODE *)node);
		XFREE(MTYPE_MIRROR, node);
		return OK;
	}
	return ERROR;
}

static nsm_mirror_t * mirror_lookup_node(ifindex_t ifindex)
{
	nsm_mirror_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_mirror_t *)lstFirst(gMirror.mirrorList);
			pstNode != NULL;  pstNode = (nsm_mirror_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->ifindex == ifindex)
		{
			return pstNode;
		}
	}
	return NULL;
}

int mirror_callback_api(mirror_cb cb, void *pVoid)
{
	int ret = OK;
	nsm_mirror_t *pstNode = NULL;
	NODE index;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	for(pstNode = (nsm_mirror_t *)lstFirst(gMirror.mirrorList);
			pstNode != NULL;  pstNode = (nsm_mirror_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && cb)
		{
			ret = (cb)(pstNode, pVoid);
			if(ret != OK)
				break;
		}
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_global_enable(zpl_bool enable)
{
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	gMirror.enable = enable;
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return OK;
}

zpl_bool nsm_mirror_global_is_enable(void)
{
	zpl_bool enable;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	enable = gMirror.enable;
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return enable;
}

int nsm_mirror_destination_set_api(ifindex_t ifindex, zpl_bool enable)
{
	int ret = 0;
	nsm_mirror_t *mirror = NULL;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	mirror = mirror_lookup_node(ifindex);
	if(!mirror)
	{
		if(enable == zpl_true)
		{
			nsm_mirror_t value;
			os_memset(&value, 0, sizeof(nsm_mirror_t));
			value.ifindex = ifindex;
			value.mirror_dst = zpl_true;
			value.enable = zpl_true;
#ifdef ZPL_HAL_MODULE
			if(hal_mirror_enable(ifindex, value.enable) == OK)
#endif
				ret = mirror_add_node(&value);
		}
		else
			ret = ERROR;
	}
	else
	{
		if(enable == zpl_true)
		{
			mirror->ifindex = ifindex;
			mirror->mirror_dst = zpl_true;
			mirror->enable = zpl_true;
#ifdef ZPL_HAL_MODULE
			if(hal_mirror_enable(ifindex, mirror->enable) == OK)
#endif
				ret = OK;
		}
		else
		{
#ifdef ZPL_HAL_MODULE
			if(hal_mirror_enable(ifindex, zpl_false) == OK)
#endif
				ret = mirror_del_node(mirror);
		}
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_destination_get_api(ifindex_t ifindex, zpl_bool *enable)
{
	int ret = 0;
	nsm_mirror_t *mirror = NULL;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	mirror = mirror_lookup_node(ifindex);
	if(!mirror)
	{
		ret = ERROR;
	}
	else
	{
		if(enable)
			*enable = mirror->enable;
/*		if(dst)
			*dst = mirror->mirror_dst;*/
		ret = OK;
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

zpl_bool nsm_mirror_is_enable_api(ifindex_t ifindex)
{
	zpl_bool ret = zpl_false;
	nsm_mirror_t *mirror = NULL;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	mirror = mirror_lookup_node(ifindex);
	if(mirror)
	{
		if(mirror->enable)
			ret = zpl_true;
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

zpl_bool nsm_mirror_is_destination_api(ifindex_t ifindex)
{
	zpl_bool ret = zpl_false;
	nsm_mirror_t *mirror = NULL;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	mirror = mirror_lookup_node(ifindex);
	if(mirror)
	{
		if(mirror->enable && mirror->mirror_dst)
			ret = zpl_true;
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}
/*
int nsm_mirror_mode_set_api(zpl_bool mac)
{
	int ret = ERROR;

	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);

	gMirror.bMac = mac;
	ret = OK;

	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_mode_get_api(zpl_bool *mac)
{
	int ret = ERROR;

	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	if(mac)
		*mac = gMirror.bMac;
	ret = OK;

	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_source_mac_set_api(zpl_bool enable, zpl_uchar *mac, mirror_dir_en dir)
{
	int ret = ERROR;

	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);

	if(mac)
	{
		ret = OK;
		os_memcpy(gMirror.mac, mac, NSM_MAC_MAX);
		gMirror.dir = dir;
	}

	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_source_mac_get_api(zpl_bool *enable, zpl_uchar *mac, mirror_dir_en *dir)
{
	int ret = ERROR;

	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	if(mac)
	{
		ret = OK;
		os_memcpy(mac, gMirror.mac, NSM_MAC_MAX);
		if(dir)
			*dir = gMirror.dir;
	}
	ret = OK;

	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}*/


int nsm_mirror_source_set_api(ifindex_t ifindex, zpl_bool enable, mirror_dir_en dir)
{
	int ret = 0;
	nsm_mirror_t *mirror = NULL;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	mirror = mirror_lookup_node(ifindex);
	if(!mirror)
	{
		if(enable == zpl_true)
		{
			nsm_mirror_t value;
			os_memset(&value, 0, sizeof(nsm_mirror_t));
			value.ifindex = ifindex;
			value.mirror_dst = zpl_false;
			value.enable = zpl_true;
			value.dir = dir;
#ifdef ZPL_HAL_MODULE
			if(hal_mirror_source_enable(ifindex, NULL, dir, zpl_true) == OK)
#endif
				ret = mirror_add_node(&value);
		}
		else
			ret = ERROR;
	}
	else
	{
		if(enable == zpl_true)
		{
			mirror->ifindex = ifindex;
			mirror->mirror_dst = zpl_false;
			mirror->enable = zpl_true;
			mirror->dir = dir;
#ifdef ZPL_HAL_MODULE
			if(hal_mirror_source_enable(ifindex, NULL, dir, zpl_true) == OK)
#endif
				ret = OK;
		}
		else
		{
#ifdef ZPL_HAL_MODULE
			if(hal_mirror_source_enable(ifindex, NULL, dir, zpl_false) == OK)
#endif
				ret = mirror_del_node(mirror);
		}
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_source_get_api(ifindex_t ifindex, zpl_bool *enable, mirror_dir_en *dir)
{
	int ret = 0;
	nsm_mirror_t *mirror = NULL;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	mirror = mirror_lookup_node(ifindex);
	if(!mirror)
	{
		ret = ERROR;
	}
	else
	{
		if(enable)
			*enable = mirror->enable;
		if(dir)
			*dir = mirror->dir;
		ret = OK;
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

zpl_bool nsm_mirror_is_source_api(void)
{
	zpl_bool ret = zpl_false;
	nsm_mirror_t *pstNode = NULL;
	NODE index;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	for(pstNode = (nsm_mirror_t *)lstFirst(gMirror.mirrorList);
			pstNode != NULL;  pstNode = (nsm_mirror_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->ifindex && pstNode->enable && pstNode->mirror_dst == zpl_false)
		{
			ret = zpl_true;
			break;
		}
	}
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_source_mac_filter_set_api(zpl_bool enable, zpl_uchar *mac, zpl_bool dst,  mirror_dir_en dir)
{
	int ret = 0;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	if(enable == zpl_true)
	{
		if(mac)
		{
			if(dir == MIRROR_INGRESS)
			{
				gMirror.in_enable = zpl_true;
				gMirror.ingress_dst = dst;
				os_memcpy(gMirror.ingress_mac, mac, NSM_MAC_MAX);
			}
			else if(dir == MIRROR_EGRESS)
			{
				gMirror.out_enable = zpl_true;
				gMirror.egress_dst = dst;
				os_memcpy(gMirror.egress_mac, mac, NSM_MAC_MAX);
			}
#ifdef ZPL_HAL_MODULE
			if(hal_mirror_source_filter_enable(zpl_true, dst, mac, dir) == OK)
#endif
				ret = OK;
		}
		else
		{
			if(dir == MIRROR_INGRESS)
			{
				gMirror.in_enable = zpl_false;
				gMirror.ingress_dst = zpl_false;
				os_memset(gMirror.ingress_mac, 0, NSM_MAC_MAX);
			}
			else if(dir == MIRROR_EGRESS)
			{
				gMirror.out_enable = zpl_false;
				gMirror.egress_dst = zpl_false;
				os_memset(gMirror.egress_mac, 0, NSM_MAC_MAX);
			}
#ifdef ZPL_HAL_MODULE
			if(hal_mirror_source_filter_enable(zpl_false, dst, NULL, dir) == OK)
#endif
				ret = OK;
		}
	}
	else
		ret = ERROR;
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}
int nsm_mirror_source_mac_filter_get_api(mirror_dir_en dir, zpl_bool *enable, zpl_uchar *mac, zpl_bool *dst)
{
	int ret = 0;
	if(gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);

	if(dir == MIRROR_INGRESS)
	{
		if(dst)
			*dst = gMirror.ingress_dst;
		if(mac)
			os_memcpy(mac, gMirror.ingress_mac, NSM_MAC_MAX);
	}
	else if(dir == MIRROR_EGRESS)
	{
		if(dst)
			*dst = gMirror.egress_dst;
		if(mac)
			os_memcpy(mac, gMirror.egress_mac, NSM_MAC_MAX);
	}
	ret = OK;
	if(gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}
