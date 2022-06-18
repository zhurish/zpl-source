#ifndef __HAL_CLIENT_H__
#define __HAL_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_types.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_netlink.h"



  /* Structure for the zebra client. */
  struct hal_client
  {
    struct hal_ipcmsg ipcmsg;
    struct hal_ipcmsg outmsg;
    zpl_uint32 debug;

    void *bsp_driver;

    struct hal_netlink *netlink;
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
