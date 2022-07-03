/*
 * nsm_vlan_database.c
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "vty.h"
#include "zmemory.h"
#include "log.h"
#include "template.h"
#include "hash.h"
#include "nsm_include.h"
#include "hal_include.h"

#ifdef ZPL_HAL_MODULE
#include "hal_vlan.h"
#endif


static Gl2vlan_Database_t gvlan;


static nsm_vlan_database_t * nsm_vlan_database_lookup_node(vlan_t value);
static int nsm_vlan_database_add_node(nsm_vlan_database_t *value);


static const zpl_uchar bitMask[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
};

void zpl_vlan_bitmap_init(zpl_vlan_bitmap_t bitmap)
{
	memset(bitmap.bitmap, 0, sizeof(zpl_vlan_bitmap_t));
}
/*****************************************************************************
*
* zpl_vlan_bitmap_Set - Set a bit in the given byte array.
*/
void  zpl_vlan_bitmap_set(zpl_vlan_bitmap_t bitmap,zpl_uint32  bit)
{
	bitmap.bitmap[(bit)/8] |= bitMask[(bit)%8];
}

/*****************************************************************************
*
* zpl_vlan_bitmap_clr - Remove a bit in the given byte array.
*/
void  zpl_vlan_bitmap_clr(zpl_vlan_bitmap_t bitmap,zpl_uint32  bit)
{
	bitmap.bitmap[(bit)/8] &= ~bitMask[(bit)%8];
}

/*****************************************************************************
*
* zpl_vlan_bitmap_tst - Return True if the bit is set, else False.
*/
int zpl_vlan_bitmap_tst(zpl_vlan_bitmap_t bitmap,zpl_uint32  bit)
{
	int bittst = 0;
	bittst = (bitmap.bitmap[(bit)/8] & bitMask[(bit)%8])? 1 : 0;
	if(bittst == 0)
		return 0;	
    return bittst;
}
void zpl_vlan_bitmap_or(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2)
{
	int k = 0;
	int nr = sizeof(src1.bitmap);
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] | src2.bitmap[k];
}

void zpl_vlan_bitmap_xor(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2)
{
	int k = 0;
	int nr = sizeof(src1.bitmap);
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] ^ src2.bitmap[k];
}

void zpl_vlan_bitmap_and(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2)
{
	int k = 0;
	int nr = sizeof(src1.bitmap);
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] & src2.bitmap[k];
}

int zpl_vlan_bitmap_cmp(const zpl_vlan_bitmap_t src1, const zpl_vlan_bitmap_t src2)
{
	return memcmp(&src1.bitmap, &src2.bitmap, sizeof(src1.bitmap));
}

void  zpl_vlan_bitmap_copy(const zpl_vlan_bitmap_t src, zpl_vlan_bitmap_t dst)
{
	memcpy(&dst.bitmap, &src.bitmap, sizeof(dst.bitmap));
	return;
}
/*****************************************************************************/

int nsm_vlan_database_list_split_api(const char *str, vlan_t *vlanlist)
{
	zpl_char tmp[32];
	int n = 0, i = 0, ret = 0, j = 0,vid = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//zpl_char *nm = NULL;
	n = strspn(str, "0123456789,-");
	if(n != strlen(str))
	{
		fprintf(stderr,"ERROR:input:%s  n = %d\r\n", str, n);
		return ERROR;
	}
	//2,4,6,7,8-12,14-33,36,78
	//2,4,6,7,8-12,14,15,56,36-78
	//8-12,14,15,56,36-78
	memset(tmp, 0, sizeof(tmp));
	n = 0;
	while(n < strlen(str))
	{
		if(str[n] == ',')
		{
			if(ibase)
			{
				iend = atoi(tmp);
				for(vid = ibase; vid <= iend; vid++)
					vlanlist[j++] = vid;
				ibase = 0;
				iend = 0;
			}
			else
			{
				value = atoi(tmp);
				vlanlist[j++] = value;	
				value = 0;
			}
			memset(tmp, 0, sizeof(tmp));
			i = 0;
		}
		else if(str[n] == '-')
		{
			ibase = atoi(tmp);
			memset(tmp, 0, sizeof(tmp));
			i = 0;
		}
		else
			tmp[i++] = str[n];
		n++;
	}
	if(i)
	{
		if(ibase)
		{
			iend = atoi(tmp);
			for(vid = ibase; vid <= iend; vid++)
				vlanlist[j++] = vid;
		}
		else
		{
			value = atoi(tmp);
			vlanlist[j++] = value;
		}
	}
	return j;
}



