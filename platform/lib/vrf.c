/*
 * VRF functions.
 * Copyright (C) 2014 6WIND S.A.
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "os_include.h"
#include "zpl_include.h"
#include "vrf.h"
#include "if.h"
#include "zmemory.h"
#include "nsm_rib.h"
#ifdef HAVE_NETNS
#undef _GNU_SOURCE
#define _GNU_SOURCE

#include <sched.h>
#endif

struct ip_vrf_master _ip_vrf_master;
/*
 * VRF bit-map
 */

#define VRF_BITMAP_NUM_OF_GROUPS 8
#define VRF_BITMAP_NUM_OF_BITS_IN_GROUP \
  (UINT16_MAX / VRF_BITMAP_NUM_OF_GROUPS)
#define VRF_BITMAP_NUM_OF_BYTES_IN_GROUP \
  (VRF_BITMAP_NUM_OF_BITS_IN_GROUP / CHAR_BIT + 1) /* +1 for ensure */

#define VRF_BITMAP_GROUP(_id) \
  ((_id) / VRF_BITMAP_NUM_OF_BITS_IN_GROUP)
#define VRF_BITMAP_BIT_OFFSET(_id) \
  ((_id) % VRF_BITMAP_NUM_OF_BITS_IN_GROUP)

#define VRF_BITMAP_INDEX_IN_GROUP(_bit_offset) \
  ((_bit_offset) / CHAR_BIT)
#define VRF_BITMAP_FLAG(_bit_offset) \
  (((zpl_uchar)1) << ((_bit_offset) % CHAR_BIT))

struct vrf_bitmap_table
{
  zpl_uchar *groups[VRF_BITMAP_NUM_OF_GROUPS];
};



vrf_bitmap_t ip_vrf_bitmap_init(void)
{
  return (vrf_bitmap_t)XCALLOC(MTYPE_VRF_BITMAP, sizeof(struct vrf_bitmap_table));
}

void ip_vrf_bitmap_free(vrf_bitmap_t bmap)
{
  struct vrf_bitmap_table *bm = (struct vrf_bitmap_table *)bmap;
  zpl_uint32 i;

  if (bmap == VRF_BITMAP_NULL)
    return;

  for (i = 0; i < VRF_BITMAP_NUM_OF_GROUPS; i++)
    if (bm->groups[i])
      XFREE(MTYPE_VRF_BITMAP, bm->groups[i]);

  XFREE(MTYPE_VRF_BITMAP, bm);
}

void ip_vrf_bitmap_set(vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap_table *bm = (struct vrf_bitmap_table *)bmap;
  zpl_uchar group = VRF_BITMAP_GROUP(vrf_id);
  zpl_uchar offset = VRF_BITMAP_BIT_OFFSET(vrf_id);

  if (bmap == VRF_BITMAP_NULL)
    return;

  if (bm->groups[group] == NULL)
    bm->groups[group] = XCALLOC(MTYPE_VRF_BITMAP,
                                VRF_BITMAP_NUM_OF_BYTES_IN_GROUP);

  SET_FLAG(bm->groups[group][VRF_BITMAP_INDEX_IN_GROUP(offset)],
           VRF_BITMAP_FLAG(offset));
}

void ip_vrf_bitmap_unset(vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap_table *bm = (struct vrf_bitmap_table *)bmap;
  zpl_uchar group = VRF_BITMAP_GROUP(vrf_id);
  zpl_uchar offset = VRF_BITMAP_BIT_OFFSET(vrf_id);

  if (bmap == VRF_BITMAP_NULL || bm->groups[group] == NULL)
    return;

  UNSET_FLAG(bm->groups[group][VRF_BITMAP_INDEX_IN_GROUP(offset)],
             VRF_BITMAP_FLAG(offset));
}

zpl_bool ip_vrf_bitmap_check(vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap_table *bm = (struct vrf_bitmap_table *)bmap;
  zpl_uchar group = VRF_BITMAP_GROUP(vrf_id);
  zpl_uchar offset = VRF_BITMAP_BIT_OFFSET(vrf_id);

  if (bmap == VRF_BITMAP_NULL || bm->groups[group] == NULL)
    return 0;

  return CHECK_FLAG(bm->groups[group][VRF_BITMAP_INDEX_IN_GROUP(offset)],
                    VRF_BITMAP_FLAG(offset))
             ? zpl_true
             : zpl_false;
}

