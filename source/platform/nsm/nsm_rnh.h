/*
 * Zebra next hop tracking header
 * Copyright (C) 2013 Cumulus Networks, Inc.
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

#ifndef __NSM_RNH_H
#define __NSM_RNH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "nsm_rib.h"
#include "nsm_zserv.h"
/* Nexthop structure. */
struct rnh
{
  zpl_uchar flags;
#define NSM_NHT_CONNECTED  	0x1
  struct rib *state;
  struct list *client_list;
  struct route_node *node;
};

extern struct rnh *nsm_rnh_add(struct prefix *p, vrf_id_t vrfid);
extern struct rnh *nsm_rnh_lookup(struct prefix *p, vrf_id_t vrfid);
extern void nsm_rnh_delete(struct rnh *rnh);
extern void nsm_rnh_client_add(struct rnh *rnh, struct zserv *client, vrf_id_t vrf_id_t);
extern void nsm_rnh_client_remove(struct rnh *rnh, struct zserv *client);
extern int nsm_rnh_evaluate_table(vrf_id_t vrfid, zpl_family_t family);
extern int nsm_rnh_dispatch_table(vrf_id_t vrfid, zpl_family_t family, struct zserv *cl);
extern void nsm_rnh_print_table(vrf_id_t vrfid, zpl_family_t family, struct vty *vty);
extern zpl_char *nsm_rnh_str(struct rnh *rnh, zpl_char *buf, zpl_size_t size);
extern int nsm_rnh_client_cleanup(vrf_id_t vrf, zpl_family_t family, struct zserv *client);


 
#ifdef __cplusplus
}
#endif

#endif /*__NSM_RNH_H */
