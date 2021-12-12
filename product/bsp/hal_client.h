#ifndef __HAL_CLIENT_H__
#define __HAL_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "os_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

  /* Structure for the zebra client. */
  struct hal_client
  {
    /* Socket to zebra daemon. */
    zpl_socket_t sock;
    zpl_int8 type;
    zpl_int8 unit;
    zpl_int8 slot;
    zpl_int8 portnum;
    char version[128];
    zpl_uint8 module;
    zpl_uint16 port;
    enum hal_client_state state;
    zpl_uint8 fail;
    zpl_uint32  timeout;
    /* Read and connect thread. */
    struct thread *t_read;
    struct thread *t_connect;
    struct thread *t_time;
    struct thread_master *master;
    struct hal_ipcmsg ipcmsg;
    struct hal_ipcmsg outmsg;
    zpl_uint32 debug;
  };


typedef struct 
{
	zpl_void	*master;
	zpl_uint32 taskid;
  zpl_void	*halbsp;
}hal_bsp_t;

/* Prototypes of zebra client service functions. */
struct hal_client *hal_client_create(int module);
int hal_client_init(struct hal_client *, zpl_int8 module, zpl_int8 unit, zpl_int8 slot, zpl_int8 portnum);
int hal_client_destroy(struct hal_client *hal_client);
int hal_client_start(struct hal_client *hal_client);
int hal_client_send_return(struct hal_client *hal_client, int ret, char *fmt,...);
int hal_client_register(struct hal_client *hal_client, struct hal_ipcmsg_porttbl *porttbl);

int hal_bsp_init(void);
int hal_bsp_task_init(void);
int hal_bsp_exit(void);
int hal_bsp_task_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_CLIENT_H__ */
