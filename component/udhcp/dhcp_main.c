#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "prefix.h"
#include "sigevent.h"
#include "version.h"
#include "log.h"
#include "getopt.h"
#include "eloop.h"
#include "os_start.h"
#include "os_module.h"

#include "dhcp_def.h"
#include "dhcp_lease.h"
#include "dhcp_pool.h"
#include "dhcp_util.h"
#include "dhcpd.h"
#include "dhcp_main.h"
#include "dhcp_api.h"

dhcp_global_t dhcp_global_config;

static int dhcpd_config_init(dhcp_global_t *config)
{
	if(config->init == TRUE)
		return OK;
	memset(config, 0, sizeof(dhcp_global_t));
	config->init = TRUE;
	lstInit(&config->pool_list);
	lstInit(&config->client_list);
	lstInit(&config->relay_list);
	if (master_eloop[MODULE_DHCP] == NULL)
		config->eloop_master = master_eloop[MODULE_DHCP] =
		eloop_master_module_create(MODULE_DHCP);

	config->server_port = DHCP_SERVER_PORT;
	config->client_port = DHCP_CLIENT_PORT;

	config->server_port_v6 = DHCP_SERVER_PORT6;
	config->client_port_v6 = DHCP_CLIENT_PORT6;
	config->r_thread = NULL;
	config->sock = 0;
	config->rawsock = 0;
	return OK;
}

static int dhcpd_config_uninit(dhcp_global_t *config)
{
	if(config->init != TRUE)
		return OK;

	if(master_eloop[MODULE_DHCP])
		eloop_master_free(master_eloop[MODULE_DHCP]);
	master_eloop[MODULE_DHCP] = NULL;
	config->init = FALSE;
	lstFree(&config->pool_list);
	lstFree(&config->client_list);
	lstFree(&config->relay_list);
	memset(config, 0, sizeof(dhcp_global_t));
	return OK;
}

int udhcp_read_thread(struct eloop *eloop)
{
	struct dhcp_packet packet;
	int bytes = 0;
	int sock = -1;
	u_int32 ifindex = 0;
	dhcp_pool_t *pool = NULL;
	dhcpd_interface_t * ifter = NULL;
	dhcp_global_t *config = NULL;
	sock = ELOOP_FD(eloop);
	config = ELOOP_ARG(eloop);

	zlog_debug(MODULE_DHCP, "====================%s", __func__);
	memset(&packet, 0, sizeof(struct dhcp_packet));
	bytes = udhcp_recv_packet(&packet, sock, &ifindex);
	if (bytes < 0) {
		/* bytes can also be -2 ("bad packet data") */
		if (bytes == -1 && errno != EINTR) {
			zlog_err(MODULE_DHCP,
					"read error: "STRERROR_FMT", reopening socket" STRERROR_ERRNO);
			close(sock);
			sock = -1;
			//config->r_thread = eloop_add_read(config->eloop_master, udhcp_read_thread, config, server_socket);
			return ERROR;
		}

	}
	pool = dhcpd_pool_interface_lookup(ifindex);
	if (pool == NULL) {
		config->r_thread = eloop_add_read(config->eloop_master,
				udhcp_read_thread, config, sock);
		zlog_err(MODULE_DHCP, " this Interface is not allow DHCP pool:%s", ifindex2ifname(ifindex));
		return ERROR;
	}
	ifter = dhcpd_lookup_interface(pool, ifindex);
	if (ifter == NULL) {
		config->r_thread = eloop_add_read(config->eloop_master,
				udhcp_read_thread, config, sock);
		zlog_err(MODULE_DHCP, " this Interface is not allow DHCP:%s", ifindex2ifname(ifindex));
		return ERROR;
	}
	udhcp_server_handle_thread(pool, ifter, &packet);

	config->r_thread = eloop_add_read(config->eloop_master, udhcp_read_thread,
			config, sock);
	return OK;
}

static int udhcp_task_main(void *p)
{
	//dhcp_pool_t *pool = NULL;
	dhcp_global_t *config = (dhcp_global_t *) p;
	module_setup_task(MODULE_DHCP, os_task_id_self());
	while (!os_load_config_done()) {
		os_sleep(1);
	}
/*	os_sleep(5);
	struct interface * ifp = if_lookup_by_name("ethernet 0/0/2");
	if (ifp) {
		pool = dhcpd_pool_lookup("test");
		//if_kname_set(ifp, "br-lan");
		dhcpd_pool_add_interface(pool, ifp->ifindex);
	}
	dhcpd_lease_load();
	os_sleep(1);*/
	zlog_debug(MODULE_DHCP, "---------udhcpd_main");

	//config->global->sock
/*	while(1)
	{
		int timeout_ms = 0, r = 0;
		struct pollfd pfd[1];

		timeout_ms = 2000;
		pfd[0].fd = pool->global->sock;
		pfd[0].events = POLLIN;

		r = safe_poll(pfd, 1, timeout_ms);
		if (r < 0)
			continue;
		if (r) {
			zlog_debug(MODULE_DHCP, "read -> udhcpd_main");
		}
	}*/
	eloop_start_running(config->eloop_master, MODULE_DHCP);
	return 0;
}


