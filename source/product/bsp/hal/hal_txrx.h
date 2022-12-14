
#ifndef __BSP_TXRX_H__
#define __BSP_TXRX_H__

#include "lib_netlink.h"



#define HAL_DATA_REQUEST_CMD (0x29)
#define HAL_DATA_NETLINK_PROTO (29)
#define HAL_ETHMAC_HEADER   12


#ifdef ZPL_SDK_BCM53125
typedef struct
{
#if BYTE_ORDER == LITTLE_ENDIAN
    zpl_uint32       srcport:5;  
    zpl_uint32       tr:1;
    zpl_uint32       reserved:21;  
    zpl_uint32       opcode:3;
#else
    zpl_uint32       opcode:3;
    zpl_uint32       reserved:21;  
    zpl_uint32       tr:1; 
    zpl_uint32       srcport:5;  
#endif    
    zpl_uint32       timestamp;       
}hw_hdr_rx_time_t __attribute__ ((packed));
typedef struct
{
#if BYTE_ORDER == LITTLE_ENDIAN
    zpl_uint32       srcport:5;
    zpl_uint32       tc:3;
    zpl_uint32       reason:8; 
    zpl_uint32       reserved:13;  
    zpl_uint32       opcode:3;
#else
    zpl_uint32       opcode:3;
    zpl_uint32       reserved:13; 
    zpl_uint32       reason:8; 
    zpl_uint32       tc:3; 
    zpl_uint32       srcport:5; 
#endif  
}hw_hdr_rx_normal_t __attribute__ ((packed));

typedef struct
{
#if BYTE_ORDER == LITTLE_ENDIAN
    zpl_uint32       reserved:24;
    zpl_uint32       te:2;
    zpl_uint32       tc:3;  
    zpl_uint32       opcode:3;
#else  
    zpl_uint32       opcode:3;
    zpl_uint32       tc:3; 
    zpl_uint32       te:2; 
    zpl_uint32       reserved:24; 
#endif         
}hw_hdr_tx_flood_t __attribute__ ((packed));

typedef struct
{
#if BYTE_ORDER == LITTLE_ENDIAN
    zpl_uint32       dstport:23; 
    zpl_uint32       ts:1; 
    zpl_uint32       te:2; 
    zpl_uint32       tc:3; 
    zpl_uint32       opcode:3;
#else    
    zpl_uint32       opcode:3;
    zpl_uint32       tc:3; 
    zpl_uint32       te:2; 
    zpl_uint32       ts:1; 
    zpl_uint32       dstport:23; 
#endif        
}hw_hdr_tx_normal_t __attribute__ ((packed));

typedef struct
{
    union 
    {
        zpl_uint32  uihdr;
        zpl_uint8   uchdr[4];
        union 
        {
            hw_hdr_rx_normal_t rx_normal;
            hw_hdr_rx_time_t   rx_time;
        }rx_pkt;
        union 
        {
            hw_hdr_tx_flood_t  tx_flood;
            hw_hdr_tx_normal_t tx_normal;
        }tx_pkt;
    }hdr_pkt;

}hw_hdr_t __attribute__ ((packed));
#endif

typedef struct hal_txrx
{
    zpl_void *master;
    zpl_uint32 taskid;
    lib_netlink_t   *netlink_data;
    struct thread *t_read;
    struct thread *t_write;
    int debug;
}hal_txrx_t;


extern int hal_txrx_module_init(void);
extern int hal_txrx_module_task_init(void);
extern int hal_txrx_module_exit(void);
extern int hal_txrx_module_task_exit(void);
extern int hal_txrx_vlan_add(char *data, int len, vlan_t vlan, int cfi, int pri);
extern int hal_txrx_vlan_del(char *data, int len, zpl_skb_vlanhdr_t *vlan);
extern int hal_txrx_sendto_port(char *data, int len, int flags, int cosq, int dportmap, vlan_t vlan, int cfi, int pri);
extern int hal_txrx_sendto_cpu(hal_txrx_t *txrx, char *data, int len, int ifindex);

#endif /* __BSP_TXRX_H__ */
