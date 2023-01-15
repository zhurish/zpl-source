/* Zebra daemon server header.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
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

#ifndef __NSM_ZSERV_H__
#define __NSM_ZSERV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_rib.h"
#include "if.h"
#include "workqueue.h"
#ifdef ZPL_VRF_MODULE

#endif
#include "route_types.h"
/* Default port information. */


/* Client structure. */
struct zserv
{
  /* Client file descriptor. */
  zpl_socket_t sock;

  /* Input/output buffer to the client. */
  struct stream *ibuf;
  struct stream *obuf;

  /* Buffer of data waiting to be written to client. */
  struct buffer *wb;

  /* Threads for read/write. */
  struct thread *t_read;
  struct thread *t_write;

  /* Thread for delayed close. */
  struct thread *t_suicide;

  /* default routing table this client munges */
  zpl_uint32 rtm_table;
#ifdef ZPL_VRF_MODULE
  /* This client's redistribute flag. */
  ip_vrf_bitmap_t redist[ZPL_ROUTE_PROTO_MAX];
#endif
  /* Redistribute default route flag. */
  ip_vrf_bitmap_t redist_default;

  /* Interface information. */
  ip_vrf_bitmap_t ifinfo;

  /* Router-id information. */
  ip_vrf_bitmap_t ridinfo;

  /* client's protocol */
  zpl_uchar proto;

  /* Statistics */
  zpl_uint32 redist_v4_add_cnt;
  zpl_uint32 redist_v4_del_cnt;
  zpl_uint32 redist_v6_add_cnt;
  zpl_uint32 redist_v6_del_cnt;
  zpl_uint32 v4_route_add_cnt;
  zpl_uint32 v4_route_upd8_cnt;
  zpl_uint32 v4_route_del_cnt;
  zpl_uint32 v6_route_add_cnt;
  zpl_uint32 v6_route_del_cnt;
  zpl_uint32 v6_route_upd8_cnt;
  zpl_uint32 connected_rt_add_cnt;
  zpl_uint32 connected_rt_del_cnt;
  zpl_uint32 ifup_cnt;
  zpl_uint32 ifdown_cnt;
  zpl_uint32 ifadd_cnt;
  zpl_uint32 ifdel_cnt;

  zpl_time_t connect_time;
  zpl_time_t last_read_time;
  zpl_time_t last_write_time;
  zpl_time_t nh_reg_time;
  zpl_time_t nh_dereg_time;
  zpl_time_t nh_last_upd_time;

  zpl_uint32 last_read_cmd;
  zpl_uint32 last_write_cmd;
};

/* Zebra instance */
struct nsm_srv_t
{
  /* Thread master */
  struct thread_master *master;
  struct list *client_list;

  /* default table */
  zpl_uint32 rtm_table_default;

  zpl_taskid_t nsm_task_id;
};

extern struct nsm_srv_t *nsm_srv;

/* Prototypes. */
extern void nsm_zserv_init (void);
extern void cmd_nsm_zserv_init(void);

extern void nsm_zserv_encode_interface(struct stream *s, struct interface *ifp);
extern void nsm_zserv_encode_interface_end(struct stream *s);

extern int nsm_zserv_send_interface_add (struct zserv *, struct interface *);
extern int nsm_zserv_send_interface_delete (struct zserv *, struct interface *);
extern int nsm_zserv_send_interface_address (zpl_uint16, struct zserv *, struct interface *,
                                    struct connected *);
extern int nsm_zserv_send_interface_state (zpl_uint16, struct zserv *, struct interface *);
extern int nsm_zserv_send_interface_mode (struct zserv *, struct interface *, zpl_uint32 mode);


extern int nsm_zserv_send_route_multipath (zpl_uint16, struct zserv *, struct prefix *, 
                                  struct rib *);
extern int nsm_zserv_send_router_id_update (struct zserv *, struct prefix *,
                                   vrf_id_t);



extern void nsm_zserv_create_header(struct stream *s, zpl_uint16 cmd, vrf_id_t);
extern int nsm_zserv_send_message(struct zserv *client);


extern void nsm_route_map_init (void);
extern void nsm_snmp_init (void);

#ifdef __cplusplus
}
#endif

#endif /* __NSM_ZSERV_H */
