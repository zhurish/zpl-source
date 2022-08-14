#ifndef __IPCBC_SERV_H__
#define __IPCBC_SERV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "ipcbc.h"



/* standby server structure. */
struct ipcbc_serv
{
  /* file descriptor. */
  zpl_socket_t sock;
  /* Input/output buffer to the client. */
  struct stream *ibuf;
  struct stream *obuf;

  zpl_void *t_read;
  zpl_void *t_timeout;

  zpl_uint32 proto;
  zpl_uint32 slot;

  zpl_uint32 hello;
  zpl_bool  state;

  char version[64];
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

struct ipcbc_server_t
{
  struct list *client_list;
  zpl_void *master;    
  zpl_void *t_accept;  
  zpl_socket_t serv_sock; 

  zpl_socket_t ack_sock[2];
  struct stream *ackbuf;
  struct timeval  wait_timeval;

  struct ipcbc_callback set_callback;
  struct ipcbc_callback get_callback;

  struct ipcbc_serv *client;
};

extern struct ipcbc_server_t ipcbc_server;

extern void ipcbc_serv_init(void * m);
extern void ipcbc_serv_exit(void);

extern int ipcbc_serv_callback(struct ipcbc_callback msg, struct ipcbc_callback res);

extern int ipcbc_serv_result(struct ipcbc_serv *client, int ret, char *data, int len);
extern int ipcbc_serv_msg_set(struct ipcbc_serv *client, char *data, int len);
extern int ipcbc_serv_msg_get(struct ipcbc_serv *client, char *data, int len);

extern int ipcbc_serv_recv_ack(struct ipcbc_serv *client, struct ipcbc_result *ack,
                                          struct ipcbc_callback *callback, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* __IPCBC_SERV_H__ */
