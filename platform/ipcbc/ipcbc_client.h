#ifndef __IPCBC_CLIENT_H__
#define __IPCBC_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "ipcbc.h"

/* Structure for the ipcbc client. */
struct ipcbc_client
{
  /* Socket to ipcbc daemon. */
  zpl_socket_t sock;

  zpl_uint32 proto;
  zpl_uint32 slot;
  /* Connection failure count. */
  zpl_uint32 fail;

  /* Input buffer for ipcbc message. */
  struct stream *ibuf;

  /* Output buffer for ipcbc message. */
  struct stream *obuf;

  /* Read and ipstack_connect thread. */
  zpl_bool is_tcp;

  zpl_socket_t ack_sock[2];
  struct stream *ackbuf;
  struct timeval  wait_timeval;
  struct ipcbc_callback set_callback;
  struct ipcbc_callback get_callback;
  zpl_void *master;  
  zpl_void *t_connect;
  zpl_void *t_timeout;
  zpl_void *t_read;
  zpl_bool state;
  /* Thread to write buffered data to ipcbc. */
  zpl_void *t_write;

  struct prefix remote;
  int debug;

  zpl_time_t connect_time;
  zpl_time_t last_read_time;
  zpl_time_t last_write_time;

  zpl_uint32  recv_cnt;
  zpl_uint32  send_cnt;
  zpl_uint32  recv_faild_cnt;
  zpl_uint32  send_faild_cnt;
  zpl_uint32  connect_cnt;
  zpl_uint32  pkt_err_cnt;
};

extern struct ipcbc_client *ipcbc_client;


/* Prototypes of ipcbc client service functions. */
extern struct ipcbc_client *ipcbc_client_new (void*m);
extern void ipcbc_client_init (struct ipcbc_client *, zpl_uint32, zpl_uint32);
extern int ipcbc_client_start (struct ipcbc_client *);
extern void ipcbc_client_stop (struct ipcbc_client *);
extern void ipcbc_client_reset (struct ipcbc_client *);
extern void ipcbc_client_free (struct ipcbc_client *);




extern int ipcbc_client_result(struct ipcbc_client *client, int ret, char *data, int len);

extern int ipcbc_client_send_message(struct ipcbc_client *client);

extern int ipcbc_client_sendmsg(struct ipcbc_client *client, int cmd, char *msg, int len);

extern int ipcbc_client_recv_message(struct ipcbc_client *client, struct ipcbc_result *ack,
                                          struct ipcbc_callback *callback, int timeout);

extern int ipcbc_client_recv_ack(struct ipcbc_client *client, struct ipcbc_result *ack,
                                          struct ipcbc_callback *callback, int timeout);

extern int ipcbc_client_callback(struct ipcbc_client *client, struct ipcbc_callback msg, struct ipcbc_callback res);

#ifdef __cplusplus
}
#endif

#endif /* __IPCBC_CLIENT_H__ */
