/*
 * service.c
 *
 *  Created on: Oct 28, 2018
 *      Author: zhurish
 */

#include "service.h"
#ifdef ZPL_SERVICE_SNTPC
#include "sntpcLib.h"
#endif
#ifdef ZPL_SERVICE_SNTPS
#include "sntpsLib.h"
#endif
#ifdef ZPL_SERVICE_TFTPC
#include "tftpLib.h"
#endif
#ifdef ZPL_SERVICE_TFTPD
#include "tftpdLib.h"
#endif
#ifdef ZPL_SERVICE_FTPC
#include "ftpLib.h"
#endif
#ifdef ZPL_SERVICE_FTPD
#include "ftpdLib.h"
#endif
#ifdef ZPL_SERVICE_TELNET
#include "telnetLib.h"
#endif
#ifdef ZPL_SERVICE_TELNETD
#include "telnetLib.h"
#endif
#ifdef ZPL_SERVICE_PING
#include "pingLib.h"
#endif
#ifdef ZPL_SERVICE_TRACEROUTE
#include "tracerouteLib.h"
#endif
#ifdef ZPL_SERVICE_UBUS_SYNC
#include "ubus_sync.h"
#endif

struct ip_tcpsrv_s
{
	char *ipaddress;
	int port;
	//sock fd, struct sockaddr_in, user param
	int (*_accept_func)(zpl_socket_t, void *, void *);
	zpl_socket_t fd;
	void *pUser;
	void *t_hread;
};

struct global_service
{
	zpl_taskid_t sys_task_id;
	void *master_eloop;
	os_list_t	ipsrvlist;
};

static struct global_service  _global_service;

struct module_list module_list_utils = 
{ 
	.module=MODULE_SERVICE, 
	.name="SERVICE\0", 
	.module_init=service_module_init, 
	.module_exit=service_module_exit, 
	.module_task_init=service_task_init, 
	.module_task_exit=service_task_exit, 
	.module_cmd_init=service_clicmd_init, 
	.taskid=0,
	.flags = 0,
};

static void ip_srv_node_free(void *p)
{
	if(p)
	{
		struct ip_tcpsrv_s *info = p;
		if(info->ipaddress)
			free(info->ipaddress);
		if(info->t_hread)
		{
			eloop_cancel(info->t_hread);
		}
			
		free(p);
	}
}

#ifdef ZPL_SERVICE_FTPD
const char *ftpd_hostname(void)
{
	return "VxWorks 5.5";
}

static int ftpd_loginVerify(char *user, char *pass)
{
	return user_authentication(user, pass);
}
#endif

static int service_main_task(void *argv)
{
	module_list_utils.taskid = os_task_id_self();
	host_waitting_loadconfig();
	eloop_mainloop(_global_service.master_eloop);
	return 0;
}

