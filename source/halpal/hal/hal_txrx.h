
#ifndef __HAL_TXRX_H__
#define __HAL_TXRX_H__


#include "zpl_skbuffer.h"

typedef struct hal_txrx
{
    zpl_void *master;
    zpl_uint32 taskid;
    int rx_fd;
    int tx_fd;
    zpl_skbqueue_t *rx_skbqueue;
    zpl_skbqueue_t *tx_skbqueue;
    void    *t_read;
    int debug;
}hal_txrx_t;


extern int hal_txrx_module_init(void);
extern int hal_txrx_module_task_init(void);
extern int hal_txrx_module_exit(void);
extern int hal_txrx_module_task_exit(void);
/* 从交换芯片获取数据并发送到接收队列 */
extern int hal_txrx_recvfrom_switchdev(int unit, char *data, int len, ifindex_t ifindex, int reason);
/* 从网络驱动获取数据并发送到发送队列 */
extern int hal_txrx_recvfrom_netdevice(int unit, char *data, int len, ifkernindex_t ifindex);

extern int hal_txrx_sendto_switchdev(int unit, char *data, int len, ifindex_t ifindex, int reason);
extern int hal_txrx_sendto_netdevice(int unit, char *data, int len, ifkernindex_t ifindex);

extern int hal_txrx_vlan_add(char *data, int len, vlan_t vlan, int cfi, int pri);
extern int hal_txrx_vlan_del(char *data, int len, zpl_skb_vlanhdr_t *vlan);

#endif /* __HAL_TXRX_H__ */