/***********************************************************************/
void *ip_vrf_list (void)
{
  return _ip_vrf_master.ip_vrf_list;
}

/* Get a VRF. If not found, create one. */
static struct ip_vrf *ip_vrf_new_one(vrf_id_t vrf_id, const char *name)
{
  int ret = 0;
  struct ip_vrf *ip_vrf = NULL;
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);

  ip_vrf = XCALLOC(MTYPE_VRF, sizeof(struct ip_vrf));
  ip_vrf->vrf_id = vrf_id;
  if (name)
    ip_vrf->name = XSTRDUP(MTYPE_VRF_NAME, name);

  /* Initialize interfaces. */
  zlog_info(MODULE_NSM, "VRF %u is created.", vrf_id);

  if (_ip_vrf_master.vrf_new_hook)
    ret = (*_ip_vrf_master.vrf_new_hook)(vrf_id, ip_vrf);

  if (_ip_vrf_master.vrf_enable_hook)
    ret = (*_ip_vrf_master.vrf_enable_hook)(vrf_id, ip_vrf);
  if(ret == OK)
  {
    lstAdd(_ip_vrf_master.ip_vrf_list, (NODE *)ip_vrf);
  }
  else
  {
    if (ip_vrf->name)
      XFREE(MTYPE_VRF_NAME, ip_vrf->name);
    XFREE(MTYPE_VRF, ip_vrf);
    ip_vrf = NULL;
  }
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_unlock(_ip_vrf_master.vrf_mutex);
  return ip_vrf;
}

/* Delete a VRF. This is called in ip_vrf_terminate(). */
static int ip_vrf_del_one(struct ip_vrf *ip_vrf)
{
  int ret = 0;
  zlog_info(MODULE_NSM, "VRF %u is to be deleted.", ip_vrf->vrf_id);
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);

  if (_ip_vrf_master.vrf_disable_hook)
    ret = (*_ip_vrf_master.vrf_disable_hook)(ip_vrf->vrf_id, ip_vrf);

  if (_ip_vrf_master.vrf_delete_hook)
    ret = (*_ip_vrf_master.vrf_delete_hook)(ip_vrf->vrf_id, ip_vrf);
  if(ret == OK)
  {
    if (ip_vrf->name)
      XFREE(MTYPE_VRF_NAME, ip_vrf->name);

    lstDelete(_ip_vrf_master.ip_vrf_list, (NODE *)ip_vrf);

    XFREE(MTYPE_VRF, ip_vrf);
  }
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_unlock(_ip_vrf_master.vrf_mutex);
  return ret;
}

/* Look up a VRF by identifier. */
struct ip_vrf *ip_vrf_lookup(vrf_id_t vrf_id)
{
  struct ip_vrf *ip_vrf = NULL;
  ip_vrf = lstFirst(_ip_vrf_master.ip_vrf_list);
  while (ip_vrf)
  {
    if (ip_vrf)
    {
      if (ip_vrf->vrf_id == vrf_id)
      {
        return ip_vrf;
      }
    }
    ip_vrf = lstNext((NODE *)ip_vrf);
  }
  return NULL;
}

/* Look up a VRF by identifier. */
struct ip_vrf *ip_vrf_lookup_by_name(const char *name)
{
  struct ip_vrf *ip_vrf = NULL;
  ip_vrf = lstFirst(_ip_vrf_master.ip_vrf_list);
  while (ip_vrf)
  {
    if (ip_vrf && ip_vrf->name)
    {
      if (strcmp(ip_vrf->name, name) == 0)
      {
        return ip_vrf;
      }
    }
    ip_vrf = lstNext((NODE *)ip_vrf);
  }
  return NULL;
}

zpl_char *ip_vrf_vrfid2name(vrf_id_t vrf_id)
{
  struct ip_vrf *ip_vrf = ip_vrf_lookup(vrf_id);
  if (ip_vrf)
    return ip_vrf->name;
  return NULL;
}

vrf_id_t ip_vrf_name2vrfid(const char *name)
{
  struct ip_vrf *ip_vrf = ip_vrf_lookup_by_name(name);
  if (ip_vrf)
    return ip_vrf->vrf_id;
  return 0;
}



