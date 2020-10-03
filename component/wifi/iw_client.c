/*
 * iw_client.c
 *
 *  Created on: Jul 15, 2018
 *      Author: zhurish
 */



#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"
#ifdef PL_DHCPC_MODULE
#include "nsm_dhcp.h"
#endif
#include "iw_config.h"
#include "iw_client.h"
#include "iw_interface.h"
#include "iwlib.h"


//static iw_client_t gIw_client_t;

/*static int iw_client_db_cleanup(ifindex_t ifindex, BOOL all);
static int iw_client_ap_cleanup(void);*/
static int iw_client_db_add_node(iw_client_t *iw_client, iw_client_db_t *value);

/*
 * WIFI DB
 */
static int iw_client_db_save_one(FILE *fp, iw_client_db_t *pstNode)
{
	if(fp && pstNode && pstNode->ifindex)
	{
		fprintf(fp, "%d:", pstNode->ifindex);
		fprintf(fp, "%s:", strlen(pstNode->SSID) ? pstNode->SSID:"none");
		fprintf(fp, "%s\n", strlen(pstNode->password) ? pstNode->password:"none");
		fflush(fp);
		return OK;
	}
	return ERROR;
}


static FILE * iw_client_db_open(void)
{
	FILE *fp = fopen(IW_CLIENT_DB_FILE, "w+");
	return fp;
}

static int iw_client_db_close(FILE *fp)
{
	if(fp)
		fclose(fp);
	return OK;
}

static int iw_client_db_save(iw_client_t *iw_client)
{
	FILE *fp = NULL;
	iw_client_db_t *pstNode = NULL;
	NODE index;
	assert(iw_client != NULL);
	if(!iw_client->db_list)
		return ERROR;
	if(lstCount(iw_client->db_list)<=0)
	{
		return OK;
	}
	rename(IW_CLIENT_DB_FILE, IW_CLIENT_DB_OLD_FILE);
	sync();
	fp = iw_client_db_open();
	if(!fp)
	{
		rename(IW_CLIENT_DB_OLD_FILE, IW_CLIENT_DB_FILE);
		sync();
		return ERROR;
	}
	if(iw_client->db_mutex)
		os_mutex_lock(iw_client->db_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_client_db_t *)lstFirst(iw_client->db_list);
			pstNode != NULL;  pstNode = (iw_client_db_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		iw_client_db_save_one(fp, pstNode);
	}
	if(iw_client->db_mutex)
		os_mutex_unlock(iw_client->db_mutex);

	iw_client_db_close(fp);
	return OK;
}


static int iw_client_db_split(char *input, iw_client_db_t *value)
{
	int num = 0, i = 0, j = 0, len = strlen(input);
	char *s = input;
	char name[64];
	assert(value != NULL);
	memset(name, 0, sizeof(name));
	for(i = 0; (i < len) && s; i++)
	{
		if(s[i] == ':')
		{
			if(num == 0)
			{
				j = 0;
				num = 1;
			}
			else if(num == 1)
			{
				j = 0;
				num = 2;
			}
		}
		else
		{
			if(num == 0)
				name[j++] = s[i];
			else if(num == 1)
				value->SSID[j++] = s[i];
			else if(num == 2)
				value->password[j++] = s[i];
		}
	}
	if(strlen(name) && all_digit(name))
	{
		if(strstr(value->SSID, "none"))
			memset(value->SSID, 0, sizeof(value->SSID));
		if(strstr(value->password, "none"))
			memset(value->password, 0, sizeof(value->password));
		value->ifindex = atoi(name);
		return 0;
	}
	return -1;
}


static int iw_client_db_load(iw_client_t *iw_client)
{
	iw_client_db_t value;
	char buf[512];
	memset(buf, 0, IW_SSID_NAME_MAX);
	assert(iw_client != NULL);
	FILE *fp = iw_client_db_open();
	if(!fp)
		return OK;
	if(iw_client->db_mutex)
		os_mutex_lock(iw_client->db_mutex, OS_WAIT_FOREVER);

	if(fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			os_memset(&value, 0, sizeof(iw_client_db_t));
			if(iw_client_db_split(buf, &value) == 0)
			{
				iw_client_db_add_node(iw_client, &value);
			}
		}
		fclose(fp);
	}
	if(iw_client->db_mutex)
		os_mutex_unlock(iw_client->db_mutex);
	return OK;
}


static int iw_client_db_cleanup(iw_client_t *iw_client, ifindex_t ifindex, BOOL all)
{
	iw_client_db_t *pstNode = NULL;
	NODE index;
	assert(iw_client != NULL);
	if(!iw_client->db_list)
		return ERROR;
	if(iw_client->db_mutex)
		os_mutex_lock(iw_client->db_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_client_db_t *)lstFirst(iw_client->db_list);
			pstNode != NULL;  pstNode = (iw_client_db_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && ifindex && pstNode->ifindex == ifindex)
		{
			lstDelete(iw_client->db_list, (NODE*)pstNode);
			XFREE(MTYPE_WIFI_DB, pstNode);
		}
		else if(pstNode && all)
		{
			lstDelete(iw_client->db_list, (NODE*)pstNode);
			XFREE(MTYPE_WIFI_DB, pstNode);
		}
	}
	if(iw_client->db_mutex)
		os_mutex_unlock(iw_client->db_mutex);
	return OK;
}


