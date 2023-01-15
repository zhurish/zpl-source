#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"

#include "nsm_include.h"
#include "hal_txrx.h"
#include "hal_netpkt.h"

hal_txrx_t hal_txrx;

struct module_list module_list_txrx =
{
  .module = MODULE_TXRX,
  .name = "TXRX\0",
  .module_init = hal_txrx_module_init,
  .module_exit = hal_txrx_module_exit,
  .module_task_init = hal_txrx_module_task_init,
  .module_task_exit = hal_txrx_module_task_exit,
  .module_cmd_init = NULL,
  .taskid = 0,
};

static int hal_txrx_sync(hal_txrx_t *txrx);

int hal_txrx_vlan_add(char *data, int len, vlan_t vlan, int cfi, int pri)
{
  zpl_skb_vlanhdr_t *vlanhdr = (zpl_skb_vlanhdr_t *)data;
  vlanhdr->vlantype = htons(0x8100);
  vlanhdr->vid = vlan;
  vlanhdr->cfi = cfi;
  vlanhdr->pri = pri;
  //zlog_debug(MODULE_TXRX,"====DEV SEND VLAN HDR: 0x%02x 0x%02x 0x%02x 0x%02x", data[0], data[1], data[2], data[3]);  
  return 4;
}

int hal_txrx_vlan_del(char *data, int len, zpl_skb_vlanhdr_t *vlan)
{
  zpl_skb_vlanhdr_t *vlanhdr = (zpl_skb_vlanhdr_t *)(data + 2*ETH_ALEN);
  if (vlan)
  {
    vlan->vlantype = ntohs(vlanhdr->vlantype);
    vlan->vid = vlanhdr->vid;
    vlan->cfi = vlanhdr->cfi;
    vlan->pri = vlanhdr->pri;
    //zlog_debug(MODULE_TXRX,"====DEV GET VLAN HDR: vlantype = 0x%x vid=%d cfi=%d pri=%d", vlan->vlantype, vlan->vid, vlan->cfi, vlan->pri);
  }
  if (ntohs(vlanhdr->vlantype) == 0x8100)
  {
    memmove(data + 4, data , 2*ETH_ALEN);  
    return 4;
  }
  return 0;
}

/* 从网络驱动获取数据并发送到交换芯片 */
int hal_txrx_recvfrom_netdevice(int unit, char *data, int len, ifkernindex_t ifindex)
{
  if (hal_txrx.tx_skbqueue)
  {
    zpl_skbuffer_t *skb = zpl_skbuffer_create(ZPL_SKBUF_TYPE_NETPKT, hal_txrx.tx_skbqueue, len);
    if (skb)
    {
      zpl_skb_vlanhdr_t vlan;
      zpl_skbuffer_put(skb, data, len);
      if (IF_IFINDEX_TYPE_GET(ifindex) == IF_LAG)
        zpl_skbuffer_source_set(skb, 0, ifindex, if_ifindex2phy(ifindex), 0);
      else 
        zpl_skbuffer_source_set(skb, 0, ifindex, 0, if_ifindex2phy(ifindex));
      zpl_skbuffer_reason_set(skb, NETPKT_REASON_NONE);
      hal_txrx_vlan_add((char *)&vlan, 4, 100, 0, 0);
      zpl_skbuffer_push(skb, 12, (uint8_t *)&vlan, 4);
      zpl_skbqueue_add(hal_txrx.tx_skbqueue, skb);
      return hal_txrx_sync(&hal_txrx);
    }
  }
  return ERROR;
}


/* 从交换芯片获取数据并发送到网络驱动 */
int hal_txrx_recvfrom_switchdev(int unit, char *data, int len, ifindex_t ifindex, int reason)
{
  int olen = 0;
  if (hal_txrx.rx_skbqueue)
  {
    zpl_skbuffer_t *skb = zpl_skbuffer_create(ZPL_SKBUF_TYPE_NETPKT, hal_txrx.rx_skbqueue, len);
    if (skb)
    {
      if (IF_IFINDEX_TYPE_GET(ifindex) == IF_LAG)
        zpl_skbuffer_source_set(skb, 0, ifindex, if_ifindex2phy(ifindex), 0);
      else
        zpl_skbuffer_source_set(skb, 0, ifindex, 0, if_ifindex2phy(ifindex));
      zpl_skbuffer_reason_set(skb, reason);

      zpl_skbuffer_put(skb, data, len - olen);
      zpl_skbqueue_add(hal_txrx.rx_skbqueue, skb);
      return hal_txrx_sync(&hal_txrx);
    }
  }
  return ERROR;
}

int hal_txrx_sendto_switchdev(int unit, char *data, int len, ifindex_t ifindex, int reason)
{
  return OK;
}

int hal_txrx_sendto_netdevice(int unit, char *data, int len, ifkernindex_t ifindex)
{
  return OK;
}

