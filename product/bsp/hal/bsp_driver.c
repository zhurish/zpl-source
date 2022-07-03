/*
 * bsp_driver.c
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"
#include "vrf.h"
#include "vty.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"

#include "bsp_driver.h"

#ifndef ZPL_SDK_KERNEL
#include "bsp_include.h"
#endif
#ifdef ZPL_SDK_USER
#include "sdk_driver.h"
#endif



#if defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_KERNEL)
static int sdk_driver_set_request(sdk_driver_t *sdkdriver, char *data, int len, void *p);
#endif

bsp_driver_t bsp_driver;

struct module_list module_list_sdk = 
{ 
	.module=MODULE_SDK, 
	.name="SDK\0", 
	.module_init=bsp_module_init, 
	.module_exit=bsp_module_exit, 
	.module_task_init=bsp_module_task_init, 
	.module_task_exit=bsp_module_task_exit, 
	.module_cmd_init=NULL, 
	.taskid=0,
};

int bsp_driver_mac_cache_add(bsp_driver_t *sdkdriver, zpl_uint8 port, zpl_uint8 *mac, vlan_t vid, zpl_uint8 isstatic, zpl_uint8 isage, zpl_uint8 vaild)
{
	int i = 0;
	for(i = 0; i < sdkdriver->mac_cache_max; i++)
	{
		if(sdkdriver->mac_cache_entry[i].use == 1 && (memcmp(sdkdriver->mac_cache_entry[i].mac, mac, ETH_ALEN) == 0))
		{
			sdkdriver->mac_cache_entry[i].port = port;
			sdkdriver->mac_cache_entry[i].vid = vid;
			sdkdriver->mac_cache_entry[i].is_valid = vaild;
			sdkdriver->mac_cache_entry[i].is_age = isage;
			sdkdriver->mac_cache_entry[i].is_static = isstatic;
			sdkdriver->mac_cache_entry[i].use = 1;
			return OK;
		}
	}
	for(i = 0; i < sdkdriver->mac_cache_max; i++)
	{
		if(sdkdriver->mac_cache_entry[i].use == 0)
		{
			sdkdriver->mac_cache_entry[i].port = port;
			sdkdriver->mac_cache_entry[i].vid = vid;
			sdkdriver->mac_cache_entry[i].is_valid = vaild;
			sdkdriver->mac_cache_entry[i].is_age = isage;
			sdkdriver->mac_cache_entry[i].is_static = isstatic;
			sdkdriver->mac_cache_entry[i].use = 1;
			memcpy(sdkdriver->mac_cache_entry[i].mac, mac, ETH_ALEN);
			sdkdriver->mac_cache_num++;
			return OK;
		}
	}
	return OK;
}


int bsp_driver_mac_cache_update(bsp_driver_t *sdkdriver, zpl_uint8 *mac, zpl_uint8 isage)
{
	int i = 0;
	for(i = 0; i < sdkdriver->mac_cache_max; i++)
	{
		if(sdkdriver->mac_cache_entry[i].use == 1 && (memcmp(sdkdriver->mac_cache_entry[i].mac, mac, ETH_ALEN) == 0))
		{
			sdkdriver->mac_cache_entry[i].use = isage?1:0;
			if(isage == 0)
				sdkdriver->mac_cache_num--;
			return OK;
		}
	}
	return OK;
}

#if defined(ZPL_SDK_USER) || defined(ZPL_SDK_NONE)
static const hal_ipccmd_callback_t const moduletable[] = {
    HAL_CALLBACK_ENTRY(HAL_MODULE_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_MODULE_MGT, NULL),
	HAL_CALLBACK_ENTRY(HAL_MODULE_GLOBAL, bsp_global_module_handle),
	HAL_CALLBACK_ENTRY(HAL_MODULE_SWITCH, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_CPU, bsp_cpu_module_handle),
	HAL_CALLBACK_ENTRY(HAL_MODULE_PORT, bsp_port_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_L3IF, bsp_l3if_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_ROUTE, bsp_route_module_handle),
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
};
#endif

int bsp_driver_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver)
{
	int ret = OS_NO_CALLBACK;
	int module = IPCCMD_MODULE_GET(cmd);
	hal_ipccmd_callback_t * callback = NULL;
	BSP_ENTER_FUNC();
#if defined(ZPL_SDK_USER) || defined(ZPL_SDK_NONE)
	if(module > HAL_MODULE_NONE && module < HAL_MODULE_MAX)
	{
		int i = 0;
		for(i = 0; i < ZPL_ARRAY_SIZE(moduletable); i++)
		{
			//zlog_warn(MODULE_HAL, "=== this module:%d  %d", moduletable[i].module, module);
			if(moduletable[i].module == module && moduletable[i].module_handle)
			{
				callback = &moduletable[i];
				//ret = (moduletable[i].module_handle)(client, IPCCMD_CMD_GET(cmd), IPCCMD_SUBCMD_GET(cmd), driver);
				break;
			}
		}
		if(callback)
			ret = (callback->module_handle)(client, IPCCMD_CMD_GET(cmd), IPCCMD_SUBCMD_GET(cmd), driver);
		else
		{
			zlog_warn(MODULE_HAL, "Can not Find this module:%d ", module);
			ret = OS_NO_CALLBACK;
		}		
	}
#elif defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_KERNEL)
	if(module > HAL_MODULE_NONE && module < HAL_MODULE_MAX)
	{
		if(IPCCMD_CMD_GET(cmd) != HAL_MODULE_CMD_ACK)
			ret = sdk_driver_set_request(client->bsp_driver, client->ipcmsg.buf, client->ipcmsg.setp, client);
		else
		{
			zlog_warn(MODULE_HAL, "=============HAL_MODULE_CMD_ACK:%d ", module);
			ret = OS_NO_CALLBACK;
		}	
	}
#endif	
	BSP_LEAVE_FUNC();
	return ret;
}

int bsp_module_func(bsp_driver_t *bspdriver, bsp_sdk_func init_func, bsp_sdk_func start_func, 
	bsp_sdk_func stop_func, bsp_sdk_func exit_func)
{
    bspdriver->bsp_sdk_init = init_func;
    bspdriver->bsp_sdk_start = start_func;
    bspdriver->bsp_sdk_stop = stop_func;
    bspdriver->bsp_sdk_exit = exit_func;
	return OK;
}


static int bsp_driver_task(void *p)
{
  struct thread_master *master = (struct thread_master *)p;
  module_setup_task(master->module, os_task_id_self());
  while (thread_mainloop(master))
    ;
  return OK;
}

int bsp_driver_init(bsp_driver_t *bspdriver)
{
  struct hal_client *bsp = hal_client_create(MODULE_BSP, zpl_false);
  if (bsp)
  {
    bsp->debug = 0xffff;
    bspdriver->hal_client = bsp;
    bsp->master = bspdriver->master = thread_master_module_create(MODULE_BSP);
  
    hal_client_start(bsp);
    return OK;
  }
  return ERROR;
}

int bsp_driver_exit(bsp_driver_t *bspdriver)
{
  hal_client_destroy(bspdriver->hal_client);
  if (bspdriver->master)
  {
    thread_master_free(bspdriver->master);
    bspdriver->master = NULL;
  }
  return OK;
}

int bsp_driver_task_init(bsp_driver_t *bspdriver)
{
  if (!bspdriver->master)
  {
    bspdriver->master = thread_master_module_create(MODULE_BSP);
  }
  if (bspdriver->taskid <= 0)
    bspdriver->taskid = os_task_create("bspTask", OS_TASK_DEFAULT_PRIORITY,
                                    0, bsp_driver_task, bspdriver->master, OS_TASK_DEFAULT_STACK * 4);
  if (bspdriver->taskid > 0)
  {
    module_setup_task(MODULE_BSP, bspdriver->taskid);
    return OK;
  }
  return ERROR;
}

int bsp_driver_task_exit(bsp_driver_t *bspdriver)
{
  if (bspdriver->taskid > 0)
    os_task_destroy(bspdriver->taskid);
  if (bspdriver->master)
  {
    thread_master_free(bspdriver->master);
    bspdriver->master = NULL;
  }
  return OK;
}


int bsp_module_init(void)
{
	memset(&bsp_driver, 0, sizeof(bsp_driver));
	bsp_driver_init(&bsp_driver);
	bsp_module_func(&bsp_driver, sdk_driver_init, sdk_driver_start, sdk_driver_stop, sdk_driver_exit);

	memset(bsp_driver.phyports, 0, sizeof(bsp_driver.phyports));
	bsp_driver.phyports[0].phyport = 0;
	bsp_driver.phyports[1].phyport = 1;
	bsp_driver.phyports[2].phyport = 2;
	bsp_driver.phyports[3].phyport = 3;
	bsp_driver.phyports[4].phyport = 4;
	bsp_driver.phyports[5].phyport = -1;
	bsp_driver.phyports[6].phyport = -1;
	bsp_driver.phyports[7].phyport = 8;// cpu port

	if(bsp_driver.bsp_sdk_init)
	{
		(bsp_driver.bsp_sdk_init)(&bsp_driver, bsp_driver.sdk_driver);
	}
	if(bsp_driver.sdk_driver)
	{
		hal_client_callback(bsp_driver.hal_client, bsp_driver_msg_handle, &bsp_driver);
	}
	return OK;
}

int bsp_module_task_init(void)
{
	bsp_driver_task_init(&bsp_driver);
	return OK;
}

int bsp_module_start(void)
{
	struct hal_ipcmsg_hwport porttbl[5];
	memset(porttbl, 0, sizeof(porttbl));

	if(bsp_driver.bsp_sdk_start)
	{
		(bsp_driver.bsp_sdk_start)(&bsp_driver, bsp_driver.sdk_driver);
	}
	bsp_driver.cpu_port = ((sdk_driver_t*)bsp_driver.sdk_driver)->cpu_port;

	porttbl[0].type = IF_ETHERNET;//lan1
	porttbl[0].port = 1;
  	porttbl[0].phyid = 4;

	porttbl[1].type = IF_ETHERNET;//lan2
	porttbl[1].port = 2;
  	porttbl[1].phyid = 0;

	porttbl[2].type = IF_ETHERNET;//lan3
	porttbl[2].port = 3;
  	porttbl[2].phyid = 1;

	porttbl[3].type = IF_ETHERNET;//lan4
	porttbl[3].port = 4;
  	porttbl[3].phyid = 2;

	porttbl[4].type = IF_ETHERNET;//wan
	porttbl[4].port = 5;
  	porttbl[4].phyid = 3;
  /*
				port0: port@0 {
					reg = <0>;
					label = "lan2";
				};

				port1: port@1 {
					reg = <1>;
					label = "lan3";
				};

				port2: port@2 {
					reg = <2>;
					label = "lan4";
				};

				port3: port@3 {
					reg = <3>;
					label = "wan";
				};

				port4: port@4 {
					reg = <4>;
					label = "lan1";
				};
        */
	os_sleep(1);
	zlog_debug(MODULE_BSP, "BSP Init");
  	hal_client_bsp_register(bsp_driver.hal_client, 0,
        0, 0, 5, "V0.0.0.1");

	os_sleep(1);
	zlog_debug(MODULE_BSP, "SDK Init, waitting...");


	zlog_debug(MODULE_BSP, "SDK Register Port Table Info.");
  	hal_client_bsp_hwport_register(bsp_driver.hal_client, 5, porttbl);

	hal_client_event(HAL_EVENT_REGISTER, bsp_driver.hal_client, 1);
	zlog_debug(MODULE_BSP, "SDK Init, Done.");
	return OK;
}

