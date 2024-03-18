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

#ifdef ZPL_SDK_BCM53125
#define NETPKT_TOCPU        1
#define NETPKT_FROMDEV      2
#define NETPKT_TOSWITCH     3
#define NETPKT_FROMSWITCH   4

typedef struct
{
    zpl_uint32      cmd;                         /* Unit number. */
    zpl_phyport_t   dstval;          

}__attribute__ ((packed)) hal_txrx_hdr_t ;



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
  hwhdr->hdr_pkt.uihdr = htonl(hwhdr->hdr_pkt.uihdr);
  //zlog_debug(MODULE_TXRX,"====DEV SEND HW HDR: 0x%02x 0x%02x 0x%02x 0x%02x", data[0], data[1], data[2], data[3]);
  return 4;
}
#endif

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
  vlan->vlantype = ntohs(vlanhdr->vlantype);
  vlan->vid = vlanhdr->vid;
  vlan->cfi = vlanhdr->cfi;
  vlan->pri = vlanhdr->pri;
  //zlog_debug(MODULE_TXRX,"====DEV GET VLAN HDR: vlantype = 0x%x vid=%d cfi=%d pri=%d", vlan->vlantype, vlan->vid, vlan->cfi, vlan->pri);    
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
  header->cmd = htonl(cmd);
  header->dstval = htonl(ifindex);
  nlh->nlmsg_len = IPSTACK_NLMSG_LENGTH (len+sizeof(hal_txrx_hdr_t));//负载的长度
  nlh->nlmsg_flags = IPSTACK_NLM_F_CREATE | IPSTACK_NLM_F_REQUEST;
  nlh->nlmsg_type = HAL_DATA_REQUEST_CMD;
	nlh->nlmsg_seq = ++txrx->netlink_data->seq;
	nlh->nlmsg_flags |= IPSTACK_NLM_F_ACK;
	nlh->nlmsg_pid = 0;
	return lib_netlink_send(txrx->netlink_data, nlh);
}
#endif


int hal_txrx_sendto_port(char *data, int len, int flags, int cosq, int dportmap, vlan_t vlan, int cfi, int pri)
{
  int txlen = 0, offset = 0, boffset = 100;
  char msgbuf[4096];
  offset = boffset;
  memcpy(msgbuf + offset, data, 2*ETH_ALEN);
  offset += 2*ETH_ALEN;
#ifdef ZPL_SDK_BCM53125  
  txlen = hal_txrx_hwhdr_add(msgbuf + offset,  len,  flags,  cosq,  dportmap);
#endif  
  offset += txlen;
  if(vlan)
  {
    txlen = hal_txrx_vlan_add(msgbuf + offset,  len,  vlan,  cfi,  pri);
    offset += txlen;
  }
  memcpy(msgbuf + offset, data + 2*ETH_ALEN, len - 2*ETH_ALEN);
  offset += (len - 2*ETH_ALEN);  
  txlen = offset - boffset;
  if(len < 64)
    txlen += (64-len);

  char tmpbuf[2048];
  os_loghex(tmpbuf, sizeof(tmpbuf), msgbuf + boffset,  txlen);
  zlog_debug(MODULE_TXRX, " send to switch data len=%d data={%s}", txlen, tmpbuf);

#ifdef ZPL_SDK_BCM53125  
  return hal_txrx_sendto_netlink(&hal_txrx, msgbuf + boffset,  txlen, NETPKT_TOSWITCH, 0);
#else
  return 0;
#endif  
}



#ifdef ZPL_SDK_BCM53125 

#if 0
#define BRCM_OPCODE_SHIFT 5
#define BRCM_OPCODE_MASK 0x7
#define BRCM_EG_RC_RSVD (3 << 6)
#define BRCM_EG_PID_MASK 0x1f
//0x00 0x00 0x20 0x04
static void khal_netpkt_sock_skb_dump(zpl_uint8 *nethdr, char *hdr)
{
  zpl_uint8 *brcm_tag = NULL;  
  brcm_tag = nethdr + HAL_ETHMAC_HEADER;  
  char *reason_str[] = {"mirroring", "SA Learning", "switching", "proto term", "proto snooping","flooding", "res"};
  if((brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK)
  {
    zlog_debug(MODULE_TXRX,"%s: type=%c port=%d", hdr, (brcm_tag[3] & 0x20)?'T':'R', brcm_tag[3] & BRCM_EG_PID_MASK);
  }
  else//if((brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK)
  {
    if(brcm_tag[2] & 0x01)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[0], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x02)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[1], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x04)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[2], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x08)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[3], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x10)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[4], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x20)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[5], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x40)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[6], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x80)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[7], brcm_tag[3] & BRCM_EG_PID_MASK);
  }  
}
#endif

