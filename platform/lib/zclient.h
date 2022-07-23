/* Zebra's client header.
 * Copyright (C) 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
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

#ifndef __ZCLIENT_H__
#define __ZCLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "route_types.h"
/* For struct zapi_ipv{4,6}. */
#include "prefix.h"

/* For struct interface and struct connected. */
#include "if.h"
#include "route_types.h"
/* For input/output buffer to zebra. */
#define ZCLIENT_MAX_PACKET_SIZ          4096

/* Zebra header size. */
#define ZCLIENT_HEADER_SIZE             6

/* Structure for the zebra client. */
struct zclient
{
  /* Socket to zebra daemon. */
  zpl_socket_t sock;

  /* Flag of communication to zebra is enabled or not.  Default is on.
     This flag is disabled by `no router zebra' statement. */
  zpl_bool enable;

  /* Connection failure count. */
  zpl_uint32 fail;

  /* Input buffer for zebra message. */
  struct stream *ibuf;

  /* Output buffer for zebra message. */
  struct stream *obuf;

  /* Buffer of data waiting to be written to zebra. */
  struct buffer *wb;

  /* Read and connect thread. */
  struct thread *t_read;
  struct thread *t_connect;

  /* Thread to write buffered data to zebra. */
  struct thread *t_write;

  /* Redistribute information. */
  zpl_uchar redist_default;
  zpl_uchar redist[ZPL_ROUTE_PROTO_MAX];

  /* Redistribute defauilt. */
  zpl_uchar default_information;

  /* Pointer to the callback functions. */
  int (*router_id_update) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*interface_add) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*interface_delete) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*interface_up) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*interface_down) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*interface_address_add) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*interface_address_delete) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*ipv4_route_add) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*ipv4_route_delete) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*ipv6_route_add) (zpl_uint16, struct zclient *, zpl_uint16);
  int (*ipv6_route_delete) (zpl_uint16, struct zclient *, zpl_uint16);
};

/* Zebra API message flag. */
#define ZAPI_MESSAGE_NEXTHOP  0x01
#define ZAPI_MESSAGE_IFINDEX  0x02
#define ZAPI_MESSAGE_DISTANCE 0x04
#define ZAPI_MESSAGE_METRIC   0x08
#define ZAPI_MESSAGE_MTU	  0x10
#define ZAPI_MESSAGE_TAG	  0x20
/* Zserv protocol message header */
struct zserv_header
{
  zpl_uint16 length;
  zpl_uint8 marker;	/* corresponds to command field in old zserv
                         * always set to 255 in new zserv.
                         */
  zpl_uint8 version;
#define ZSERV_VERSION	2
  zpl_uint16 command;
};

/* Zebra IPv4 route message API. */
struct zapi_ipv4
{
  zpl_uchar type;

  zpl_uchar flags;

  zpl_uchar message;

  safi_t safi;

  zpl_uchar nexthop_num;
  struct ipstack_in_addr **nexthop;

  zpl_uchar ifindex_num;
  zpl_uint32  *ifindex;

  zpl_uchar distance;

  zpl_uint32 metric;

  zpl_uint32 tag;
};

/* Prototypes of zebra client service functions. */
extern struct zclient *zclient_new (void);
extern void zclient_init (struct zclient *, zpl_uint32);
extern int zclient_start (struct zclient *);
extern void zclient_stop (struct zclient *);
extern void zclient_reset (struct zclient *);
extern void zclient_free (struct zclient *);

extern zpl_socket_t  zclient_socket_connect (struct zclient *);
extern void zclient_serv_path_set  (zpl_char *path);
extern const char *const zclient_serv_path_get (void);

/* Send redistribute command to zebra daemon. Do not update zclient state. */
extern int zclient_redistribute_send (zpl_uint16 command, struct zclient *, zpl_uint32 type);

/* If state has changed, update state and call zclient_redistribute_send. */
extern void zclient_redistribute (zpl_uint16 command, struct zclient *, zpl_uint32 type);

/* If state has changed, update state and send the command to zebra. */
extern void zclient_redistribute_default (zpl_uint16 command, struct zclient *);

/* Send the message in zclient->obuf to the zebra daemon (or enqueue it).
   Returns 0 for success or -1 on an I/O error. */
extern int zclient_send_message(struct zclient *);

/* create header for command, length to be filled in by user later */
extern void zclient_create_header (struct stream *, zpl_uint16);

extern struct interface *zclient_interface_add_read (struct stream *);
extern struct interface *zclient_interface_state_read (struct stream *s);
extern struct connected *zclient_interface_address_read (zpl_uint16, struct stream *);
extern void zclient_interface_if_set_value (struct stream *, struct interface *);
extern void zclient_router_id_update_read (struct stream *s, struct prefix *rid);
extern int zapi_ipv4_route (zpl_uint16, struct zclient *, struct prefix_ipv4 *, 
                            struct zapi_ipv4 *);

#ifdef ZPL_BUILD_IPV6
/* IPv6 prefix add and delete function prototype. */

struct zapi_ipv6
{
  zpl_uchar type;

  zpl_uchar flags;

  zpl_uchar message;

  safi_t safi;

  zpl_uchar nexthop_num;
  struct ipstack_in6_addr **nexthop;

  zpl_uchar ifindex_num;
  zpl_uint32  *ifindex;

  zpl_uchar distance;

  zpl_uint32 metric;

  zpl_uint32 tag;
};

extern int zapi_ipv6_route (zpl_uint16 cmd, struct zclient *zclient, 
                     struct prefix_ipv6 *p, struct zapi_ipv6 *api);
#endif /* ZPL_BUILD_IPV6 */
 
#ifdef __cplusplus
}
#endif

#endif /* __ZCLIENT_H__ */