int bsp_module_exit(void)
{
	if(bsp_driver.bsp_sdk_stop)
		return (bsp_driver.bsp_sdk_stop)(&bsp_driver, bsp_driver.sdk_driver);
	bsp_driver_exit(&bsp_driver);
	return OK;
}

int bsp_module_task_exit(void)
{
	bsp_driver_task_exit(&bsp_driver);
	if(bsp_driver.bsp_sdk_exit)
		return (bsp_driver.bsp_sdk_exit)(&bsp_driver, bsp_driver.sdk_driver);
	return OK;
}

#ifdef ZPL_SDK_NONE
static sdk_driver_t * sdk_driver_malloc(void)
{
	return (sdk_driver_t *)malloc(sizeof(sdk_driver_t));
}

static int sdk_driver_free(sdk_driver_t *sdkdriver)
{
	if(sdkdriver)
		free(sdkdriver);
	sdkdriver = NULL;
	return OK;
}

int sdk_driver_init(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	bsp->sdk_driver = sdk_driver_malloc();
	if(bsp->sdk_driver)
		return OK;
	else
		return ERROR;	
}

int sdk_driver_start(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	return OK;
}

int sdk_driver_stop(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	return OK;
}

int sdk_driver_exit(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	if(sdkdriver)
		sdk_driver_free(sdkdriver);
	return OK;
}
#elif defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_KERNEL)
static int hal_bsp_klog_set(zpl_uint32 val)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, val);
	command = IPCCMD_SET(HAL_MODULE_DEBUG, HAL_MODULE_CMD_REQ, HAL_CLIENT_DEBUG+3);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

