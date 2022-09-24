#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"
#include "vrf.h"
#include "nsm_include.h"
#include "hal_txrx.h"
#include "hal_netpkt.h"

hal_txrx_t hal_txrx;

struct module_list module_list_txrx =
{
  .module = MODULE_TXRX,
  .name = "TXRX\0",
  .module_init = hal_module_txrx_init,
  .module_exit = hal_module_txrx_exit,
  .module_task_init = hal_module_txrx_task_init,
  .module_task_exit = hal_module_txrx_task_exit,
  .module_cmd_init = NULL,
  .taskid = 0,
};

#ifdef ZPL_SDK_BCM53125
#define NETPKT_TOCPU        1
#define NETPKT_FROMDEV      2
#define NETPKT_TOSWITCH     3
#define NETPKT_FROMSWITCH   4

typedef struct
{
    zpl_uint8       cmd;                         /* Unit number. */
    zpl_phyport_t   dstval;          

}hal_txrx_hdr_t __attribute__ ((aligned (1)));



static int hal_txrx_hwhdr_add(char *data, int len, int flags, int cosq, int dportmap)
{
  hw_hdr_t *hwhdr = (hw_hdr_t *)data;
  if(dportmap == -1)
  {
    hwhdr->hdr_pkt.tx_pkt.tx_flood.opcode = 0;
    hwhdr->hdr_pkt.tx_pkt.tx_flood.reserved = 0;
    hwhdr->hdr_pkt.tx_pkt.tx_flood.te = flags;
    hwhdr->hdr_pkt.tx_pkt.tx_flood.tc = cosq;  
  }
  else
  {
    hwhdr->hdr_pkt.tx_pkt.tx_normal.dstport = dportmap; 
    hwhdr->hdr_pkt.tx_pkt.tx_normal.ts = 0; 
    hwhdr->hdr_pkt.tx_pkt.tx_normal.te = flags; 
    hwhdr->hdr_pkt.tx_pkt.tx_normal.tc = cosq; 
    hwhdr->hdr_pkt.tx_pkt.tx_normal.opcode = 0x01;
  }
  return 4;
}
#endif

static int hal_txrx_vlan_add(char *data, int len, vlan_t vlan, int cfi, int pri)
{
  zpl_skb_vlanhdr_t *vlanhdr = (zpl_skb_vlanhdr_t *)data;
  vlanhdr->vlantype = htons(0x8100);
  vlanhdr->vid = vlan;
  vlanhdr->cfi = cfi;
  vlanhdr->pri = pri;
  return 4;
}

static int hal_txrx_vlan_del(char *data, int len, zpl_skb_vlanhdr_t *vlan)
{
  zpl_skb_vlanhdr_t *vlanhdr = (zpl_skb_vlanhdr_t *)(data + 2*ETH_ALEN);
  vlan->vlantype = ntohs(vlanhdr->vlantype);
  vlan->vid = vlanhdr->vid;
  vlan->cfi = vlanhdr->cfi;
  vlan->pri = vlanhdr->pri;
  if(vlan->vlantype == 0x8100)
  {
    memmove(data + 4, data , 2*ETH_ALEN);  
    return 4;
  }
  return 0;
}

#ifdef ZPL_SDK_BCM53125
static int hal_txrx_sendto_netlink(hal_txrx_t *txrx, char *data, int len, int cmd, int ifindex)
{
	struct ipstack_nlmsghdr *nlh = NULL;
	hal_txrx_hdr_t *header = (hal_txrx_hdr_t*)(data - sizeof(hal_txrx_hdr_t));
  nlh = (struct ipstack_nlmsghdr *)(data - sizeof(hal_txrx_hdr_t) - sizeof(struct ipstack_nlmsghdr));
	memset (nlh, 0, sizeof (struct ipstack_nlmsghdr));
  memset (header, 0, sizeof (hal_txrx_hdr_t));
  header->cmd = cmd;
  header->dstval = htonl(ifindex);
  nlh->nlmsg_len = IPSTACK_NLMSG_LENGTH (len+sizeof(hal_txrx_hdr_t));
  nlh->nlmsg_flags = IPSTACK_NLM_F_CREATE | IPSTACK_NLM_F_REQUEST;
  nlh->nlmsg_type = cmd;
	nlh->nlmsg_seq = ++txrx->netlink_data->seq;
	nlh->nlmsg_flags |= IPSTACK_NLM_F_ACK;
	nlh->nlmsg_pid = 0;
	return lib_netlink_send(txrx->netlink_data, nlh);
}
#endif