static int iw_client_db_add_node(iw_client_t *iw_client, iw_client_db_t *value)
{
	assert(iw_client != NULL);
	assert(value != NULL);
	if(!iw_client->db_list)
		return ERROR;
	iw_client_db_t *node = XMALLOC(MTYPE_WIFI_DB, sizeof(iw_client_db_t));
	if(node)
	{
		os_memset(node, 0, sizeof(iw_client_db_t));
		os_memcpy(node, value, sizeof(iw_client_db_t));
		lstAdd(iw_client->db_list, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static int iw_client_db_del_node(iw_client_t *iw_client, iw_client_db_t *node)
{
	if(node && iw_client->db_list)
	{
		lstDelete(iw_client->db_list, (NODE *)node);
		XFREE(MTYPE_WIFI_DB, node);
		return OK;
	}
	return ERROR;
}

static iw_client_db_t * iw_client_db_lookup_node(iw_client_t *iw_client, char *ssid)
{
	iw_client_db_t *pstNode = NULL;
	NODE index;
	char SSID[IW_SSID_NAME_MAX];
	assert(iw_client != NULL);
	if(!iw_client->db_list)
		return NULL;
	memset(SSID, 0, IW_SSID_NAME_MAX);
	if(ssid)
		strcpy(SSID, ssid);

	for(pstNode = (iw_client_db_t *)lstFirst(iw_client->db_list);
			pstNode != NULL;  pstNode = (iw_client_db_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(memcmp(pstNode->SSID, SSID, IW_SSID_NAME_MAX) == 0)
		{
			return pstNode;
		}
	}
	return NULL;
}




int iw_client_db_set_api(iw_client_t *iw_client, char *ssid, char *pass)
{
	int ret = 0;
	char SSID[IW_SSID_NAME_MAX];
	iw_client_db_t *client = NULL;
	if(!iw_client || !ssid)
		return ERROR;
	if(!iw_client->db_list)
		return ERROR;
	memset(SSID, 0, IW_SSID_NAME_MAX);
	if(ssid)
		strcpy(SSID, ssid);
	if(iw_client->db_mutex)
		os_mutex_lock(iw_client->db_mutex, OS_WAIT_FOREVER);
	client = iw_client_db_lookup_node(iw_client, SSID);
	if(!client)
	{
		iw_client_db_t value;
		os_memset(&value, 0, sizeof(iw_client_db_t));
		memcpy(value.SSID, SSID, IW_SSID_NAME_MAX);
		if(pass)
		{
			if(os_memcmp(pass, "*#@", 3) == 0)
			{
				memset(value.encrypt_password, 0, sizeof(value.encrypt_password));
				strcpy(value.encrypt_password, pass);
				//md5_encrypt_password(value.password, value.encrypt_password);
			}
			else
			{
				strcpy(value.password, pass);
				memset(value.encrypt_password, 0, sizeof(value.encrypt_password));
				md5_encrypt_password(value.password, value.encrypt_password);
			}

			if(IW_DEBUG(DB))
			{
				zlog_debug(ZLOG_WIFI, "add password %s for SSID=%s", value.SSID, value.password);
			}
		}
		else
		{
			if(IW_DEBUG(DB))
			{
				zlog_debug(ZLOG_WIFI, "add SSID=%s", value.SSID);
			}
		}
		value.ifindex = iw_client->ifindex;
		ret = iw_client_db_add_node(iw_client, &value);
		if(iw_client->now == NULL)
			iw_client->now = iw_client_db_lookup_node(iw_client, SSID);
		os_memcpy(&iw_client->cu, &value, sizeof(iw_client_db_t));

	}
	else
	{
		os_memset(client->password, 0, sizeof(client->password));
		if(pass)
		{
			if(os_memcmp(pass, "*#@", 3) == 0)
			{
				memset(client->encrypt_password, 0, sizeof(client->encrypt_password));
				strcpy(client->encrypt_password, pass);
				//md5_encrypt_password(value.password, value.encrypt_password);
			}
			else
			{
				strcpy(client->password, pass);
				memset(client->encrypt_password, 0, sizeof(client->encrypt_password));
				md5_encrypt_password(client->password, client->encrypt_password);
			}
/*			strcpy(client->password, pass);
			memset(client->encrypt_password, 0, sizeof(client->encrypt_password));
			md5_encrypt_password(client->password, client->encrypt_password);*/
			if(IW_DEBUG(DB))
			{
				zlog_debug(ZLOG_WIFI, "update password %s for SSID=%s", client->SSID, client->password);
			}
		}
		client->ifindex = iw_client->ifindex;
		os_memcpy(&iw_client->cu, client, sizeof(iw_client_db_t));
		ret = OK;
	}
	if(iw_client->db_mutex)
		os_mutex_unlock(iw_client->db_mutex);
	if(ret == OK)
		iw_client_db_save(iw_client);
	return ret;
}

int iw_client_db_del_api(iw_client_t *iw_client, char *ssid, BOOL pass)
{
	int ret = 0;
	char SSID[IW_SSID_NAME_MAX];
	iw_client_db_t *client = NULL;
	if(!iw_client || !ssid)
		return ERROR;
	if(!iw_client->db_list)
		return ERROR;
	memset(SSID, 0, IW_SSID_NAME_MAX);
	if(ssid)
		strcpy(SSID, ssid);
	if(iw_client->db_mutex)
		os_mutex_lock(iw_client->db_mutex, OS_WAIT_FOREVER);
	client = iw_client_db_lookup_node(iw_client, SSID);
	if(!client)
	{
		ret = OK;
	}
	else
	{
		if(pass)
		{
			memset(client->encrypt_password, 0, sizeof(client->encrypt_password));
			os_memset(client->password, 0, sizeof(client->password));
			os_memcpy(&iw_client->cu, client, sizeof(iw_client_db_t));
			ret = OK;
			if(IW_DEBUG(DB))
			{
				zlog_debug(ZLOG_WIFI, "delete password %s for SSID=%s", client->SSID, client->password);
			}
		}
		else
		{
			if(IW_DEBUG(DB))
			{
				zlog_debug(ZLOG_WIFI, "delete SSID=%s", client->SSID);
			}
			os_memset(&iw_client->cu, 0, sizeof(iw_client_db_t));
			if(iw_client->now == client)
				iw_client->now = NULL;
			ret = iw_client_db_del_node(iw_client, client);
		}
	}
	if(iw_client->db_mutex)
		os_mutex_unlock(iw_client->db_mutex);
	if(ret == OK)
		iw_client_db_save(iw_client);
	return ret;
}

iw_client_db_t * iw_client_db_lookup_api(iw_client_t *iw_client, char *ssid)
{
	char SSID[IW_SSID_NAME_MAX];
	iw_client_db_t *client = NULL;
	if(!iw_client || !ssid)
		return NULL;
	if(!iw_client->db_list)
		return NULL;
	memset(SSID, 0, IW_SSID_NAME_MAX);
	if(ssid)
		strcpy(SSID, ssid);
	if(iw_client->db_mutex)
		os_mutex_lock(iw_client->db_mutex, OS_WAIT_FOREVER);
	client = iw_client_db_lookup_node(iw_client, SSID);
	if(iw_client->db_mutex)
		os_mutex_unlock(iw_client->db_mutex);
	return client;
}


int iw_client_db_callback_api(iw_client_t *iw_client, int (*cb)(iw_client_db_t *, void *), void *pVoid)
{
	int ret = OK;
	iw_client_db_t *pstNode = NULL;
	NODE index;
	if(!iw_client)
		return ERROR;
	if(!iw_client->db_list)
		return ERROR;
	if(iw_client->db_mutex)
		os_mutex_lock(iw_client->db_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_client_db_t *)lstFirst(iw_client->db_list);
			pstNode != NULL;  pstNode = (iw_client_db_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && cb)
		{
			ret = (cb)(pstNode, pVoid);
			if(ret != OK)
				break;
		}
	}
	if(iw_client->db_mutex)
		os_mutex_unlock(iw_client->db_mutex);
	return ret;
}

/*
 * AP CLIENT
 */

/*
 * global
 */

iw_client_t * iw_client_lookup_api(struct interface *ifp)
{
	iw_t * iw = nsm_iw_get(ifp);
	if(iw)
	{
		if(iw->mode == IW_MODE_CLIENT || iw->mode == IW_MODE_MANAGE)
			return &iw->private.client;
	}
	return NULL;
}


static int iw_client_ap_cleanup(iw_client_t *iw_client, BOOL use)
{
	NODE index;
	LIST *list = NULL;
	iw_client_ap_t *pstNode = NULL;
	assert(iw_client != NULL);
	if(use)
		list = iw_client->ap_list;
	else
		list = iw_client->ap_unlist;
	if(!list)
		return ERROR;
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_client_ap_t *)lstFirst(list);
			pstNode != NULL;  pstNode = (iw_client_ap_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(list, (NODE*)pstNode);
			XFREE(MTYPE_WIFI_AP, pstNode);
		}
	}
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return OK;
}


static int iw_client_ap_add_sort_node(iw_client_t *iw_client, iw_client_ap_t *value)
{
	iw_client_ap_t *pnode = NULL;
	NODE index;
	assert(iw_client != NULL);
	assert(value != NULL);
	if(!iw_client->ap_list)
		return ERROR;
	for(pnode = (iw_client_ap_t *)lstFirst(iw_client->ap_list);
			pnode != NULL;  pnode = (iw_client_ap_t *)lstNext((NODE*)&index))
	{
		index = pnode->node;
		if(pnode)
		{
			if(value->signal > pnode->signal)
			{
				break;
			}
		}
	}
	if(pnode)
	{
		lstInsert (iw_client->ap_list, (NODE*)pnode, (NODE*)value);
	}
	else
		lstAdd(iw_client->ap_list, (NODE *)value);

	return OK;
}

static int iw_client_ap_add_node(iw_client_t *iw_client, iw_client_ap_t *value)
{
	iw_client_ap_t *node = NULL;
	assert(iw_client != NULL);
	assert(value != NULL);
	if(!iw_client->ap_unlist)
		return ERROR;
	if(lstCount(iw_client->ap_unlist))
	{
		node = (iw_client_ap_t *)lstFirst(iw_client->ap_unlist);
		lstDelete(iw_client->ap_unlist, (NODE *)node);
	}
	if(node == NULL)
		node = XMALLOC(MTYPE_WIFI_AP, sizeof(iw_client_ap_t));
	if(node)
	{
		os_memset(node, 0, sizeof(iw_client_ap_t));
		os_memcpy(node, value, sizeof(iw_client_ap_t));

		node->ttl = 2;
		iw_client_ap_add_sort_node(iw_client, node);
		//lstAdd(iw_client->ap_list, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static int iw_client_ap_del_node(iw_client_t *iw_client, iw_client_ap_t *node)
{
	if(node && iw_client->ap_list && iw_client->ap_unlist)
	{
		lstDelete(iw_client->ap_list, (NODE *)node);
		lstAdd(iw_client->ap_unlist, (NODE *)node);
		//XFREE(MTYPE_WIFI_AP, node);
		return OK;
	}
	return ERROR;
}

static iw_client_ap_t * iw_client_ap_lookup_node(iw_client_t *iw_client, u_int8 *bssid, char *ssid)
{
	iw_client_ap_t *pstNode = NULL;
	NODE index;
	char SSID[IW_SSID_NAME_MAX];
	u_int8 BSSID[IW_SSID_NAME_MAX];
	assert(iw_client != NULL);
	if(!iw_client->ap_list)
		return NULL;
	memset(SSID, 0, IW_SSID_NAME_MAX);
	memset(BSSID, 0, IW_SSID_NAME_MAX);
	if(ssid)
		strcpy(SSID, ssid);
	if(bssid)
		memcpy(BSSID, bssid, NSM_MAC_MAX);

	for(pstNode = (iw_client_ap_t *)lstFirst(iw_client->ap_list);
			pstNode != NULL;  pstNode = (iw_client_ap_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(ssid)
		{
			if(memcmp(pstNode->SSID, SSID, IW_SSID_NAME_MAX) == 0)
			{
				return pstNode;
			}
		}
		if(bssid)
		{
			if(memcmp(pstNode->BSSID, BSSID, NSM_MAC_MAX) == 0)
			{
				return pstNode;
			}
		}
	}
	return NULL;
}


static int iw_client_scan_update(iw_client_t *iw_client)
{
	iw_client_ap_t *pstNode = NULL;
	NODE index;
	assert(iw_client != NULL);
	if(!iw_client->ap_list)
		return ERROR;
	for(pstNode = (iw_client_ap_t *)lstFirst(iw_client->ap_list);
			pstNode != NULL;  pstNode = (iw_client_ap_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		pstNode->ttl--;
	}

	for(pstNode = (iw_client_ap_t *)lstFirst(iw_client->ap_list);
			pstNode != NULL;  pstNode = (iw_client_ap_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->ttl == 0)
		{
			if(IW_DEBUG(SCAN))
			{
				zlog_debug(ZLOG_WIFI, "delete network SSID = %s", pstNode->SSID);
			}
			iw_client_ap_del_node(iw_client, pstNode);
		}
	}
	return OK;
}

int iw_client_ap_set_api(iw_client_t *iw_client, u_int8 *bssid, iw_client_ap_t *ap)
{
	int ret = 0;
	u_int8 BSSID[IW_SSID_NAME_MAX];
	iw_client_ap_t *client = NULL;
	if(!iw_client || !bssid || !ap)
		return ERROR;
	if(!iw_client->ap_list)
		return ERROR;
	memset(BSSID, 0, IW_SSID_NAME_MAX);
	if(bssid)
		memcpy(BSSID, bssid, NSM_MAC_MAX);
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	client = iw_client_ap_lookup_node(iw_client, BSSID, NULL);
	if(!client)
	{
		ret = iw_client_ap_add_node(iw_client, ap);
		if(IW_DEBUG(SCAN))
		{
			char buf[128];
			struct prefix prefix_eth;
			union prefix46constptr pu;
			prefix_eth.family = AF_ETHERNET;
			memcpy(prefix_eth.u.prefix_eth.octet, ap->BSSID, NSM_MAC_MAX);
			pu.p = &prefix_eth;
			memset(buf, 0, sizeof(buf));
			zlog_debug(ZLOG_WIFI, "add network by BSSID = %s SSID = %s",
					prefix_2_address_str (pu, buf, sizeof(buf)), ap->SSID);
		}
	}
	else
	{
		NODE node;
		node = client->node;
		os_memcpy(client, ap, sizeof(iw_client_ap_t));
		client->node = node;
		client->ttl = 2;
		//zlog_debug(ZLOG_PAL, "%s update", __func__);
		ret = OK;
	}
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return ret;
}

int iw_client_ap_del_api(iw_client_t *iw_client, u_int8 *bssid, char *ssid)
{
	int ret = 0;
	iw_client_ap_t *client = NULL;
	if(!iw_client || (!bssid && !ssid))
		return ERROR;
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	client = iw_client_ap_lookup_node(iw_client, bssid, ssid);
	if(!client)
	{
		ret = OK;
	}
	else
	{
		if(IW_DEBUG(SCAN))
		{
			char buf[128];
			struct prefix prefix_eth;
			union prefix46constptr pu;
			prefix_eth.family = AF_ETHERNET;
			memcpy(prefix_eth.u.prefix_eth.octet, client->BSSID, NSM_MAC_MAX);
			pu.p = &prefix_eth;
			memset(buf, 0, sizeof(buf));
			zlog_debug(ZLOG_WIFI, "add network by BSSID = %s SSID = %s",
					prefix_2_address_str (pu, buf, sizeof(buf)), client->SSID);
		}
		if(iw_client->now == client)
			iw_client->now = NULL;
		ret = iw_client_ap_del_node(iw_client, client);
	}
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return ret;
}

iw_client_ap_t * iw_client_ap_lookup_api(iw_client_t *iw_client, u_int8 *bssid, char *ssid)
{
	iw_client_ap_t *client = NULL;
	if(!iw_client || (!bssid && !ssid))
		return NULL;
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	client = iw_client_ap_lookup_node(iw_client, bssid, ssid);
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return client;
}


int iw_client_neighbor_callback_api(iw_client_t *iw_client, int (*cb)(iw_client_ap_t *, void *), void *pVoid)
{
	int ret = OK;
	iw_client_ap_t *pstNode = NULL;
	NODE index;
	if(!iw_client)
		return ERROR;
	if(!iw_client->ap_list)
		return ERROR;
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_client_ap_t *)lstFirst(iw_client->ap_list);
			pstNode != NULL;  pstNode = (iw_client_ap_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && cb)
		{
			ret = (cb)(pstNode, pVoid);
			if(ret != OK)
				break;
		}
	}
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return ret;
}


int iw_client_connect_interval_api(iw_client_t *iw_client, int connect_interval)
{
	int ret = OK;
	if(!iw_client)
		return ERROR;
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	iw_client->connect_delay = connect_interval;
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return ret;
}

int iw_client_scan_interval_api(iw_client_t *iw_client, int scan_interval)
{
	int ret = OK;
	if(!iw_client)
		return ERROR;
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	iw_client->scan_interval = scan_interval;
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return ret;
}

int iw_client_scan_max_api(iw_client_t *iw_client, int scan_max)
{
	int ret = OK;
	if(!iw_client)
		return ERROR;
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	iw_client->scan_max = scan_max;
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return ret;
}

static int iw_client_is_connect(struct interface *ifp, iw_client_ap_t *ap, u_int8 *bssid)
{
	if(!ifp || !bssid)
		return ERROR;
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		if(iw_client_dev_is_connect(ifp->k_name, bssid) == 0)
		{
			if(IW_DEBUG(EVENT))
			{
				zlog_debug(ZLOG_WIFI, "interface %s is connect",ifp->name);
			}
			return OK;
		}
		if(IW_DEBUG(EVENT))
		{
			zlog_debug(ZLOG_WIFI, "interface %s is not connect",ifp->name);
		}
	}
	return ERROR;
}

static int iw_client_try_connect(iw_client_t *iw_client, iw_client_ap_t *ap, int retry)
{
	//struct interface *ifp = if_lookup_by_index(ap->ifindex);
	if(!iw_client || !ap)
		return ERROR;
	iw_client_db_t * db = iw_client_db_lookup_api(iw_client, ap->SSID);
	if(db)
	{
		if(IW_DEBUG(EVENT))
		{
			//zlog_debug(ZLOG_WIFI, "got the password config on \"%s\" start to connect",ap->SSID);
		}
		//TODO setup password and start to connect
		struct interface *ifp = if_lookup_by_index(db->ifindex);
		if(ifp)
			return iw_client_dev_connect(ifp, ap, db->SSID, db->password);
	}
	if(IW_DEBUG(EVENT))
	{
		;//zlog_debug(ZLOG_WIFI, "can not gtt the password config on \"%s\"",ap->SSID);
	}
	return ERROR;
}


static int iw_client_connect(iw_client_t *iw_client, iw_client_ap_t *ap, u_int8 *bssid)
{
	int count = 10;
	if(!iw_client || !ap || !bssid)
		return ERROR;
	struct interface *ifp = if_lookup_by_index(ap->ifindex);
	if(ifp && iw_client_try_connect(iw_client, ap, 3)==OK)
	{
		os_sleep(1);
		if(iw_client_is_connect(ifp, ap, bssid) == OK)
		{
			if(IW_DEBUG(EVENT))
			{
				zlog_debug(ZLOG_WIFI, "running DHCPC on interface %s on \"%s\"",ifp->name, ap->SSID);
			}
			ap->connect = 1;
#ifdef PL_DHCPC_MODULE
			if(DHCP_CLIENT == nsm_interface_dhcp_mode_get_api(ifp))
			{
				return nsm_interface_dhcpc_start(ifp, TRUE);
			}
			else if((DHCP_RELAY == nsm_interface_dhcp_mode_get_api(ifp)) ||
					(DHCP_SERVER == nsm_interface_dhcp_mode_get_api(ifp)) )
				return ERROR;
			else
			{
				//nsm_interface_address_dhcpc_set_api
				if(nsm_interface_dhcp_mode_set_api(ifp, DHCP_CLIENT, NULL) == OK)
					return nsm_interface_dhcpc_start(ifp, TRUE);
			}
#else
			return iw_client_dev_start_dhcpc(ifp);
#endif
		}
		ap->connect = 0;
		count--;
		if(count == 0)
			return ERROR;
	}
	return ERROR;
}

static int iw_client_connect_process(iw_client_t *iw_client)
{
	int ret = ERROR;
	iw_client_ap_t *pstNode = NULL;
	NODE index;
	u_int8 bssid[8] = { 0 };
	if(!iw_client)
		return ERROR;
	if(!iw_client->ap_list)
		return ERROR;
	struct interface *ifp = if_lookup_by_index(iw_client->ifindex);
	if(!(ifp && ifp->k_ifindex && if_is_wireless(ifp)))
		return OK;
	if(iw_client->ap)
	{
		if(iw_client_is_connect(ifp, iw_client->ap, bssid) == OK)
		{
			if(IW_DEBUG(EVENT))
			{
				zlog_debug(ZLOG_WIFI, "is already connect to \"%s\" on interface %s ",
						iw_client->ap->SSID, ifp->name);
			}
			iw_client->ap->connect = 1;
			return OK;
		}
		else
		{
			iw_client->ap->connect = 0;
			iw_client->ap = NULL;
		}
	}
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_client_ap_t *)lstFirst(iw_client->ap_list);
			pstNode != NULL;  pstNode = (iw_client_ap_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(iw_client_connect(iw_client, pstNode, bssid) == OK)
			{
				iw_client->ap = iw_client_ap_lookup_node(iw_client, bssid, NULL);
				ret = OK;
				break;
			}
		}
	}
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return ret;
}

