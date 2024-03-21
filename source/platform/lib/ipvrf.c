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

#include "auto_include.h"
#include "zpl_type.h"
#include "os_sem.h"
#include "module.h"
#include "zmemory.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"

#include "log.h"
#ifdef ZPL_VRF_MODULE
#include "ipvrf.h"
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

ip_vrf_bitmap_t ip_vrf_bitmap_init(void)
{
  return (ip_vrf_bitmap_t)XCALLOC(MTYPE_VRF_BITMAP, sizeof(struct vrf_bitmap_table));
}

void ip_vrf_bitmap_free(ip_vrf_bitmap_t bmap)
{
  struct vrf_bitmap_table *bm = (struct vrf_bitmap_table *)bmap;
  zpl_uint32 i;

  if (bmap == IP_VRF_BITMAP_NULL)
    return;

  for (i = 0; i < VRF_BITMAP_NUM_OF_GROUPS; i++)
    if (bm->groups[i])
      XFREE(MTYPE_VRF_BITMAP, bm->groups[i]);

  XFREE(MTYPE_VRF_BITMAP, bm);
}

void ip_vrf_bitmap_set(ip_vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap_table *bm = (struct vrf_bitmap_table *)bmap;
  zpl_uchar group = VRF_BITMAP_GROUP(vrf_id);
  zpl_uchar offset = VRF_BITMAP_BIT_OFFSET(vrf_id);

  if (bmap == IP_VRF_BITMAP_NULL)
    return;

  if (bm->groups[group] == NULL)
    bm->groups[group] = XCALLOC(MTYPE_VRF_BITMAP,
                                VRF_BITMAP_NUM_OF_BYTES_IN_GROUP);

  SET_FLAG(bm->groups[group][VRF_BITMAP_INDEX_IN_GROUP(offset)],
           VRF_BITMAP_FLAG(offset));
}

void ip_vrf_bitmap_unset(ip_vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap_table *bm = (struct vrf_bitmap_table *)bmap;
  zpl_uchar group = VRF_BITMAP_GROUP(vrf_id);
  zpl_uchar offset = VRF_BITMAP_BIT_OFFSET(vrf_id);

  if (bmap == IP_VRF_BITMAP_NULL || bm->groups[group] == NULL)
    return;

  UNSET_FLAG(bm->groups[group][VRF_BITMAP_INDEX_IN_GROUP(offset)],
             VRF_BITMAP_FLAG(offset));
}

zpl_bool ip_vrf_bitmap_check(ip_vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap_table *bm = (struct vrf_bitmap_table *)bmap;
  zpl_uchar group = VRF_BITMAP_GROUP(vrf_id);
  zpl_uchar offset = VRF_BITMAP_BIT_OFFSET(vrf_id);

  if (bmap == IP_VRF_BITMAP_NULL || bm->groups[group] == NULL)
    return 0;

  return CHECK_FLAG(bm->groups[group][VRF_BITMAP_INDEX_IN_GROUP(offset)],
                    VRF_BITMAP_FLAG(offset))
             ? zpl_true
             : zpl_false;
}
/***********************************************************************/
/***********************************************************************/

void ip_vrf_add_hook(int type, int (*func)(struct ip_vrf *, void *))
{
  switch (type)
  {
  case VRF_NEW_HOOK:
    _ip_vrf_master.ip_vrf_new_hook = func;
    break;
  case VRF_DELETE_HOOK:
    _ip_vrf_master.ip_vrf_delete_hook = func;
    break;
  case VRF_SET_VRFID_HOOK:
    _ip_vrf_master.ip_vrf_set_vrfid_hook = func;
    break;
    break;
  default:
    break;
  }
}

void *ip_vrf_list(void)
{
  return _ip_vrf_master.ip_vrf_list;
}

/* Get a VRF. If not found, create one. */
static struct ip_vrf *ip_vrf_new_one(vrf_id_t vrf_id, const char *name)
{
  int ret = 0;
  struct ip_vrf *ip_vrf = NULL;
  ip_vrf = XCALLOC(MTYPE_VRF, sizeof(struct ip_vrf));
  if (!ip_vrf)
    return NULL;
  memset(ip_vrf, 0, sizeof(struct ip_vrf));
  ip_vrf->vrf_id = vrf_id;
  strcpy(ip_vrf->name, name);
  if (_ip_vrf_master.ip_vrf_new_hook)
    ret = (_ip_vrf_master.ip_vrf_new_hook)(ip_vrf, NULL);
  zlog_info(MODULE_NSM, "VRF %s %u is created.", ip_vrf->name, vrf_id);
  if (ret == OK)
  {
    lstAdd(_ip_vrf_master.ip_vrf_list, (NODE *)ip_vrf);
  }
  else
  {
    XFREE(MTYPE_VRF, ip_vrf);
    ip_vrf = NULL;
  }
  return ip_vrf;
}

