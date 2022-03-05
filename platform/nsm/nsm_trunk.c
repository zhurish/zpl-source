/*
 * nsm_trunk.c
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"

#ifdef ZPL_HAL_MODULE
#include "hal_trunk.h"
#endif
static Gl2trunk_t gtrunk;

static l2trunk_group_t * l2trunk_group_lookup_node(zpl_uint32 trunkid);
#ifdef ZPL_SHELL_MODULE
static int _trunk_interface_show_one(struct vty *vty, struct interface *ifp);

int nsm_trunk_interface_write_config(struct vty *vty, struct interface *ifp)
{
	NSM_ENTER_FUNC();
	return _trunk_interface_show_one(vty, ifp);
}
#endif
l2trunk_group_t * nsm_port_channel_get(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	NSM_ENTER_FUNC();
	return (l2trunk_group_t *)nsm->nsm_client[NSM_INTF_TRUNK];
}


static int nsm_port_channel_add(struct interface *ifp)
{
	l2trunk_group_t * port_channel = NULL;
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!if_is_lag(ifp))
		return OK;
	NSM_ENTER_FUNC();
	nsm_trunk_create_api(IF_IFINDEX_ID_GET(ifp->ifindex), TRUNK_STATIC);

	port_channel = nsm->nsm_client[NSM_INTF_TRUNK] = l2trunk_group_lookup_node(IF_IFINDEX_ID_GET(ifp->ifindex));
	if(!port_channel)
		return ERROR;
	port_channel->ifp = ifp;
	return OK;
}


static int nsm_port_channel_del(struct interface *ifp)
{
	l2trunk_group_t * port_channel = NULL;
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!if_is_lag(ifp))
		return OK;
	NSM_ENTER_FUNC();
	port_channel = nsm->nsm_client[NSM_INTF_TRUNK];
	if(port_channel)
	{
		if(nsm_trunk_destroy_api(port_channel->trunkId) == ERROR)
			return ERROR;
		nsm->nsm_client[NSM_INTF_TRUNK] = NULL;
	}
	return OK;
}


int nsm_trunk_interface_create_api(struct interface *ifp)
{
	NSM_ENTER_FUNC();
	return nsm_port_channel_add(ifp);
}

int nsm_trunk_interface_del_api(struct interface *ifp)
{
	NSM_ENTER_FUNC();
	return nsm_port_channel_del(ifp);
}


int nsm_trunk_init(void)
{
	zpl_uint32 i = 0;
	NSM_ENTER_FUNC();
	os_memset(&gtrunk, 0, sizeof(Gl2trunk_t));
	gtrunk.group = XMALLOC(MTYPE_TRUNK, sizeof(l2trunk_group_t) * NSM_TRUNK_ID_MAX);
	if(!gtrunk.group)
		return ERROR;
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		os_memset(&gtrunk.group[i], 0, sizeof(gtrunk.group[i]));
		gtrunk.group[i].trunkList = malloc(sizeof(LIST));
		gtrunk.group[i].global = &gtrunk;
		//gtrunk.lacp_system_priority = LACP_SYSTEM_PRIORITY_DEFAULT;
		//gtrunk.load_balance = LOAD_BALANCE_DEFAULT;
		lstInit(gtrunk.group[i].trunkList);
	}
	gtrunk.enable = zpl_true;
	gtrunk.mutex = os_mutex_init();

	nsm_interface_hook_add(NSM_INTF_TRUNK, nsm_trunk_interface_create_api, nsm_trunk_interface_del_api);
	return OK;
}

static int l2trunk_cleanup(l2trunk_group_t *group)
{
	l2trunk_t *pstNode = NULL;
	NODE index;
	NSM_ENTER_FUNC();
	for(pstNode = (l2trunk_t *)lstFirst(group->trunkList);
			pstNode != NULL;  pstNode = (l2trunk_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(group->trunkList, (NODE*)pstNode);
			XFREE(MTYPE_TRUNK, pstNode);
		}
	}
	return OK;
}

int nsm_trunk_exit(void)
{
	zpl_uint32 i = 0;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	//nsm_port_channel_client_exit();
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(lstCount(gtrunk.group[i].trunkList))
			l2trunk_cleanup(&gtrunk.group[i]);
		free(gtrunk.group[i].trunkList);
		gtrunk.group[i].trunkList = NULL;
	}
	XFREE(MTYPE_TRUNK, gtrunk.group);
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	if(gtrunk.mutex)
		os_mutex_exit(gtrunk.mutex);
	return OK;
}


int nsm_trunk_enable(void)
{
	int ret = 0;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	ret = hal_trunk_enable(zpl_true);
#else
	ret = OK;
#endif
	if(ret == OK)
		gtrunk.enable = zpl_true;
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

zpl_bool nsm_trunk_is_enable(void)
{
	return gtrunk.enable;
}


//
static l2trunk_group_t * l2trunk_group_lookup_node(zpl_uint32 trunkid)
{
	zpl_uint32 i = 0;
	NSM_ENTER_FUNC();
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(gtrunk.group[i].trunkId == trunkid)
			return &(gtrunk.group[i]);
	}
	return NULL;
}


/*
static int l2trunk_add_sort_node(l2trunk_t *value)
{
	zpl_uint32 = 1;
	l2trunk_t *node = l2trunk_lookup_node(value->trunkId);
	while(!node)
		node = l2trunk_lookup_node(value->trunkId);
	if(node)
	{
		lstInsert (gtrunk.trunkList, (NODE*)node, (NODE*)value);
	}
	else
		lstAdd(gtrunk.trunkList, (NODE *)node);
	return OK;
}
*/