int service_task_init (void)
{
	if(_global_service.master_eloop == NULL)
		_global_service.master_eloop = eloop_master_name_create("SERVICE");
	if(_global_service.sys_task_id == 0)
		_global_service.sys_task_id = os_task_create("sysTask", OS_TASK_DEFAULT_PRIORITY,
	               0, service_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if(_global_service.sys_task_id)
	{
		module_list_utils.taskid = _global_service.sys_task_id;
		//module_setup_task(MODULE_SERVICE, sys_task_id);
		return OK;
	}
	return ERROR;
}

int service_task_exit (void)
{
	if(_global_service.sys_task_id)
		os_task_destroy(_global_service.sys_task_id);
	_global_service.sys_task_id = 0;
	return OK;
}


int service_module_init (void)
{
	memset(&_global_service, 0, sizeof(_global_service));
	if(_global_service.master_eloop == NULL)
		_global_service.master_eloop = eloop_master_name_create("SERVICE");
#ifdef ZPL_SERVICE_TFTPC
#endif
#ifdef ZPL_SERVICE_TFTPD
	tftpdInit(_global_service.master_eloop, NULL);
#endif
#ifdef ZPL_SERVICE_FTPC
	ftpLibInit(5);
#endif
#ifdef ZPL_SERVICE_FTPD
	ftpdInit (_global_service.master_eloop, ftpd_loginVerify);
#endif
#ifdef ZPL_SERVICE_SNTPS
	sntpsInit(_global_service.master_eloop);
#endif
#ifdef ZPL_SERVICE_SNTPC
	sntpcInit(_global_service.master_eloop);
#endif
#ifdef ZPL_SERVICE_TELNET
#endif
#ifdef ZPL_SERVICE_TELNETD
#endif
#ifdef ZPL_SERVICE_PING
#endif
#ifdef ZPL_SERVICE_TRACEROUTE
#endif
#ifdef ZPL_SERVICE_UBUS_SYNC
	ubus_sync_init(_global_service.master_eloop);
#endif
	os_list_init(&_global_service.ipsrvlist, ip_srv_node_free);
	return OK;
}

int service_module_exit (void)
{
#ifdef ZPL_SERVICE_FTPD
	ftpdDisable();
#endif
#ifdef ZPL_SERVICE_TFTPD
	tftpdUnInit();
#endif
#ifdef ZPL_SERVICE_SNTPS
	sntpsDisable();
#endif
#ifdef ZPL_SERVICE_SNTPC
	sntpcDisable();
#endif
#ifdef ZPL_SERVICE_UBUS_SYNC
	ubus_sync_exit();
#endif
	if(_global_service.master_eloop)
		eloop_master_free(_global_service.master_eloop);
	_global_service.master_eloop = NULL;
	os_list_delete_all_node_withdata(&_global_service.ipsrvlist);
	return OK;
}


static int ip_srv_accept_thread (struct eloop *t)
{
	struct ip_tcpsrv_s *data = ELOOP_ARG(t);
	if(data)
	{
		struct ipstack_sockaddr_in client;
		data->t_hread = NULL;
		zpl_socket_t fd = ipstack_sock_accept (data->fd, &client);
		if(!ipstack_invalid(fd))
		{
			(data->_accept_func)(fd, &client, data->pUser);
		}
		data->t_hread = eloop_add_read(_global_service.master_eloop, ip_srv_accept_thread, data, data->fd);
	}
	return OK;
}



int ip_tcpsrv_add(char *ipaddress, int port, int (*_accept_func)(zpl_socket_t, void * , void *), void *pUser)
{
	os_listnode *node = NULL;
	struct ip_tcpsrv_s *data = NULL;
	for(OS_ALL_LIST_ELEMENTS_RO(&_global_service.ipsrvlist, node, data))
	{
		if(data && data->port == port)
		{
			return ERROR;
		}
	}
	data = malloc(sizeof(struct ip_tcpsrv_s));
	if(data)
	{
		memset(data, 0, sizeof(struct ip_tcpsrv_s));
		if(ipaddress)
			data->ipaddress = strdup(ipaddress);
		data->port = port;
		data->_accept_func = _accept_func;
		data->pUser = pUser;
		data->fd = ipstack_sock_create(IPSTACK_IPCOM, zpl_true);
		if(!ipstack_invalid(data->fd))
		{
			if(ipstack_sock_bind(data->fd, ipaddress, port) == OK)
			{
				if(ipstack_sock_listen(data->fd, 16) == OK)
				{
					data->t_hread = eloop_add_read(_global_service.master_eloop, ip_srv_accept_thread, data, data->fd);
					os_listnode_add(&_global_service.ipsrvlist, data);
					return OK;
				}
				ipstack_close(data->fd);
				ipstack_drstroy(data->fd);
			}
			ipstack_close(data->fd);
			ipstack_drstroy(data->fd);
		}
		ipstack_drstroy(data->fd);
		if(data->ipaddress)
			free(data->ipaddress);
		free(data);	
	}
	return ERROR;
}
int ip_tcpsrv_del(int port)
{
	os_listnode *node = NULL;
	struct ip_tcpsrv_s *data = NULL;
	int flag = 0;
	for(OS_ALL_LIST_ELEMENTS_RO(&_global_service.ipsrvlist, node, data))
	{
		if(data && data->port == port)
		{
			flag = 1;
			break;
		}
	}
	if(data && flag)
	{
		os_listnode_delete(&_global_service.ipsrvlist, data);
		eloop_cancel(data->t_hread);
		ipstack_close(data->fd);
		ipstack_drstroy(data->fd);
		ip_srv_node_free(data);
	}
	return OK;	
}