/* Add a VRF hook. Please add hooks before calling vrf_init(). */
void ip_vrf_add_hook(zpl_uint32 type, int (*func)(vrf_id_t, struct ip_vrf *))
{
  switch (type)
  {
  case VRF_NEW_HOOK:
    _ip_vrf_master.vrf_new_hook = func;
    break;
  case VRF_DELETE_HOOK:
    _ip_vrf_master.vrf_delete_hook = func;
    break;
  case VRF_ENABLE_HOOK:
    _ip_vrf_master.vrf_enable_hook = func;
    break;
  case VRF_DISABLE_HOOK:
    _ip_vrf_master.vrf_disable_hook = func;
    break;
  default:
    break;
  }
}


int ip_vrf_foreach(ip_vrf_call func, void *pVoid)
{
  int ret = 0;
	struct ip_vrf *ip_vrf = NULL;
	NODE index;
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
	for(ip_vrf = (struct ip_vrf *)lstFirst(_ip_vrf_master.ip_vrf_list);
			ip_vrf != NULL;  ip_vrf = (struct ip_vrf *)lstNext((NODE*)&index))
	{
		index = ip_vrf->node;
		if(ip_vrf && ip_vrf->info)
		{
			if(func)
			{
				ret = (func)(ip_vrf, pVoid);
        if(ret != OK)
          break;
			}
		}
	}
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_unlock(_ip_vrf_master.vrf_mutex);
	return OK;
}
/* Look up the data pointer of the specified VRF. */
void *
ip_vrf_info_lookup(vrf_id_t vrf_id)
{
  struct ip_vrf *ip_vrf = ip_vrf_lookup(vrf_id);
  return ip_vrf ? ip_vrf->info : NULL;
}

/* Initialize VRF module. */
struct ip_vrf *ip_vrf_create(const char *name)
{
  struct ip_vrf *ip_vrf = NULL;
  ip_vrf = ip_vrf_new_one(VRF_DEFAULT, name);
  return ip_vrf;
}

int ip_vrf_delete(const char *name)
{
  struct ip_vrf *ip_vrf = NULL;
  /* The default VRF always exists. */
  ip_vrf = ip_vrf_lookup_by_name(name);
  if (!ip_vrf)
  {
    zlog_err(MODULE_NSM, "vrf_init: failed to create the default VRF!");
    return ERROR;
  }
  ip_vrf_del_one(ip_vrf);
  return 0;
}

int ip_vrf_set_vrfid(struct ip_vrf *ip_vrf, vrf_id_t vrf_id)
{
  if (ip_vrf)
  {
    struct nsm_ip_vrf *zvrf;
    if (_ip_vrf_master.vrf_mutex)
      os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
    ip_vrf->vrf_id = vrf_id;

    zvrf = ip_vrf->info;
    zvrf->vrf_id = vrf_id;

    if (_ip_vrf_master.vrf_mutex)
      os_mutex_unlock(_ip_vrf_master.vrf_mutex);
    return 0;
  }
  return 0;
}



/* Zebra VRF initialization. */
void ip_vrf_init(void)
{
  memset(&_ip_vrf_master, 0, sizeof(struct ip_vrf_master));
  _ip_vrf_master.ip_vrf_list = XMALLOC(MTYPE_VRF, sizeof(LIST));
  if(_ip_vrf_master.ip_vrf_list)
  {
    lstInit(_ip_vrf_master.ip_vrf_list);
    ip_vrf_add_hook(VRF_NEW_HOOK, nsm_vrf_create);
    ip_vrf_add_hook(VRF_ENABLE_HOOK, nsm_vrf_enable);
    ip_vrf_add_hook(VRF_DISABLE_HOOK, nsm_vrf_disable);
    ip_vrf_new_one(VRF_DEFAULT, "Default");
  }
}

/* Terminate VRF module. */
void ip_vrf_terminate(void)
{
  struct ip_vrf *ip_vrf = NULL;
  NODE index;
  if (lstCount(_ip_vrf_master.ip_vrf_list) == 0)
    return;
  for (ip_vrf = (struct ip_vrf *)lstFirst(_ip_vrf_master.ip_vrf_list);
       ip_vrf != NULL; ip_vrf = (struct ip_vrf *)lstNext((NODE *)&index))
  {
    index = ip_vrf->node;
    if (ip_vrf)
    {
      ip_vrf_del_one(ip_vrf);
    }
  }
  return;
}
