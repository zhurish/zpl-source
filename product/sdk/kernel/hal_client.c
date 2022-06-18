#include "bsp_types.h"
#include "hal_client.h"
#include "bsp_include.h"
#include "hal_netpkt.h"

static struct hal_client *halclient = NULL;
static int bsp_driver_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver);
static void hal_client_sock_input(struct sk_buff *__skb);

static struct netlink_kernel_cfg _hal_client_sock_nkc = {
    .groups = 0,
    .flags = 0,
    .input = hal_client_sock_input,
    .cb_mutex = NULL,
    .bind = NULL,
    .unbind = NULL,
    .compare = NULL,
};

static int hal_client_read_handle(struct hal_client *client)
{
  int ret = 0;
  struct hal_ipcmsg_header hdr;
  hal_ipcmsg_get_header(&client->ipcmsg, &hdr);

  if (ZPL_TST_BIT(client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(client->debug, KLOG_DEBUG_RECV))
    zlog_debug(MODULE_HAL, "Client Recv message received [%s] %d ",
               hal_module_cmd_name(hdr.command), hdr.length);

  if (IPCCMD_MODULE_GET(hdr.command) >= HAL_MODULE_MGT &&
      IPCCMD_MODULE_GET(hdr.command) <= HAL_MODULE_MAX)
  {
    ret = bsp_driver_msg_handle(client, hdr.command, client->bsp_driver);
    if (ret == OS_NO_CALLBACK)
    {
      hal_client_send_return(client, OS_NO_CALLBACK, "%s","NO Callback Func");
    }
    else
    {
      if (IPCCMD_CMD_GET(hdr.command) != HAL_MODULE_CMD_GET)
        hal_client_send_return(client, ret, "cmd:%s", hal_module_cmd_name(hdr.command));
    }
  }
  else
  {
    ret = OS_NO_SDKSPUUORT;
    hal_client_send_return(client, -1, "Client Recv unknown command %d", hdr.command);
    zlog_warn(MODULE_HAL, "Client Recv unknown command %d", hdr.command);
  }
  return 0;
}

static int hal_client_putdata(struct hal_client *client, int cmd, int seqno, int dstpid, char *data, int len)
{
  if (client && client->netlink)
  {
    client->netlink->cmd = cmd;
    client->netlink->seqno = seqno;
    client->netlink->dstpid = dstpid;
    hal_ipcmsg_reset(&client->ipcmsg);
    hal_ipcmsg_put(&client->ipcmsg, data, len);
    return hal_client_read_handle(client);
  }
  return 0;
}

static void hal_client_sock_input(struct sk_buff *__skb)
{
  struct nlmsghdr *nlh = nlmsg_hdr(__skb);
  const char *nlmsg = NULL;
  int *from_pid = NULL;
  nlmsg = (const char *)nlmsg_data(nlh);
  from_pid = (int *)nlmsg;
  zlog_debug(MODULE_HAL, " cfgnk_sock_input nlmsg_type %d seq %d dstpid %d\n", nlh->nlmsg_type, nlh->nlmsg_seq, ntohl(*from_pid));
  if (halclient && nlh->nlmsg_type == HAL_CFG_REQUEST_CMD)
    hal_client_putdata(halclient, nlh->nlmsg_type, nlh->nlmsg_seq, ntohl(*from_pid), nlmsg + 4, nlmsg_len(nlh) - 4);
  else if (halclient && nlh->nlmsg_type == HAL_DATA_REQUEST_CMD)
  {
    // netpkt_netlink_dstpid(ntohl(*from_pid));
    // netpkt_netlink_bind(moudle, value);
    // netpkt_netlink_debug_set(moudle, value);
  }
  else if (halclient && nlh->nlmsg_type == HAL_KLOG_REQUEST_CMD)
  {
    // klog_dstpid(ntohl(*from_pid));
    // klog_level(value);
  }
}

/* Allocate hal_client structure. */
static struct hal_client *hal_client_new(void)
{
  struct hal_client *hal_client;
  hal_client = XMALLOC(MTYPE_HALIPCCLIENT, sizeof(struct hal_client)); // kmalloc(, GFP_KERNEL);
  if (hal_client)
  {
    memset(hal_client, 0, sizeof(struct hal_client));
    hal_client->ipcmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_client->outmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ / 2;
    if (hal_ipcmsg_create(&hal_client->ipcmsg) != OK)
    {
      XFREE(MTYPE_HALIPCCLIENT, hal_client);
      halclient = NULL;
      return hal_client;
    }
    if (hal_ipcmsg_create(&hal_client->outmsg) != OK)
    {
      hal_ipcmsg_destroy(&hal_client->ipcmsg);
      XFREE(MTYPE_HALIPCCLIENT, hal_client);
      halclient = NULL;
      return hal_client;
    }
    hal_client->debug = 0x00ffffff;
  }
  return hal_client;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free hal_client structure. */
static void hal_client_free(struct hal_client *hal_client)
{
  if (hal_client)
  {
    hal_ipcmsg_destroy(&hal_client->ipcmsg);
    hal_ipcmsg_destroy(&hal_client->outmsg);
    if (hal_client->netlink)
    {
      hal_netlink_destroy(hal_client->netlink);
      hal_client->netlink = NULL;
    }
    XFREE(MTYPE_HALIPCCLIENT, hal_client);
    halclient = NULL;
  }
}

/* Initialize zebra client.  Argument redist_default is unwanted
   redistribute route type. */
struct hal_client *hal_client_create(void *bsp_driver)
{
  /* Enable zebra client connection by default. */
  struct hal_client *hal_client = hal_client_new();
  if (hal_client)
  {
    //初始化netlink
    hal_client->netlink = hal_netlink_create("cfg", HAL_CFG_NETLINK_PROTO, 0, &_hal_client_sock_nkc);
    if (!hal_client->netlink)
    {
      zlog_err(MODULE_SDK, "[netlink] create netlink socket error!");
      hal_client_free(hal_client);
      hal_client = NULL;
      return hal_client;
    }
    hal_client->bsp_driver = bsp_driver;
    halclient = hal_client;
  }
  return hal_client;
}

int hal_client_destroy(struct hal_client *hal_client)
{
  hal_client_free(hal_client);
  return OK;
}

static int hal_client_send_message(struct hal_client *hal_client, enum hal_ipcmsg_type type)
{
  char *nldata = NULL;
  int *result = NULL;
  struct nlmsghdr *nlh = NULL;
  struct sk_buff *skb = NULL;
  hal_ipcmsg_hdrlen_set(&hal_client->outmsg);
  skb = XNLMSG_NEW(MTYPE_SDK_DATA, hal_client->outmsg.setp+4);
  if (skb && hal_client->netlink)
  {
    int ret = 0;
    nlh = nlmsg_put(skb, 0, hal_client->netlink->seqno, hal_client->netlink->cmd, hal_client->outmsg.setp, 0);
    nldata = nlmsg_data(nlh);
    if (hal_client->outmsg.setp)
      memcpy(nldata, hal_client->outmsg.buf, hal_client->outmsg.setp);
    ret = hal_netlink_unicast(hal_client->netlink, hal_client->netlink->dstpid, skb);
    return ret;
  }
  return 0;
}

int hal_client_send_return(struct hal_client *hal_client, int ret, char *fmt, ...)
{
  int len = 0;
  va_list args;
  char logbuf[1024];
  struct hal_ipcmsg_result getvalue;
  struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
  memset(logbuf, 0, sizeof(logbuf));
  memset(&getvalue, 0, sizeof(getvalue));
  hal_ipcmsg_reset(&hal_client->outmsg);
  hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, 0));
  getvalue.result = ret;
  hal_ipcmsg_result_set(&hal_client->outmsg, &getvalue);
  if (ret != OK)
  {
    va_start(args, fmt);
    len = vsnprintf(logbuf, sizeof(logbuf), fmt, args);
    va_end(args);
    if (len > 0 && len < sizeof(logbuf))
      hal_ipcmsg_put(&hal_client->outmsg, logbuf, len);
  }
  if (ZPL_TST_BIT(hal_client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(hal_client->debug, KLOG_DEBUG_SEND))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] seqno %d dstpid %d result %d %s",
               hal_client->netlink->cmd, hal_client->netlink->seqno, hal_client->netlink->dstpid, ret, (ret != OK) ? logbuf : "OK");
  return hal_client_send_message(hal_client, 0);
}

