/*
 * Redistribution Handler
 * Copyright (C) 1999 Kunihiro Ishiguro
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

#ifndef __NSM_REDISTRIBUTE_H
#define __NSM_REDISTRIBUTE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "table.h"
#include "nsm_zserv.h"

extern void nsm_redistribute_add (zpl_uint16, struct zserv *, zpl_size_t, vrf_id_t);
extern void nsm_redistribute_delete (zpl_uint16, struct zserv *, zpl_size_t, vrf_id_t);

extern void nsm_redistribute_default_add (zpl_uint16, struct zserv *, zpl_size_t,
    vrf_id_t);
extern void nsm_redistribute_default_delete (zpl_uint16, struct zserv *, zpl_size_t,
    vrf_id_t);

extern void nsm_redistribute_route_add (struct prefix *, struct rib *new, struct rib *old);
extern void nsm_redistribute_route_delete (struct prefix *, struct rib *);

extern void nsm_redistribute_interface_updown (struct interface *, zpl_bool updown);


extern void nsm_redistribute_interface_create (struct interface *);
extern void nsm_redistribute_interface_destroy (struct interface *);

extern void nsm_redistribute_interface_address_add (struct interface *,
					 	struct connected *);
extern void nsm_redistribute_interface_address_delete (struct interface *,
						   struct connected *c);

extern void nsm_redistribute_interface_mode_update (struct interface *ifp, zpl_uint32  mode);



extern int nsm_check_addr (struct prefix *);

extern int is_default (struct prefix *);
 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_REDISTRIBUTE_H */