static int hal_bsp_netpkt_set(zpl_uint32 val)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, val);
	command = IPCCMD_SET(HAL_MODULE_DEBUG, HAL_MODULE_CMD_REQ, HAL_NETPKT_DEBUG + 1);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
static int hal_bsp_netpkt_bind_set(zpl_uint32 val)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, val?1:0);
	hal_ipcmsg_putl(&ipcmsg, val);
	command = IPCCMD_SET(HAL_MODULE_DEBUG, HAL_MODULE_CMD_REQ, HAL_NETPKT_DEBUG + 2);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

static int sdk_cfg_socket_create(sdk_driver_t *sdkdriver)
{
	int ret = 0;
	struct ipstack_sockaddr_nl bindaddr;
	sdkdriver->cfg_sock = ipstack_socket(OS_STACK, IPSTACK_AF_NETLINK, IPSTACK_SOCK_RAW, HAL_CFG_NETLINK_PROTO);
	if (ipstack_invalid(sdkdriver->cfg_sock))
		return ERROR;
	memset(&bindaddr, 0, sizeof(struct  ipstack_sockaddr_nl));	
	bindaddr.nl_family = IPSTACK_AF_NETLINK;
	bindaddr.nl_pid = getpid();
	bindaddr.nl_groups = 0;
	ret = ipstack_bind(sdkdriver->cfg_sock, &bindaddr, sizeof(bindaddr));
	if(ret < 0)
	{
		zlog_err(MODULE_SDK, "Can not bind to socket:%s", ipstack_strerror(ipstack_errno));
		ipstack_close(sdkdriver->cfg_sock);
	}
	ipstack_set_nonblocking(sdkdriver->cfg_sock);
	return OK;
	hal_bsp_klog_set(getpid());
	hal_bsp_netpkt_set(getpid());
	hal_bsp_netpkt_bind_set(if_nametoindex("eth0"));
	return OK;
}