static l2trunk_t * l2trunk_lookup_node(void *pList, ifindex_t ifindex)
{
	l2trunk_t *pstNode = NULL;
	NODE index;
	NSM_ENTER_FUNC();
	for(pstNode = (l2trunk_t *)lstFirst(pList);
			pstNode != NULL;  pstNode = (l2trunk_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->ifindex == ifindex)
		{
			return pstNode;
		}
	}
	return NULL;
}

static int l2trunk_add_node(l2trunk_group_t *group, l2trunk_t *value)
{
	//l2trunk_group_t *group = l2trunk_group_lookup_node(value->trunkId);
	NSM_ENTER_FUNC();
	if(!group)
		return ERROR;
	l2trunk_t *node = XMALLOC(MTYPE_TRUNK, sizeof(l2trunk_t));
	if(node)
	{
		memset(node, 0, sizeof(l2trunk_t));
		memcpy(node, value, sizeof(l2trunk_t));
		node->group = group;
		node->lacp_port_priority = LACP_PORT_PRIORITY_DEFAULT;
		node->lacp_timeout = LACP_TIMEOUT_DEFAULT;
		lstAdd(group->trunkList, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static int l2trunk_del_node(l2trunk_group_t *group, l2trunk_t *value)
{
	//l2trunk_group_t *group = l2trunk_group_lookup_node(value->trunkId);
	NSM_ENTER_FUNC();
	if(!group)
		return ERROR;
	if(value)
	{
		lstDelete(group->trunkList, (NODE *)value);
		value->group = NULL;
		XFREE(MTYPE_TRUNK, value);
		return OK;
	}
	return ERROR;
}



static l2trunk_t * l2trunk_lookup_port(ifindex_t ifindex)
{
	zpl_uint32 i = 0;
	l2trunk_t *pstNode = NULL;
	NSM_ENTER_FUNC();
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		pstNode = l2trunk_lookup_node(gtrunk.group[i].trunkList, ifindex);
		if(pstNode)
			return pstNode;
	}
	return NULL;
}


static int l2trunk_add_port(l2trunk_group_t *group, l2trunk_t *value)
{
	//	zpl_uint32 i = 0;
	NSM_ENTER_FUNC();
#ifdef ZPL_HAL_MODULE
	if(hal_trunk_add_interface(group->trunkId, value->ifindex) == OK)
#else
	if(1)
#endif
	{
/*		l2trunk_t value;
		value.ifindex = ifindex;
		value.trunkId = group->trunkId;*/
		return l2trunk_add_node(group, value);
	}
	else
		return ERROR;
	return ERROR;
}

static int l2trunk_del_port(l2trunk_group_t *group, l2trunk_t *value)
{
	NSM_ENTER_FUNC();
#ifdef ZPL_HAL_MODULE
	if(hal_trunk_del_interface(group->trunkId, value->ifindex) == OK)
#else
	if(1)
#endif
	{
/*		l2trunk_t value;
		value.ifindex = ifindex;
		value.trunkId = group->trunkId;*/
		return l2trunk_del_node(group, value);
	}
	else
		return ERROR;
}

zpl_bool l2trunk_lookup_api(zpl_uint32 trunkid)
{
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	if(l2trunk_group_lookup_node(trunkid) != NULL)
	{
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
		return zpl_true;
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return zpl_false;
}

int l2trunk_lookup_interface_count_api(zpl_uint32 trunkid)
{
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	l2trunk_group_t *group = l2trunk_group_lookup_node(trunkid);
	if(group)
	{
		int ret = lstCount(group->trunkList);
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
		return ret;
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return -1;
}

int nsm_trunk_create_api(zpl_uint32 trunkid, trunk_type_t type)
{
	int ret = ERROR;
	l2trunk_group_t *pstNode = NULL;
	NSM_ENTER_FUNC();
	if(!nsm_trunk_is_enable())
		nsm_trunk_enable();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	pstNode = l2trunk_group_lookup_node(trunkid);
	if(!pstNode)
	{
		zpl_uint32 i = 0;
		for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
		{
			if(gtrunk.group[i].trunkId == 0)
			{
				#ifdef ZPL_HAL_MODULE
				ret = hal_trunk_create( trunkid, zpl_true);
				#else
				ret = OK;
				#endif
				if(ret == OK)
				{
					gtrunk.group[i].trunkId = trunkid;
					gtrunk.group[i].type = type;
					gtrunk.group[i].lacp_system_priority = LACP_SYSTEM_PRIORITY_DEFAULT;
					gtrunk.group[i].load_balance = LOAD_BALANCE_DEFAULT;

					//gtrunk.group[i].global = &gtrunk;
					//gtrunk.group[i].lacp_system_priority = gtrunk.lacp_system_priority;
					//gtrunk.group[i].load_balance = gtrunk.load_balance;
					ret = OK;
				}
			}
		}
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}


int nsm_trunk_destroy_api(zpl_uint32 trunkid)
{
	int ret = ERROR;
	l2trunk_group_t *pstNode = NULL;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	if(l2trunk_lookup_interface_count_api(trunkid) >= 1)
	{
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
		return ret;
	}
	pstNode = l2trunk_group_lookup_node(trunkid);
	if(pstNode)
	{
		#ifdef ZPL_HAL_MODULE
		ret = hal_trunk_create( pstNode->trunkId, zpl_false);
		#else
		ret = OK;
		#endif
		if(ret == OK)
		{
			pstNode->trunkId = 0;
			pstNode->type = 0;
			//pstNode->global = NULL;
			ret = OK;
		}
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}


zpl_bool l2trunk_lookup_interface_api(ifindex_t ifindex)
{
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	if(l2trunk_lookup_port(ifindex) != NULL)
	{
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
		return zpl_true;
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return zpl_false;
}

int nsm_trunk_get_ID_interface_api(ifindex_t ifindex, zpl_uint32 *trunkId)
{
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	l2trunk_t *pstNode = l2trunk_lookup_port(ifindex);
	if(pstNode)
	{
		if(trunkId)
			*trunkId = pstNode->trunkId;
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
		return OK;
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return -1;
}

int nsm_trunk_add_interface_api(zpl_uint32 trunkid, trunk_type_t type, trunk_mode_t mode, struct interface *ifp)
{
	int ret = ERROR;
	l2trunk_t *value = NULL;
	l2trunk_group_t *group = NULL;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	value = l2trunk_lookup_port(ifp->ifindex);
	if(value)
	{
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
		return ERROR;
	}
	group = l2trunk_group_lookup_node(trunkid);
	if(group)
	{
		l2trunk_t trunk;
		os_memset(&trunk, 0, sizeof(l2trunk_t));
		trunk.ifindex = ifp->ifindex;
		trunk.trunkId = trunkid;
		trunk.type = type;
		trunk.mode = mode;
		ret = l2trunk_add_port(group, &trunk);
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}


int nsm_trunk_del_interface_api(zpl_uint32 trunkid, struct interface *ifp)
{
	//zpl_uint32 i = 0;
	int ret = ERROR;
	l2trunk_t *value;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	value = l2trunk_lookup_port(ifp->ifindex);
	if(value)
	{
		ret = l2trunk_del_port(l2trunk_group_lookup_node(value->trunkId), value);
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}


int nsm_trunk_load_balance_api(zpl_uint32 trunkid, load_balance_t mode)
{
	int ret = ERROR;
	l2trunk_group_t *value;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);

	value = l2trunk_group_lookup_node(trunkid);
	if(value)
	{
		value->load_balance = mode;
#ifdef ZPL_HAL_MODULE
		if(hal_trunk_mode(trunkid, mode) == OK)
#else
		if(1)
#endif
		{
			value->load_balance = mode;
			if(mode == TRUNK_LOAD_BALANCE_NONE)
				value->load_balance = LOAD_BALANCE_DEFAULT;

			ret = OK;
		}
		else
			ret = ERROR;
	}
/*	if(hal_trunk_mode(trunkid, mode) == OK)
	{
		gtrunk.load_balance = mode;
		if(mode == TRUNK_LOAD_BALANCE_NONE)
			gtrunk.load_balance = LOAD_BALANCE_DEFAULT;

		ret = OK;
	}
	else
		ret = ERROR;*/
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

int nsm_trunk_lacp_port_priority_api(ifindex_t ifindex, zpl_uint32 pri)
{
	zpl_uint32 i = 0;
	int ret = ERROR;
	l2trunk_t *value;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);

	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(gtrunk.group[i].trunkId)
		{
			value = l2trunk_lookup_node(gtrunk.group[i].trunkList, ifindex);
			if(value)
			{
				value->lacp_port_priority = pri;
				if(pri == 0)
					value->lacp_port_priority = LACP_PORT_PRIORITY_DEFAULT;

				ret = OK;
				break;
			}
		}
	}

	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

int nsm_trunk_lacp_timeout_api(ifindex_t ifindex, zpl_uint32 timeout)
{
	zpl_uint32 i = 0;
	int ret = ERROR;
	l2trunk_t *value;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);

	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(gtrunk.group[i].trunkId)
		{
			value = l2trunk_lookup_node(gtrunk.group[i].trunkList, ifindex);
			if(value)
			{
				value->lacp_timeout = timeout;
				if(timeout == 0)
					value->lacp_timeout = LACP_TIMEOUT_DEFAULT;

				ret = OK;
				break;
			}
		}
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

int nsm_trunk_lacp_system_priority_api(zpl_uint32 trunkid, zpl_uint32 pri)
{
	int ret = ERROR;
	l2trunk_group_t *value;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	value = l2trunk_group_lookup_node(trunkid);
	if(value)
	{
		value->lacp_system_priority = pri;
		if(pri == 0)
			value->lacp_system_priority = LACP_SYSTEM_PRIORITY_DEFAULT;

		ret = OK;
	}
/*
	gtrunk.lacp_system_priority = pri;
	if(pri == 0)
		gtrunk.lacp_system_priority = LACP_SYSTEM_PRIORITY_DEFAULT;
*/

	ret = OK;
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

int nsm_trunk_group_callback_api(l2trunk_group_cb cb, void *pVoid)
{
	zpl_uint32 i = 0, ret = 0;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(cb)
			ret = (cb)(&gtrunk.group[i], pVoid);
		if(ret == ERROR)
			break;
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return OK;
}

int nsm_trunk_callback_api(l2trunk_cb cb, void *pVoid)
{
	zpl_uint32 i = 0, ret = 0;
	l2trunk_t *pstNode = NULL;
	NODE index;
	NSM_ENTER_FUNC();
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		for(pstNode = (l2trunk_t *)lstFirst(gtrunk.group[i].trunkList);
				pstNode != NULL;  pstNode = (l2trunk_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(pstNode->ifindex)
			{
				if(cb)
					ret = (cb)(pstNode, pVoid);
				if(ret == ERROR)
					break;
			}
		}
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return OK;
}

#ifdef ZPL_SHELL_MODULE
static int _trunk_interface_show_one(struct vty *vty, struct interface *ifp)
{
	if (if_is_lag(ifp))
	{
		const char *load[] = { "NONE", "dst-mac", "src-mac", "dst-src-mac", "dst-ip" };
		if(gtrunk.mutex)
			os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
		l2trunk_group_t * trunk_group = nsm_port_channel_get(ifp);
		if (trunk_group)
		{

			if (trunk_group->lacp_system_priority
					!= LACP_SYSTEM_PRIORITY_DEFAULT)
				vty_out(vty, " lacp system-priority %d%s",
						trunk_group->lacp_system_priority, VTY_NEWLINE);
			if (trunk_group->load_balance != LOAD_BALANCE_DEFAULT)
				vty_out(vty, " port-channel load-balance %s%s",
						load[trunk_group->load_balance], VTY_NEWLINE);
		}
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
	}
	else if (if_is_ethernet(ifp))
	{
		if(gtrunk.mutex)
			os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
		l2trunk_t * trunk = l2trunk_lookup_port(ifp->ifindex);
		if (trunk)
		{
			const char *mode[] = { "NONE", "active", "passive" };
			if (trunk->type == TRUNK_STATIC)
			{
				vty_out(vty, " static-channel-group %d%s", trunk->trunkId,
						VTY_NEWLINE);
			}
			else
			{
				vty_out(vty, " channel-group %d %s%s", trunk->trunkId,
						mode[trunk->mode], VTY_NEWLINE);
				if (trunk->lacp_port_priority != LACP_PORT_PRIORITY_DEFAULT)
					vty_out(vty, " lacp port-priority %d%s",
							trunk->lacp_port_priority, VTY_NEWLINE);
				if (trunk->lacp_timeout != LACP_TIMEOUT_DEFAULT)
					vty_out(vty, " lacp timeout %d%s", trunk->lacp_timeout,
							VTY_NEWLINE);
			}
		}
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
	}
	return 0;
}
#endif