int hal_txrx_sendto_port(char *data, int len, int flags, int cosq, int dportmap, vlan_t vlan, int cfi, int pri)
{
  int txlen = 0, offset = 64;
  char msgbuf[2048];
  memcpy(msgbuf + offset, data, 2*ETH_ALEN);
  txlen += hal_txrx_vlan_add(msgbuf + offset + 4 + 2*ETH_ALEN,  len,  vlan,  cfi,  pri);
#ifdef ZPL_SDK_BCM53125  
  txlen += hal_txrx_hwhdr_add(msgbuf + offset + 2*ETH_ALEN,  len,  flags,  cosq,  dportmap);
#endif  
  memcpy(msgbuf + offset + 2*ETH_ALEN + txlen, data + 2*ETH_ALEN, len - 2*ETH_ALEN);
  txlen += len;  
#ifdef ZPL_SDK_BCM53125  
  return hal_txrx_sendto_netlink(&hal_txrx, msgbuf + offset,  txlen, NETPKT_TOSWITCH, 0);
#else
  return 0;
#endif  
}



#ifdef ZPL_SDK_BCM53125 
static int hal_txrx_hw_hdr_get(zpl_uint8 *nethdr, hw_hdr_t *hwhdr)
{
  hw_hdr_t *hdr = (hw_hdr_t *)(nethdr+12);
  memcpy(hwhdr, hdr, hdr->hdr_pkt.rx_pkt.rx_normal.opcode?8:4);
  memmove(nethdr + 4, nethdr , 12);
  if(hdr->hdr_pkt.rx_pkt.rx_normal.opcode == 0x01)
    return 8;
  return 4;  
}

static void hal_txrx_hw_hdr_dump(hw_hdr_t *hwhdr, zpl_uint8 *nethdr)
{
  char *reason_str[] = {"mirroring", "SA Learning", "switching", "proto term", "proto snooping","flooding", "res"};
  if(hwhdr->hdr_pkt.rx_pkt.rx_normal.opcode)
  {
    zlog_debug(MODULE_TXRX,"DEV RECV: type=%c port=%d timestamp=%u", hwhdr->hdr_pkt.rx_pkt.rx_time.tr?'T':'R', hwhdr->hdr_pkt.rx_pkt.rx_time.srcport, hwhdr->hdr_pkt.rx_pkt.rx_time.timestamp);
  }
  else
  {
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x01)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[0], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x02)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[1], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x04)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[2], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x08)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[3], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x10)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[4], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x20)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[5], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x40)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[6], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x80)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[7], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
  }   

  if (IS_ZPL_IPCMSG_DEBUG_RECV(hal_txrx.debug) && IS_ZPL_IPCMSG_DEBUG_DETAIL(hal_txrx.debug)
      && nethdr[16] == 0x81 && nethdr[17] == 0x00)
      zlog_debug(MODULE_TXRX,"DEV RECV: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                 nethdr[0], nethdr[1], nethdr[2], nethdr[3], nethdr[4], nethdr[5],
                 nethdr[6], nethdr[7], nethdr[8], nethdr[9], nethdr[10], nethdr[11],
                 nethdr[12], nethdr[13], nethdr[14], nethdr[15], nethdr[16], nethdr[17], nethdr[18], nethdr[19]);   
}
#endif

#ifdef ZPL_SDK_BCM53125 
static int hal_txrx_tocpu(hal_txrx_t *txrx, char *data, int len, int ifindex)
{
  return hal_txrx_sendto_netlink(txrx, data,  len, NETPKT_TOCPU, ifindex);
}

static int hal_txrx_toswitch(hal_txrx_t *txrx, char *data, int len, int flags, int cosq, vlan_t vlan, int cfi, int pri, zpl_phyport_t ifindex)
{
  int txlen = 0;
  char *msgbuf = data - 8;
  memmove(msgbuf, data, 2*ETH_ALEN);
  txlen += hal_txrx_vlan_add(msgbuf + 4 + 2*ETH_ALEN,  len,  vlan,  cfi,  pri);
  txlen += hal_txrx_hwhdr_add(msgbuf + 2*ETH_ALEN,  len,  flags,  cosq,  0);
  txlen += len;  
  return hal_txrx_sendto_netlink(&hal_txrx, msgbuf,  txlen, NETPKT_TOSWITCH, 0);
}

