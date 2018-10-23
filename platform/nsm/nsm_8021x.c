/*
 * nsm_8021x.c
 *
 *  Created on: May 10, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include <log.h>

#include "os_list.h"

#include "nsm_mac.h"
#include "nsm_vlan.h"
#include "nsm_8021x.h"

static Gdot1x_t gDot1x_t;

static int dot1x_cleanup(ifindex_t ifindex, BOOL all);

int nsm_dot1x_init(void)
{
	os_memset(&gDot1x_t, 0, sizeof(Gdot1x_t));
	gDot1x_t.dot1xList = malloc(sizeof(LIST));
	gDot1x_t.mutex = os_mutex_init();
	lstInit(gDot1x_t.dot1xList);
	return OK;
}


int nsm_dot1x_exit(void)
{
	if(lstCount(gDot1x_t.dot1xList))
	{
		dot1x_cleanup(0, TRUE);
		lstFree(gDot1x_t.dot1xList);
		free(gDot1x_t.dot1xList);
		gDot1x_t.dot1xList = NULL;
	}

	if(gDot1x_t.mutex)
		os_mutex_exit(gDot1x_t.mutex);

	return OK;
}

static int dot1x_cleanup(ifindex_t ifindex, BOOL all)
{
	dot1x_t *pstNode = NULL;
	NODE index;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	for(pstNode = (dot1x_t *)lstFirst(gDot1x_t.dot1xList);
			pstNode != NULL;  pstNode = (dot1x_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && ifindex && pstNode->ifindex == ifindex)
		{
			lstDelete(gDot1x_t.dot1xList, (NODE*)pstNode);
			XFREE(MTYPE_DOT1X, pstNode);
		}
		else if(pstNode && all)
		{
			lstDelete(gDot1x_t.dot1xList, (NODE*)pstNode);
			XFREE(MTYPE_DOT1X, pstNode);
		}
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return OK;
}

static int dot1x_nsm_client_setup(ifindex_t ifindex, void *p)
{
	struct interface *ifp = if_lookup_by_index(ifindex);
	if(ifp)
	{
		struct nsm_interface *nsm = ifp->info[MODULE_NSM];
		if(nsm)
			nsm->nsm_client[NSM_DOT1X] = p;
	}
	return OK;
}

static int dot1x_add_node(dot1x_t *value)
{
	dot1x_t *node = XMALLOC(MTYPE_DOT1X, sizeof(dot1x_t));
	if(node)
	{
		os_memset(node, 0, sizeof(dot1x_t));
		os_memcpy(node, value, sizeof(dot1x_t));
		lstAdd(gDot1x_t.dot1xList, (NODE *)node);
		dot1x_nsm_client_setup(node->ifindex, node);
		return OK;
	}
	return ERROR;
}

static int dot1x_del_node(dot1x_t *node)
{
	if(node)
	{
		dot1x_nsm_client_setup(node->ifindex, NULL);
		lstDelete(gDot1x_t.dot1xList, (NODE *)node);
		XFREE(MTYPE_DOT1X, node);
		return OK;
	}
	return ERROR;
}

static dot1x_t * dot1x_lookup_node(ifindex_t ifindex)
{
	dot1x_t *pstNode = NULL;
	NODE index;
	for(pstNode = (dot1x_t *)lstFirst(gDot1x_t.dot1xList);
			pstNode != NULL;  pstNode = (dot1x_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->ifindex == ifindex)
		{
			return pstNode;
		}
	}
	return NULL;
}

int dot1x_callback_api(dot1x_cb cb, void *pVoid)
{
	int ret = OK;
	dot1x_t *pstNode = NULL;
	NODE index;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	for(pstNode = (dot1x_t *)lstFirst(gDot1x_t.dot1xList);
			pstNode != NULL;  pstNode = (dot1x_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && cb)
		{
			ret = (cb)(pstNode, pVoid);
			if(ret != OK)
				break;
		}
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_global_enable(BOOL enable)
{
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	gDot1x_t.enable = enable;
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return OK;
}

BOOL nsm_dot1x_global_is_enable()
{
	BOOL enable;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	enable = gDot1x_t.enable;
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return enable;
}

int nsm_dot1x_enable_set_api(ifindex_t ifindex, BOOL enable, dot1x_type_en type)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		if(enable == TRUE)
		{
			dot1x_t value;
			os_memset(&value, 0, sizeof(dot1x_t));
			value.ifindex = ifindex;
			value.enable = TRUE;
			value.type = type;
			value.port_mode = TRUE;
			ret = dot1x_add_node(&value);
		}
		else
			ret = ERROR;
	}
	else
	{
		if(enable == TRUE)
		{
			dot1x->enable = TRUE;
			dot1x->type = type;
			ret = OK;
		}
		else
		{
			dot1x->enable = FALSE;
			ret = dot1x_del_node(dot1x);
		}
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_enable_get_api(ifindex_t ifindex, BOOL *enable, dot1x_type_en *type)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(enable)
			*enable = dot1x->enable;
		if(type)
			*type = dot1x->type;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

BOOL nsm_dot1x_is_enable_api(ifindex_t ifindex)
{
	BOOL ret = FALSE;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(dot1x)
	{
		if(dot1x->enable)
			ret = TRUE;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_auth_type_set_api(ifindex_t ifindex, dot1x_type_en type)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->type = type;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_auth_type_get_api(ifindex_t ifindex, dot1x_type_en *type)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(type)
			*type = dot1x->type;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}


int nsm_dot1x_auth_state_set_api(ifindex_t ifindex, dot1x_state_en state)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->state = state;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_auth_state_get_api(ifindex_t ifindex, dot1x_state_en *state)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(state)
			*state = dot1x->state;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_auth_version_set_api(ifindex_t ifindex, u_int version)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->eap_version = version;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_auth_version_get_api(ifindex_t ifindex, u_int *version)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(version)
			*version = dot1x->eap_version;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_reauthentication_set_api(ifindex_t ifindex, BOOL enable)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->reauthentication = enable;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_reauthentication_get_api(ifindex_t ifindex, BOOL *enable)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(enable)
			*enable = dot1x->reauthentication;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_port_mode_set_api(ifindex_t ifindex, BOOL port)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->port_mode = port;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_port_mode_get_api(ifindex_t ifindex, BOOL *port)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(port)
			*port = dot1x->port_mode;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_mac_auth_bypass_set_api(ifindex_t ifindex, BOOL enable)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->mac_auth_bypass = enable;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_mac_auth_bypass_get_api(ifindex_t ifindex, BOOL *enable)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(enable)
			*enable = dot1x->mac_auth_bypass;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_guest_vlan_set_api(ifindex_t ifindex, vlan_t vlan)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->guest_vlan = vlan;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_guest_vlan_get_api(ifindex_t ifindex, vlan_t *vlan)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(vlan)
			*vlan = dot1x->guest_vlan;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_max_user_set_api(ifindex_t ifindex, u_int maxUser)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->max_user = maxUser;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_max_user_get_api(ifindex_t ifindex, u_int *maxUser)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(maxUser)
			*maxUser = dot1x->max_user;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_reauth_timeout_set_api(ifindex_t ifindex, u_int timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->reauth_timeout = timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_reauth_timeout_get_api(ifindex_t ifindex, u_int *timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(timeout)
			*timeout = dot1x->reauth_timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_server_timeout_set_api(ifindex_t ifindex, u_int timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->server_timeout = timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_server_timeout_get_api(ifindex_t ifindex, u_int *timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(timeout)
			*timeout = dot1x->server_timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_supp_timeout_set_api(ifindex_t ifindex, u_int timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->supp_timeout = timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_supp_timeout_get_api(ifindex_t ifindex, u_int *timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(timeout)
			*timeout = dot1x->supp_timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_period_timeout_set_api(ifindex_t ifindex, u_int timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->tx_period_timeout = timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_period_timeout_get_api(ifindex_t ifindex, u_int *timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(timeout)
			*timeout = dot1x->tx_period_timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_quiet_period_timeout_set_api(ifindex_t ifindex, u_int timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->quiet_period_timeout = timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_quiet_period_timeout_get_api(ifindex_t ifindex, u_int *timeout)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(timeout)
			*timeout = dot1x->quiet_period_timeout;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}


int nsm_dot1x_max_req_set_api(ifindex_t ifindex, u_int count)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		dot1x->max_req = count;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}

int nsm_dot1x_max_req_get_api(ifindex_t ifindex, u_int *count)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		if(count)
			*count = dot1x->max_req;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}


int nsm_dot1x_reset_api(ifindex_t ifindex)
{
	int ret = 0;
	dot1x_t *dot1x = NULL;
	if(gDot1x_t.mutex)
		os_mutex_lock(gDot1x_t.mutex, OS_WAIT_FOREVER);
	dot1x = dot1x_lookup_node(ifindex);
	if(!dot1x)
	{
		ret = ERROR;
	}
	else
	{
		//dot1x->max_req = count;
		ret = OK;
	}
	if(gDot1x_t.mutex)
		os_mutex_unlock(gDot1x_t.mutex);
	return ret;
}