int nsm_vlan_database_list_lookup_api(vlan_t *vlanlist, zpl_uint32 num)
{
	zpl_uint32 n = 0;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	for(n = 0; n < num; n++)
	{
		if(vlanlist[n])
		{
			if(!nsm_vlan_database_lookup_node(vlanlist[n]))
			{
				if(gvlan.mutex)
					os_mutex_unlock(gvlan.mutex);
				return 0;
			}
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return 1;	
}

int nsm_vlan_database_default(void)
{
	nsm_vlan_database_t value;
	memset(&value, 0, sizeof(value));
	value.vlan = 1;
	if(nsm_vlan_database_enable(zpl_true) == OK)
		nsm_vlan_database_add_node(&value);
	return OK;
}

int nsm_vlan_database_init(void)
{
	gvlan.vlanList = malloc(sizeof(LIST));
	gvlan.mutex = os_mutex_init();
	lstInit(gvlan.vlanList);
	return OK;
}



int nsm_vlan_qinq_enable(zpl_bool enable)
{
	int ret = 0;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	ret = hal_qinq_enable(enable);
#endif
	if(ret == OK)
		gvlan.qinq_enable = enable;
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

zpl_bool nsm_vlan_qinq_is_enable(void)
{
	return gvlan.qinq_enable;
}

int nsm_vlan_dot1q_tpid_set_api(vlan_t num)
{
	int ret = 0;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	ret = hal_qinq_vlan_tpid(num);
#endif
	if(ret == OK)
		gvlan.qinq_tpid = num;
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}
vlan_t nsm_vlan_dot1q_tpid_get_api(void)
{
	return gvlan.qinq_tpid;
}

static int nsm_vlan_database_cleanup(int all)
{
	nsm_vlan_database_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_vlan_database_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (nsm_vlan_database_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(all)
			{
				lstDelete(gvlan.vlanList, (NODE*)pstNode);
				if(pstNode->vlan_name)
					XFREE(MTYPE_VLAN, pstNode->vlan_name);
				XFREE(MTYPE_VLAN, pstNode);
			}
			else if(pstNode->vlan != 1)
			{
				lstDelete(gvlan.vlanList, (NODE*)pstNode);
				if(pstNode->vlan_name)
					XFREE(MTYPE_VLAN, pstNode->vlan_name);
				XFREE(MTYPE_VLAN, pstNode);
			}
		}
	}
	return OK;
}

int nsm_vlan_database_exit(void)
{
	if(lstCount(gvlan.vlanList))
	{
		nsm_vlan_database_cleanup(1);
		lstFree(gvlan.vlanList);
		free(gvlan.vlanList);
		gvlan.vlanList = NULL;
	}
	if(gvlan.mutex)
		os_mutex_exit(gvlan.mutex);
	return OK;
}

int nsm_vlan_database_cleanall(void)
{
	return nsm_vlan_database_cleanup(0);
}

int nsm_vlan_database_enable(zpl_bool enable)
{
	int ret = 0;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	ret = hal_vlan_enable(enable);
#endif
	if(ret == OK)
		gvlan.enable = enable;
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

zpl_bool nsm_vlan_database_is_enable(void)
{
	return gvlan.enable;
}

int nsm_vlan_portbase_enable(zpl_bool enable)
{
	int ret = 0;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	if(ret == OK)
		gvlan.port_base_enable = enable;
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

zpl_bool nsm_vlan_portbase_is_enable(void)
{
	return gvlan.port_base_enable;
}


static int nsm_vlan_database_add_sort_node(nsm_vlan_database_t *value)
{
	zpl_uint32 i = 1;
	nsm_vlan_database_t *node = nsm_vlan_database_lookup_node(value->vlan - 1);
	while(!node)
		node = nsm_vlan_database_lookup_node(value->vlan - (i++));
	if(node)
	{
		lstInsert (gvlan.vlanList, (NODE*)node, (NODE*)value);
	}
	else
		lstAdd(gvlan.vlanList, (NODE *)value);
	return OK;
}

static int nsm_vlan_database_add_node(nsm_vlan_database_t *value)
{
	nsm_vlan_database_t *node = XMALLOC(MTYPE_VLAN, sizeof(nsm_vlan_database_t));
	if(node)
	{
		memset(node, 0, sizeof(nsm_vlan_database_t));
		memcpy(node, value, sizeof(nsm_vlan_database_t));
		if(value->vlan_name)
		{
			node->vlan_name = XSTRDUP(MTYPE_VLAN, value->vlan_name);
			node->name_hash = string_hash_make(node->vlan_name);
			XFREE(MTYPE_VLAN, value->vlan_name);
		}
		if(lstCount(gvlan.vlanList) == 0)
			lstAdd(gvlan.vlanList, (NODE *)node);
		else
			nsm_vlan_database_add_sort_node(node);
		return OK;
	}
	return ERROR;
}

static nsm_vlan_database_t * nsm_vlan_database_lookup_node(vlan_t value)
{
	nsm_vlan_database_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_vlan_database_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (nsm_vlan_database_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->vlan == value)
		{
			return pstNode;
		}
	}
	return NULL;
}

static nsm_vlan_database_t * nsm_vlan_database_lookup_node_by_name(const char *name)
{
	nsm_vlan_database_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_vlan_database_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (nsm_vlan_database_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->name_hash == string_hash_make(name))
		{
			return pstNode;
		}
	}
	return NULL;
}

static int _nsm_vlan_database_lookup_port(nsm_vlan_database_t *value, ifindex_t ifindex, nsm_vlan_mode_t mode)
{
	zpl_uint32 i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == NSM_VLAN_UNTAG)
		{
			if(value->untagport[i] == ifindex)
			{
				return 1;
			}
		}
		if(mode == NSM_VLAN_TAG)
		{
			if(value->tagport[i] == ifindex)
			{
				return 1;
			}
		}
	}
	return 0;
}


