
#ifndef __BSP_TXRX_H__
#define __BSP_TXRX_H__

#include "bsp_netlink.h"


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
}hw_hdr_rx_time_t __attribute__ ((aligned (1)));
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
}hw_hdr_rx_normal_t __attribute__ ((aligned (1)));

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
}hw_hdr_tx_flood_t __attribute__ ((aligned (1)));

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
}hw_hdr_tx_normal_t __attribute__ ((aligned (1)));

typedef struct
{
    union 
    {
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

}hw_hdr_t __attribute__ ((aligned (1)));

typedef struct
{
    zpl_uint16      vlantype; 
#if BYTE_ORDER == LITTLE_ENDIAN
    zpl_uint16       vid:12;
    zpl_uint16       cfi:1; 
    zpl_uint16       pri:3; 
#else    
    zpl_uint16       pri:3; 
    zpl_uint16       cfi:1; 
    zpl_uint16       vid:12; 
#endif 
    zpl_uint16      ethtype; 
}vlan_hdt_t __attribute__ ((aligned (1)));

typedef struct
{
    zpl_uint8       dmac[ETH_ALEN]; 
    zpl_uint8       smac[ETH_ALEN]; 
    union 
    {
        zpl_uint16      ethtype; 
        vlan_hdt_t      vlanhdr;
    }ethhdr;

}ethhdr_vlan_t __attribute__ ((aligned (1)));

typedef struct bsp_txrx
{
    zpl_void *master;
    zpl_uint32 taskid;
    bsp_netlink_t   *netlink_data;
    struct thread *t_read;
    struct thread *t_write;
    int debug;
}bsp_txrx_t;


int bsp_module_txrx_init(void);
int bsp_module_txrx_task_init(void);
int bsp_module_txrx_exit(void);
int bsp_module_txrx_task_exit(void);

int bsp_txrx_sendto_port(char *data, int len, int flags, int cosq, int dportmap, vlan_t vlan, int cfi, int pri);
int bsp_txrx_vlan_flood(char *data, int len, int flags, int cosq, vlan_t vlan, int cfi, int pri);


#endif /* __BSP_TXRX_H__ */