int hal_client_send_result(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue)
{
  va_list args;
  struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
  hal_ipcmsg_reset(&hal_client->outmsg);
  hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, 0));
  getvalue->result = ret;
  hal_ipcmsg_result_set(&hal_client->outmsg, getvalue);
  if (ZPL_TST_BIT(hal_client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(hal_client->debug, KLOG_DEBUG_SEND))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] seqno %d dstpid %d result %d",
               hal_client->netlink->cmd, hal_client->netlink->seqno, hal_client->netlink->dstpid, ret);
  return hal_client_send_message(hal_client, 0);
}

int hal_client_send_result_msg(struct hal_client *hal_client, int ret,
                               struct hal_ipcmsg_result *getvalue, int subcmd, char *msg, int msglen)
{
  int len = 0;
  va_list args;
  struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
  hal_ipcmsg_reset(&hal_client->outmsg);
  hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, subcmd));
  getvalue->result = ret;
  hal_ipcmsg_result_set(&hal_client->outmsg, getvalue);

  if (msglen && msg)
    hal_ipcmsg_put(&hal_client->outmsg, msg, msglen);
  if (ZPL_TST_BIT(hal_client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(hal_client->debug, KLOG_DEBUG_SEND))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] seqno %d dstpid %d result %d",
               hal_client->netlink->cmd, hal_client->netlink->seqno, hal_client->netlink->dstpid, ret);
  return hal_client_send_message(hal_client, 0);
}

