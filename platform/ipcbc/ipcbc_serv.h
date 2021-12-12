#ifndef __IPCBC_SERV_H__
#define __IPCBC_SERV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_rib.h"
#include "if.h"
#include "workqueue.h"
#include "nsm_vrf.h"
#include "route_types.h"
/* Default port information. */
//#define ZEBRA_VTY_PORT                2610

/* Default configuration filename. */
//#define DEFAULT_CONFIG_FILE "zebra.conf"

/* Client structure. */
struct zserv
{
  /* Client file descriptor. */
  int sock;

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

  /* This client's redistribute flag. */
  vrf_bitmap_t redist[ZEBRA_ROUTE_MAX];

  /* Redistribute default route flag. */
  vrf_bitmap_t redist_default;

  /* Interface information. */
  vrf_bitmap_t ifinfo;

  /* Router-id information. */
  vrf_bitmap_t ridinfo;

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
struct zebra_t
{
  /* Thread master */
  struct thread_master *master;
  struct list *client_list;

  /* default table */
  zpl_uint32 rtm_table_default;

  /* rib work queue */
  struct work_queue *ribq;
  struct meta_queue *mq;
};

//extern struct zebra_t zebrad;

/* Prototypes. */
extern void zebra_init (void);
extern void zebra_if_init (void);
extern void zebra_zserv_socket_init (zpl_char *path);

extern void hostinfo_get (void);
extern void rib_init (void);

extern void kernel_load(struct nsm_vrf *zvrf);
extern void kernel_load_all();


extern void zebra_route_map_init (void);
extern void zebra_snmp_init (void);
extern void zebra_vty_init (void);

extern int zsend_interface_add (struct zserv *, struct interface *);
extern int zsend_interface_delete (struct zserv *, struct interface *);
extern int zsend_interface_address (zpl_uint16, struct zserv *, struct interface *,
                                    struct connected *);
extern int zsend_interface_state (zpl_uint16, struct zserv *, struct interface *);
extern int zsend_interface_mode (struct zserv *, struct interface *, zpl_uint32 mode);


extern int zsend_route_multipath (zpl_uint16, struct zserv *, struct prefix *, 
                                  struct rib *);
extern int zsend_router_id_update (struct zserv *, struct prefix *,
                                   vrf_id_t);

//extern int zsend_interface_link_params (struct zserv *, struct interface *);

//extern pid_t pid;

extern void zserv_create_header(struct stream *s, zpl_uint16 cmd, vrf_id_t);
extern int zebra_server_send_message(struct zserv *client);


 
#ifdef __cplusplus
}
#endif

#endif /* __IPCBC_SERV_H__ */
