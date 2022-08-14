/*
 * Prefix list functions.
 * Copyright (C) 1999 Kunihiro Ishiguro
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

#ifndef __LIB_PLIST_H
#define __LIB_PLIST_H

#ifdef __cplusplus
extern "C" {
#endif

enum prefix_list_type 
{
  PREFIX_DENY,
  PREFIX_PERMIT,
};

struct prefix_list;

struct orf_prefix
{
  zpl_uint32 seq;
  zpl_uchar ge;
  zpl_uchar le;
  struct prefix p;
};

/* Prototypes. */
extern void prefix_list_init (void);
extern void prefix_list_reset (void);
extern void prefix_list_add_hook (void (*func) (struct prefix_list *));
extern void prefix_list_delete_hook (void (*func) (struct prefix_list *));

extern const char *prefix_list_name (struct prefix_list *);
extern struct prefix_list *prefix_list_lookup (afi_t, const char *);
extern enum prefix_list_type prefix_list_apply (struct prefix_list *, void *);

extern struct prefix_list *prefix_bgp_orf_lookup (afi_t, const char *);
extern struct stream * prefix_bgp_orf_entry (struct stream *,
                                             struct prefix_list *,
                                             zpl_uchar, zpl_uchar, zpl_uchar);
extern int prefix_bgp_orf_set (zpl_char *, afi_t, struct orf_prefix *, int, int);
extern void prefix_bgp_orf_remove_all (afi_t, zpl_char *);
#ifdef ZPL_SHELL_MODULE
extern int prefix_bgp_show_prefix_list (struct vty *, afi_t, zpl_char *);
 #endif
#ifdef __cplusplus
}
#endif

#endif /* __LIB_PLIST_H */
