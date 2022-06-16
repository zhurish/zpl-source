#ifndef __IPCSTANDBY_CLIENT_H__
#define __IPCSTANDBY_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "ipcstandby.h"

/* Structure for the ipcstandby client. */
struct ipcstandby_client
{
   zpl_uint32 slot;
  /* Socket to ipcstandby daemon. */
  zpl_socket_t sock;

  /* Flag of communication to ipcstandby is enabled or not.  Default is on.
     This flag is disabled by `no router ipcstandby' statement. */
  zpl_bool enable;

  /* Connection failure count. */
  zpl_uint32 fail;

  /* Input buffer for ipcstandby message. */
  struct stream *ibuf;

  /* Output buffer for ipcstandby message. */
  struct stream *obuf;

  /* Read and ipstack_connect thread. */
  zpl_bool is_tcp;

  zpl_void *master;  
  zpl_void *t_connect;
  zpl_void *t_timeout;
  zpl_bool state;
  /* Thread to write buffered data to ipcstandby. */
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

extern struct ipcstandby_client *ipcstandby_client;


/* Prototypes of ipcstandby client service functions. */
extern struct ipcstandby_client *ipcstandby_client_new (void*m);
extern void ipcstandby_client_init (struct ipcstandby_client *, zpl_uint32);
extern int ipcstandby_client_start (struct ipcstandby_client *);
extern void ipcstandby_client_stop (struct ipcstandby_client *);
extern void ipcstandby_client_reset (struct ipcstandby_client *);
extern void ipcstandby_client_free (struct ipcstandby_client *);


/* Send the message in zclient->obuf to the ipcstandby daemon (or enqueue it).
   Returns 0 for success or -1 on an I/O error. */
extern int ipcstandby_client_send_message(struct ipcstandby_client *client, struct ipcstanby_result *ack,
                                          struct ipcstandby_callback *callback, int timeout);


extern int ipcstandby_client_sendmsg(struct ipcstandby_client *client, struct ipcstanby_result *ack,
                                   struct ipcstandby_callback *callback, int cmd, char *msg, int len);


#ifdef __cplusplus
}
#endif

#endif /* __IPCSTANDBY_CLIENT_H__ */