static int hal_txrx_recv_callback(lib_netlink_t *nsock, int type, char *data, int len, hal_txrx_t *txrx)
{
  if(type == NETPKT_FROMDEV)
  {
    int flags = 0;
    int cosq =0;
    vlan_t vlan = 1;
    int cfi = 0;
    int pri = 0;    
    hal_txrx_hdr_t *txhdr = (hal_txrx_hdr_t*)data;
    zpl_phyport_t  dstport;
    dstport = if_ifindex2phy(txhdr->dstval);
    hal_txrx_toswitch(txrx, data + sizeof(hal_txrx_hdr_t), len - sizeof(hal_txrx_hdr_t), flags, cosq, vlan, cfi, pri, dstport);
  }
  if(type == NETPKT_FROMSWITCH)
  {
    int ret = 0, rxlen = len;
    char *p = data;
    zpl_skb_vlanhdr_t vlan;
    ifindex_t ifindex;
    hw_hdr_t hwhdr;
    p += sizeof(hal_txrx_hdr_t);
    rxlen -= sizeof(hal_txrx_hdr_t);
    ret = hal_txrx_hw_hdr_get(p, &hwhdr);
    p += ret;
    rxlen -= ret;
    hal_txrx_hw_hdr_dump(&hwhdr, p);

    ret = hal_txrx_vlan_del(p, rxlen, &vlan);
    p += ret;
    rxlen -= ret;
    ifindex = if_phy2ifindex(hwhdr.hdr_pkt.rx_pkt.rx_normal.srcport);
    hal_netpkt_filter_distribute(p, rxlen);
    hal_txrx_tocpu(txrx, p, rxlen, ifindex2ifkernel(ifindex));
  }
  return OK;
}
#endif

static int hal_txrx_thread(struct thread *thread)
{
    hal_txrx_t *txrx = THREAD_ARG(thread);
    txrx->t_read = NULL;
    if(lib_netlink_recv(txrx->netlink_data))
    {
      zlog_err(MODULE_TXRX, "lib_netlink_recv =====================");
#ifdef ZPL_SDK_BCM53125     
      lib_netlink_msg_callback(txrx->netlink_data, hal_txrx_recv_callback, txrx);
#endif     
      txrx->t_read = thread_add_read(txrx->master, hal_txrx_thread, txrx, txrx->netlink_data->sock);
    }
    return 0;
}

static int hal_txrx_task(void *pVoid)
{
  struct thread_master *master = (struct thread_master *)pVoid;
  module_setup_task(master->module, os_task_id_self());
  
  while (thread_mainloop(master))
    ;
  return OK;
}

int hal_module_txrx_init(void)
{
    memset(&hal_txrx, 0, sizeof(hal_txrx));
    if (!hal_txrx.master)
    {
        hal_txrx.master = thread_master_module_create(MODULE_TXRX);
    }
    hal_txrx.netlink_data = lib_netlink_create(4096, 64);
    if(hal_txrx.netlink_data == NULL)
    {
      return ERROR;
    }
    if(lib_netlink_open(hal_txrx.netlink_data, HAL_DATA_NETLINK_PROTO) != OK)
    {
      hal_module_txrx_exit();
      return ERROR;
    }
    if(!ipstack_invalid(hal_txrx.netlink_data->sock))
      hal_txrx.t_read = thread_add_read(hal_txrx.master, hal_txrx_thread, &hal_txrx, hal_txrx.netlink_data->sock);
    return OK;
}

int hal_module_txrx_task_init(void)
{
    if (!hal_txrx.master)
    {
        hal_txrx.master = thread_master_module_create(MODULE_TXRX);
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

int hal_module_txrx_exit(void)
{
	if(hal_txrx.netlink_data)
	{
		lib_netlink_close(hal_txrx.netlink_data);
		lib_netlink_destroy(hal_txrx.netlink_data);
		hal_txrx.netlink_data = NULL;
	}
 	if (hal_txrx.master)
	{
    	thread_master_free(hal_txrx.master);
    	hal_txrx.master = NULL;
	}   
    return OK;
}

int hal_module_txrx_task_exit(void)
{
    if (hal_txrx.taskid > 0)
        os_task_destroy(hal_txrx.taskid);
    if (hal_txrx.master)
    {
        thread_master_free(hal_txrx.master);
        hal_txrx.master = NULL;
    }
    return OK;
}