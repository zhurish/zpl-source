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
#include "table.h"
#include "nsm_rib.h"
#include "nsm_ipvrf.h"
#include "router-id.h"



#ifdef ZPL_VRF_MODULE

DEFUN (ip_vrf_cli_create,
		ip_vrf_cli_create_cmd,
       "ip vrf NAME",
	   IP_STR
       "Specify the VRF\n"
       "Specify the VRF name\n")
{
	struct ip_vrf *vrf = NULL;
	vrf = ip_vrf_lookup_by_name(argv[0]);
	if(vrf)
	{
		vty->index = vrf;
		vty->node = VRF_NODE;
		return CMD_SUCCESS;
	}
	vrf = ipvrf_create (argv[0]);
	if(vrf)
	{
		vty->index = vrf;
		vty->node = VRF_NODE;
		return CMD_SUCCESS;
	}
	vty_out(vty, "Can not create VRF by VRF name%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (ip_vrf_cli_delete,
	ip_vrf_cli_delete_cmd,
    "no ip vrf NAME",
	NO_STR
	IP_STR
	"Specify the VRF\n"
	"Specify the VRF name\n")
{
	int ret = 0;
	struct ip_vrf *vrf = NULL;
	vrf = ip_vrf_lookup_by_name(argv[0]);
	if(!vrf)
	{
		vty_out(vty, "Can not find VRF by VRF name%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = ipvrf_delete(argv[0]);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_vrf_cli_set_vrfid,
		ip_vrf_cli_set_vrfid_cmd,
       "rd A.B.C.D <0-65535>",
	   "Route Distinguisher\n"
       "Specify Route ID the VRF\n"
       "The VRF ID\n")
{
	vrf_id_t vrf_id = 0;
	struct prefix rid;
	rid.u.prefix4.s_addr = ipstack_inet_addr (argv[0]);
	if (!rid.u.prefix4.s_addr)
		return CMD_WARNING;

	rid.prefixlen = 32;
	rid.family = IPSTACK_AF_INET;
	if (argc > 1)
		VTY_GET_INTEGER ("VRF ID", vrf_id, argv[1]);

	ip_vrf_set_vrfid(vty->index, vrf_id);

	router_id_set (&rid, vrf_id);

	return CMD_SUCCESS;
}

static int ip_vrf_show_one (struct ip_vrf *vrf, void *pVoid)
{
	//rd A.B.C.D <0-65535>
	struct prefix p;
	struct vty *vty = pVoid;
	struct nsm_ipvrf *zvrf = vrf->info;
	if(vrf && zvrf && vrf->vrf_id != VRF_DEFAULT)
	{
		//fprintf(stdout, "=========%s\r\n", __func__);
		router_id_get (&p, zvrf->vrf_id);
		if(vrf->name)
			vty_out (vty, "ip vrf %s%s", vrf->name, VTY_NEWLINE);
		if(p.u.prefix4.s_addr)
			vty_out (vty, " rd %s %d%s",
					ipstack_inet_ntoa (p.u.prefix4), zvrf->vrf_id, VTY_NEWLINE);
		vty_out (vty, "exit%s", VTY_NEWLINE);			
	}
	return 0;
}


static int ip_vrf_write (struct vty *vty)
{
	ip_vrf_foreach(ip_vrf_show_one, vty);
	return 1;
}


static struct cmd_node vrf_node =
{
	VRF_NODE,
	"%s(config-vrf)# ",
	1
};

void cmd_ipvrf_init (void)
{
	install_node(&vrf_node, ip_vrf_write);

	install_default(VRF_NODE);
	install_default_basic(VRF_NODE);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &ip_vrf_cli_create_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &ip_vrf_cli_delete_cmd);

	install_element(VRF_NODE, CMD_CONFIG_LEVEL, &ip_vrf_cli_set_vrfid_cmd);
}
#endif

