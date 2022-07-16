#ifndef __KHAL_CLIENT_H__
#define __KHAL_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_types.h"
#include "khal_ipccmd.h"
#include "khal_ipcmsg.h"
#include "khal_util.h"
#include "khal_netlink.h"

#define KHAL_CLIENT_DEVICE_NAME	    "halclient"
#define KHAL_CLIENT_MODULE_NAME 	"halclient"

#ifdef ZPL_SDK_KERNEL

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
#endif

/* Structure for the zebra client. */
struct hal_client
{
    dev_t dev_num;
	struct class *class;
	struct device *dev;

    struct hal_ipcmsg ipcmsg;
    struct hal_ipcmsg outmsg;
    zpl_uint32 debug;

    void *bsp_driver;

    struct hal_netlink *netlink;
};

#ifdef ZPL_SDK_KERNEL
#define BSP_DRIVER(bspdev, bsp)    hal_client *bspdev =  (bsp_driver_t*)(bsp)
#else
#define BSP_DRIVER(bspdev, bsp)
#endif



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

#endif /* __KHAL_CLIENT_H__ */
