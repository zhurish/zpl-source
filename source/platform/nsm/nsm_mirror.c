/*
 * nsm_mirror.c
 *
 *  Created on: May 11, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"

#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "algorithm.h"
#include "nsm_include.h"
#include "hal_include.h"

static Gmirror_t gMirror;


int nsm_mirror_init(void)
{
	template_t * temp = NULL;

	os_memset(&gMirror, 0, sizeof(Gmirror_t));
	gMirror.mutex = os_mutex_name_init("mirror-mutex");

	temp = lib_template_new (zpl_true);
	if(temp)
	{
		temp->module = 0;
		strcpy(temp->name, "mirror");
		//strcpy(temp->prompt, "service-mqtt"); /* (config-app-esp)# */
		temp->write_template = bulid_mirror_config;
		temp->pVoid = NULL;
		lib_template_config_list_install(temp, 0);
	}

	return OK;
}

int nsm_mirror_exit(void)
{
	if (gMirror.mutex)
		os_mutex_exit(gMirror.mutex);
	return OK;
}

int nsm_mirror_destination_set_api(zpl_uint32 id, ifindex_t ifindex, zpl_bool enable)
{
	int ret = 0, i = 0;
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);

#ifdef ZPL_HAL_MODULE
	ret = hal_mirror_enable(ifindex, enable);
