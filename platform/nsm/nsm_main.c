/* zebra daemon main routine.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"
#include "nsm_main.h"
#include "nsm_zserv.h"

static struct nsm_srv_t m_nsm_srv =
{
	.rtm_table_default = 0,
	.nsm_task_id = 0,
};

static int nsm_main_task(void *argv)
{
	module_setup_task(MODULE_NSM, os_task_id_self());
	host_waitting_loadconfig();
	thread_mainloop(nsm_srv->master);
	return 0;
}

int nsm_module_init(void)
{
	nsm_srv = &m_nsm_srv;
	if(nsm_srv->master == NULL)
		nsm_srv->master = thread_master_module_create(MODULE_NSM);

	nsm_event_init();

	if_init();
	nsm_vrf_init();
	nsm_interface_init();

	access_list_init ();
	prefix_list_init ();
	keychain_init ();

	nsm_template_init();

	rib_init();

	nsm_global_init();
	nsm_port_init();

#ifdef ZPL_NSM_8021X
	nsm_dot1x_init();
#endif
#ifdef ZPL_NSM_ARP
	nsm_ip_arp_init();
#endif
#ifdef ZPL_NSM_BRIDGE
	nsm_bridge_init();
#endif

#ifdef ZPL_NSM_DNS
	nsm_ip_dns_init();
#endif
#ifdef ZPL_NSM_DOS
	nsm_dos_init();
#endif
#ifdef ZPL_NSM_FIREWALLD
	nsm_firewall_init();
#endif
#ifdef ZPL_NSM_MAC
	nsm_mac_init();
#endif
#ifdef ZPL_NSM_VLAN
	nsm_vlan_init();
#endif
#ifdef ZPL_NSM_QOS
	nsm_qos_init();
#endif
#ifdef ZPL_NSM_TRUNK
	nsm_trunk_init();
#endif
#ifdef ZPL_NSM_MIRROR
	nsm_mirror_init();
#endif
#ifdef ZPL_NSM_TUNNEL
	nsm_tunnel_init();
#endif
#ifdef ZPL_NSM_SERIAL
	nsm_serial_init();
#endif
#ifdef ZPL_NSM_PPP
	nsm_ppp_init();
#endif
#ifdef ZPL_NSM_SECURITY
	nsm_security_init();
#endif
#ifdef ZPL_NSM_VLANETH
	nsm_vlaneth_init();
#endif

#ifdef ZPL_RTPL_SRV
	zserv_init();
#endif
	return 0;
}


int nsm_module_exit(void)
{
#ifdef ZPL_NSM_MAC
	nsm_mac_exit();
#endif
#ifdef ZPL_NSM_VLAN
	nsm_vlan_exit();
#endif

#ifdef ZPL_NSM_ARP
	nsm_ip_arp_exit();
#endif
#ifdef ZPL_NSM_TRUNK
	nsm_trunk_exit();
#endif
#ifdef ZPL_NSM_DOS
	nsm_dos_exit();
#endif
#ifdef ZPL_NSM_8021X
	nsm_dot1x_exit();
#endif
#ifdef ZPL_NSM_MIRROR
	nsm_mirror_exit();
#endif

#ifdef ZPL_NSM_SERIAL
	nsm_serial_exit();
#endif
#ifdef ZPL_NSM_DNS
	nsm_ip_dns_exit();
#endif
#ifdef ZPL_NSM_QOS
	nsm_qos_exit();
#endif
#ifdef ZPL_NSM_VLANETH
	nsm_vlaneth_exit();
#endif

#ifdef ZPL_NSM_TUNNEL
	nsm_tunnel_exit();
#endif
#ifdef ZPL_NSM_BRIDGE
	nsm_bridge_exit();
#endif
#ifdef ZPL_NSM_FIREWALLD
	nsm_firewall_exit();
#endif

#ifdef ZPL_NSM_SECURITY
	nsm_security_exit();
#endif
#ifdef ZPL_NSM_PPP
	nsm_ppp_exit();
#endif
	nsm_global_exit();
	nsm_port_exit();
	
	access_list_reset();
	prefix_list_reset();

	nsm_event_exit();
	return OK;
}

int nsm_module_cmd_init(void)
{
	#ifdef ZPL_RTPL_MODULE
	cmd_router_id_init();
	#endif
	cmd_interface_init();
	cmd_route_init();
#ifdef ZPL_IPCOM_STACK_MODULE
	cmd_ip_vrf_init();
#endif

	cmd_global_init();

#ifdef ZPL_NSM_MAC
	cmd_mac_init();
#endif
#ifdef ZPL_NSM_VLAN
	cmd_vlan_init();
#endif
	cmd_port_init();
#ifdef ZPL_NSM_ARP
	cmd_arp_init();
#endif
#ifdef ZPL_NSM_TRUNK
	cmd_trunk_init();
#endif
#ifdef ZPL_NSM_DOS
	cmd_dos_init();
#endif
#ifdef ZPL_NSM_8021X
	cmd_dot1x_init();
#endif
#ifdef ZPL_NSM_MIRROR
	cmd_mirror_init();
#endif

#ifdef ZPL_NSM_SERIAL
	cmd_serial_init();
#endif
#ifdef ZPL_NSM_DNS
	cmd_dns_init();
#endif
#ifdef ZPL_NSM_PPP
	cmd_ppp_init();
#endif
#ifdef ZPL_NSM_TUNNEL
	cmd_tunnel_init();
#endif
#ifdef ZPL_NSM_BRIDGE
	cmd_bridge_init();
#endif
#ifdef ZPL_NSM_SECURITY
	cmd_security_init();
#endif
#ifdef ZPL_NSM_QOS
	cmd_qos_init();
#endif
	return OK;
}


int nsm_module_start(void)
{
	nsm_global_start();
	nsm_port_start();
	nsm_vlan_default();
	return OK;
}


int nsm_task_init(void)
{
	if(nsm_srv->nsm_task_id == 0)
		nsm_srv->nsm_task_id = os_task_create("nsmTask", OS_TASK_DEFAULT_PRIORITY,
								 0, nsm_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if (nsm_srv->nsm_task_id)
	{
		module_setup_task(MODULE_NSM, nsm_srv->nsm_task_id);
		return OK;
	}	
	return ERROR;
}

int nsm_task_exit (void)
{
	if(nsm_srv->nsm_task_id)
		os_task_destroy(nsm_srv->nsm_task_id);
	nsm_srv->nsm_task_id = 0;
	return OK;
}


struct module_list module_list_nsm = {
		.module = MODULE_NSM,
		.name = "NSM",
		.module_init = nsm_module_init,
		.module_exit = nsm_module_exit,
		.module_task_init = nsm_task_init,
		.module_task_exit = nsm_task_exit,
		.module_cmd_init = nsm_module_cmd_init,
		.module_write_config = NULL,
		.module_show_config = NULL,
		.module_show_debug = NULL,
		.taskid = 0,
		.flags=0,
};