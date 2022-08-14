#ifndef __IPCSTANDBY_SERV_H__
#define __IPCSTANDBY_SERV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "ipcstandby.h"


struct ipcstandby_server_t;

/* standby server structure. */
struct ipcstandby_serv
{
  /* file descriptor. */
  zpl_socket_t sock;
  /* Input/output buffer to the client. */
  struct stream *ibuf;
  struct stream *obuf;

  zpl_void *t_read;
  zpl_void *t_timeout;

  zpl_uint32 slot;
  zpl_uint32 hello;
  zpl_bool  state;

  char version[64];
  struct prefix remote;
  int debug;

  struct ipcstandby_server_t  *ipcserver;

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

struct ipcstandby_server_t
{
  struct list *client_list;
  zpl_void *master;    
  zpl_void *t_accept;  
  zpl_socket_t serv_sock; 
  struct ipcstandby_callback cli_callback;
  struct ipcstandby_callback msg_callback;
  struct ipcstandby_callback res_callback;

  struct ipcstandby_serv *client;
  struct vty *vty;
};

extern struct ipcstandby_server_t *ipcstandby_serv_init(void * m);
extern void ipcstandby_serv_exit(struct ipcstandby_server_t *);

extern int ipcstandby_serv_callback(struct ipcstandby_server_t *, struct ipcstandby_callback cli, struct ipcstandby_callback msg, struct ipcstandby_callback res);

extern int  ipcstandby_serv_result(struct ipcstandby_serv *client, int process, int ret, char *data, int len);

 
#ifdef __cplusplus
}
#endif

#endif /* __IPCSTANDBY_SERV_H__ */