int iw_client_disconnect_api(iw_client_t *iw_client)
{
	if(!iw_client)
		return ERROR;
	struct interface *ifp = NULL;
	if(!iw_client->ap)
		return ERROR;
	ifp = if_lookup_by_index(iw_client->ap->ifindex);
	if(!ifp)
		return ERROR;
	if(IW_DEBUG(EVENT))
	{
		zlog_debug(ZLOG_WIFI, "disconnect on interface %s ",ifp->name);
	}
#ifdef PL_DHCPC_MODULE
	if(DHCP_CLIENT == nsm_interface_dhcp_mode_get_api(ifp))
	{
		nsm_interface_dhcpc_start(ifp, FALSE);
	}
#endif
	iw_client_dev_disconnect(ifp);

	return OK;
}

static int iw_client_disconnect_process(iw_client_t *iw_client)
{
	int ret = ERROR;
	iw_client_ap_t *pstNode = NULL;
	NODE index;
	if(!iw_client)
		return ERROR;
	if(!iw_client->ap_list)
		return ERROR;
	struct interface *ifp = if_lookup_by_index(iw_client->ifindex);
	if(!(ifp && ifp->k_ifindex && if_is_wireless(ifp)))
		return OK;
	if(iw_client->ap)
	{
		iw_client_disconnect_api(iw_client);
	}
	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_client_ap_t *)lstFirst(iw_client->ap_list);
			pstNode != NULL;  pstNode = (iw_client_ap_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			ifp = if_lookup_by_index(pstNode->ifindex);
			if(ifp)
			{
#ifdef PL_DHCPC_MODULE
				if(DHCP_CLIENT == nsm_interface_dhcp_mode_get_api(ifp))
				{
					nsm_interface_dhcpc_start(ifp, FALSE);
				}
#endif
				iw_client_dev_disconnect(ifp);
			}
		}
	}
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return ret;
}