/* Delete a VRF. This is called in ip_vrf_terminate(). */
static int ip_vrf_del_one(struct ip_vrf *ip_vrf)
{
  int ret = 0;
  zlog_info(MODULE_NSM, "VRF %u is to be deleted.", ip_vrf->vrf_id);
  if (_ip_vrf_master.ip_vrf_delete_hook)
    ret = (_ip_vrf_master.ip_vrf_delete_hook)(ip_vrf, NULL);
  if (ret == OK)
  {
    lstDelete(_ip_vrf_master.ip_vrf_list, (NODE *)ip_vrf);

    XFREE(MTYPE_VRF, ip_vrf);
  }

  return ret;
}

/* Look up a VRF by identifier. */
struct ip_vrf *ip_vrf_lookup(vrf_id_t vrf_id)
{
  struct ip_vrf *ip_vrf = NULL;
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
  ip_vrf = (struct ip_vrf *)lstFirst(_ip_vrf_master.ip_vrf_list);
  while (ip_vrf)
  {
    if (ip_vrf)
    {
      if (ip_vrf->vrf_id == vrf_id)
      {
        if (_ip_vrf_master.vrf_mutex)
          os_mutex_unlock(_ip_vrf_master.vrf_mutex);
        return ip_vrf;
      }
    }
    ip_vrf = (struct ip_vrf *)lstNext((NODE *)ip_vrf);
  }
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_unlock(_ip_vrf_master.vrf_mutex);
  return NULL;
}

/* Look up a VRF by identifier. */
struct ip_vrf *ip_vrf_lookup_by_name(const char *name)
{
  struct ip_vrf *ip_vrf = NULL;
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
  ip_vrf = (struct ip_vrf *)lstFirst(_ip_vrf_master.ip_vrf_list);
  while (ip_vrf)
  {
    if (ip_vrf && ip_vrf->name)
    {
      if (strcmp(ip_vrf->name, name) == 0)
      {
        if (_ip_vrf_master.vrf_mutex)
          os_mutex_unlock(_ip_vrf_master.vrf_mutex);
        return ip_vrf;
      }
    }
    ip_vrf = (struct ip_vrf *)lstNext((NODE *)ip_vrf);
  }
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_unlock(_ip_vrf_master.vrf_mutex);
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

int ip_vrf_foreach(ip_vrf_call func, void *pVoid)
{
  int ret = 0;
  struct ip_vrf *ip_vrf = NULL;
  NODE index;
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
  for (ip_vrf = (struct ip_vrf *)lstFirst(_ip_vrf_master.ip_vrf_list);
       ip_vrf != NULL; ip_vrf = (struct ip_vrf *)lstNext((NODE *)&index))
  {
    index = ip_vrf->node;
    if (ip_vrf && ip_vrf->info)
    {
      if (func)
      {
        ret = (func)(ip_vrf, pVoid);
        if (ret != OK)
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
  static int local_vrf_id = 0;
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
  ip_vrf = ip_vrf_new_one(local_vrf_id++, name);
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_unlock(_ip_vrf_master.vrf_mutex);
  return ip_vrf;
}

int ip_vrf_delete(const char *name)
{
  int ret = ERROR;
  struct ip_vrf *ip_vrf = NULL;
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
  ip_vrf = ip_vrf_lookup_by_name(name);
  if (!ip_vrf)
  {
    if (_ip_vrf_master.vrf_mutex)
      os_mutex_unlock(_ip_vrf_master.vrf_mutex);
    zlog_err(MODULE_NSM, "vrf_init: failed to create the default VRF!");
    return ERROR;
  }
  ret = ip_vrf_del_one(ip_vrf);
  if (_ip_vrf_master.vrf_mutex)
    os_mutex_unlock(_ip_vrf_master.vrf_mutex);
  return ret;
}

int ip_vrf_set_vrfid(struct ip_vrf *ip_vrf, vrf_id_t vrf_id, struct prefix *p)
{
  int ret = ERROR;
  vrf_id_t old_vrfid;
  if (ip_vrf)
  {
    if (_ip_vrf_master.vrf_mutex)
      os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
    old_vrfid = ip_vrf->vrf_id;
    ip_vrf->vrf_id = vrf_id;
    if (_ip_vrf_master.ip_vrf_set_vrfid_hook)
      ret = (_ip_vrf_master.ip_vrf_set_vrfid_hook)(ip_vrf, p);
    if (ret != OK)
      ip_vrf->vrf_id = old_vrfid;
    else
      memcpy(&ip_vrf->rd_id, p, sizeof(struct prefix));
    if (_ip_vrf_master.vrf_mutex)
      os_mutex_unlock(_ip_vrf_master.vrf_mutex);
    return ret;
  }
  return ret;
}

/* Zebra VRF initialization. */
void ip_vrf_init(void)
{
  memset(&_ip_vrf_master, 0, sizeof(struct ip_vrf_master));
  _ip_vrf_master.ip_vrf_list = XMALLOC(MTYPE_VRF, sizeof(LIST));
  if (_ip_vrf_master.ip_vrf_list)
  {
    lstInit(_ip_vrf_master.ip_vrf_list);
    _ip_vrf_master.vrf_mutex = os_mutex_name_create("vrfmutex");
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
