#include "kbsp_types.h"
#include "khal_client.h"
#ifdef ZPL_SDK_KERNEL
#include "bsp_include.h"
#endif
#include "khal_netpkt.h"
#include "keth_drv.h"

static struct khal_client *khalclient = NULL;
static int kbsp_driver_msg_handle(struct khal_client *client, zpl_uint32 cmd, void *driver);
static void khal_client_sock_input(struct sk_buff *__skb);


static int khal_client_ioctl_callback(struct khal_client *client, zpl_uint32 cmd, void *data, int len);

static struct netlink_kernel_cfg _khal_client_sock_nkc = {
    .groups = 0,
    .flags = 0,
    .input = khal_client_sock_input,
    .cb_mutex = NULL,
    .bind = NULL,
    .unbind = NULL,
    .compare = NULL,
};



static int khal_client_open(struct inode *inode, struct file *file)
{
	file->private_data = khalclient;
	return nonseekable_open(inode, file);
}

static int khal_client_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}




static long khal_client_ioctl(struct file *file, unsigned int command,
			     unsigned long u)
{
	int ret = -EINVAL;
	void __user *argp = (void __user *)u;
  char udata[512];
  struct khal_client *client = file->private_data;
  if(client)
  {
    if (copy_from_user(udata, argp, 256))
      return -EFAULT;

    ret = khal_client_ioctl_callback(client, command, udata, 256);
  }
	return ret;
}


static const struct file_operations khal_client_dev_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = khal_client_ioctl,
	.read = NULL,
	.write = NULL,
	.open = khal_client_open,
	.release = khal_client_release,
	.llseek = no_llseek,
};


static int khal_client_register(struct khal_client *khal_client)
{
	khal_client->dev_num = register_chrdev(0, KHAL_CLIENT_DEVICE_NAME, &khal_client_dev_fops);
	if (khal_client->dev_num < 0) 
	{
		printk(KERN_ERR "%s[%d] register_chrdev() error.\n", __FILE__,__LINE__);
		return -1;
	}
	if ((khal_client->class = class_create(THIS_MODULE, KHAL_CLIENT_MODULE_NAME)) == NULL) 
	{
		unregister_chrdev(khal_client->dev_num, KHAL_CLIENT_DEVICE_NAME);
		printk(KERN_ERR "%s[%d] class_create() error.\n", __FILE__,__LINE__);
		return -1;
	}
	khal_client->dev = device_create(khal_client->class, NULL, MKDEV(khal_client->dev_num, 0),
			  NULL, "%s%d", KHAL_CLIENT_DEVICE_NAME, 0);
	if (khal_client->dev == NULL) 
	{
		unregister_chrdev(khal_client->dev_num, KHAL_CLIENT_DEVICE_NAME);
		printk(KERN_ERR "%s[%d] device_create() error.\n", __FILE__,__LINE__);
		return -1;
	}
	pr_info("mdio driver module load(%d)\n", khal_client->dev_num);
	return 0;
}

static void khal_client_unregister(struct khal_client *khal_client)
{
	unregister_chrdev(khal_client->dev_num, KHAL_CLIENT_DEVICE_NAME);
	device_destroy(khal_client->class, khal_client->dev_num);
}