#ifndef IW_ONCE_TASK
static int iw_client_task(iw_client_t *iw_client)
{
	if(!iw_client)
		return ERROR;
	int	scan_interval = iw_client->scan_interval;
	int	connect_delay = iw_client->connect_delay;
/*	struct interface *ifp = if_lookup_by_index(iw_client->ifindex);
	if(ifp)
	{
		if_kname_set(ifp, "wlp3s0");
		SET_FLAG(ifp->status, ZEBRA_INTERFACE_ATTACH);

		pal_interface_update_flag(ifp);
		ifp->k_ifindex = pal_interface_ifindex(ifp->k_name);
		pal_interface_get_lladdr(ifp);
	}*/
	os_sleep(5);
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	while(1)
	{
		if(iw_client->scan_enable)
		{
			if(scan_interval <= 0/* >= iw_client->scan_interval*/)
			{
				scan_interval = iw_client->scan_interval;
				iw_client_scan_process(iw_client);
				iw_client_scan_update(iw_client);
			}
			scan_interval -= 2;
		}
		if(iw_client->connect_enable)
		{
			if(connect_delay <= 0/* >= iw_client->connect_delay*/)
			{
				connect_delay = iw_client->connect_delay;
				iw_client_connect_process(iw_client);
			}
			connect_delay -= 2;
		}
		os_sleep(2);
	}
	return OK;
}
#endif