static int hal_txrx_hw_hdr_get(zpl_uint8 *nethdr, hw_hdr_t *hwhdr)
{
  zpl_uint8 ethmac[HAL_ETHMAC_HEADER];
  zpl_uint32 offset = 0, *phwhdr;
  hw_hdr_t *hdr = (hw_hdr_t *)(nethdr+HAL_ETHMAC_HEADER);
  phwhdr = (zpl_uint32*)(nethdr+HAL_ETHMAC_HEADER);
  *phwhdr = ntohl(*phwhdr);

  offset = hdr->hdr_pkt.rx_pkt.rx_normal.opcode?8:4;
  memcpy(ethmac, nethdr, HAL_ETHMAC_HEADER);
  memcpy(hwhdr, hdr, offset);

  memcpy(nethdr + offset, ethmac , HAL_ETHMAC_HEADER);
  return offset;  
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
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x01)//
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[0], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x02)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[1], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x04)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[2], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x08)//
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[3], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x10)
      zlog_debug(MODULE_TXRX,"DEV RECV: reason=(%x)%s port=%d", hwhdr->hdr_pkt.rx_pkt.rx_normal.reason, reason_str[4], hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x20)//
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

  /*char data[26];
  zpl_skb_vlanhdr_t vlan;
  hal_txrx_hwhdr_add(data, 16, 0, 1, hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport);   
  hal_txrx_vlan_add(data + 2*ETH_ALEN,  10,  101,  0,  2);     
  hal_txrx_vlan_del(data, 16, &vlan);   */    
}
#endif

#ifdef ZPL_SDK_BCM53125 
static int hal_txrx_sendto_cpu(hal_txrx_t *txrx, char *data, int len, int ifindex)
{
  return hal_txrx_sendto_netlink(txrx, data,  len, NETPKT_TOCPU, ifindex);
}



static int hal_txrx_toswitch(hal_txrx_t *txrx, char *data, int len, int flags, int cosq, vlan_t vlan, int cfi, int pri, zpl_phyport_t dportmap)
{
  int txlen = 0, offset = 0, boffset = 100;
  char msgbuf[4096];
  offset = boffset;
  memcpy(msgbuf + offset, data, 2*ETH_ALEN);
  offset += 2*ETH_ALEN;
#ifdef ZPL_SDK_BCM53125  
  txlen = hal_txrx_hwhdr_add(msgbuf + offset,  len,  flags,  cosq,  dportmap);
#endif  
  offset += txlen;
  if(vlan)
  {
    txlen = hal_txrx_vlan_add(msgbuf + offset,  len,  vlan,  cfi,  pri);
    offset += txlen;
  }
  memcpy(msgbuf + offset, data + 2*ETH_ALEN, len - 2*ETH_ALEN);
  offset += (len - 2*ETH_ALEN);  
  txlen = offset - boffset;
  if(len < 64)
    txlen += (64-len);

#ifdef ZPL_SDK_BCM53125  
  return hal_txrx_sendto_netlink(txrx, msgbuf + boffset,  txlen, NETPKT_TOSWITCH, 0);
#else
  return 0;
#endif 
}