int udhcp_module_init()
{
	//int server_socket = 0;
	//unsigned num_ips = 0;
	//struct option_set *option = NULL;
	//dhcp_pool_t *pool = NULL;
	dhcpd_config_init(&dhcp_global_config);
#if 0
	pool = dhcpd_pool_create("test");
	if (pool) {
		dhcpd_pool_set_address_range(pool, inet_addr("10.10.10.1"), inet_addr("10.10.10.253"));
		dhcpd_pool_set_leases(pool, 3600, 60);
		dhcpd_pool_set_autotime(pool, 7200);
		dhcpd_pool_set_decline_time(pool, 3600);
		dhcpd_pool_set_conflict_time(pool, 3600);
		dhcpd_pool_set_offer_time(pool, 60);
		dhcpd_pool_set_siaddr(pool, 0);

		dhcpd_pool_set_option(pool, DHCP_HOST_NAME, "SWP-V0.0.0.1");
		dhcpd_pool_set_option(pool, DHCP_ROUTER, "10.10.10.3");
		dhcpd_pool_set_option(pool, DHCP_DNS_SERVER, "10.10.10.4");
		dhcpd_pool_set_option(pool, DHCP_DOMAIN_NAME, "www.aa.com");

		//dhcpd_pool_set_option(pool, DHCP_DNS_SERVER, "www.aa.com");
		dhcpd_pool_set_option(pool, DHCP_SUBNET, "255.255.255.0");
		dhcpd_pool_set_option(pool, DHCP_BROADCAST, "10.10.10.255");
		dhcpd_pool_set_option(pool, DHCP_NTP_SERVER, "10.10.10.25");
		/*
		 dhcpd_pool_set_notify_file(pool, char *str);
		 dhcpd_pool_set_sname(pool, char *str);
		 dhcpd_pool_set_boot_file(pool, char *str);
		 //option	subnet	255.255.255.0
		 int dhcpd_pool_set_option(pool, char *str);
		 //00:60:08:11:CE:4E 192.168.0.54
		 int dhcpd_pool_set_static_lease(pool, char *str);
		 //POP_SAVED_FUNCTION_VISIBILITY

		 int dhcpd_pool_add_interface(pool, u_int32 ifindex);
		 int dhcpd_pool_del_interface(pool, u_int32 ifindex);
		 dhcpd_interface_t * dhcpd_lookup_interface(pool, u_int32 ifindex);
		 uint32_t dhcpd_lookup_address_on_interface(pool, u_int32 ifindex);
		 */
		/**********************************************************************/
		/**********************************************************************/
	}
#endif
	/*
	option = udhcp_find_option(pool->options, DHCP_LEASE_TIME);
	pool->max_lease_sec = DEFAULT_LEASE_TIME;
	if (option) {
		move_from_unaligned32(pool->max_lease_sec, option->data + OPT_DATA);
		pool->max_lease_sec = ntohl(pool->max_lease_sec);
	}

	if (server_socket <= 0) {
		server_socket = udhcp_udp_socket(SERVER_PORT);
	}
	dhcp_global_config.sock = server_socket;
	dhcp_global_config.rawsock = udhcp_raw_socket();

	dhcp_global_config.r_thread = eloop_add_read(
			dhcp_global_config.eloop_master, udhcp_read_thread,
			&dhcp_global_config, server_socket);
	*/
	return OK;
}

int udhcp_module_exit()
{
	return dhcpd_config_uninit(&dhcp_global_config);
}

int udhcp_module_task_init()
{
	if(dhcp_global_config.task_id == 0)
		dhcp_global_config.task_id = os_task_create("udhcpTask", OS_TASK_DEFAULT_PRIORITY, 0,
			udhcp_task_main, &dhcp_global_config,
			OS_TASK_DEFAULT_STACK);
	if(dhcp_global_config.task_id)
		return OK;
	return ERROR;
}

int udhcp_module_task_exit ()
{
	if(dhcp_global_config.task_id == 0)
		return OK;
	if(dhcp_global_config.r_thread)
	{
		eloop_cancel(dhcp_global_config.r_thread);
		dhcp_global_config.r_thread = NULL;
	}
	if(dhcp_global_config.sock)		//udp socket, just for server
		close(dhcp_global_config.sock);

	if(dhcp_global_config.rawsock)	//raw socket, just for server send MSG to client
		close(dhcp_global_config.rawsock);

	if(dhcp_global_config.sock_v6)
		close(dhcp_global_config.sock_v6);

	if(dhcp_global_config.rawsock_v6)
		close(dhcp_global_config.rawsock_v6);

	if(dhcp_global_config.client_sock)		//udp socket, just for client
		close(dhcp_global_config.client_sock);

	dhcp_client_interface_clean();
	dhcpd_pool_clean();
	if(dhcp_global_config.task_id)
		os_task_destroy(dhcp_global_config.task_id);
	dhcp_global_config.task_id = 0;
	if(master_eloop[MODULE_DHCP])
		eloop_master_free(master_eloop[MODULE_DHCP]);
	master_eloop[MODULE_DHCP] = NULL;
	return OK;
}

struct module_list module_list_dhcp = 
{ 
	.module=MODULE_DHCP, 
	.name="DHCP", 
	.module_init=udhcp_module_init, 
	.module_exit=udhcp_module_exit, 
	.module_task_init=udhcp_module_task_init, 
	.module_task_exit=udhcp_module_task_exit, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};
/*
int dhcpc_enable_test()
{
	return ERROR;
}
*/
