#ifndef __IPCBC_CLIENT_H__
#define __IPCBC_CLIENT_H__

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
#define ZEBRA_MAX_PACKET_SIZ          4096

/* Zebra header size. */
#define ZEBRA_HEADER_SIZE             6

/* Structure for the zebra client. */
struct zclient
{
  /* Socket to zebra daemon. */
  int sock;

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

  /* Read and ipstack_connect thread. */
  struct thread *t_read;
  struct thread *t_connect;

  /* Thread to write buffered data to zebra. */
  struct thread *t_write;

  /* Redistribute information. */
  zpl_uchar redist_default;
  zpl_uchar redist[ZEBRA_ROUTE_MAX];

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

extern int  zclient_socket_connect (struct zclient *);
extern void zclient_serv_path_set  (zpl_char *path);
extern const char *const zclient_serv_path_get (void);

/* Send redistribute command to zebra daemon. Do not update zclient state. */
extern int zebra_redistribute_send (zpl_uint16 command, struct zclient *, zpl_uint32 type);

/* If state has changed, update state and call zebra_redistribute_send. */
extern void zclient_redistribute (zpl_uint16 command, struct zclient *, zpl_uint32 type);

/* If state has changed, update state and ipstack_send the command to zebra. */
extern void zclient_redistribute_default (zpl_uint16 command, struct zclient *);

/* Send the message in zclient->obuf to the zebra daemon (or enqueue it).
   Returns 0 for success or -1 on an I/O error. */
extern int zclient_send_message(struct zclient *);

/* create header for command, length to be filled in by user later */
extern void zclient_create_header (struct stream *, zpl_uint16);

extern struct interface *zebra_interface_add_read (struct stream *);
extern struct interface *zebra_interface_state_read (struct stream *s);
extern struct connected *zebra_interface_address_read (zpl_uint16, struct stream *);
extern void zebra_interface_if_set_value (struct stream *, struct interface *);
extern void zebra_router_id_update_read (struct stream *s, struct prefix *rid);
extern int zapi_ipv4_route (zpl_uint16, struct zclient *, struct prefix_ipv4 *, 
                            struct zapi_ipv4 *);

#ifdef HAVE_IPV6
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
#endif /* HAVE_IPV6 */
 
#ifdef __cplusplus
}
#endif

#endif /* __IPCBC_CLIENT_H__ */