#endif
	if (ret == OK)
	{
		gMirror.mirror_session[id-1].ifindex = enable ? ifindex : 0;
		gMirror.mirror_session[id-1].enable = enable;
		if (enable == zpl_false)
		{
			for (i = 0; i < MIRROR_SOURCE_MAX; i++)
			{
				if (gMirror.mirror_session[id-1].srcport[i].enable == zpl_true &&
					gMirror.mirror_session[id-1].srcport[i].ifindex)
				{
					gMirror.mirror_session[id-1].enable = zpl_true;
					if (gMirror.mutex)
						os_mutex_unlock(gMirror.mutex);
					return ret;
				}
			}
		}
		if (enable == zpl_false)
		{
			for (i = 0; i < MIRROR_SOURCE_MAX; i++)
			{
				if (gMirror.mirror_session[id-1].ingress_filter[i].enable == zpl_true &&
					gMirror.mirror_session[id-1].ingress_filter[i].ifindex)
				{
					gMirror.mirror_session[id-1].enable = zpl_true;
					if (gMirror.mutex)
						os_mutex_unlock(gMirror.mutex);
					return ret;
				}
				if (gMirror.mirror_session[id-1].egress_filter[i].enable == zpl_true &&
					gMirror.mirror_session[id-1].egress_filter[i].ifindex)
				{
					gMirror.mirror_session[id-1].enable = zpl_true;
					if (gMirror.mutex)
						os_mutex_unlock(gMirror.mutex);
					return ret;
				}
			}
		}
	}
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_destination_get_api(zpl_uint32 id, ifindex_t *ifindex, zpl_bool *enable)
{
	int ret = 0;
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	if (enable)
		*enable = gMirror.mirror_session[id-1].enable;
	if (ifindex)
		*ifindex = gMirror.mirror_session[id-1].ifindex;
	ret = OK;
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

zpl_bool nsm_mirror_is_destination_api(ifindex_t ifindex, zpl_uint32 *id)
{
	int i = 0;
	zpl_bool ret = zpl_false;
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	for (i = 0; i < MIRROR_SESSION_MAX; i++)
	{
		if (gMirror.mirror_session[i].ifindex == ifindex)
		{
			if (id)
				*id = i+1;
			if (gMirror.mutex)
				os_mutex_unlock(gMirror.mutex);
			return zpl_true;
		}
	}
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_source_set_api(zpl_uint32 id, ifindex_t ifindex, zpl_bool enable, mirror_dir_en dir)
{
	int ret = 0, i = 0;
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);

#ifdef ZPL_HAL_MODULE
	if (enable)
		ret = hal_mirror_source_enable(ifindex, enable, dir);
#endif
	if (ret == OK)
	{
		gMirror.mirror_session[id-1].enable = enable;
		gMirror.mirror_session[id-1].mode = enable ? MIRROR_SOURCE_PORT : 0;
		for (i = 0; i < MIRROR_SOURCE_MAX; i++)
		{
			if (enable)
			{
				if (gMirror.mirror_session[id-1].srcport[i].enable == zpl_false)
				{
					gMirror.mirror_session[id-1].srcport[i].ifindex = ifindex;
					gMirror.mirror_session[id-1].srcport[i].dir = dir;
					gMirror.mirror_session[id-1].srcport[i].enable = zpl_true;
					break;
				}
			}
			else
			{
				if (gMirror.mirror_session[id-1].srcport[i].enable == zpl_true &&
					gMirror.mirror_session[id-1].srcport[i].ifindex == ifindex)
				{
#ifdef ZPL_HAL_MODULE
					ret = hal_mirror_source_enable(ifindex, enable, dir);
#endif
					if (ret == OK)
					{
						gMirror.mirror_session[id-1].srcport[i].ifindex = 0;
						gMirror.mirror_session[id-1].srcport[i].dir = 0;
						gMirror.mirror_session[id-1].srcport[i].enable = zpl_false;
						break;
					}
					break;
				}
			}
		}
	}
	if (enable == zpl_false)
	{
		for (i = 0; i < MIRROR_SOURCE_MAX; i++)
		{
			if (gMirror.mirror_session[id-1].srcport[i].enable == zpl_true &&
				gMirror.mirror_session[id-1].srcport[i].ifindex)
			{
				gMirror.mirror_session[id-1].enable = zpl_true;
				gMirror.mirror_session[id-1].mode = MIRROR_SOURCE_PORT;
				if (gMirror.mutex)
					os_mutex_unlock(gMirror.mutex);
				return ret;
			}
		}
	}
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_source_get_api(zpl_uint32 id, ifindex_t ifindex, zpl_bool *enable, mirror_mode_t *mode, mirror_dir_en *dir)
{
	int ret = 0, i = 0;
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);

	for (i = 0; i < MIRROR_SOURCE_MAX; i++)
	{
		if (gMirror.mirror_session[id-1].srcport[i].ifindex == ifindex)
		{
			if (mode)
				*mode = gMirror.mirror_session[id-1].mode;
			if (dir)
				*dir = gMirror.mirror_session[id-1].srcport[i].dir;
			if (enable)
				*enable = gMirror.mirror_session[id-1].srcport[i].enable;
			break;
		}
	}
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

zpl_bool nsm_mirror_is_source_api(ifindex_t ifindex, zpl_uint32 *id, mirror_mode_t *mode, 
	mirror_dir_en *dir, mirror_filter_t *filter, zpl_uint32 *index)
{
	int i = 0, j = 0;
	zpl_bool ret = zpl_false;
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	for (i = 0; i < MIRROR_SESSION_MAX; i++)
	{
		for (j = 0; j < MIRROR_SOURCE_MAX; j++)
		{
			if (gMirror.mirror_session[i].srcport[i].enable && 
				gMirror.mirror_session[i].srcport[i].ifindex == ifindex)
			{
				if(id)
					*id = i+1;
				if(mode)
					*mode = gMirror.mirror_session[i].mode;
				if(dir)
					*dir = gMirror.mirror_session[i].srcport[j].dir;
				if(filter)
					*filter = 0;
				if(index)
					*index = j;
				ret = zpl_true;	
				if (gMirror.mutex)
					os_mutex_unlock(gMirror.mutex);
				return ret;
			}
			if (gMirror.mirror_session[i].ingress_filter[i].enable && 
				gMirror.mirror_session[i].ingress_filter[i].ifindex == ifindex)
			{
				if(id)
					*id = i+1;
				if(mode)
					*mode = gMirror.mirror_session[i].mode;
				if(dir)
					*dir = gMirror.mirror_session[i].ingress_filter[j].dir;
				if(filter)
					*filter = gMirror.mirror_session[i].ingress_filter[j].filter;
				if(index)
					*index = j;
				ret = zpl_true;	
				if (gMirror.mutex)
					os_mutex_unlock(gMirror.mutex);
				return ret;
			}
			if (gMirror.mirror_session[i].egress_filter[i].enable && 
				gMirror.mirror_session[i].egress_filter[i].ifindex == ifindex)
			{
				if(id)
					*id = i;
				if(mode)
					*mode = gMirror.mirror_session[i].mode;
				if(dir)
					*dir = gMirror.mirror_session[i].egress_filter[j].dir;
				if(filter)
					*filter = gMirror.mirror_session[i].egress_filter[j].filter;
				if(index)
					*index = j;
				ret = zpl_true;	
				if (gMirror.mutex)
					os_mutex_unlock(gMirror.mutex);
				return ret;
			}
		}
	}
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}

int nsm_mirror_source_mac_filter_set_api(zpl_uint32 id, ifindex_t ifindex, zpl_bool enable,
	mirror_dir_en dir, mirror_filter_t filter, zpl_uchar *mac)
{
	int ret = 0, i = 0;
	nsm_mirror_source_t	*mirror_filter = NULL;
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);

	if (dir != MIRROR_INGRESS && dir != MIRROR_EGRESS)
	{
		ret = ERROR;
		if (gMirror.mutex)
			os_mutex_unlock(gMirror.mutex);
		return ret;
	}
#ifdef ZPL_HAL_MODULE
	if (enable)
		ret = hal_mirror_source_filter_enable(ifindex, enable, dir, filter, mac);
#endif
	if (ret == OK)
	{
		if(dir == MIRROR_INGRESS)
			mirror_filter = gMirror.mirror_session[id-1].ingress_filter;
		else if(dir == MIRROR_EGRESS)
			mirror_filter = gMirror.mirror_session[id-1].egress_filter;
		gMirror.mirror_session[id-1].enable = zpl_true;
		gMirror.mirror_session[id-1].mode = enable ? MIRROR_SOURCE_MAC : 0;
		for (i = 0; i < MIRROR_SOURCE_MAX; i++)
		{
			if (enable)
			{
				if (mirror_filter[i].enable == zpl_false)
				{
					mirror_filter[i].ifindex = ifindex;
					mirror_filter[i].dir = dir;
					mirror_filter[i].enable = zpl_true;
					mirror_filter[i].filter = filter;
					os_memcpy(mirror_filter[i].mac, mac, NSM_MAC_MAX);
					break;
				}
			}
			else
			{
				if (mirror_filter[i].enable == zpl_true &&
					mirror_filter[i].ifindex == ifindex)
				{
#ifdef ZPL_HAL_MODULE
					ret = hal_mirror_source_filter_enable(ifindex, enable, dir, filter, mac);
#endif
					if (ret == OK)
					{
						mirror_filter[i].ifindex = 0;
						mirror_filter[i].dir = 0;
						mirror_filter[i].enable = zpl_false;
						mirror_filter[i].filter = 0;
						os_memset(mirror_filter[i].mac, 0, NSM_MAC_MAX);
					}
				}
				break;
			}
		}
	}
	if (enable == zpl_false)
	{
		if(dir == MIRROR_INGRESS)
			mirror_filter = gMirror.mirror_session[id-1].ingress_filter;
		else if(dir == MIRROR_EGRESS)
			mirror_filter = gMirror.mirror_session[id-1].egress_filter;
			
		for (i = 0; i < MIRROR_SOURCE_MAX; i++)
		{
			if (mirror_filter[i].enable == zpl_true &&
				mirror_filter[i].ifindex)
			{
				gMirror.mirror_session[id-1].enable = zpl_true;
				gMirror.mirror_session[id-1].mode = MIRROR_SOURCE_MAC;
				if (gMirror.mutex)
					os_mutex_unlock(gMirror.mutex);
				return ret;
			}
		}
	}
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return ret;
}




static int bulid_mirror_dest(nsm_mirror_session_t *node, zpl_uint32 index, struct vty *vty)
{
	if(node && node->enable)
	{
		vty_out(vty, "monitor session %d destination interface %s%s",
					index, if_ifname_make(node->ifindex), VTY_NEWLINE);
	}
	return 0;
}
static int bulid_mirror_source_port(nsm_mirror_session_t *node, zpl_uint32 index, struct vty *vty)
{
	if(node && node->mode == MIRROR_SOURCE_PORT)
	{
		zpl_uint32 i = 0;
		const char *both[] = {"none", "ingress", "egress", "both"};
		for(i = 0; i < MIRROR_SOURCE_MAX; i++)
		{
			if(node->srcport[i].enable)
			{
				if(node->srcport[i].dir != MIRROR_BOTH)
					vty_out(vty, "monitor session %d source interface %s %s%s",
							index, if_ifname_make(node->srcport[i].ifindex), 
							both[node->srcport[i].dir], VTY_NEWLINE);
				else
					vty_out(vty, "monitor session %d source interface %s%s",
							index, if_ifname_make(node->srcport[i].ifindex), VTY_NEWLINE);
			}
		}
	}
	return 0;
}

static int bulid_mirror_source_mac(nsm_mirror_session_t *node, zpl_uint32 index, struct vty *vty)
{
	if(node && node->mode == MIRROR_SOURCE_MAC)
	{
		zpl_uint32 i = 0;
		const char *both[] = {"none", "ingress", "egress", "both"};
		const char *filter[] = {"none", "destination", "source", "both"};
		for(i = 0; i < MIRROR_SOURCE_MAX; i++)
		{
			if(node->ingress_filter[i].enable)
			{
				vty_out(vty, "monitor session %d source interface %s %s %s %s%s",
						index, if_ifname_make(node->ingress_filter[i].ifindex), 
						both[node->ingress_filter[i].dir], 
						filter[node->ingress_filter[i].filter], 
						if_mac_out_format(node->ingress_filter[i].mac), VTY_NEWLINE);
			}
		}
		for(i = 0; i < MIRROR_SOURCE_MAX; i++)
		{
			if(node->egress_filter[i].enable)
			{
				vty_out(vty, "monitor session %d source interface %s %s %s %s%s",
						index, if_ifname_make(node->egress_filter[i].ifindex), 
						both[node->egress_filter[i].dir], 
						filter[node->egress_filter[i].filter], 
						if_mac_out_format(node->egress_filter[i].mac), VTY_NEWLINE);
			}
		}
	}
	return 0;
}



int bulid_mirror_config(struct vty *vty, void *p)
{
	int i = 0;
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	for (i = 0; i < MIRROR_SESSION_MAX; i++)
	{	
		if(vty->type != VTY_FILE)
		{
		bulid_mirror_dest(&gMirror.mirror_session[i], i+1, vty);
		bulid_mirror_source_port(&gMirror.mirror_session[i], i+1, vty);
		bulid_mirror_source_mac(&gMirror.mirror_session[i], i+1, vty);
		}
	}
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return 1;
}

int bulid_mirror_show(struct vty *vty)
{
	int i = 0,j = 0;
	const char *both[] = {"none", "ingress", "egress", "both"};
	//const char *filter[] = {"none", "destination", "source", "both"};
	if (gMirror.mutex)
		os_mutex_lock(gMirror.mutex, OS_WAIT_FOREVER);
	for (i = 0; i < MIRROR_SESSION_MAX; i++)
	{	
		if(gMirror.mirror_session[i].enable)
		{
			vty_out(vty, "Monitor Session               : %d%s",i+1,VTY_NEWLINE);
			if(gMirror.mirror_session[i].ifindex)
				vty_out(vty, " destination                  : interface %s%s",if_ifname_make(gMirror.mirror_session[i].ifindex),VTY_NEWLINE);
			if(gMirror.mirror_session[i].mode == MIRROR_SOURCE_PORT)
			{
				for (j = 0; j < MIRROR_SOURCE_MAX; j++)
				{
					if(gMirror.mirror_session[i].srcport[j].enable)
						vty_out(vty, " source                       : interface %s %s %s",
							if_ifname_make(gMirror.mirror_session[i].srcport[j].ifindex),
							both[gMirror.mirror_session[i].srcport[j].dir],
							VTY_NEWLINE);
				}
			}
			if(gMirror.mirror_session[i].mode == MIRROR_SOURCE_MAX)
			{
				for (j = 0; j < MIRROR_SOURCE_MAX; j++)
				{
					if(gMirror.mirror_session[i].ingress_filter[j].enable)
						vty_out(vty, " source                       : interface %s %s %s",
							if_ifname_make(gMirror.mirror_session[i].ingress_filter[j].ifindex),
							both[gMirror.mirror_session[i].ingress_filter[j].dir],
							VTY_NEWLINE);
					if(gMirror.mirror_session[i].egress_filter[j].enable)
						vty_out(vty, " source                       : interface %s %s %s",
							if_ifname_make(gMirror.mirror_session[i].egress_filter[j].ifindex),
							both[gMirror.mirror_session[i].egress_filter[j].dir],
							VTY_NEWLINE);
				}
			}
		}
	}
	if (gMirror.mutex)
		os_mutex_unlock(gMirror.mutex);
	return 0;
}