static int bsp_debug_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
  int ret = OK;
  int moudle, value;
  BSP_ENTER_FUNC();
  hal_ipcmsg_getl(&client->ipcmsg, &moudle);
  hal_ipcmsg_getl(&client->ipcmsg, &value);

  if (subcmd == KLOG_DEST)
    klog_dstpid(value);
  else if (subcmd == KLOG_LEVEL)
    klog_level(value);
  else if (subcmd == NETPKT_DEST)
    netpkt_netlink_dstpid(value);
  else if (subcmd == NETPKT_BIND)
    netpkt_netlink_bind(moudle, value);
  else if (subcmd == NETPKT_DEBUG)
    netpkt_netlink_debug_set(moudle, value);
  else if (subcmd == HALCLIENT_DEBUG)
  {
    if (moudle)
      ZPL_SET_BIT(client->debug, value);
    else
      ZPL_CLR_BIT(client->debug, value);
  }
  BSP_LEAVE_FUNC();
  return ret;
}

#if 1
static const hal_ipccmd_callback_t const moduletable[] = {
    HAL_CALLBACK_ENTRY(HAL_MODULE_NONE, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_MGT, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_GLOBAL, bsp_global_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_SWITCH, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_CPU, bsp_cpu_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_PORT, bsp_port_module_handle),
    // HAL_CALLBACK_ENTRY(HAL_MODULE_L3IF, bsp_l3if_module_handle),
    // HAL_CALLBACK_ENTRY(HAL_MODULE_ROUTE, bsp_route_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_STP, NULL),
#ifdef ZPL_NSM_MSTP
    HAL_CALLBACK_ENTRY(HAL_MODULE_MSTP, bsp_mstp_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_MSTP, NULL),
#endif
#ifdef ZPL_NSM_8021X
    HAL_CALLBACK_ENTRY(HAL_MODULE_8021X, bsp_8021x_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_8021X, NULL),
#endif
#ifdef ZPL_NSM_IGMP
    HAL_CALLBACK_ENTRY(HAL_MODULE_IGMP, bsp_snooping_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_IGMP, NULL),
#endif
#ifdef ZPL_NSM_DOS
    HAL_CALLBACK_ENTRY(HAL_MODULE_DOS, bsp_dos_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_DOS, NULL),
#endif
#ifdef ZPL_NSM_MAC
    HAL_CALLBACK_ENTRY(HAL_MODULE_MAC, bsp_mac_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_MAC, NULL),
#endif
#ifdef ZPL_NSM_MIRROR
    HAL_CALLBACK_ENTRY(HAL_MODULE_MIRROR, bsp_mirror_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_MIRROR, NULL),
#endif

#ifdef ZPL_NSM_VLAN
    HAL_CALLBACK_ENTRY(HAL_MODULE_QINQ, bsp_qinq_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_VLAN, bsp_vlan_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_QINQ, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_VLAN, NULL),
#endif
#ifdef ZPL_NSM_QOS
    HAL_CALLBACK_ENTRY(HAL_MODULE_QOS, bsp_qos_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_QOS, NULL),
#endif
    HAL_CALLBACK_ENTRY(HAL_MODULE_ACL, NULL),
#ifdef ZPL_NSM_TRUNK
    HAL_CALLBACK_ENTRY(HAL_MODULE_TRUNK, bsp_trunk_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_TRUNK, NULL),
#endif
    HAL_CALLBACK_ENTRY(HAL_MODULE_ARP, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_BRIDGE, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_PPP, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_SECURITY, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_SNMP, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_VRF, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_MPLS, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_STATISTICS, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_EVENT, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_STATUS, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_DEBUG, bsp_debug_module_handle),
};

static int bsp_driver_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver)
{
  int ret = OS_NO_CALLBACK;
  int module = IPCCMD_MODULE_GET(cmd);
  hal_ipccmd_callback_t *callback = NULL;
  BSP_ENTER_FUNC();
  if (module > HAL_MODULE_NONE && module < HAL_MODULE_MAX)
  {
    int i = 0;
    for (i = 0; i < ARRAY_SIZE(moduletable); i++)
    {
      if (moduletable[i].module == module && moduletable[i].module_handle)
      {
        callback = &moduletable[i];
        break;
      }
    }
    if (callback)
      ret = (callback->module_handle)(client, IPCCMD_CMD_GET(cmd), IPCCMD_SUBCMD_GET(cmd), driver);
    else
    {
      ret = OS_NO_CALLBACK;
    }
  }
  BSP_LEAVE_FUNC();
  return ret;
}
#endif