static int _nsm_vlan_database_del_port(nsm_vlan_database_t *value, ifindex_t ifindex, nsm_vlan_mode_t mode)
{
	zpl_uint32 i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == NSM_VLAN_UNTAG)
		{
			if(value->untagport[i] == ifindex)
			{
				value->untagport[i] = 0;
				return 1;
			}
		}
		else if(mode == NSM_VLAN_TAG)
		{
			if(value->tagport[i] == ifindex)
			{
				value->tagport[i] = 0;
				return 1;
			}
		}
	}
	return 0;
}

int nsm_vlan_database_clear_port(ifindex_t ifindex, nsm_vlan_mode_t mode)
{
	nsm_vlan_database_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_vlan_database_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (nsm_vlan_database_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(_nsm_vlan_database_lookup_port(pstNode,  ifindex,  mode))
			{
				_nsm_vlan_database_del_port(pstNode,  ifindex,  mode);
			}
		}
	}
	return 0;
}


static int _nsm_vlan_database_add_port(nsm_vlan_database_t *value, ifindex_t ifindex, nsm_vlan_mode_t mode)
{
	zpl_uint32 i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == NSM_VLAN_UNTAG)
		{
			if(value->untagport[i] == 0)
			{
				value->untagport[i] = ifindex;
				return 1;
			}
		}
		else if(mode == NSM_VLAN_TAG)
		{
			if(value->tagport[i] == 0)
			{
				value->tagport[i] = ifindex;
				return 1;
			}
		}
	}
	return 0;
}

int nsm_vlan_database_add_port(vlan_t vlan, ifindex_t ifindex, nsm_vlan_mode_t mode)
{
	nsm_vlan_database_t *value = NULL;
	value = nsm_vlan_database_lookup_node(vlan);
	if(value)
	{
		_nsm_vlan_database_add_port(value,  ifindex,  mode);
	}
	return 0;
}

int nsm_vlan_database_del_port(vlan_t vlan, ifindex_t ifindex, nsm_vlan_mode_t mode)
{
	nsm_vlan_database_t *value = NULL;
	value = nsm_vlan_database_lookup_node(vlan);
	if(value)
	{
		_nsm_vlan_database_del_port(value,  ifindex,  mode);
	}
	return 0;
}


static int nsm_vlan_database_lookup_on_port_check(struct interface *ifp, void *pVoid)
{
	vlan_t *vlan = (vlan_t*)pVoid;
	if(if_is_ethernet(ifp) && nsm_interface_trunk_add_allowed_vlan_lookup_api(ifp,  *vlan))
	{
		nsm_interface_trunk_add_allowed_vlan_api(ifp, *vlan);
		return OK;
	}
	return OK;	
}


static int nsm_vlan_database_update_batch_list(nsm_vlan_database_t *value, void *pVoid)
{
	nsm_vlan_database_t *vlanb = (nsm_vlan_database_t*)pVoid;
	if(value->vlan && value->minvlan && value->maxvlan)
	{
		if(value->vlan < vlanb->vlan)
		{
			value->maxvlan = vlanb->maxvlan;
		}
		if(value->vlan < vlanb->vlan)
		{
			value->minvlan = vlanb->minvlan;
		}
	}
	return OK;	
}

