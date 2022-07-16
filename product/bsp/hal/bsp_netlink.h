
#ifndef __BSP_NETLINK_H__
#define __BSP_NETLINK_H__

#include "zplos_include.h"

#define HAL_CFG_REQUEST_CMD (30) 
#define HAL_DATA_REQUEST_CMD (29)
#define HAL_KLOG_REQUEST_CMD (28)

#define HAL_CFG_NETLINK_PROTO (30)
#define HAL_DATA_NETLINK_PROTO (29)
#define HAL_KLOG_NETLINK_PROTO (28)

typedef struct bsp_netlink_s 
{
    zpl_socket_t    sock;
    int             seq;
    int             debug;
    zpl_uint32      msgmax;
    zpl_uint8       *msgbuf;
    zpl_uint32      msglen;
    zpl_uint32      msgoffset;
}bsp_netlink_t;

extern bsp_netlink_t * bsp_netlink_create(int maxsize, int msgoffset);
extern int bsp_netlink_destroy(bsp_netlink_t *nsock);
extern int bsp_netlink_open(bsp_netlink_t * nsock, int proto);
extern int bsp_netlink_close(bsp_netlink_t * nsock);
extern int bsp_netlink_talk(bsp_netlink_t * nsock, struct ipstack_nlmsghdr *n, int (*filter)(void *, int, char *, int, void *), void *p);
extern int bsp_netlink_send(bsp_netlink_t *nsock, struct ipstack_nlmsghdr *n);
extern int bsp_netlink_recv(bsp_netlink_t *nsock);
extern int bsp_netlink_msg_callback(bsp_netlink_t *nsock, int (*filter)(void *, int, char *, int, void *), void *p);

#endif /* __BSP_NETPKT_H__ */