static int sdk_cfg_socket_close(sdk_driver_t *sdkdriver)
{
	/* Close ipstack_socket. */
	if (!ipstack_invalid(sdkdriver->cfg_sock))
	{
		ipstack_close(sdkdriver->cfg_sock);
	}
	return OK;
}


static int sdk_cfg_netlink_flush(sdk_driver_t *sdkdriver)
{
	int ret = 0;
	fd_set readfds;	
	zpl_uint8 msgbuf[4096];
	struct ipstack_sockaddr_nl snl;
	struct ipstack_msghdr msg;
	struct ipstack_iovec iov;
	FD_ZERO(&readfds);
	FD_SET(sdkdriver->cfg_sock._fd, &readfds);
	while (1)
	{
		iov.iov_base = msgbuf;
		iov.iov_len = sizeof(msgbuf);
		msg.msg_name = (void *)&snl;
		msg.msg_namelen = sizeof snl;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		ret = os_select_wait(sdkdriver->cfg_sock._fd + 1, &readfds, NULL, 5);
		if (ret == OS_TIMEOUT)
		{
			return OS_TIMEOUT;
		}
		if (ret == ERROR)
		{
			_OS_ERROR("os_select_wait to read(%d) %s\n", fd, strerror(ipstack_errno));
			return ERROR;
		}	
		if (FD_ISSET(sdkdriver->cfg_sock._fd, &readfds))
		{
			ret = ipstack_recvmsg(sdkdriver->cfg_sock, &msg, 0);
		}
	}
	return 0;
}
static int sdk_cfg_netlink_parse_info(sdk_driver_t *sdkdriver,
									  int (*filter)(sdk_driver_t *, char *, int, void *), void *p)
{
	zpl_int32 status;
	int ret = 0;
	int error = 0;
	fd_set readfds;	
	zpl_uint8 msgbuf[4096];
	struct ipstack_sockaddr_nl snl;
	struct ipstack_nlmsghdr *h = NULL;
	struct ipstack_msghdr msg;
	struct ipstack_iovec iov;
	FD_ZERO(&readfds);
	FD_SET(sdkdriver->cfg_sock._fd, &readfds);
	while (1)
	{
		iov.iov_base = msgbuf;
		iov.iov_len = sizeof(msgbuf);

		msg.msg_name = (void *)&snl;
		msg.msg_namelen = sizeof snl;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;

		ret = os_select_wait(sdkdriver->cfg_sock._fd + 1, &readfds, NULL, 2000);
		if (ret == OS_TIMEOUT)
		{
			return OS_TIMEOUT;
		}
		if (ret == ERROR)
		{
			_OS_ERROR("os_select_wait to read(%d) %s\n", fd, strerror(ipstack_errno));
			return ERROR;
		}
		if (!FD_ISSET(sdkdriver->cfg_sock._fd, &readfds))
		{
			_OS_ERROR("no events on sockfd(%d) found\n", fd);
			return ERROR;
		}
		status = ipstack_recvmsg(sdkdriver->cfg_sock, &msg, 0);
		if (status < 0)
		{
			if (ipstack_errno == IPSTACK_ERRNO_EINTR)
				continue;
			if (ipstack_errno == IPSTACK_ERRNO_EWOULDBLOCK || ipstack_errno == IPSTACK_ERRNO_EAGAIN)
				break;
			zlog_err(MODULE_PAL, "ipstack_recvmsg from [%d] overrun: %s", sdkdriver->cfg_sock._fd,
					 ipstack_strerror(ipstack_errno));
			continue;
		}

		if (status == 0)
		{
			zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] EOF", sdkdriver->cfg_sock._fd);
			return -1;
		}

		if (msg.msg_namelen != sizeof snl)
		{
			zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] sender address length error: length %d",
					 sdkdriver->cfg_sock._fd, msg.msg_namelen);
			return -1;
		}
		h = (struct ipstack_nlmsghdr *)msgbuf;
		while (IPSTACK_NLMSG_OK(h, status))
		{
			/* Finish of reading. */
			if (h->nlmsg_type == IPSTACK_NLMSG_DONE)
			{
				zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] IPSTACK_NLMSG_DONE", sdkdriver->cfg_sock._fd);
				return ret;
			}

			/* Error handling. */
			if (h->nlmsg_type == IPSTACK_NLMSG_ERROR)
			{
				struct ipstack_nlmsgerr *err = (struct ipstack_nlmsgerr *)IPSTACK_NLMSG_DATA(h);
				int errnum = err->error;
				int msg_type = err->msg.nlmsg_type;

				/* If the error field is zero, then this is an ACK */
				if (err->error == 0)
				{
					if (IS_HAL_IPCMSG_DEBUG_EVENT(sdkdriver->debug))
					{
						zlog_debug(MODULE_SDK,
								   "%s: ipstack_recvmsg [%d] ACK: type=%u, seq=%u, pid=%u",
								   __FUNCTION__, sdkdriver->cfg_sock._fd,
								   err->msg.nlmsg_type, err->msg.nlmsg_seq,
								   err->msg.nlmsg_pid);
					}

					/* return if not a multipart message, otherwise continue */
					if (!(h->nlmsg_flags & IPSTACK_NLM_F_MULTI))
					{
						zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] IPSTACK_NLM_F_MULTI", sdkdriver->cfg_sock._fd);
						return 0;
					}
					continue;
				}

				if (h->nlmsg_len < IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_nlmsgerr)))
				{
					zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] error: message truncated", sdkdriver->cfg_sock._fd);
					return -1;
				}

				zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] error: %s, type=%u, seq=%u, pid=%u",
						 sdkdriver->cfg_sock._fd, ipstack_strerror(-errnum),
						 msg_type,
						 err->msg.nlmsg_seq, err->msg.nlmsg_pid);
				return -1;
			}

			if (filter)
			{
				error = (*filter)(sdkdriver, IPSTACK_NLMSG_DATA (h), IPSTACK_NLMSG_PAYLOAD(h, 0), p);
				if (error < 0)
				{
					zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] filter function error", sdkdriver->cfg_sock._fd);
					ret = error;
				}
				ret = error;
			}
			else
				ret = 0;
			if ((h->nlmsg_flags & IPSTACK_NLM_F_MULTI))
				h = IPSTACK_NLMSG_NEXT(h, status);	
			else
			{
				zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] END", sdkdriver->cfg_sock._fd);
				return ret;	
			}
		}

		/* After error care. */
		if (msg.msg_flags & IPSTACK_MSG_TRUNC)
		{
			zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] error: message truncated!", sdkdriver->cfg_sock._fd);
			zlog_err(MODULE_SDK, "Must restart with larger --nl-bufsize value!");
			continue;
		}
		if (status)
		{
			zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] error: data remnant size %d", sdkdriver->cfg_sock._fd, status);
			return -1;
		}
	}
	zlog_err(MODULE_SDK, "ipstack_recvmsg [%d] end", sdkdriver->cfg_sock._fd);
	return ret;
}


