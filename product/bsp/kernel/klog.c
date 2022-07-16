
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#ifndef ZPL_SDK_KERNEL
#undef  ZPL_SDK_NONE
#undef  ZPL_SDK_USER
#endif
#include "bsp_types.h"
#include "khal_netlink.h"


static struct hal_netlink *klog = NULL;


static const char *sdklog_priority[] = { "emerg", "alert", "crit", "err",
		"warning", "notice", "info", "debug", "trapping", "focetrap", NULL, };
		
static struct netlink_kernel_cfg _klog_sock_nkc = {
	.groups = 0,
	.flags = 0,
	.input = NULL,
	.cb_mutex = NULL,
	.bind = NULL,
	.unbind = NULL,
	.compare = NULL,
};

int klog_init(void)
{
	klog = hal_netlink_create("klog", HAL_KLOG_NETLINK_PROTO, 0, &_klog_sock_nkc);
	if (!klog) 
    {
		zlog_err(MODULE_SDK, "[netlink] create netlink socket error!");
		return ERROR;
	}
	klog->debug = 0xff;
	return 0;
}

void klog_exit(void)
{
  if(klog)
  {
    hal_netlink_destroy(klog);
    klog = NULL;
  }
}


void klog_printk(int module, int priority, const char *fmt, ...)
{
	int n = 0;
	va_list args;
	char buf[2048];
	char *nldata = NULL;
	struct nlmsghdr *nlh = NULL;
  	struct sk_buff *skb = NULL;// (buf,  size);
	/*if(klog && klog->dstpid)
	{
		va_start(args, fmt);
		n = vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);
		if(priority <= klog->debug)
		{
			skb = XNLMSG_NEW(MTYPE_SDK_DATA, 2048);
			nlh = nlmsg_put(skb, 0, klog->seqno++, module<<8|priority, n, 0);
			nldata = nlmsg_data(nlh);
			if(n)
				memcpy(nldata, buf, n);
			hal_netlink_unicast(klog, klog->dstpid, skb);
		}
	}
	else*/
	{
		va_start(args, fmt);
		n = vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);		
		printk("%s\n", buf);
	}
}

int klog_dstpid(int pid)
{
  if(klog)
    hal_netlink_group_dstpid(klog, 0,  pid);
  return OK;  
}

int klog_level(int level)
{
  if(klog)
    klog->debug = level;
  return OK;  
}




void sdk_log(int module, int priority, const char *file, const char *func, const int line, const char *format, ...)
{
	char buflog[2048];
	va_list args;
	va_start(args, format);
	printk("%-8s: ", sdklog_priority[priority]);
	printk("(%s line %d:) ", file, line);
	vsprintf(buflog, format, args);
	printk("%s", buflog);
	va_end(args);
}