#ifdef IW_ONCE_TASK
static int iw_client_connect_thread(struct thread * thread)
{
	assert(thread != NULL);
	iw_client_t *iw_client = THREAD_ARG(thread);
	if(iw_client && iw_client->master)
	{
		if(iw_client->mutex)
			os_mutex_lock(iw_client->mutex, OS_WAIT_FOREVER);

		iw_client->connect_thread = NULL;

		if(iw_client->connect_enable)
		{
			iw_client_connect_process(iw_client);
		}
		iw_client->connect_thread = thread_add_timer(iw_client->master,
				iw_client_connect_thread, iw_client, iw_client->connect_delay);

		if(iw_client->mutex)
			os_mutex_unlock(iw_client->mutex);
	}
	return OK;
}

static int iw_client_scan_thread(struct thread * thread)
{
	assert(thread != NULL);
	iw_client_t *iw_client = THREAD_ARG(thread);
	if(iw_client && iw_client->master)
	{
		if(iw_client->mutex)
			os_mutex_lock(iw_client->mutex, OS_WAIT_FOREVER);

		iw_client->scan_thread = NULL;

		if(iw_client->scan_enable)
		{
			iw_client_scan_process(iw_client);
			iw_client_scan_update(iw_client);
		}
		iw_client->scan_thread = thread_add_timer(iw_client->master,
				iw_client_scan_thread, iw_client, iw_client->scan_interval);
/*		THREAD_TIMER_ON(iw_client->master, iw_client->scan_thread,
				iw_client_scan_thread, iw_client, iw_client->scan_interval);*/

		if(iw_client->mutex)
			os_mutex_unlock(iw_client->mutex);
	}
	return OK;
}
#endif

