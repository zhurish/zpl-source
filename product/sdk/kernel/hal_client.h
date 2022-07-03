#ifndef __HAL_CLIENT_H__
#define __HAL_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_types.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_util.h"
#include "hal_netlink.h"


typedef struct bsp_driver
{
    zpl_void *hal_client;
    //bsp private data
    zpl_phyport_t 	cpu_port;
	  struct sdk_driver_port	phyports[PHY_PORT_MAX];
	  zpl_uint32	mac_cache_max;
	  zpl_uint32	mac_cache_num;
	  hal_mac_cache_t *mac_cache_entry;

    void *sdk_driver;
    
} bsp_driver_t;
  /* Structure for the zebra client. */
  struct hal_client
  {
    struct hal_ipcmsg ipcmsg;
    struct hal_ipcmsg outmsg;
    zpl_uint32 debug;

    void *bsp_driver;

    struct hal_netlink *netlink;
  };

 #define BSP_DRIVER(bspdev, bsp)    bsp_driver_t *bspdev =  (bsp_driver_t*)(bsp)

  /*
   *   hal client   ------------------- > bsp driver -------------------> sdk driver
   *
   */

  /* Prototypes of zebra client service functions. */
  extern struct hal_client *hal_client_create(void *sdk_driver);
  extern int hal_client_destroy(struct hal_client *hal_client);
  extern int hal_client_send_return(struct hal_client *hal_client, int ret, char *fmt, ...);
  extern int hal_client_send_result(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue);
  extern int hal_client_send_result_msg(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue, 
    int subcmd, char *msg, int len);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_CLIENT_H__ */