int nsm_vlan_database_create_api(vlan_t vlan, const char *name)
{
	int ret = ERROR;
	nsm_vlan_database_t value;
	vlan_t tmpvlan = vlan;
	memset(&value, 0, sizeof(value));
	value.vlan = vlan;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	#ifdef ZPL_HAL_MODULE
		ret = hal_vlan_create(vlan);
	#else
		ret = OK;
	#endif
		if(ret == OK)
		{
			if(name)
				value.vlan_name = XSTRDUP(MTYPE_VLAN, name);
			ret = nsm_vlan_database_add_node(&value);
		}

	else
		ret = OK;
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);

	if(ret == OK)
	{
		ret = if_list_each(nsm_vlan_database_lookup_on_port_check, &tmpvlan);
	}
	return ret;
}


int nsm_vlan_database_destroy_api(vlan_t vlan)
{
	int ret = ERROR;
	nsm_vlan_database_t *value = NULL;
	nsm_vlan_database_t tmpvlan;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	value = nsm_vlan_database_lookup_node(vlan);
	if(value)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_vlan_destroy(vlan);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			tmpvlan.vlan = value->vlan;
			tmpvlan.maxvlan = value->maxvlan;
			tmpvlan.minvlan = value->minvlan;
			lstDelete(gvlan.vlanList, (NODE*)value);
			if(value->vlan_name)
				XFREE(MTYPE_VLAN, value->vlan_name);
			XFREE(MTYPE_VLAN, value);
			ret = OK;
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	if(ret == OK && tmpvlan.vlan)
	{
		ret = nsm_vlan_database_callback_api(nsm_vlan_database_update_batch_list, &tmpvlan);
	}	
	return ret;
}