static int khal_client_read_handle(struct khal_client *client)
{
  int ret = 0;
  struct khal_ipcmsg_header hdr;
  khal_ipcmsg_get_header(&client->ipcmsg, &hdr);

  if (ZPL_TST_BIT(client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(client->debug, KLOG_DEBUG_RECV))
    zlog_debug(MODULE_HAL, "Client Recv message received [%s] %d ",
               khal_module_cmd_name(hdr.command), hdr.length);

  if (IPCCMD_MODULE_GET(hdr.command) >= HAL_MODULE_MGT &&
      IPCCMD_MODULE_GET(hdr.command) <= HAL_MODULE_MAX)
  {
    ret = kbsp_driver_msg_handle(client, hdr.command, client->kbsp_driver);
    if (ret == OS_NO_CALLBACK)
    {
      khal_client_send_return(client, OS_NO_CALLBACK, "%s","NO Callback Func");
    }
    else
    {
      if (IPCCMD_CMD_GET(hdr.command) != HAL_MODULE_CMD_GET)
        khal_client_send_return(client, ret, "cmd:%s", khal_module_cmd_name(hdr.command));
    }
  }
  else
  {
    ret = OS_NO_SDKSPUUORT;
    khal_client_send_return(client, -1, "Client Recv unknown command %d", hdr.command);
    zlog_warn(MODULE_HAL, "Client Recv unknown command %d", hdr.command);
  }
  return 0;
}

static int khal_client_putdata(struct khal_client *client, int cmd, int seqno, int dstpid, char *data, int len)
{
  if (client && client->netlink)
  {
    client->netlink->cmd = cmd;
    client->netlink->seqno = seqno;
    client->netlink->dstpid = dstpid;
    khal_ipcmsg_reset(&client->ipcmsg);
    khal_ipcmsg_put(&client->ipcmsg, data, len);
    return khal_client_read_handle(client);
  }
  return 0;
}

static void khal_client_sock_input(struct sk_buff *__skb)
{
  struct nlmsghdr *nlh = nlmsg_hdr(__skb);
  const char *nlmsg = NULL;
  int *from_pid = NULL;
  nlmsg = (const char *)nlmsg_data(nlh);
  from_pid = (int *)nlmsg;

  if (khalclient && nlh->nlmsg_type == HAL_CFG_REQUEST_CMD)
  {
    khal_client_putdata(khalclient, nlh->nlmsg_type, nlh->nlmsg_seq, ntohl(*from_pid), nlmsg + 4, nlmsg_len(nlh) - 4);
  }
    
  else if (khalclient && nlh->nlmsg_type == HAL_DATA_REQUEST_CMD)
  {
    khal_client_putdata(khalclient, nlh->nlmsg_type, nlh->nlmsg_seq, ntohl(*from_pid), nlmsg + 4, nlmsg_len(nlh) - 4);
  }
  else if (khalclient && nlh->nlmsg_type == HAL_KLOG_REQUEST_CMD)
  {
  }
}

/* Allocate khal_client structure. */
static struct khal_client *khal_client_new(void)
{
  struct khal_client *khal_client;
  khal_client = XMALLOC(MTYPE_HALIPCCLIENT, sizeof(struct khal_client)); // kmalloc(, GFP_KERNEL);
  if (khal_client)
  {
#ifdef ZPL_SDK_KERNEL    
    khal_client->kbsp_driver = XMALLOC(MTYPE_BSP, sizeof(bsp_driver_t)); // kmalloc(, GFP_KERNEL);
    if(khal_client->kbsp_driver == NULL)
    {
      XFREE(MTYPE_HALIPCCLIENT, khal_client);
      khalclient = NULL;
      return khal_client;
    }
#endif    
    memset(khal_client, 0, sizeof(struct khal_client));
    khal_client->ipcmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    khal_client->outmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ / 2;
    if (khal_ipcmsg_create(&khal_client->ipcmsg) != OK)
    {
#ifdef ZPL_SDK_KERNEL      
      XFREE(MTYPE_BSP, khal_client->kbsp_driver);
#endif      
      XFREE(MTYPE_HALIPCCLIENT, khal_client);
      khalclient = NULL;
      return khal_client;
    }
    if (khal_ipcmsg_create(&khal_client->outmsg) != OK)
    {
      khal_ipcmsg_destroy(&khal_client->ipcmsg);
#ifdef ZPL_SDK_KERNEL      
      XFREE(MTYPE_BSP, khal_client->kbsp_driver);
#endif 
      XFREE(MTYPE_HALIPCCLIENT, khal_client);
      khalclient = NULL;
      return khal_client;
    }
    khal_client->debug = 0;
  }
  return khal_client;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free khal_client structure. */
static void khal_client_free(struct khal_client *khal_client)
{
  if (khal_client)
  {
    khal_ipcmsg_destroy(&khal_client->ipcmsg);
    khal_ipcmsg_destroy(&khal_client->outmsg);
    if (khal_client->netlink)
    {
      khal_netlink_destroy(khal_client->netlink);
      khal_client->netlink = NULL;
    }
    khal_client_unregister(khal_client);
#ifdef ZPL_SDK_KERNEL    
    XFREE(MTYPE_BSP, khal_client->kbsp_driver);
#endif    
    XFREE(MTYPE_HALIPCCLIENT, khal_client);
    khalclient = NULL;
  }
}

/* Initialize zebra client.  Argument redist_default is unwanted
   redistribute route type. */
struct khal_client *khal_client_create(void *sdk_driver)
{
  /* Enable zebra client connection by default. */
#ifdef ZPL_SDK_KERNEL
  bsp_driver_t *bspdev = NULL;
#endif
  struct khal_client *khal_client = khal_client_new();
  if (khal_client)
  {
    khal_client_register(khal_client);
    //初始化netlink
    khal_client->netlink = khal_netlink_create("cfg", HAL_CFG_NETLINK_PROTO, 0, &_khal_client_sock_nkc);
    if (!khal_client->netlink)
    {
      zlog_err(MODULE_SDK, "[netlink] create netlink socket error!");
      khal_client_free(khal_client);
      khal_client = NULL;
      return khal_client;
    }
#ifdef ZPL_SDK_KERNEL
    bspdev = (bsp_driver_t*)khal_client->kbsp_driver;
    if(bspdev)
      bspdev->sdk_driver = sdk_driver;
#endif
    khalclient = khal_client;
  }
  return khal_client;
}

int khal_client_destroy(struct khal_client *khal_client)
{
  khal_client_free(khal_client);
  return OK;
}

static int khal_client_send_message(struct khal_client *khal_client)
{
  char *nldata = NULL;
  //int *result = NULL;
  struct nlmsghdr *nlh = NULL;
  struct sk_buff *skb = NULL;
  khal_ipcmsg_hdrlen_set(&khal_client->outmsg);
  skb = XNLMSG_NEW(MTYPE_SDK_DATA, khal_client->outmsg.setp+4);
  if (skb && khal_client->netlink)
  {
    int ret = 0;
    nlh = nlmsg_put(skb, 0, khal_client->netlink->seqno, khal_client->netlink->cmd, khal_client->outmsg.setp, 0);
    nldata = nlmsg_data(nlh);
    if (khal_client->outmsg.setp)
      memcpy(nldata, khal_client->outmsg.buf, khal_client->outmsg.setp);
    ret = khal_netlink_unicast(khal_client->netlink, khal_client->netlink->dstpid, skb);
    return ret;
  }
  return 0;
}

int khal_client_send_report(struct khal_client *khal_client, char *data, int len)
{
  khal_ipcmsg_reset(&khal_client->outmsg);
  khal_ipcmsg_create_header(&khal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_REPORT, 0));
  if (data && len)
  {
      khal_ipcmsg_put(&khal_client->outmsg, data, len);
  }
  if (ZPL_TST_BIT(khal_client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(khal_client->debug, KLOG_DEBUG_SEND))
    zlog_debug(MODULE_HAL, "Client Send Report msg [%d] seqno %d dstpid %d led %d ",
               khal_client->netlink->cmd, khal_client->netlink->seqno, khal_client->netlink->dstpid, len);
  return khal_client_send_message(khal_client);
}

int khal_client_send_return(struct khal_client *khal_client, int ret, char *fmt, ...)
{
  int len = 0;
  va_list args;
  char logbuf[1024];
  struct khal_ipcmsg_result getvalue;
  //struct khal_ipcmsg_result *result = (struct khal_ipcmsg_result *)(khal_client->outmsg.buf + sizeof(struct khal_ipcmsg_header));
  memset(logbuf, 0, sizeof(logbuf));
  memset(&getvalue, 0, sizeof(getvalue));
  khal_ipcmsg_reset(&khal_client->outmsg);
  khal_ipcmsg_create_header(&khal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, 0));
  getvalue.result = ret;
  khal_ipcmsg_result_set(&khal_client->outmsg, &getvalue);
  if (ret != OK)
  {
    va_start(args, fmt);
    len = vsnprintf(logbuf, sizeof(logbuf), fmt, args);
    va_end(args);
    if (len > 0 && len < sizeof(logbuf))
      khal_ipcmsg_put(&khal_client->outmsg, logbuf, len);
  }
  if (ZPL_TST_BIT(khal_client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(khal_client->debug, KLOG_DEBUG_SEND))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] seqno %d dstpid %d result %d %s",
               khal_client->netlink->cmd, khal_client->netlink->seqno, khal_client->netlink->dstpid, ret, (ret != OK) ? logbuf : "OK");
  return khal_client_send_message(khal_client);
}