static int sdk_cfg_netlink_talk(sdk_driver_t *sdkdriver, struct ipstack_nlmsghdr *n, int (*filter)(sdk_driver_t *, char *, int, void *), void *p)
{
	zpl_uint32 status;
	struct ipstack_sockaddr_nl snl;
	struct ipstack_iovec iov = { .iov_base = (void *) n, .iov_len = n->nlmsg_len };
	struct ipstack_msghdr msg ={ .msg_name = (void *) &snl, .msg_namelen = sizeof snl, .msg_iov = &iov, .msg_iovlen = 1, };
	int save_errno;

	memset(&snl, 0, sizeof snl);
	snl.nl_family = IPSTACK_AF_NETLINK;

	if (IS_HAL_IPCMSG_DEBUG_EVENT(sdkdriver->debug))
		zlog_debug(MODULE_PAL, "netlink_talk: cfg type (%u), seq=%u", n->nlmsg_type, n->nlmsg_seq);

	sdk_cfg_netlink_flush(sdkdriver);

	/* Send message to netlink interface. */
	status = ipstack_sendmsg(sdkdriver->cfg_sock, &msg, 0);
	save_errno = ipstack_errno;
	if (status < 0)
	{
		zlog_err(MODULE_PAL, "netlink_talk ipstack_sendmsg() error: %s",
				ipstack_strerror(save_errno));
		return -1;
	}
	
	return sdk_cfg_netlink_parse_info(sdkdriver, filter, p);
}

