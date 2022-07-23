/*
 * VRF related header.
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

#ifndef __IP_VRF_H__
#define __IP_VRF_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_VRF_MODULE

#include "os_list.h"
/* The default VRF ID */
#define VRF_DEFAULT 0

/*
 * The command strings
 */



#define VRF_ALL_CMD_STR         "vrf all"
#define VRF_ALL_CMD_HELP_STR    "Specify the VRF\nAll VRFs\n"


#define VRF_CMD_STR		"vrf (NAME|<0-65535>)"
#define VRF_CMD_HELP_STR	"Specify the VRF\nSpecify the VRF name\nThe VRF ID\n"

/*
 * VRF hooks
 */

#define VRF_NEW_HOOK        0   /* a new VRF is just created */
#define VRF_DELETE_HOOK     1   /* a VRF is to be deleted */
#define VRF_ENABLE_HOOK     2   /* a VRF is ready to use */
#define VRF_DISABLE_HOOK    3   /* a VRF is to be unusable */
#define VRF_UPDATE_HOOK    4  /* a VRF is to be unusable */
#define VRF_NAME_MAX    32


struct ip_vrf
{
  NODE node;
  /* Identifier, same as the vector index */
  vrf_id_t vrf_id;
  /* File descriptor */
  zpl_socket_t fd;
  /* Name */
  char name[VRF_NAME_MAX];
  /* User data */
  void *info;
};

struct ip_vrf_master
{
  LIST *ip_vrf_list;
  void *vrf_mutex;  
};


struct ip_vrf_temp
{
	  zpl_uchar proto;
    vrf_id_t vrf_id;
    zpl_socket_t fd;
    char name[VRF_NAME_MAX];
    zpl_uint32 value;
		zpl_ulong cnt;
    void    *p;
};

typedef int (*ip_vrf_call)(struct ip_vrf *, void *);


extern struct ip_vrf_master _ip_vrf_master;


/* Look up the data pointer of the specified VRF. */
extern void *ip_vrf_info_lookup (vrf_id_t);
struct ip_vrf * ip_vrf_lookup (vrf_id_t vrf_id);
struct ip_vrf * ip_vrf_lookup_by_name (const char *name);

zpl_char * ip_vrf_vrfid2name (vrf_id_t vrf_id);
vrf_id_t ip_vrf_name2vrfid (const char *name);

extern void *ip_vrf_list (void);
/*
 * Utilities to obtain the interface list
 */



/*
 * VRF bit-map: maintaining flags, one bit per VRF ID
 */

typedef void *              vrf_bitmap_t;
#define VRF_BITMAP_NULL     NULL

extern vrf_bitmap_t ip_vrf_bitmap_init (void);
extern void ip_vrf_bitmap_free (vrf_bitmap_t);
extern void ip_vrf_bitmap_set (vrf_bitmap_t, vrf_id_t);
extern void ip_vrf_bitmap_unset (vrf_bitmap_t, vrf_id_t);
extern zpl_bool ip_vrf_bitmap_check (vrf_bitmap_t, vrf_id_t);

/*
 * VRF initializer/destructor
 */
/* Please add hooks before calling vrf_init(). */
//extern void vrf_init (void);

extern struct ip_vrf * ip_vrf_create (const char *name);
extern int ip_vrf_delete (const char *name);
extern int ip_vrf_set_vrfid (struct ip_vrf *ip_vrf, vrf_id_t vrf_id);
extern int ip_vrf_foreach(ip_vrf_call func, void *pVoid);

extern void ip_vrf_terminate (void);

extern void ip_vrf_init (void);


#endif

#ifdef __cplusplus
}
#endif

#endif /*__IP_VRF_H__*/

