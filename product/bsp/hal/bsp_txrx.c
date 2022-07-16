#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"
#include "vrf.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "bsp_txrx.h"
#include "bsp_driver.h"

bsp_txrx_t bsp_txrx;

struct module_list module_list_txrx =
    {
        .module = MODULE_TXRX,
        .name = "TXRX\0",
        .module_init = bsp_module_txrx_init,
        .module_exit = bsp_module_txrx_exit,
        .module_task_init = bsp_module_txrx_task_init,
        .module_task_exit = bsp_module_txrx_task_exit,
        .module_cmd_init = NULL,
        .taskid = 0,
};

#define NETPKT_TOCPU        1
#define NETPKT_FROMDEV      2
#define NETPKT_TOSWITCH     3
#define NETPKT_FROMSWITCH   4

typedef struct
{
    zpl_uint8       cmd;                         /* Unit number. */
    zpl_phyport_t   dstval;          

}bsp_txrx_hdr_t __attribute__ ((aligned (1)));



static int bsp_txrx_hwhdr_add(char *data, int len, int flags, int cosq, int dportmap)
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


static int bsp_txrx_vlan_add(char *data, int len, vlan_t vlan, int cfi, int pri)
{
  vlan_hdt_t *vlanhdr = (vlan_hdt_t *)data;
  vlanhdr->vlantype = htons(0x8100);
  vlanhdr->vid = vlan;
  vlanhdr->cfi = cfi;
  vlanhdr->pri = pri;
  return 4;
}

static int bsp_txrx_vlan_del(char *data, int len, vlan_hdt_t *vlan)
{
  vlan_hdt_t *vlanhdr = (vlan_hdt_t *)(data + 2*ETH_ALEN);
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

static int bsp_txrx_sendto_netlink(bsp_txrx_t *txrx, char *data, int len, int cmd, int ifindex)
{
	struct ipstack_nlmsghdr *nlh = NULL;
	bsp_txrx_hdr_t *header = (bsp_txrx_hdr_t*)(data - sizeof(bsp_txrx_hdr_t));
  nlh = (struct ipstack_nlmsghdr *)(data - sizeof(bsp_txrx_hdr_t) - sizeof(struct ipstack_nlmsghdr));
	memset (nlh, 0, sizeof (struct ipstack_nlmsghdr));
  memset (header, 0, sizeof (bsp_txrx_hdr_t));
  header->cmd = cmd;
  header->dstval = htonl(ifindex);
  nlh->nlmsg_len = IPSTACK_NLMSG_LENGTH (len+sizeof(bsp_txrx_hdr_t));
  nlh->nlmsg_flags = IPSTACK_NLM_F_CREATE | IPSTACK_NLM_F_REQUEST;
  nlh->nlmsg_type = cmd;
	nlh->nlmsg_seq = ++txrx->netlink_data->seq;
	nlh->nlmsg_flags |= IPSTACK_NLM_F_ACK;
	nlh->nlmsg_pid = 0;
	return bsp_netlink_send(txrx->netlink_data, nlh);
}


int bsp_txrx_sendto_port(char *data, int len, int flags, int cosq, int dportmap, vlan_t vlan, int cfi, int pri)
{
  int txlen = 0, offset = 64;
  char msgbuf[2048];
  memcpy(msgbuf + offset, data, 2*ETH_ALEN);
  txlen += bsp_txrx_vlan_add(msgbuf + offset + 4 + 2*ETH_ALEN,  len,  vlan,  cfi,  pri);
  txlen += bsp_txrx_hwhdr_add(msgbuf + offset + 2*ETH_ALEN,  len,  flags,  cosq,  dportmap);
  memcpy(msgbuf + offset + 2*ETH_ALEN + txlen, data + 2*ETH_ALEN, len - 2*ETH_ALEN);
  txlen += len;  
  return bsp_txrx_sendto_netlink(&bsp_txrx, msgbuf + offset,  txlen, NETPKT_TOSWITCH, 0);
}

int bsp_txrx_vlan_flood(char *data, int len, int flags, int cosq, vlan_t vlan, int cfi, int pri)
{
  int txlen = 0, offset = 64;
  char msgbuf[2048];
  memcpy(msgbuf + offset, data, 2*ETH_ALEN);
  txlen += bsp_txrx_vlan_add(msgbuf + offset + 4 + 2*ETH_ALEN,  len,  vlan,  cfi,  pri);
  txlen += bsp_txrx_hwhdr_add(msgbuf + offset + 2*ETH_ALEN,  len,  flags,  cosq,  0);
  memcpy(msgbuf + offset + 2*ETH_ALEN + txlen, data + 2*ETH_ALEN, len - 2*ETH_ALEN);
  txlen += len;  
  return bsp_txrx_sendto_netlink(&bsp_txrx, msgbuf + offset,  txlen, NETPKT_TOSWITCH, 0);
}



static int bsp_txrx_hw_hdr_get(zpl_uint8 *nethdr, hw_hdr_t *hwhdr)
{
  hw_hdr_t *hdr = (hw_hdr_t *)(nethdr+12);
  memcpy(hwhdr, hdr, hdr->hdr_pkt.rx_pkt.rx_normal.opcode?8:4);
  memmove(nethdr + 4, nethdr , 12);
  if(hdr->hdr_pkt.rx_pkt.rx_normal.opcode == 0x01)
    return 8;
  return 4;  
}

#if 0
static int bsp_hdr_setup(hw_hdr_t *hwhdr, zpl_bsp_hdr_t *netkthdr)
{
  netkthdr->unit = 0;
  if(hwhdr->hdr_pkt.rx_pkt.rx_normal.opcode)
  {
    netkthdr->phyid = hwhdr->hdr_pkt.rx_pkt.rx_time.srcport;
    netkthdr->timestamp = hwhdr->hdr_pkt.rx_pkt.rx_time.timestamp;
  }
  else
  {
    netkthdr->phyid = hwhdr->hdr_pkt.rx_pkt.rx_normal.srcport;
    netkthdr->cos = hwhdr->hdr_pkt.rx_pkt.rx_normal.tc;
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x01)
      netkthdr->reason = NETPKT_REASON_CONTROL; 
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x02)
      netkthdr->reason = NETPKT_REASON_CPULEARN;
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x04)
      netkthdr->reason = NETPKT_REASON_CONTROL;
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x08)
      netkthdr->reason = NETPKT_REASON_PROTOCOL;
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x10)
      netkthdr->reason = NETPKT_REASON_PROTOCOL;
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x20)
      netkthdr->reason = NETPKT_REASON_BROADCAST;
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x40)
      netkthdr->reason = NETPKT_REASON_NONE;
    if(hwhdr->hdr_pkt.rx_pkt.rx_normal.reason & 0x80)
      netkthdr->reason = NETPKT_REASON_NONE;
  }
  return OK;
}