static int sdk_cfg_message_request(sdk_driver_t *sdkdriver, char *data, int len, int (*filter)(sdk_driver_t *, char *, int, void *), void *p)
{
	int	*from_pid = NULL;
	struct ipstack_nlmsghdr *nlh = NULL;
	struct hal_ipcmsg_header *header = NULL;
	struct hal_client *client = (struct hal_client *)p;
  	struct 
  	{
    	struct ipstack_nlmsghdr nlh;
		char buf[2048];
  	} req;	

	memset (&req.nlh, 0, sizeof (struct ipstack_nlmsghdr));  
  	nlh = &req.nlh;
  	nlh->nlmsg_len = IPSTACK_NLMSG_LENGTH (len+4);
  	nlh->nlmsg_flags = IPSTACK_NLM_F_CREATE | IPSTACK_NLM_F_REQUEST;
  	nlh->nlmsg_type = HAL_CFG_REQUEST_CMD;
	nlh->nlmsg_seq = ++sdkdriver->cfg_seq;
	nlh->nlmsg_flags |= IPSTACK_NLM_F_ACK;
	nlh->nlmsg_pid = 0;
	from_pid = (int*)req.buf;
	*from_pid = htonl(getpid());
	if(len)
		memcpy (&req.buf[4], data, len); 
	header = (struct hal_ipcmsg_header *)data;
	//zlog_debug(MODULE_HAL, "===========Client send message relay [%s] %d ",
    //           hal_module_cmd_name(ntohl(header->command)), ntohs(header->length));
	return sdk_cfg_netlink_talk(sdkdriver, nlh, filter, p);
}

