#ifndef __HAL_CLIENT_H__
#define __HAL_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_types.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"



#define HAL_CFG_NETLINK_PROTO (30)
#define HAL_DATA_NETLINK_PROTO (29)
  /* Structure for the zebra client. */
  struct hal_client
  {
    struct hal_ipcmsg ipcmsg;
    struct hal_ipcmsg outmsg;
    zpl_uint32 debug;

    void *bsp_driver;

    struct sock *nlsock;
    int cmd;
    int seqno;
    int pid;
  };

 

  /*
   *   hal client   ------------------- > bsp driver -------------------> sdk driver
   *
   */

  /* Prototypes of zebra client service functions. */
  extern struct hal_client *hal_client_create(void *bsp_driver);
  extern int hal_client_destroy(struct hal_client *hal_client);
  extern int hal_client_send_return(struct hal_client *hal_client, int ret, char *fmt, ...);
  extern int hal_client_send_result(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue);
  extern int hal_client_send_result_msg(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue, 
    int subcmd, char *msg, int len);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_CLIENT_H__ */