int iw_client_task_start(iw_client_t *iw_client)
{
	if(!iw_client)
		return ERROR;
#ifndef IW_ONCE_TASK
	if(iw_client->taskid == 0)
		iw_client->taskid = os_task_create("iwApClientTask", OS_TASK_DEFAULT_PRIORITY,
	               0, iw_client_task, iw_client, OS_TASK_DEFAULT_STACK);
	if(iw_client->taskid)
		return OK;
#endif
	return ERROR;
}


int iw_client_task_exit(iw_client_t *iw_client)
{
	if(!iw_client)
		return ERROR;
#ifndef IW_ONCE_TASK
	if(iw_client->taskid)
	{
		iw_client_disconnect_process(iw_client);
		if(os_task_destroy(iw_client->taskid)==OK)
			iw_client->taskid = 0;
	}
#endif
	return OK;
}





int iw_client_connect_start(iw_client_t *iw_client)
{
	if(!iw_client)
		return ERROR;
	iw_client->connect_enable = TRUE;
#ifdef IW_ONCE_TASK
	if(iw_client->master)
	{
		if(iw_client->connect_thread)
			thread_cancel(iw_client->connect_thread);
		iw_client->connect_thread = NULL;
		iw_client->connect_thread = thread_add_timer(iw_client->master,
				iw_client_connect_thread, iw_client, iw_client->connect_delay);
	/*	THREAD_TIMER_ON(iw_client->master, iw_client->connect_thread,
				iw_client_connect_thread, iw_client, iw_client->connect_delay);*/
	}
#endif
	return OK;
}


int iw_client_connect_exit(iw_client_t *iw_client)
{
	if(!iw_client)
		return ERROR;
#ifdef IW_ONCE_TASK
	if(iw_client->connect_thread)
		thread_cancel(iw_client->connect_thread);
	iw_client->connect_thread = NULL;
#endif
	iw_client->connect_enable = FALSE;
	iw_client_disconnect_process(iw_client);
	return OK;
}

int iw_client_connect_api(iw_client_t *iw_client, BOOL auto_connect)
{
	if(!iw_client)
		return ERROR;

	if(iw_client->ap_mutex)
		os_mutex_lock(iw_client->ap_mutex, OS_WAIT_FOREVER);

	iw_client->auto_connect = auto_connect;
	if(iw_client->ap_mutex)
		os_mutex_unlock(iw_client->ap_mutex);
	return OK;
}



int iw_client_scan_start(iw_client_t *iw_client)
{
	int scan_interval = 0;
	if(!iw_client)
		return ERROR;
	iw_client->scan_enable = TRUE;
#ifdef IW_ONCE_TASK
	if(iw_client->scan_thread)
	{
		thread_cancel(iw_client->scan_thread);
		iw_client->scan_thread = NULL;
		scan_interval = iw_client->scan_interval;
	}
	else
		scan_interval = 1;
	if(iw_client->master)
	{
		//iw_client->scan_thread = NULL;
		iw_client->scan_thread = thread_add_timer(iw_client->master,
				iw_client_scan_thread, iw_client, scan_interval);
	}
#endif
	return OK;
}


int iw_client_scan_exit(iw_client_t *iw_client)
{
	if(!iw_client)
		return ERROR;
#ifdef IW_ONCE_TASK
	if(iw_client->scan_thread)
		thread_cancel(iw_client->scan_thread);
	iw_client->scan_thread = NULL;
#endif
	iw_client->scan_enable = FALSE;

	iw_client_scan_process_exit(iw_client);
	return OK;
}