static int sdk_cfg_message_parse_default(sdk_driver_t *sdkdriver, char *buf, int len, void *p)
{
	struct hal_ipcmsg_header header;
	struct hal_client *halclient = p;
	hal_ipcmsg_reset(&halclient->outmsg);
	hal_ipcmsg_put(&halclient->outmsg, buf, len);
	hal_ipcmsg_get_header(&halclient->outmsg, &header);
	halclient->outmsg.getp -= sizeof(struct hal_ipcmsg_header);

	//zlog_debug(MODULE_HAL, "===========Client Recv message relay [%s] %d ",
    //           hal_module_cmd_name(header.command), header.length);

	return hal_client_send_message(halclient, 0);
}


static int sdk_driver_set_request(sdk_driver_t *sdkdriver, char *data, int len, void *p)
{
	if(!ipstack_invalid(sdkdriver->cfg_sock))
		return sdk_cfg_message_request(sdkdriver, data, len, sdk_cfg_message_parse_default, p);
	return OK;	
}

static sdk_driver_t * sdk_driver_malloc(void)
{
	return (sdk_driver_t *)malloc(sizeof(sdk_driver_t));
}

static int sdk_driver_free(sdk_driver_t *sdkdriver)
{
	if(sdkdriver)
		free(sdkdriver);
	sdkdriver = NULL;
	return OK;
}

int sdk_driver_init(struct bsp_driver *bsp, zpl_void *p)
{
	bsp->sdk_driver = sdk_driver_malloc();
	if(bsp->sdk_driver)
	{
		if(sdk_cfg_socket_create(bsp->sdk_driver) != OK)
		{
			sdk_driver_free(bsp->sdk_driver);
			return ERROR;
		}
		return OK;
	}
	else
		return ERROR;	
}

int sdk_driver_start(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	return OK;
}

int sdk_driver_stop(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	return OK;
}

int sdk_driver_exit(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	if(sdkdriver)
	{
		sdk_cfg_socket_close(sdkdriver);
		sdk_driver_free(sdkdriver);
	}
	return OK;
}
#endif