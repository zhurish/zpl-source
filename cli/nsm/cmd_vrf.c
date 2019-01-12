/*
 * cmd_routeid.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */


#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "stream.h"
#include "command.h"
#include "memory.h"
#include "network.h"
#include "log.h"
#include "table.h"
#include "rib.h"
#include "vrf.h"

#include "zserv.h"
#include "router-id.h"
#include "redistribute.h"


#ifdef USE_IPSTACK_IPCOM

DEFUN (ip_vrf_create,
		ip_vrf_create_cmd,
       "ip vrf NAME",
	   IP_STR
       "Specify the VRF\n"
       "Specify the VRF name\n")
{
	struct vrf *vrf = NULL;
	vrf = vrf_lookup_by_name(argv[0]);
	if(vrf)
	{
		vty->index = vrf;
		vty->node = VRF_NODE;
		return CMD_SUCCESS;
	}
	vrf = nsm_vrf_create (argv[0]);
	if(vrf)
	{
		vty->index = vrf;
		vty->node = VRF_NODE;
		return CMD_SUCCESS;
	}
	vty_out(vty, "Can not create VRF by VRF name%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (ip_vrf_delete,
	ip_vrf_delete_cmd,
    "no ip vrf NAME",
	NO_STR
	IP_STR
	"Specify the VRF\n"
	"Specify the VRF name\n")
{
	int ret = 0;
	struct vrf *vrf = NULL;
	vrf = vrf_lookup_by_name(argv[0]);
	if(!vrf)
	{
		vty_out(vty, "Can not find VRF by VRF name%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = nsm_vrf_delete(argv[0]);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_vrf_set_vrfid,
		ip_vrf_set_vrfid_cmd,
       "rd A.B.C.D <0-65535>",
	   "Route Distinguisher\n"
       "Specify Route ID the VRF\n"
       "The VRF ID\n")
{
	struct vrf *vrf = NULL;
	vrf_id_t vrf_id = 0;
	struct prefix rid;
	rid.u.prefix4.s_addr = inet_addr (argv[0]);
	if (!rid.u.prefix4.s_addr)
		return CMD_WARNING;

	rid.prefixlen = 32;
	rid.family = AF_INET;
	if (argc > 1)
		VTY_GET_INTEGER ("VRF ID", vrf_id, argv[1]);

	nsm_vrf_set_vrfid(vty->index, vrf_id);

	router_id_set (&rid, vrf_id);

	return CMD_SUCCESS;
}

static int ip_vrf_show_one (struct vty *vty, struct nsm_vrf *zvrf)
{
	//rd A.B.C.D <0-65535>
	struct prefix p;
	struct vrf *vrf = vrf_lookup(zvrf->vrf_id);
	router_id_get (&p, zvrf->vrf_id);
	if(vrf)
	{
		vty_out (vty, "ip vrf %s%s", vrf->name, VTY_NEWLINE);
		if(p.u.prefix4.s_addr)
			vty_out (vty, " rd %s %d%s",
					inet_ntoa (p.u.prefix4), zvrf->vrf_id, VTY_NEWLINE);
	}
	return 1;
}


static int ip_vrf_write (struct vty *vty)
{
  int ret = 0;
  struct nsm_vrf *zvrf;
  vrf_iter_t iter;

  for (iter = vrf_first (); iter != VRF_ITER_INVALID; iter = vrf_next (iter))
  {
    if ((zvrf = vrf_iter2info (iter)) != NULL)
    {
    	if(zvrf->vrf_id != VRF_DEFAULT)
    	{
			ret = ip_vrf_show_one(vty, zvrf);
			if(ret)
				vty_out (vty, "exit%s", VTY_NEWLINE);
    	}
    }
  }
  return 1;
}


static struct cmd_node vrf_node =
{
	VRF_NODE,
	"%s(config-vrf)# ",
	1
};

void cmd_ip_vrf_init (void)
{
	install_node(&vrf_node, ip_vrf_write);

	install_default(VRF_NODE);
	install_default_basic(VRF_NODE);

	install_element(CONFIG_NODE, &ip_vrf_create_cmd);
	install_element(CONFIG_NODE, &ip_vrf_delete_cmd);

	install_element(VRF_NODE, &ip_vrf_set_vrfid_cmd);
}
#endif

