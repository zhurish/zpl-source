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
#include "lib_include.h"
#include "nsm_include.h"

#ifdef HAVE_NETNS
#undef _GNU_SOURCE
#define _GNU_SOURCE

#include <sched.h>
#endif


/* Holding VRF hooks  */
struct vrf_master
{
  int (*vrf_new_hook)(vrf_id_t, void **);
  int (*vrf_delete_hook)(vrf_id_t, void **);
  int (*vrf_enable_hook)(vrf_id_t, void **);
  int (*vrf_disable_hook)(vrf_id_t, void **);
} vrf_master = {
    0,
};

/* VRF table */
static LIST vrf_list_table;
static void *vrfMutex = NULL;
/*
static int vrf_is_enabled (struct vrf *vrf);
*/
static int vrf_enable(struct vrf *vrf);
static int vrf_disable(struct vrf *vrf);

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

struct vrf_bitmap
{
  zpl_uchar *groups[VRF_BITMAP_NUM_OF_GROUPS];
};

vrf_bitmap_t
vrf_bitmap_init(void)
{
  return (vrf_bitmap_t)XCALLOC(MTYPE_VRF_BITMAP, sizeof(struct vrf_bitmap));
}

void vrf_bitmap_free(vrf_bitmap_t bmap)
{
  struct vrf_bitmap *bm = (struct vrf_bitmap *)bmap;
  zpl_uint32 i;

  if (bmap == VRF_BITMAP_NULL)
    return;

  for (i = 0; i < VRF_BITMAP_NUM_OF_GROUPS; i++)
    if (bm->groups[i])
      XFREE(MTYPE_VRF_BITMAP, bm->groups[i]);

  XFREE(MTYPE_VRF_BITMAP, bm);
}