int khal_client_send_result(struct khal_client *khal_client, int ret, struct khal_ipcmsg_result *getvalue)
{
  //va_list args;
  //struct khal_ipcmsg_result *result = (struct khal_ipcmsg_result *)(khal_client->outmsg.buf + sizeof(struct khal_ipcmsg_header));
  khal_ipcmsg_reset(&khal_client->outmsg);
  khal_ipcmsg_create_header(&khal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, 0));
  getvalue->result = ret;
  khal_ipcmsg_result_set(&khal_client->outmsg, getvalue);
  if (ZPL_TST_BIT(khal_client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(khal_client->debug, KLOG_DEBUG_SEND))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] seqno %d dstpid %d result %d",
               khal_client->netlink->cmd, khal_client->netlink->seqno, khal_client->netlink->dstpid, ret);
  return khal_client_send_message(khal_client);
}

int khal_client_send_result_msg(struct khal_client *khal_client, int ret,
                               struct khal_ipcmsg_result *getvalue, int subcmd, char *msg, int msglen)
{
  //int len = 0;
  //va_list args;
  //struct khal_ipcmsg_result *result = (struct khal_ipcmsg_result *)(khal_client->outmsg.buf + sizeof(struct khal_ipcmsg_header));
  khal_ipcmsg_reset(&khal_client->outmsg);
  khal_ipcmsg_create_header(&khal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, subcmd));
  getvalue->result = ret;
  khal_ipcmsg_result_set(&khal_client->outmsg, getvalue);

  if (msglen && msg)
    khal_ipcmsg_put(&khal_client->outmsg, msg, msglen);
  if (ZPL_TST_BIT(khal_client->debug, KLOG_DEBUG_EVENT) && ZPL_TST_BIT(khal_client->debug, KLOG_DEBUG_SEND))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] seqno %d dstpid %d result %d",
               khal_client->netlink->cmd, khal_client->netlink->seqno, khal_client->netlink->dstpid, ret);
  return khal_client_send_message(khal_client);
}


