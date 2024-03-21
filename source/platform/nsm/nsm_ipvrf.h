/*
 * Routing Information Base header
 * Copyright (C) 1997 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef __NSM_IPVRF_H__
#define __NSM_IPVRF_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "nsm_rtadv.h"
/* Routing table instance.  */
struct nsm_ipvrf
{
  /* Identifier. */
  vrf_id_t vrf_id;

  /* Routing table name.  */
  zpl_char *name;

  /* Description.  */
  zpl_char *desc;

  /* FIB identifier.  */
  zpl_uchar fib_id;

  /* Routing table.  */
  struct route_table *table[AFI_MAX][SAFI_MAX];

  /* Static route configuration.  */
  struct route_table *stable[AFI_MAX][SAFI_MAX];

  /* 2nd pointer type used primarily to quell a warning on
   * ALL_LIST_ELEMENTS_RO
   */
  struct list _rid_all_sorted_list;
  struct list _rid_lo_sorted_list;
  struct list *rid_all_sorted_list;
  struct list *rid_lo_sorted_list;
  struct prefix rid_user_assigned;

#if defined (ZPL_NSM_RTADV) 
  struct rtadv rtadv;
#endif /* ZPL_NSM_RTADV */


  /* Recursive Nexthop table */
  struct route_table *rnh_table[AFI_MAX];
};

extern int ipvrf_nsm_init(void);

#ifdef __cplusplus
}
#endif

#endif /*__NSM_IPVRF_H__ */