static int hal_txrx_sync(hal_txrx_t *txrx)
{
  zpl_char buftmp[8];
  if(txrx && txrx->tx_fd)
    return os_write_timeout(txrx->tx_fd, buftmp, 4, 100);
  return ERROR; 
}
static int hal_txrx_thread(os_ansync_t *thread)
{
  hal_txrx_t *txrx = OS_ANSYNC_ARGV(thread);
  zpl_char buftmp[512];
  zpl_skbuffer_t *rx_skb = NULL;
  zpl_skbuffer_t *tx_skb = NULL;
  struct interface *ifp = NULL;
  if (txrx && txrx->rx_fd)
  {
    if(os_read_timeout(txrx->rx_fd, buftmp, 4, 100) > 0)
    {
      rx_skb = zpl_skbqueue_get(txrx->rx_skbqueue);
      if (rx_skb)
      {
        // TODO
        ifp = if_lookup_by_index(rx_skb->skb_hdr.net_hdr.ifindex);
        if(ifp)
        {
          if(if_is_l3intf(ifp))
            hal_txrx_sendto_netdevice(0, ZPL_SKB_DATA(rx_skb), ZPL_SKB_DATA_LEN(rx_skb), ifindex2ifkernel(rx_skb->skb_hdr.net_hdr.ifindex));
          else
          {
            ifp = if_lookup_by_index(vlanif2ifindex(rx_skb->skb_hdr.net_hdr.vlan));
            if(ifp && if_is_vlan(ifp))
              hal_txrx_sendto_netdevice(0, ZPL_SKB_DATA(rx_skb), ZPL_SKB_DATA_LEN(rx_skb), ifp->ker_ifindex);
          }  
        }
        zpl_skbqueue_finsh(txrx->rx_skbqueue, rx_skb);
      }
      tx_skb = zpl_skbqueue_get(txrx->tx_skbqueue);
      if (tx_skb)
      {
        // TODO
        //hal_txrx_sendto_switchdev();
        zpl_skbqueue_finsh(txrx->rx_skbqueue, tx_skb);
      }
    }
  }
  return 0;
}

static int hal_txrx_task(void *pVoid)
{
  os_ansync_lst *master = (struct os_ansync_lst *)pVoid;
  module_setup_task(master->module, os_task_id_self());
  os_ansync_main(master, OS_ANSYNC_EXECUTE_ARGV);
  return OK;
}

int hal_txrx_module_init(void)
{
    memset(&hal_txrx, 0, sizeof(hal_txrx));
    if (!hal_txrx.master)
    {
      hal_txrx.master = os_ansync_lst_create(MODULE_TXRX, 5);
    }
    hal_txrx.debug = 0xffffff;
    if(os_unix_sockpair_create(zpl_true, &hal_txrx.rx_fd, &hal_txrx.tx_fd) == OK)
    {
      hal_txrx.rx_skbqueue = zpl_skbqueue_create("halrx_skbqueue", 512, zpl_false);
      hal_txrx.tx_skbqueue = zpl_skbqueue_create("haltx_skbqueue", 512, zpl_false);

      if (hal_txrx.tx_skbqueue == NULL)
      {
        if (hal_txrx.rx_skbqueue)
          zpl_skbqueue_destroy(hal_txrx.rx_skbqueue);
        if (hal_txrx.tx_fd)
          close(hal_txrx.tx_fd);
        return ERROR;
      }
      if (hal_txrx.rx_skbqueue == NULL)
      {
        if (hal_txrx.tx_skbqueue)
          zpl_skbqueue_destroy(hal_txrx.tx_skbqueue);
        if (hal_txrx.rx_fd)
          close(hal_txrx.rx_fd);
        return ERROR;
      }
      hal_netpkt_filter_init();
      hal_txrx.t_read = os_ansync_add(hal_txrx.master, OS_ANSYNC_INPUT, hal_txrx_thread, &hal_txrx, hal_txrx.rx_fd);
      return OK;
    }
    return ERROR;
}

int hal_txrx_module_task_init(void)
{
    if (!hal_txrx.master)
    {
      hal_txrx.master = os_ansync_lst_create(MODULE_TXRX, 5);
    }
    if (hal_txrx.taskid <= 0)
        hal_txrx.taskid = os_task_create("txrxTask", OS_TASK_DEFAULT_PRIORITY,
                                         0, hal_txrx_task, hal_txrx.master, OS_TASK_DEFAULT_STACK * 4);
    if (hal_txrx.taskid > 0)
    {
        module_setup_task(MODULE_TXRX, hal_txrx.taskid);
        return OK;
    }
    return ERROR;
}

int hal_txrx_module_exit(void)
{
  if (hal_txrx.t_read)
  {
    os_ansync_cancel(hal_txrx.master, hal_txrx.t_read);
    hal_txrx.t_read = NULL;
  }
  hal_netpkt_filter_exit();
  if (hal_txrx.tx_skbqueue == NULL)
  {
    if (hal_txrx.rx_skbqueue)
      zpl_skbqueue_destroy(hal_txrx.rx_skbqueue);
  }
  if (hal_txrx.rx_skbqueue == NULL)
  {
    if (hal_txrx.tx_skbqueue)
      zpl_skbqueue_destroy(hal_txrx.tx_skbqueue);
  }
  if (hal_txrx.tx_fd)
    close(hal_txrx.tx_fd);
  if (hal_txrx.rx_fd)
    close(hal_txrx.rx_fd);
  if (hal_txrx.master)
	{
    	thread_master_free(hal_txrx.master);
    	hal_txrx.master = NULL;
	}   
  return OK;
}

int hal_txrx_module_task_exit(void)
{
    if (hal_txrx.taskid > 0)
        os_task_destroy(hal_txrx.taskid);
    if (hal_txrx.master)
    {
      os_ansync_lst_destroy(hal_txrx.master);
      hal_txrx.master = NULL;
    }
    return OK;
}