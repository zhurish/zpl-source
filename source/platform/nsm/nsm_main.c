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

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"

#include "host.h"
#include "prefix.h"
#include "vty.h"
#include "plist.h"
#include "filter.h"
#include "keychain.h"
#include "template.h"
#include "lib_event.h"

#include "nsm_main.h"
#include "nsm_zserv.h"
#include "nsm_include.h"
#ifdef ZPL_NSM_FPM
#include "fpm.h"
#include "nsm_fpm.h"
#endif
#ifdef ZPL_IPCBC_MODULE
#include "ipcbc_serv.h"
#endif

struct module_list module_list_nsm = {
		.module = MODULE_NSM,
		.name = "NSM\0",
		.module_init = nsm_module_init,
		.module_exit = nsm_module_exit,
		.module_task_init = nsm_task_init,
		.module_task_exit = nsm_task_exit,
		.module_cmd_init = nsm_module_cmd_init,
		.taskid = 0,
		.flags=ZPL_MODULE_NEED_INIT,
};

static struct nsm_srv_t m_nsm_srv =
{
	.rtm_table_default = 0,
	.nsm_task_id = 0,
};


int nsm_module_init(void)
{
	nsm_srv = &m_nsm_srv;
	if(nsm_srv->master == NULL)
		nsm_srv->master = thread_master_name_create("Nsm");

	lib_event_init();
#if defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP)
	memset(&nsm_rtadv, 0, sizeof(struct nsm_rtadv_t));
		nsm_rtadv.master = eloop_master_name_create("Nsm");
	if(nsm_rtadv.master)
		nsm_rtadv.initialise = 1;	
#endif /* defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP) */

#ifdef ZPL_VRF_MODULE
	ip_vrf_init();
#endif
	ipvrf_nsm_init();
	if_init();
	rib_init();

	nsm_interface_init();

	access_list_init ();
	prefix_list_init ();
	keychain_init ();




#ifdef ZPL_IPCBC_MODULE
	//ipcbc_serv_init(nsm_srv->master);
#endif
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
#ifdef ZPL_NSM_FPM
  zfpm_init (nsm_rtadv.master, 1, 0, "FPM");
#endif
#ifdef ZPL_NSM_SNMP
  nsm_snmp_init ();
#endif /* ZPL_NSM_SNMP */

	nsm_zserv_init();

	return 0;
}


int nsm_module_exit(void)
{
#if defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP)
	//extern struct nsm_rtadv_t nsm_rtadv;
#endif /* defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP) */	

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
	lib_event_exit();
#if defined(ZPL_NSM_IRDP)
	nsm_irdp_finish();
#endif
#ifdef ZPL_VRF_MODULE
	ip_vrf_terminate();
#endif

	rib_close();
	if_terminate();
	return OK;
}

int nsm_module_cmd_init(void)
{

	cmd_router_id_init();

	cmd_interface_init();
#ifdef ZPL_NSM_L3MODULE
	cmd_route_init();
#endif

#ifdef ZPL_VRF_MODULE
	cmd_ipvrf_init();
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
#ifdef ZPL_NSM_DHCP
	cmd_dhcp_init();
#endif
#ifdef ZPL_NSM_QOS
	cmd_qos_init();
#endif
#if defined(ZPL_NSM_IRDP)
	cmd_nsm_irdp_init();
#endif
#if defined(ZPL_NSM_RTADV)
	nsm_rtadv_cmd_init();
#endif
	cmd_nsm_zserv_init();

	return OK;
}


int nsm_module_start(void)
{
	nsm_global_start();
	nsm_port_start();
#ifdef ZPL_NSM_VLAN	
	nsm_vlan_default();
#endif
	ip_vrf_create("Default-IP-Routing-Table");
	return OK;
}



static int nsm_main_task(void *argv)
{
	//module_setup_task(MODULE_NSM, os_task_id_self());
	host_waitting_loadconfig();
	thread_mainloop(nsm_srv->master);
	return 0;
}


#if defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP)
static int nsm_rtadv_task(void *argv)
{
	host_waitting_loadconfig();
	eloop_mainloop(nsm_rtadv.master);
	return 0;
}
#endif /* defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP) */

int nsm_task_init(void)
{
#if defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP)
	if(nsm_rtadv.initialise)
	{
		if(nsm_rtadv.irdp_task_id == 0)
			nsm_rtadv.irdp_task_id = os_task_create("rtadvTask", OS_TASK_DEFAULT_PRIORITY,
									0, nsm_rtadv_task, NULL, OS_TASK_DEFAULT_STACK);
		if (nsm_rtadv.irdp_task_id)
		{
			//module_setup_task(MODULE_NSM, nsm_rtadv.irdp_task_id);
		}
	}
#endif /* defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP) */	
	if(nsm_srv->nsm_task_id == 0)
		nsm_srv->nsm_task_id = os_task_create("nsmTask", OS_TASK_DEFAULT_PRIORITY,
								 0, nsm_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if (nsm_srv->nsm_task_id)
	{
		//module_setup_task(MODULE_NSM, nsm_srv->nsm_task_id);
		return OK;
	}	
	return ERROR;
}

int nsm_task_exit (void)
{
	if(nsm_srv->nsm_task_id)
		os_task_destroy(nsm_srv->nsm_task_id);
	nsm_srv->nsm_task_id = 0;
#if defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP)
	if(nsm_rtadv.irdp_task_id)
		os_task_destroy(nsm_rtadv.irdp_task_id);
	nsm_rtadv.irdp_task_id = 0;
#endif /* defined (ZPL_NSM_RTADV) || defined(ZPL_NSM_IRDP) */
	return OK;
}