static int iw_client_neighbor_show_one(iw_client_ap_t *ap, struct vty *vty)
{
	char buf[128];
	struct prefix prefix_eth;
	union prefix46constptr pu;
	assert(ap != NULL);
	assert(vty != NULL);
	prefix_eth.family = AF_ETHERNET;
	memcpy(prefix_eth.u.prefix_eth.octet, ap->BSSID, NSM_MAC_MAX);
	pu.p = &prefix_eth;
/*	iw_printf("%-18s %-10s %-16s %-8s %-20s %-10s\n",
			"------------------", "----------", "----------------",
			"--------", "--------------------", "----------");
	iw_printf("%-18s %-10s %-16s %-8s %-20s %-10s\n",
			"      BSSIED      ", "   FREQ   ", "qual/max signal",
			"  KEY", "        NAME", "   MODE");
	iw_printf("%-18s %-10s %-16s %-8s %-20s %-10s\n",
			"------------------", "----------", "----------------",
			"--------", "--------------------", "----------");*/
	//SSID BSSID
	memset(buf, 0, sizeof(buf));
	vty_out(vty, "%-20s ", ap->SSID);
	vty_out(vty, "%-18s ", 	prefix_2_address_str (pu, buf, sizeof(buf)));

	//SIGNAL
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d/%d dBm", ap->signal, ap->nosie);
	vty_out(vty, "%-10s ", buf);

	//QAUL
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ap->qaul);
	vty_out(vty, "%-8s ", buf);

	//KEY
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s", ap->auth ? "on":"off");
	vty_out(vty, "%-8s ", buf);

	//FREQ
	memset(buf, 0, sizeof(buf));
	iw_print_freq_value(buf, sizeof(buf), ap->freq);
	vty_out(vty, "%-10s ", buf);

	vty_out(vty, "%-20s%s", ifindex2ifname(ap->ifindex), VTY_NEWLINE);

	return OK;
}

int iw_client_neighbor_show(iw_client_t *iw_client, struct vty *vty, BOOL all)
{
	assert(iw_client != NULL);
	assert(vty != NULL);
	if(all)
	{
		if(iw_client->ap_list && lstCount(iw_client->ap_list))
		{
			vty_out(vty,"%-20s %-18s %-10s %-8s %-8s %-10s %-20s%s",
					"--------------------", "------------------", "----------", "--------",
						"--------",  "----------", "--------------------", VTY_NEWLINE);
			vty_out(vty,"%-20s %-18s %-10s %-8s %-8s %-10s %-20s%s",
						"        NAME        ", "      BSSID      ", "  SIGNAL  ",
						"  QAUL  ", "  AUTH  ", "   FREQ   ", "     INTERFACE",VTY_NEWLINE);
			vty_out(vty,"%-20s %-18s %-10s %-8s %-8s %-10s %-20s%s",
					"--------------------", "------------------", "----------", "--------",
						"--------",  "----------", "--------------------", VTY_NEWLINE);
			return iw_client_neighbor_callback_api(iw_client, iw_client_neighbor_show_one, vty);
		}
		return OK;
	}
	else
	{

	}
	return OK;
}


int iw_client_connect_ap_show(iw_client_t *iw_client, struct vty *vty)
{
	if(!iw_client || !vty)
		return ERROR;
	struct interface *ifp = if_lookup_by_index(iw_client->ifindex);
	if(ifp && ifp->k_ifindex && if_is_wireless(ifp))
	{
		if(!nsm_iw_enable_get_api(ifp))
		{
			vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
			return OK;
		}
		return iw_client_dev_connect_show(ifp, vty);
	}
	return OK;
}

int iw_client_station_dump_show(iw_client_t *iw_client, struct vty *vty)
{
	if(!iw_client || !vty)
		return ERROR;
	struct interface *ifp = if_lookup_by_index(iw_client->ifindex);
	if(ifp && ifp->k_ifindex && if_is_wireless(ifp))
	{
		if(!nsm_iw_enable_get_api(ifp))
		{
			vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
			return OK;
		}
		return iw_client_dev_station_dump_show(ifp, vty);
	}
	return OK;
}

int iw_client_scan_ap_show(iw_client_t *iw_client, struct vty *vty)
{
	if(!iw_client || !vty)
		return ERROR;
	struct interface *ifp = if_lookup_by_index(iw_client->ifindex);
	if(ifp && ifp->k_ifindex && if_is_wireless(ifp))
	{
		if(!nsm_iw_enable_get_api(ifp))
		{
			vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
			return OK;
		}
		return iw_client_dev_scan_ap_show(ifp, vty);
	}
	return OK;
}

int iw_client_init(iw_client_t *iw_client, ifindex_t ifindex)
{
	assert(iw_client != NULL);
	assert(ifindex != NULL);
	os_memset(iw_client, 0, sizeof(iw_client_t));
	iw_client->db_list = malloc(sizeof(LIST));
	iw_client->db_mutex = os_mutex_init();
	lstInit(iw_client->db_list);

	iw_client->ap_list = malloc(sizeof(LIST));
	iw_client->ap_mutex = os_mutex_init();
	lstInit(iw_client->ap_list);

	iw_client->ap_unlist = malloc(sizeof(LIST));
	lstInit(iw_client->ap_unlist);

	iw_client->mutex = os_mutex_init();

	iw_client->connect_delay = IW_CLIENT_CON_DEFAULT;
	iw_client->scan_interval = IW_CLIENT_SCAN_DEFAULT;

	iw_client_db_load(iw_client);
#ifdef IW_ONCE_TASK
	iw_client->master = master_thread[PL_WIFI_MODULE] = thread_master_module_create (PL_WIFI_MODULE);
#endif


	iw_client_task_start(iw_client);

	iw_client_scan_start(iw_client);
	iw_client_connect_start(iw_client);


	iw_client->ifindex = ifindex;
	//iw_dev_mode_set();
	return OK;
}