static int khal_client_ioctl_callback(struct khal_client *client, zpl_uint32 cmd, void *data, int len)
{
  struct khal_ipcmsg_header hdr;
  khal_ipcmsg_reset(&client->ipcmsg);
  khal_ipcmsg_put(&client->ipcmsg, data, 256);
  khal_ipcmsg_get_header(&client->ipcmsg, &hdr);
  return kbsp_driver_msg_handle(client, hdr.command, client->kbsp_driver);
}

static int kbsp_debug_module_handle(struct khal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
  int ret = OK;
  int moudle = 0, enable = 0, value = 0;

  BSP_ENTER_FUNC();
	khal_ipcmsg_getl(&client->ipcmsg, &moudle);
  khal_ipcmsg_getl(&client->ipcmsg, &enable);
	khal_ipcmsg_getl(&client->ipcmsg, &value);
  if (moudle == HAL_KNETPKT_MODULE)
    khal_netpkt_debug_set(enable, value);
  else if (moudle == HAL_KHALCLIENT_MODULE)
  {
    if (enable)
      ZPL_SET_BIT(client->debug, value);
    else
      ZPL_CLR_BIT(client->debug, value);
  }
  BSP_LEAVE_FUNC();
  return ret;
}

static int kbsp_global_module_handle(struct khal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
  BSP_ENTER_FUNC();
  if (subcmd == HAL_GLOBAL_START)
  {
      int dstpid = 0;
      int ifindex = 0;
	    khal_ipcmsg_getl(&client->ipcmsg, &dstpid);
	    khal_ipcmsg_getl(&client->ipcmsg, &ifindex);
      khal_netpkt_dstpid(dstpid);
      khal_netpkt_bind(ifindex, dstpid?1:0);
  }
  BSP_LEAVE_FUNC();
  return 0;
}

