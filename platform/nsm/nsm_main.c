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
#include "vrf.h"
#include "rib.h"
#include "zserv.h"
#include "router-id.h"
struct zebra_t zebrad =
{
  .rtm_table_default = 0,
};
static int nsm_task_id = 0;


static int nsm_main_task(void *argv)
{
	struct thread thread;
	printf("%s\r\n",__func__);

	//os_log_reopen(ZLOG_NSM);
	while (thread_fetch (zebrad.master, &thread))
		thread_call (&thread);
	/* Not reached... */
	return 0;
}

int nsm_module_init ()
{
	/* Make master thread emulator. */
	master_thread[MODULE_NSM] = thread_master_module_create (MODULE_NSM);
	zebrad.master = master_thread[MODULE_NSM];

	if_init();
	zserv_init ();
	rib_init ();
	nsm_vrf_init ();
	nsm_interface_init();
	nsm_client_init ();

	//kernel_init(NULL);
	//zebra_debug_init ();
	//zclient_new(master_thread[ZLOG_NSM]);
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

int nsm_task_init ()
{
	nsm_task_id = os_task_create("nsmTask", OS_TASK_DEFAULT_PRIORITY,
	               0, nsm_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if(nsm_task_id)
		return OK;
	return ERROR;

}

int nsm_module_exit ()
{
	return OK;
}