int iw_client_exit(iw_client_t *iw_client)
{
	assert(iw_client != NULL);
	iw_client_task_exit(iw_client);
	iw_client_scan_exit(iw_client);
	iw_client_connect_exit(iw_client);
#ifdef IW_ONCE_TASK
	iw_client->master = NULL;
#endif
	if(iw_client->db_list && lstCount(iw_client->db_list))
	{
		iw_client_db_cleanup(iw_client, 0, TRUE);
		lstFree(iw_client->db_list);
		//iw_client->db_list = NULL;
	}
	rename(IW_CLIENT_DB_FILE, IW_CLIENT_DB_OLD_FILE);
	sync();

	if(iw_client->ap_list && lstCount(iw_client->ap_list))
	{
		iw_client_ap_cleanup(iw_client, TRUE);
		lstFree(iw_client->ap_list);
		//iw_client->ap_list = NULL;
	}
	if(iw_client->ap_unlist && lstCount(iw_client->ap_unlist))
	{
		iw_client_ap_cleanup(iw_client, FALSE);
		lstFree(iw_client->ap_unlist);
		//iw_client->ap_unlist = NULL;
	}
	if(iw_client->db_mutex)
		os_mutex_exit(iw_client->db_mutex);
	iw_client->db_mutex = NULL;

	if(iw_client->ap_mutex)
		os_mutex_exit(iw_client->ap_mutex);
	iw_client->ap_mutex = NULL;

	if(iw_client->db_list)
		free(iw_client->db_list);
	iw_client->db_list = NULL;

	if(iw_client->ap_list)
		free(iw_client->ap_list);
	iw_client->ap_list = NULL;

	if(iw_client->ap_unlist)
		free(iw_client->ap_unlist);
	iw_client->ap_unlist = NULL;

	if(iw_client->mutex)
		os_mutex_exit(iw_client->mutex);
	iw_client->mutex = NULL;

	return OK;
}

int iw_client_enable(iw_client_t *iw_client, BOOL enable)
{
	if(!iw_client)
		return ERROR;
	if(enable)
	{
		if(iw_client->mutex)
			os_mutex_lock(iw_client->mutex, OS_WAIT_FOREVER);
		iw_client_task_start(iw_client);
		iw_client_scan_start(iw_client);
		iw_client_connect_start(iw_client);
		if(iw_client->mutex)
			os_mutex_unlock(iw_client->mutex);
	}
	else
	{
		if(iw_client->mutex)
			os_mutex_lock(iw_client->mutex, OS_WAIT_FOREVER);

		iw_client_task_exit(iw_client);
		iw_client_scan_exit(iw_client);
		iw_client_connect_exit(iw_client);
		if(iw_client->ap_list && lstCount(iw_client->ap_list))
		{
			iw_client_ap_cleanup(iw_client, TRUE);
			//lstFree(iw_client->ap_list);
		}
		if(iw_client->ap_unlist && lstCount(iw_client->ap_unlist))
		{
			iw_client_ap_cleanup(iw_client, FALSE);
			//lstFree(iw_client->ap_unlist);
		}
		if(iw_client->mutex)
			os_mutex_unlock(iw_client->mutex);
	}
	return OK;
}
/*
                    IE: WPA Version 1
                        Group Cipher : CCMP
                        Pairwise Ciphers (1) : CCMP
                        Authentication Suites (1) : PSK
                    IE: IEEE 802.11i/WPA2 Version 1
                        Group Cipher : CCMP
                        Pairwise Ciphers (1) : CCMP
                        Authentication Suites (1) : PSK

			WPA
					IE: WPA Version 1
                        Group Cipher : CCMP
                        Pairwise Ciphers (1) : CCMP
                        Authentication Suites (1) : PSK

            WPA2
                    IE: IEEE 802.11i/WPA2 Version 1
                        Group Cipher : CCMP
                        Pairwise Ciphers (1) : CCMP
                        Authentication Suites (1) : PSK

			WPA2/WPA
                    IE: IEEE 802.11i/WPA2 Version 1
                        Group Cipher : CCMP
                        Pairwise Ciphers (1) : CCMP
                        Authentication Suites (1) : PSK
                    IE: WPA Version 1
                        Group Cipher : CCMP
                        Pairwise Ciphers (1) : CCMP
                        Authentication Suites (1) : PSK

		OPEN/SHARED
                    Frequency:2.432 GHz (Channel 5)
                    Quality=70/70  Signal level=-36 dBm
                    Encryption key:on
                    ESSID:"openwrt"
                    Bit Rates:1 Mb/s; 2 Mb/s; 5.5 Mb/s; 11 Mb/s; 6 Mb/s
                              9 Mb/s; 12 Mb/s; 18 Mb/s
                    Bit Rates:24 Mb/s; 36 Mb/s; 48 Mb/s; 54 Mb/s
                    Mode:Master
                    Extra:tsf=00000000997df384
                    Extra: Last beacon: 536ms ago
                    IE: Unknown: 00076F70656E777274
                    IE: Unknown: 010882848B960C121824
                    IE: Unknown: 030105
                    IE: Unknown: 0706303020010B14
                    IE: Unknown: 2A0100
                    IE: Unknown: 32043048606C
                    IE: Unknown: 0B050000000000
                    IE: Unknown: 7F080000000000000040
                    IE: Unknown: DD180050F2020101000003A4000027A4000042435E0062322F00

           NO
           AP: 08 - Address: 66:E8:C1:E2:4F:47
                    Channel:5
                    Frequency:2.432 GHz (Channel 5)
                    Quality=70/70  Signal level=-28 dBm
                    Encryption key:off
                    ESSID:"openwrt"
                    Bit Rates:1 Mb/s; 2 Mb/s; 5.5 Mb/s; 11 Mb/s; 6 Mb/s
                              9 Mb/s; 12 Mb/s; 18 Mb/s
                    Bit Rates:24 Mb/s; 36 Mb/s; 48 Mb/s; 54 Mb/s
                    Mode:Master
                    Extra:tsf=000000009ff854ad
                    Extra: Last beacon: 513ms ago
                    IE: Unknown: 00076F70656E777274
                    IE: Unknown: 010882848B960C121824
                    IE: Unknown: 030105
                    IE: Unknown: 0706303020010B14
                    IE: Unknown: 2A0100
                    IE: Unknown: 32043048606C
                    IE: Unknown: 0B050000000000
                    IE: Unknown: 2D1AEC0117FFFF000000000000000000000100000000000000000000
                    IE: Unknown: 3D1605000000000000000000000000000000000000000000
                    IE: Unknown: 7F080000000000000040
                    IE: Unknown: DD180050F2020101000003A4000027A4000042435E0062322F00
 */
