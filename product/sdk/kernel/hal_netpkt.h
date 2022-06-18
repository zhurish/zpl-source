#ifndef __HAL_NETPKT_H__
#define __HAL_NETPKT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_types.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

enum zpl_netpkt_reason 
{
    NETPKT_REASON_NONE,
    NETPKT_REASON_COPYTOCPU,
	NETPKT_REASON_TONETDEV,
};

typedef struct
{
    zpl_uint8       unit;                         /* Unit number. */
    zpl_uint8       cos;                          /* The local COS queue to use. */
    zpl_uint8       prio_int;                     /* Internal priority of the packet. */
    vlan_t          vlan;                    /* 802.1q VID or VSI or VPN. */
    zpl_uint8       vlan_pri;                     /* Vlan tag priority . */
    zpl_uint8       vlan_cfi;                     /* Vlan tag CFI bit. */
    vlan_t          inner_vlan;              /* Inner VID or VSI or VPN. */
    zpl_uint8       inner_vlan_pri;               /* Inner vlan tag priority . */
    zpl_uint8       inner_vlan_cfi;               /* Inner vlan tag CFI bit. */
    zpl_uint16      ethtype;

    zpl_uint8       color;                  /* Packet color. */
    zpl_uint32      reason;         /* Opcode from packet. */
    zpl_uint8       untagged;       /* The packet was untagged on ingress. */
    ifindex_t       ifindex;
    zpl_phyport_t   trunk;          /* Source trunk group ID used in header/tag, -1 if src_port set . */
    zpl_phyport_t   phyid;          /* Source port used in header/tag. */

    volatile zpl_uint32      reference;

}zpl_netpkt_hdr_t __attribute__ ((aligned (1)));

int netpkt_netlink_init(void);
void netpkt_netlink_exit(void);
int netpkt_netlink_debug_set(int set, int val);
int netpkt_netlink_bind(int ifindex, int bind);
int netpkt_netlink_dstpid(int pid);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_NETPKT_H__ */