static const khal_ipccmd_callback_t  kmoduletable[] = {
#ifdef ZPL_SDK_KERNEL
    HAL_CALLBACK_ENTRY(HAL_MODULE_NONE, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_MGT, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_GLOBAL, kbsp_global_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_SWITCH, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_CPU, kbsp_cpu_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_PORT, kbsp_port_module_handle),
    // HAL_CALLBACK_ENTRY(HAL_MODULE_L3IF, kbsp_l3if_module_handle),
    // HAL_CALLBACK_ENTRY(HAL_MODULE_ROUTE, kbsp_route_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_STP, NULL),
#ifdef ZPL_NSM_MSTP
    HAL_CALLBACK_ENTRY(HAL_MODULE_MSTP, kbsp_mstp_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_MSTP, NULL),
#endif
#ifdef ZPL_NSM_8021X
    HAL_CALLBACK_ENTRY(HAL_MODULE_8021X, kbsp_8021x_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_8021X, NULL),
#endif
#ifdef ZPL_NSM_IGMP
    HAL_CALLBACK_ENTRY(HAL_MODULE_IGMP, kbsp_snooping_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_IGMP, NULL),
#endif
#ifdef ZPL_NSM_DOS
    HAL_CALLBACK_ENTRY(HAL_MODULE_DOS, kbsp_dos_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_DOS, NULL),
#endif
#ifdef ZPL_NSM_MAC
    HAL_CALLBACK_ENTRY(HAL_MODULE_MAC, kbsp_mac_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_MAC, NULL),
#endif
#ifdef ZPL_NSM_MIRROR
    HAL_CALLBACK_ENTRY(HAL_MODULE_MIRROR, kbsp_mirror_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_MIRROR, NULL),
#endif

#ifdef ZPL_NSM_VLAN
    HAL_CALLBACK_ENTRY(HAL_MODULE_QINQ, kbsp_qinq_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_VLAN, kbsp_vlan_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_QINQ, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_VLAN, NULL),
#endif
#ifdef ZPL_NSM_QOS
    HAL_CALLBACK_ENTRY(HAL_MODULE_QOS, kbsp_qos_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_QOS, NULL),
#endif
    HAL_CALLBACK_ENTRY(HAL_MODULE_ACL, NULL),
#ifdef ZPL_NSM_TRUNK
    HAL_CALLBACK_ENTRY(HAL_MODULE_TRUNK, kbsp_trunk_module_handle),
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
    HAL_CALLBACK_ENTRY(HAL_MODULE_DEBUG, kbsp_debug_module_handle),
#else
    HAL_CALLBACK_ENTRY(HAL_MODULE_GLOBAL, kbsp_global_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_L3IF, kbsp_l3if_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_DEBUG, kbsp_debug_module_handle),
#endif  
};





static int kbsp_driver_msg_handle(struct khal_client *client, zpl_uint32 cmd, void *driver)
{
  int ret = OS_NO_CALLBACK;
  int module = IPCCMD_MODULE_GET(cmd);
  khal_ipccmd_callback_t *callback = NULL;
  BSP_ENTER_FUNC();
  if (module > HAL_MODULE_NONE && module < HAL_MODULE_MAX)
  {
    int i = 0;
    for (i = 0; i < ARRAY_SIZE(kmoduletable); i++)
    {
      if (kmoduletable[i].module == module && kmoduletable[i].module_handle)
      {
        callback = &kmoduletable[i];
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



#ifndef ZPL_SDK_KERNEL
static __init int linux_kbe_init(void)
{
		khalclient = khal_client_create(NULL);
		if(khalclient)
		{
			khal_netpkt_init();
			return OK;
		}
		else
		{
			printk("khal_client_create ERROR\r\n");
			return ERROR;
		}	
	  return OK;	
}

static __exit void linux_kbe_exit(void)
{
		if(khalclient)
		{
			khal_client_destroy(khalclient);
			khalclient = NULL;
		}
		khal_netpkt_exit();
	  return ;
}


module_init(linux_kbe_init);
module_exit(linux_kbe_exit);

MODULE_AUTHOR("zhurish <zhurish@163.com>");
MODULE_DESCRIPTION("Linux kbe library");
MODULE_LICENSE("Dual BSD/GPL");
#endif