static int hal_txrx_recv_callback(lib_netlink_t *nsock, int type, char *data, int len, hal_txrx_t *txrx)
{
  hal_txrx_hdr_t *txhdr = (hal_txrx_hdr_t*)data;

  if(ntohl(txhdr->cmd) == NETPKT_FROMDEV)
  {
    int flags = 0;
    int cosq =0;
    vlan_t vlan = 1;
    int cfi = 0;
    int pri = 0;    
    
    zpl_phyport_t  dstport;
    dstport = if_ifindex2phy(txhdr->dstval);
    if(dstport == -1)
      hal_txrx_toswitch(txrx, data + sizeof(hal_txrx_hdr_t), len - sizeof(hal_txrx_hdr_t), flags, cosq, vlan, cfi, pri, dstport);
    else
      hal_txrx_toswitch(txrx, data + sizeof(hal_txrx_hdr_t), len - sizeof(hal_txrx_hdr_t), flags, cosq, vlan, cfi, pri, 1<<dstport);
  }
  else if(ntohl(txhdr->cmd) == NETPKT_FROMSWITCH)
  {
    int ret = 0, rxlen = len;
    char *p = data;
    zpl_skb_vlanhdr_t vlan;
    ifindex_t ifindex;
    hw_hdr_t hwhdr;
    p += (sizeof(hal_txrx_hdr_t));
    rxlen -= (sizeof(hal_txrx_hdr_t));
    char tmpbuf[2048];
    memset(&vlan, 0, sizeof(zpl_skb_vlanhdr_t));
    memset(tmpbuf, 0, sizeof(tmpbuf));
    os_loghex(tmpbuf, sizeof(tmpbuf), p, rxlen);
    zlog_debug(MODULE_TXRX, " recv from switch data ifindex=0x%x len=%d data={%s}", vlanif2ifindex(1), rxlen, tmpbuf);

    ret = hal_txrx_hw_hdr_get(p, &hwhdr);
    p += ret;
    rxlen -= ret;
    hal_txrx_hw_hdr_dump(&hwhdr, p);

    ret = hal_txrx_vlan_del(p, rxlen, &vlan);
    p += ret;
    rxlen -= ret;
    /* 物理接口 */
    ifindex = if_phy2ifindex(hwhdr.hdr_pkt.rx_pkt.rx_normal.srcport);
    if(ifindex > 0)
    {
      struct interface *ifp = if_lookup_by_index(ifindex);
      if(ifp)
      {
        vlan_t acc_vlan = 0;
        nsm_interface_access_vlan_get_api(ifp, &acc_vlan);
        if(acc_vlan && acc_vlan == vlan.vid)
        {
          /* VLAN 接口 */
          ifindex = vlanif2ifindex(vlan.vid);
        }
      }
    }
    if(eth_l2protocol_type(p) != 0)
    {
      zpl_skbuffer_t * skb = zpl_skbuffer_create(ZPL_SKBUF_TYPE_NETPKT, NULL, rxlen);
      if(skb)
      {
        zpl_skbuffer_netpkt_build_source(ifindex, 0, hwhdr.hdr_pkt.rx_pkt.rx_normal.srcport, skb);
        zpl_skbuffer_netpkt_build_reason(hwhdr.hdr_pkt.rx_pkt.rx_normal.reason, skb);
        //zpl_skbuffer_netpkt_build_timestamp(zpl_uint32 timestamp, skb);
        zpl_skbuffer_netpkt_build_header(0, skb, p);
        zpl_skbuffer_put(skb, p, rxlen);
      }
    }
    hal_netpkt_filter_distribute(p, rxlen);
    /*p[0] = 0xcc;
    p[1] = 0xcc;
    p[2] = 0xcc;
    p[3] = 0xcc;
    p[4] = 0xcc;
    p[5] = 0xcc;*/
    p[6] = 0x00;

    p[7] = 0x01;
    p[8] = 0x01;
    p[9] = 0x01;
    hal_txrx_sendto_port(p, rxlen, 0, 0, 1<<hwhdr.hdr_pkt.rx_pkt.rx_normal.srcport, 0, 0, 0);
    p[9] = 0x02;
    hal_txrx_sendto_port(p, rxlen, 0, 0, 1<<hwhdr.hdr_pkt.rx_pkt.rx_normal.srcport, 1, 0, 0);
    p[9] = 0x03;
    hal_txrx_sendto_port(p, rxlen, 0, 0, -1, 1, 0, 0);
    p[9] = 0x04;
    hal_txrx_sendto_port(p, rxlen, 0, 0, -1, 0, 0, 0);
    return 0;
    hal_txrx_sendto_cpu(txrx, p, rxlen, ifindex2ifkernel(ifindex));
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
  //module_setup_task(master->module, os_task_id_self());
  
  while (thread_mainloop(master))
    ;
  return OK;
}

int hal_txrx_module_init(void)
{
    memset(&hal_txrx, 0, sizeof(hal_txrx));
    if (!hal_txrx.master)
    {
        hal_txrx.master = thread_master_name_create("TXRX");
    }
    hal_txrx.debug = 0xffffff;
    hal_txrx.netlink_data = lib_netlink_create(4096, 64);
    if(hal_txrx.netlink_data == NULL)
    {
      return ERROR;
    }
    hal_netpkt_filter_init();
    if(lib_netlink_open(hal_txrx.netlink_data, HAL_DATA_NETLINK_PROTO) != OK)
    {
      hal_txrx_module_exit();
      return ERROR;
    }
    if(!ipstack_invalid(hal_txrx.netlink_data->sock))
      hal_txrx.t_read = thread_add_read(hal_txrx.master, hal_txrx_thread, &hal_txrx, hal_txrx.netlink_data->sock);
    return OK;
}

int hal_txrx_module_task_init(void)
{
    if (!hal_txrx.master)
    {
        hal_txrx.master = thread_master_name_create("TXRX");
    }
    if (hal_txrx.taskid <= 0)
        hal_txrx.taskid = os_task_create("txrxTask", OS_TASK_DEFAULT_PRIORITY,
                                         0, hal_txrx_task, hal_txrx.master, OS_TASK_DEFAULT_STACK * 4);
    if (hal_txrx.taskid > 0)
    {
        //module_setup_task(MODULE_TXRX, hal_txrx.taskid);
        return OK;
    }
    return ERROR;
}

int hal_txrx_module_exit(void)
{
  hal_netpkt_filter_exit();
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

int hal_txrx_module_task_exit(void)
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