static int bsp_hdr_vlan_setup(zpl_uint8 *nethdr, zpl_bsp_hdr_t *netkthdr)
{
    zpl_uint8       resmac[ETH_ALEN] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x20}; 
    ethhdr_vlan_t *ethvlan = (ethhdr_vlan_t *)nethdr;
    if(ntohs(ethvlan->ethhdr.ethtype) == 0x8100 || ntohs(ethvlan->ethhdr.ethtype) == 0x9100)
    {
        netkthdr->tpid = (ethvlan->ethhdr.ethtype);
        netkthdr->vlan = ethvlan->ethhdr.vlanhdr.vid;                    /* 802.1q VID or VSI or VPN. */
        netkthdr->vlan_pri = ethvlan->ethhdr.vlanhdr.pri;                     /* Vlan tag priority . */
        netkthdr->vlan_cfi = ethvlan->ethhdr.vlanhdr.cfi;                     /* Vlan tag CFI bit. */
        netkthdr->ethtype = (ethvlan->ethhdr.vlanhdr.ethtype);

        ethvlan = (ethhdr_vlan_t *)(nethdr + 4);
        if(ntohs(ethvlan->ethhdr.ethtype) == 0x8100)//qinq
        {
            netkthdr->inner_tpid = (ethvlan->ethhdr.ethtype);
            netkthdr->inner_vlan = ethvlan->ethhdr.vlanhdr.vid;              /* Inner VID or VSI or VPN. */
            netkthdr->inner_vlan_pri = ethvlan->ethhdr.vlanhdr.pri;               /* Inner vlan tag priority . */
            netkthdr->inner_vlan_cfi = ethvlan->ethhdr.vlanhdr.cfi;               /* Inner vlan tag CFI bit. */
            netkthdr->ethtype = (ethvlan->ethhdr.vlanhdr.ethtype);
        }
        netkthdr->untagged = 0;       /* The packet was untagged on ingress. */
        //netkthdr->ifindex;
        //netkthdr->trunk;          /* Source trunk group ID used in header/tag, -1 if src_port set . */
        //netkthdr->phyid;          /* Source port used in header/tag. */
    }
    else
    {
        netkthdr->tpid = 0;
        netkthdr->vlan = 0;                    /* 802.1q VID or VSI or VPN. */
        netkthdr->vlan_pri = 0;                     /* Vlan tag priority . */
        netkthdr->vlan_cfi = 0;                     /* Vlan tag CFI bit. */
        netkthdr->inner_tpid = 0;
        netkthdr->inner_vlan = 0;              /* Inner VID or VSI or VPN. */
        netkthdr->inner_vlan_pri = 0;               /* Inner vlan tag priority . */
        netkthdr->inner_vlan_cfi = 0;               /* Inner vlan tag CFI bit. */
        netkthdr->ethtype = (ethvlan->ethhdr.ethtype);
        netkthdr->untagged = 1;       /* The packet was untagged on ingress. */

        ethvlan = (ethhdr_vlan_t *)(nethdr + 4);
        if(ntohs(ethvlan->ethhdr.ethtype) == 0x8100)//qinq
        {
            netkthdr->inner_tpid = (ethvlan->ethhdr.ethtype);
            netkthdr->inner_vlan = ethvlan->ethhdr.vlanhdr.vid;              /* Inner VID or VSI or VPN. */
            netkthdr->inner_vlan_pri = ethvlan->ethhdr.vlanhdr.pri;               /* Inner vlan tag priority . */
            netkthdr->inner_vlan_cfi = ethvlan->ethhdr.vlanhdr.cfi;               /* Inner vlan tag CFI bit. */
            netkthdr->ethtype = (ethvlan->ethhdr.vlanhdr.ethtype);
            netkthdr->untagged = 0;

            ethvlan = (ethhdr_vlan_t *)(nethdr);
            netkthdr->tpid = (ethvlan->ethhdr.ethtype);
            netkthdr->vlan = ethvlan->ethhdr.vlanhdr.vid;                    /* 802.1q VID or VSI or VPN. */
            netkthdr->vlan_pri = ethvlan->ethhdr.vlanhdr.pri;                     /* Vlan tag priority . */
            netkthdr->vlan_cfi = ethvlan->ethhdr.vlanhdr.cfi;                     /* Vlan tag CFI bit. */
            //netkthdr->ethtype = ntohs(ethvlan->ethhdr.vlanhdr.ethtype);
        }
    }
    if(ntohs(netkthdr->ethtype) == 0x0806)
      netkthdr->reason = NETPKT_REASON_ARP;
    if(memcmp(nethdr, resmac, 3) == 0)
      netkthdr->reason = NETPKT_REASON_BPDU;
    return OK;
}
#endif
static void bsp_txrx_hw_hdr_dump(hw_hdr_t *hwhdr, zpl_uint8 *nethdr)
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

  if (IS_ZPL_IPCMSG_DEBUG_RECV(bsp_txrx.debug) && IS_ZPL_IPCMSG_DEBUG_DETAIL(bsp_txrx.debug)
      && nethdr[16] == 0x81 && nethdr[17] == 0x00)
      zlog_debug(MODULE_TXRX,"DEV RECV: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                 nethdr[0], nethdr[1], nethdr[2], nethdr[3], nethdr[4], nethdr[5],
                 nethdr[6], nethdr[7], nethdr[8], nethdr[9], nethdr[10], nethdr[11],
                 nethdr[12], nethdr[13], nethdr[14], nethdr[15], nethdr[16], nethdr[17], nethdr[18], nethdr[19]);   
}


