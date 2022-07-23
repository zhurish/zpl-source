/*
 * cmd_routeid.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */



#include "auto_include.h"
#include <zplos_include.h>
#include "route_types.h"
#include "nsm_event.h"
#include "zmemory.h"
#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "command.h"
#include "nsm_zserv.h"
#include "router-id.h"




void router_id_write (struct vty *vty)
{
	struct prefix p;
	router_id_get (&p, VRF_DEFAULT);
	if(p.u.prefix4.s_addr)
		vty_out (vty, "router-id %s%s",
             ipstack_inet_ntoa (p.u.prefix4),VTY_NEWLINE);
}

DEFUN (router_id,
       router_id_cmd,
       "router-id A.B.C.D",
       "Manually set the router-id\n"
       "IP address to use for router-id\n")
{
  struct prefix rid;
  vrf_id_t vrf_id = VRF_DEFAULT;

  rid.u.prefix4.s_addr = ipstack_inet_addr (argv[0]);
  if (!rid.u.prefix4.s_addr)
    return CMD_WARNING;
		if(IPV4_NET127(rid.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(rid.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
  rid.prefixlen = 32;
  rid.family = IPSTACK_AF_INET;

  if (argc > 1)
    VTY_GET_INTEGER ("VRF ID", vrf_id, argv[1]);

  router_id_set (&rid, vrf_id);

  return CMD_SUCCESS;
}


DEFUN (no_router_id,
       no_router_id_cmd,
       "no router-id",
       NO_STR
       "Remove the manually configured router-id\n")
{
  struct prefix rid;
  vrf_id_t vrf_id = VRF_DEFAULT;

  rid.u.prefix4.s_addr = 0;
  rid.prefixlen = 0;
  rid.family = IPSTACK_AF_INET;

  if (argc > 0)
    VTY_GET_INTEGER ("VRF ID", vrf_id, argv[0]);

  router_id_set (&rid, vrf_id);

  return CMD_SUCCESS;
}


void cmd_router_id_init (void)
{
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &router_id_cmd);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_router_id_cmd);
  //install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &router_id_vrf_cmd);
  //install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_router_id_vrf_cmd);
}