void vrf_bitmap_set(vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap *bm = (struct vrf_bitmap *)bmap;
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

void vrf_bitmap_unset(vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap *bm = (struct vrf_bitmap *)bmap;
  zpl_uchar group = VRF_BITMAP_GROUP(vrf_id);
  zpl_uchar offset = VRF_BITMAP_BIT_OFFSET(vrf_id);

  if (bmap == VRF_BITMAP_NULL || bm->groups[group] == NULL)
    return;

  UNSET_FLAG(bm->groups[group][VRF_BITMAP_INDEX_IN_GROUP(offset)],
             VRF_BITMAP_FLAG(offset));
}

zpl_bool vrf_bitmap_check(vrf_bitmap_t bmap, vrf_id_t vrf_id)
{
  struct vrf_bitmap *bm = (struct vrf_bitmap *)bmap;
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

/* Get a VRF. If not found, create one. */
static struct vrf *vrf_new_one(vrf_id_t vrf_id, const char *name)
{
  struct vrf *vrf;
  if (vrfMutex)
    os_mutex_lock(vrfMutex, OS_WAIT_FOREVER);

  vrf = XCALLOC(MTYPE_VRF, sizeof(struct vrf));
  vrf->vrf_id = vrf_id;
  if (name)
    vrf->name = XSTRDUP(MTYPE_VRF_NAME, name);

  /* Initialize interfaces. */
  zlog_info(MODULE_NSM, "VRF %u is created.", vrf_id);
  lstAdd(&vrf_list_table, (NODE *)vrf);
  if (vrf_master.vrf_new_hook)
    (*vrf_master.vrf_new_hook)(vrf_id, &vrf->info);

  vrf_enable(vrf);

  if (vrfMutex)
    os_mutex_unlock(vrfMutex);
  return vrf;
}

/* Delete a VRF. This is called in vrf_terminate(). */
static int vrf_del_one(struct vrf *vrf)
{
  zlog_info(MODULE_NSM, "VRF %u is to be deleted.", vrf->vrf_id);
  if (vrfMutex)
    os_mutex_lock(vrfMutex, OS_WAIT_FOREVER);

  vrf_disable(vrf);

  if (vrf_master.vrf_delete_hook)
    (*vrf_master.vrf_delete_hook)(vrf->vrf_id, &vrf->info);

  if (vrf->name)
    XFREE(MTYPE_VRF_NAME, vrf->name);

  lstDelete(&vrf_list_table, (NODE *)vrf);

  XFREE(MTYPE_VRF, vrf);

  if (vrfMutex)
    os_mutex_unlock(vrfMutex);
  return OK;
}

/* Look up a VRF by identifier. */
struct vrf *
vrf_lookup(vrf_id_t vrf_id)
{
  struct vrf *vrf = NULL;
  vrf = lstFirst(&vrf_list_table);
  while (vrf)
  {
    if (vrf)
    {
      if (vrf->vrf_id == vrf_id)
      {
        return vrf;
      }
    }
    vrf = lstNext((NODE *)vrf);
  }
  return NULL;
}

/* Look up a VRF by identifier. */
struct vrf *
vrf_lookup_by_name(const char *name)
{
  struct vrf *vrf = NULL;
  vrf = lstFirst(&vrf_list_table);
  while (vrf)
  {
    if (vrf && vrf->name)
    {
      if (strcmp(vrf->name, name) == 0)
      {
        return vrf;
      }
    }
    vrf = lstNext((NODE *)vrf);
  }
  return NULL;
}

zpl_char *vrf_vrfid2name(vrf_id_t vrf_id)
{
  struct vrf *vrf = vrf_lookup(vrf_id);
  if (vrf)
    return vrf->name;
  return NULL;
}

vrf_id_t vrf_name2vrfid(const char *name)
{
  struct vrf *vrf = vrf_lookup_by_name(name);
  if (vrf)
    return vrf->vrf_id;
  return 0;
}
/*
 * Enable a VRF - that is, let the VRF be ready to use.
 * The VRF_ENABLE_HOOK callback will be called to inform
 * that they can allocate resources in this VRF.
 *
 * RETURN: 1 - enabled successfully; otherwise, 0.
 */
static int
vrf_enable(struct vrf *vrf)
{
  zlog_info(MODULE_NSM, "VRF %u is enabled.", vrf->vrf_id);
  if (vrf_master.vrf_enable_hook)
    (*vrf_master.vrf_enable_hook)(vrf->vrf_id, &vrf->info);
  return 1;
}

/*
 * Disable a VRF - that is, let the VRF be unusable.
 * The VRF_DELETE_HOOK callback will be called to inform
 * that they must release the resources in the VRF.
 */
static int
vrf_disable(struct vrf *vrf)
{
  zlog_info(MODULE_NSM, "VRF %u is to be disabled.", vrf->vrf_id);
  if (vrf_master.vrf_disable_hook)
    (*vrf_master.vrf_disable_hook)(vrf->vrf_id, &vrf->info);
  return OK;
}

/* Add a VRF hook. Please add hooks before calling vrf_init(). */
void vrf_add_hook(zpl_uint32 type, int (*func)(vrf_id_t, void **))
{
  switch (type)
  {
  case VRF_NEW_HOOK:
    vrf_master.vrf_new_hook = func;
    break;
  case VRF_DELETE_HOOK:
    vrf_master.vrf_delete_hook = func;
    break;
  case VRF_ENABLE_HOOK:
    vrf_master.vrf_enable_hook = func;
    break;
  case VRF_DISABLE_HOOK:
    vrf_master.vrf_disable_hook = func;
    break;
  default:
    break;
  }
}

/* Return the iterator of the first VRF. */
vrf_iter_t
vrf_first(void)
{
  struct vrf *rn;
  rn = lstFirst(&vrf_list_table);
  return rn ? (vrf_iter_t)rn : VRF_ITER_INVALID;
}

/* Return the next VRF iterator to the given iterator. */
vrf_iter_t
vrf_next(vrf_iter_t iter)
{
  struct vrf *rn = NULL;

  rn = lstNext((NODE *)iter);
  return rn ? (vrf_iter_t)rn : VRF_ITER_INVALID;
}

/* Return the VRF iterator of the given VRF ID. If it does not exist,
 * the iterator of the next existing VRF is returned. */
vrf_iter_t
vrf_iterator(vrf_id_t vrf_id)
{
  struct vrf *vrf = vrf_lookup(vrf_id);
  return (vrf) ? (vrf_iter_t)vrf : VRF_ITER_INVALID;
}

/* Obtain the VRF ID from the given VRF iterator. */
vrf_id_t
vrf_iter2id(vrf_iter_t iter)
{
  struct vrf *rn = (struct vrf *)iter;
  return (rn) ? rn->vrf_id : VRF_DEFAULT;
}

/* Obtain the data pointer from the given VRF iterator. */
void *
vrf_iter2info(vrf_iter_t iter)
{
  struct vrf *rn = (struct vrf *)iter;
  return (rn && rn->info) ? (rn->info) : NULL;
}

/* Get the data pointer of the specified VRF. If not found, create one. */

/* Look up the data pointer of the specified VRF. */
void *
vrf_info_lookup(vrf_id_t vrf_id)
{
  struct vrf *vrf = vrf_lookup(vrf_id);
  return vrf ? vrf->info : NULL;
}

/* Initialize VRF module. */
static int
vrf_init(void)
{
  lstInit(&vrf_list_table);
  vrf_new_one(VRF_DEFAULT, "Default-IP-Routing-Table");
  return OK;
}

struct vrf *nsm_vrf_create(const char *name)
{
  struct vrf *vrf;
  vrf = vrf_new_one(VRF_DEFAULT, name);
  return vrf;
}

int nsm_vrf_delete(const char *name)
{
  struct vrf *vrf;
  /* The default VRF always exists. */
  vrf = vrf_lookup_by_name(name);
  if (!vrf)
  {
    zlog_err(MODULE_NSM, "vrf_init: failed to create the default VRF!");
  }
  vrf_del_one(vrf);
  return 0;
}

int nsm_vrf_set_vrfid(struct vrf *vrf, vrf_id_t vrf_id)
{
  if (vrf)
  {
#ifdef ZPL_NSM_MODULE
    struct nsm_vrf *zvrf;
#endif
    if (vrfMutex)
      os_mutex_lock(vrfMutex, OS_WAIT_FOREVER);
    vrf->vrf_id = vrf_id;
#ifdef ZPL_NSM_MODULE
    zvrf = vrf->info;
    zvrf->vrf_id = vrf_id;
#endif
#ifdef ZPL_PAL_MODULE
    if (vrf_id)
      nsm_halpal_create_vr(vrf_id);
#endif
    if (vrfMutex)
      os_mutex_unlock(vrfMutex);
    return 0;
  }
  return 0;
}

/* Callback upon creating a new VRF. */
static int
nsm_vrf_new(vrf_id_t vrf_id, void **info)
{
#ifdef ZPL_NSM_MODULE
  struct nsm_vrf *zvrf = *info;
  if (!zvrf)
  {
    zvrf = nsm_vrf_alloc(vrf_id);
    *info = (void *)zvrf;
    #ifdef ZPL_NSM_RTPL
    router_id_init(zvrf);
    #endif
    if (vrf_id)
      nsm_halpal_create_vr(vrf_id);
  }
#endif
  return 0;
}

/* Callback upon enabling a VRF. */
static int
nsm_vrf_enable(vrf_id_t vrf_id, void **info)
{
#ifdef ZPL_NSM_MODULE
  struct nsm_vrf *zvrf = (struct nsm_vrf *)(*info);

  assert(zvrf);
#endif
  return 0;
}

/* Callback upon disabling a VRF. */
static int
nsm_vrf_disable(vrf_id_t vrf_id, void **info)
{
#ifdef ZPL_NSM_MODULE
  struct nsm_vrf *zvrf = (struct nsm_vrf *)(*info);

  assert(zvrf);

  rib_close_table(zvrf->table[AFI_IP][SAFI_UNICAST]);
  rib_close_table(zvrf->table[AFI_IP6][SAFI_UNICAST]);

  list_delete_all_node(zvrf->rid_all_sorted_list);
  list_delete_all_node(zvrf->rid_lo_sorted_list);
  nsm_halpal_delete_vr(vrf_id);
#endif
  return 0;
}

/* Zebra VRF initialization. */
void nsm_vrf_init(void)
{
  vrf_add_hook(VRF_NEW_HOOK, nsm_vrf_new);
  vrf_add_hook(VRF_ENABLE_HOOK, nsm_vrf_enable);
  vrf_add_hook(VRF_DISABLE_HOOK, nsm_vrf_disable);
  vrf_init();
}

/* Terminate VRF module. */
void vrf_terminate(void)
{
  struct vrf *vrf;
  NODE index;
  if (lstCount(&vrf_list_table) == 0)
    return;
  for (vrf = (struct vrf *)lstFirst(&vrf_list_table);
       vrf != NULL; vrf = (struct vrf *)lstNext((NODE *)&index))
  {
    index = vrf->node;
    if (vrf)
    {
      vrf_del_one(vrf);
    }
  }
  return;
}