static int bsp_txrx_tocpu(bsp_txrx_t *txrx, char *data, int len, int ifindex)
{
  return bsp_txrx_sendto_netlink(txrx, data,  len, NETPKT_TOCPU, ifindex);
}

static int bsp_txrx_toswitch(bsp_txrx_t *txrx, char *data, int len, int flags, int cosq, vlan_t vlan, int cfi, int pri, zpl_phyport_t ifindex)
{
  int txlen = 0;
  char *msgbuf = data - 8;
  memmove(msgbuf, data, 2*ETH_ALEN);
  txlen += bsp_txrx_vlan_add(msgbuf + 4 + 2*ETH_ALEN,  len,  vlan,  cfi,  pri);
  txlen += bsp_txrx_hwhdr_add(msgbuf + 2*ETH_ALEN,  len,  flags,  cosq,  0);
  txlen += len;  
  return bsp_txrx_sendto_netlink(&bsp_txrx, msgbuf,  txlen, NETPKT_TOSWITCH, 0);
}

static int bsp_txrx_recv_callback(bsp_netlink_t *nsock, int type, char *data, int len, bsp_txrx_t *txrx)
{
  if(type == NETPKT_FROMDEV)
  {
    int flags = 0;
    int cosq =0;
    vlan_t vlan = 1;
    int cfi = 0;
    int pri = 0;    
    bsp_txrx_hdr_t *txhdr = (bsp_txrx_hdr_t*)data;
    zpl_phyport_t  dstport;
    dstport = if_ifindex2phy(txhdr->dstval);
    bsp_txrx_toswitch(txrx, data + sizeof(bsp_txrx_hdr_t), len - sizeof(bsp_txrx_hdr_t), flags, cosq, vlan, cfi, pri, dstport);
  }
  if(type == NETPKT_FROMSWITCH)
  {
    int ret = 0, rxlen = len;
    char *p = data;
    vlan_hdt_t vlan;
    ifindex_t ifindex;
    hw_hdr_t hwhdr;
    p += sizeof(bsp_txrx_hdr_t);
    rxlen -= sizeof(bsp_txrx_hdr_t);
    ret = bsp_txrx_hw_hdr_get(p, &hwhdr);
    p += ret;
    rxlen -= ret;
    bsp_txrx_hw_hdr_dump(&hwhdr, p);

    ret = bsp_txrx_vlan_del(p, rxlen, &vlan);
    p += ret;
    rxlen -= ret;
    ifindex = if_phy2ifindex(hwhdr.hdr_pkt.rx_pkt.rx_normal.srcport);
    bsp_txrx_tocpu(txrx, p, rxlen, ifindex2ifkernel(ifindex));
  }
  return OK;
}

