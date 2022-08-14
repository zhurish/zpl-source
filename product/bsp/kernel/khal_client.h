#ifndef __KHAL_CLIENT_H__
#define __KHAL_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "kbsp_types.h"
#include "khal_ipccmd.h"
#include "khal_ipcmsg.h"
#include "khal_util.h"
#include "khal_netlink.h"

#define KHAL_CLIENT_DEVICE_NAME	    "halclient"
#define KHAL_CLIENT_MODULE_NAME 	"halclient"

#ifdef ZPL_SDK_KERNEL

typedef struct kbsp_driver
{
    zpl_void *khal_client;
    //bsp private data
    zpl_phyport_t 	cpu_port;
	struct sdk_driver_port	phyports[PHY_PORT_MAX];
	zpl_uint32	mac_cache_max;
	zpl_uint32	mac_cache_num;
	khal_mac_cache_t *mac_cache_entry;
    void *sdk_driver;
} kbsp_driver_t;
#endif

/* Structure for the zebra client. */
struct khal_client
{
    dev_t dev_num;
	struct class *class;
	struct device *dev;

    struct khal_ipcmsg ipcmsg;
    struct khal_ipcmsg outmsg;
    zpl_uint32 debug;

    void *kbsp_driver;

    struct khal_netlink *netlink;
};

#ifdef ZPL_SDK_KERNEL
#define BSP_DRIVER(bspdev, bsp)    khal_client *bspdev =  (kbsp_driver_t*)(bsp)
#else
#define BSP_DRIVER(bspdev, bsp)
#endif



  /*
   *   hal client   ------------------- > bsp driver -------------------> sdk driver
   *
   */

  /* Prototypes of zebra client service functions. */
extern struct khal_client *khal_client_create(void *sdk_driver);
extern int khal_client_destroy(struct khal_client *khal_client);
extern int khal_client_send_return(struct khal_client *khal_client, int ret, char *fmt, ...);
extern int khal_client_send_result(struct khal_client *khal_client, int ret, struct khal_ipcmsg_result *getvalue);
extern int khal_client_send_result_msg(struct khal_client *khal_client, int ret, struct khal_ipcmsg_result *getvalue, 
    int subcmd, char *msg, int len);
extern int khal_client_send_report(struct khal_client *khal_client, char *data, int len);

#ifdef __cplusplus
}
#endif

#endif /* __KHAL_CLIENT_H__ */
