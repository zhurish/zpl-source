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

#include <zebra.h>

#include <version.h>
#include "getopt.h"
#include "command.h"
#include "thread.h"
#include "memory.h"
#include "prefix.h"
#include "log.h"
#include "sigevent.h"
#include "nsm_vrf.h"
#include "nsm_rib.h"
#include "nsm_zserv.h"
#include "nsm_router-id.h"

struct zebra_t zebrad =
	{
		.rtm_table_default = 0,
};
static int nsm_task_id = 0;

static int nsm_main_task(void *argv)
{
	os_start_running(zebrad.master, MODULE_NSM);
#if 0
	struct thread thread;

	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	//os_log_reopen(MODULE_NSM);
	while (thread_fetch (zebrad.master, &thread))
		thread_call (&thread);
	/* Not reached... */
#endif
	return 0;
}

int nsm_module_init()
{
	/* Make master thread emulator. */
	master_thread[MODULE_NSM] = thread_master_module_create(MODULE_NSM);
	zebrad.master = master_thread[MODULE_NSM];

	//if_init();
	zserv_init();
	rib_init();
	//nsm_vrf_init ();
	nsm_interface_init();
	nsm_client_init();

#ifdef PL_NSM_8021X
	nsm_dot1x_init();
#endif
#ifdef PL_NSM_ARP
	nsm_ip_arp_init();
#endif
#ifdef PL_NSM_BRIDGE

#endif
#ifdef PL_NSM_DHCP

#endif
#ifdef PL_NSM_DNS
	nsm_ip_dns_init();
#endif
#ifdef PL_NSM_DOS
	nsm_dos_init();

#endif
#ifdef PL_NSM_FIREWALLD

#endif
#ifdef PL_NSM_MAC
	nsm_mac_init();
#endif
#ifdef PL_NSM_VLAN
	nsm_vlan_init();
#endif
#ifdef PL_NSM_QOS
	nsm_qos_init();
#endif
#ifdef PL_NSM_TRUNK
	nsm_trunk_init();
#endif
#ifdef PL_NSM_MIRROR
	nsm_mirror_init();
#endif
#ifdef PL_NSM_TUNNEL
	nsm_tunnel_client_init();
#endif
#ifdef PL_NSM_SERIAL
	nsm_serial_client_init();
#endif
#ifdef PL_NSM_PPP

#endif
#ifdef PL_NSM_SECURITY
	nsm_security_init();
#endif
#ifdef PL_NSM_VETH
	nsm_veth_client_init();
#endif
	//kernel_init(NULL);
	//zebra_debug_init ();
	//zclient_new(master_thread[MODULE_NSM]);
	/*	zebra_init ();

	zebra_if_init ();
	zebra_debug_init ();
	router_id_cmd_init ();
	zebra_vty_init ();
	access_list_init ();
	prefix_list_init ();
#if defined (HAVE_RTADV)
	rtadv_cmd_init ();
#endif
#ifdef HAVE_IRDP
	irdp_init();
#endif

	//extern int ipkernel_module_init(void);
	//ipkernel_module_init();
	 For debug purpose.
	 SET_FLAG (zebra_debug_event, ZEBRA_DEBUG_EVENT);

	 Initialize VRF module, and make kernel routing socket.
	zebra_vrf_init ();

#ifdef HAVE_SNMP
	zebra_snmp_init ();
#endif  HAVE_SNMP

#ifdef HAVE_FPM
	zfpm_init (zebrad.master, 1, 0, fpm_format);
#else
	zfpm_init (zebrad.master, 0, 0, fpm_format);
#endif

	 Clean up rib.
	rib_weed_tables ();

	//if (! keep_kernel_mode)
	//  rib_sweep_route ();

	 This must be done only after locking pidfile (bug #403).
	zebra_zserv_socket_init (NULL);*/
	return 0;
}

int nsm_task_init()
{
	if(nsm_task_id == 0)
		nsm_task_id = os_task_create("nsmTask", OS_TASK_DEFAULT_PRIORITY,
								 0, nsm_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if (nsm_task_id)
		return OK;
	return ERROR;
}

int nsm_task_exit ()
{
	if(nsm_task_id)
		os_task_destroy(nsm_task_id);
	nsm_task_id = 0;
	return OK;
}

int nsm_module_exit()
{
#ifdef PL_NSM_MAC
	nsm_mac_exit();
#endif
#ifdef PL_NSM_VLAN
	nsm_vlan_exit();
#endif

#ifdef PL_NSM_ARP
	nsm_ip_arp_exit();
#endif
#ifdef PL_NSM_TRUNK
	nsm_trunk_exit();
#endif
#ifdef PL_NSM_DOS
	nsm_dos_exit();
#endif
#ifdef PL_NSM_8021X
	nsm_dot1x_exit();
#endif
#ifdef PL_NSM_MIRROR
	nsm_mirror_exit();
#endif

#ifdef PL_NSM_SERIAL
	nsm_serial_client_exit();
#endif
#ifdef PL_NSM_DNS
	nsm_ip_dns_exit();
#endif
#ifdef PL_NSM_QOS
	nsm_qos_exit();
#endif
#ifdef PL_NSM_VETH
	nsm_veth_client_exit();
#endif

#ifdef PL_NSM_TUNNEL
	nsm_tunnel_client_exit();
#endif
#ifdef PL_NSM_BRIDGE

#endif
#ifdef PL_NSM_SECURITY
	nsm_security_exit();
#endif

	return OK;
}

int nsm_module_cmd_init()
{
	cmd_router_id_init();
	cmd_interface_init();
	cmd_route_init();

#ifdef PL_NSM_MAC
	cmd_mac_init();
#endif
#ifdef PL_NSM_VLAN
	cmd_vlan_init();
#endif
	cmd_port_init();
#ifdef PL_NSM_ARP
	cmd_arp_init();
#endif
#ifdef PL_NSM_TRUNK
	cmd_trunk_init();
#endif
#ifdef PL_NSM_DOS
	cmd_dos_init();
#endif
#ifdef PL_NSM_8021X
	cmd_dot1x_init();
#endif
#ifdef PL_NSM_MIRROR
	cmd_mirror_init();
#endif

#ifdef PL_NSM_SERIAL
	cmd_serial_init();
#endif
#ifdef PL_NSM_DNS
	cmd_dns_init();
#endif
#ifdef PL_NSM_PPP
	cmd_ppp_init();
#endif
#ifdef PL_NSM_TUNNEL
	cmd_tunnel_init();
#endif
#ifdef PL_NSM_BRIDGE
	cmd_bridge_init();
#endif
#ifdef PL_NSM_SECURITY
	cmd_security_init();
#endif
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
};