static int bsp_txrx_thread(struct thread *thread)
{
    bsp_txrx_t *txrx = THREAD_ARG(thread);
    txrx->t_read = NULL;
    if(bsp_netlink_recv(txrx->netlink_data))
    {
      bsp_netlink_msg_callback(txrx->netlink_data, bsp_txrx_recv_callback, txrx);
      txrx->t_read = thread_add_read(txrx->master, bsp_txrx_thread, txrx, txrx->netlink_data->sock);
    }
    return 0;
}

static int bsp_txrx_task(void *pVoid)
{
  struct thread_master *master = (struct thread_master *)pVoid;
  module_setup_task(master->module, os_task_id_self());
  while (thread_mainloop(master))
    ;
  return OK;
}

int bsp_module_txrx_init(void)
{
    memset(&bsp_txrx, 0, sizeof(bsp_txrx));
    if (!bsp_txrx.master)
    {
        bsp_txrx.master = thread_master_module_create(MODULE_TXRX);
    }
    bsp_txrx.netlink_data = bsp_netlink_create(4096, 64);
	if(bsp_txrx.netlink_data == NULL)
	{
		return ERROR;
	}
	if(bsp_netlink_open(bsp_txrx.netlink_data, HAL_DATA_NETLINK_PROTO) != OK)
	{
		bsp_module_txrx_exit();
		return ERROR;
	}
  if(!ipstack_invalid(bsp_txrx.netlink_data->sock))
    bsp_txrx.t_read = thread_add_read(bsp_txrx.master, bsp_txrx_thread, &bsp_txrx, bsp_txrx.netlink_data->sock);
  return OK;
}

int bsp_module_txrx_task_init(void)
{
    if (!bsp_txrx.master)
    {
        bsp_txrx.master = thread_master_module_create(MODULE_TXRX);
    }
    if (bsp_txrx.taskid <= 0)
        bsp_txrx.taskid = os_task_create("txrxTask", OS_TASK_DEFAULT_PRIORITY,
                                         0, bsp_txrx_task, bsp_txrx.master, OS_TASK_DEFAULT_STACK * 4);
    if (bsp_txrx.taskid > 0)
    {
        module_setup_task(MODULE_TXRX, bsp_txrx.taskid);
        return OK;
    }
    return ERROR;
}

int bsp_module_txrx_exit(void)
{
	if(bsp_txrx.netlink_data)
	{
		bsp_netlink_close(bsp_txrx.netlink_data);
		bsp_netlink_destroy(bsp_txrx.netlink_data);
		bsp_txrx.netlink_data = NULL;
	}
 	if (bsp_txrx.master)
	{
    	thread_master_free(bsp_txrx.master);
    	bsp_txrx.master = NULL;
	}   
    return OK;
}

int bsp_module_txrx_task_exit(void)
{
    if (bsp_txrx.taskid > 0)
        os_task_destroy(bsp_txrx.taskid);
    if (bsp_txrx.master)
    {
        thread_master_free(bsp_txrx.master);
        bsp_txrx.master = NULL;
    }
    return OK;
}