int nsm_vlan_database_batch_create_api(vlan_t minvlan, vlan_t maxvlan)
{
	zpl_uint32 i = 0, j = 0;
	zpl_vlan_bitmap_t	vlanbitmap;
	int ret = ERROR;
	nsm_vlan_database_t *value = NULL;
	nsm_vlan_database_t tmpvlan;
	zpl_vlan_bitmap_init(vlanbitmap);
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	for(i = minvlan; i <= maxvlan; i++)
	{
		value = nsm_vlan_database_lookup_node(i);
		if(value)
		{
			if(gvlan.mutex)
				os_mutex_unlock(gvlan.mutex);
			return ret;
		}
	}
	for(i = minvlan; i <= maxvlan; i++)
	{
		zpl_vlan_bitmap_set(vlanbitmap, i);
	}
#ifdef ZPL_HAL_MODULE
	ret = hal_vlan_batch_create(vlanbitmap, minvlan, maxvlan);
#else
	ret = OK;
#endif
	if(ret == OK)
	{
		for(i = minvlan; i <= maxvlan; i++)
		{
			memset(&tmpvlan, 0, sizeof(nsm_vlan_database_t));
			tmpvlan.vlan = i;
			tmpvlan.minvlan = minvlan;
			tmpvlan.maxvlan = maxvlan;
			nsm_vlan_database_add_node(&tmpvlan);
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

int nsm_vlan_database_batch_destroy_api(vlan_t minvlan, vlan_t maxvlan)
{
	zpl_uint32 i = 0, j = 0;
	int ret = ERROR;
	zpl_vlan_bitmap_t	vlanbitmap;
	nsm_vlan_database_t *value;
	nsm_vlan_database_t tmpvlan;
	zpl_vlan_bitmap_init(vlanbitmap);
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	value = nsm_vlan_database_lookup_node(minvlan);
	if(value && value->minvlan <= minvlan && value->maxvlan >= maxvlan)
	{
		for(i = minvlan; i <= maxvlan; i++)
		{
			zpl_vlan_bitmap_set(vlanbitmap, i);
		}
#ifdef ZPL_HAL_MODULE
		ret = hal_vlan_batch_destroy(vlanbitmap, minvlan, maxvlan);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			for(i = minvlan; i <= maxvlan; i++)
			{
				value = nsm_vlan_database_lookup_node(i);
				if(value)
				{
					lstDelete(gvlan.vlanList, (NODE*)value);
					if(value->vlan_name)
						XFREE(MTYPE_VLAN, value->vlan_name);
					XFREE(MTYPE_VLAN, value);
				}
			}
		}
		tmpvlan.vlan = value->vlan;
		tmpvlan.maxvlan = maxvlan;
		tmpvlan.minvlan = value->minvlan;	
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	if(ret == OK && tmpvlan.vlan)
	{
		ret = nsm_vlan_database_callback_api(nsm_vlan_database_update_batch_list, &tmpvlan);
	}	
	return ret;
}


int nsm_vlan_database_list_create_api(const char *str)
{
	zpl_char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//zpl_char *nm = NULL;
	n = strspn(str, "0123456789,-");
	if(n != strlen(str))
	{
		fprintf(stderr,"ERROR:input:%s  n = %d\r\n", str, n);
		return ERROR;
	}
	//2,4,6,7,8-12,14-33,36,78
	//2,4,6,7,8-12,14,15,56,36-78
	//8-12,14,15,56,36-78
	memset(tmp, 0, sizeof(tmp));
	n = 0;
	while(n < strlen(str))
	{
		if(str[n] == ',')
		{
			if(ibase)
			{
				iend = atoi(tmp);
				ret |= nsm_vlan_database_batch_create_api(ibase, iend);
				ibase = 0;
				iend = 0;
			}
			else
			{
				value = atoi(tmp);
				if(!nsm_vlan_database_lookup_api(value))
					ret |= nsm_vlan_database_create_api(value, NULL);	
				value = 0;
			}
			memset(tmp, 0, sizeof(tmp));
			i = 0;
		}
		else if(str[n] == '-')
		{
			ibase = atoi(tmp);
			memset(tmp, 0, sizeof(tmp));
			i = 0;
		}
		else
			tmp[i++] = str[n];
		n++;
	}
	if(i)
	{
		if(ibase)
		{
			iend = atoi(tmp);
			ret |= nsm_vlan_database_batch_create_api(ibase, iend);
		}
		else
		{
			value = atoi(tmp);
			if(!nsm_vlan_database_lookup_api(value))
				ret |= nsm_vlan_database_create_api(value, NULL);
		}
	}
	return ret;
}


int nsm_vlan_database_list_destroy_api(const char *str)
{
	zpl_char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//zpl_char *nm = NULL;
	n = strspn(str, "0123456789,-");
	if(n != strlen(str))
	{
		fprintf(stderr,"ERROR:input:%s  n = %d\r\n", str, n);
		return ERROR;
	}
	//2,4,6,7,8-12,14-33,36,78
	//2,4,6,7,8-12,14,15,56,36-78
	//8-12,14,15,56,36-78
	memset(tmp, 0, sizeof(tmp));
	n = 0;
	while(n < strlen(str))
	{
		if(str[n] == ',')
		{
			if(ibase)
			{
				iend = atoi(tmp);
				ret |= nsm_vlan_database_batch_destroy_api(ibase, iend);
				ibase = 0;
				iend = 0;
			}
			else
			{
				value = atoi(tmp);
				if(nsm_vlan_database_lookup_node(value))
					ret |= nsm_vlan_database_destroy_api(value);
				value = 0;
			}
			memset(tmp, 0, sizeof(tmp));
			i = 0;
		}
		else if(str[n] == '-')
		{
			ibase = atoi(tmp);
			memset(tmp, 0, sizeof(tmp));
			i = 0;
		}
		else
			tmp[i++] = str[n];
		n++;
	}
	if(i)
	{
		if(ibase)
		{
			iend = atoi(tmp);
			ret |= nsm_vlan_database_batch_destroy_api(ibase, iend);
		}
		else
		{
			value = atoi(tmp);
			if(nsm_vlan_database_lookup_node(value))
				ret |= nsm_vlan_database_destroy_api(value);
		}
	}
	return ret;
}

int nsm_vlan_database_name_api(vlan_t vlan, const char *name)
{
	int ret = ERROR;
	nsm_vlan_database_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	value = nsm_vlan_database_lookup_node(vlan);
	if(value)
	{
		if(value->vlan_name)
			XFREE(MTYPE_VLAN, value->vlan_name);
		if(name)
			value->vlan_name = XSTRDUP(MTYPE_VLAN, name);
		ret = OK;
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}


void * nsm_vlan_database_lookup_api(vlan_t vlan)
{
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	nsm_vlan_database_t *value = nsm_vlan_database_lookup_node(vlan);
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return value;
}

void * nsm_vlan_database_lookup_by_name_api(const char *name)
{
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	nsm_vlan_database_t *value = nsm_vlan_database_lookup_node_by_name(name);
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return value;
}

int nsm_vlan_database_callback_api(nsm_vlan_database_cb cb, void *pVoid)
{
	nsm_vlan_database_t *pstNode = NULL;
	NODE index;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	for(pstNode = (nsm_vlan_database_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (nsm_vlan_database_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(cb)
			{
				(cb)(pstNode, pVoid);
			}
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return OK;
}


