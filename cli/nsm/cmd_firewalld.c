/*
 * cmd_firewalld.c
 *
 *  Created on: 2019年8月31日
 *      Author: zhurish
 */

#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"

#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"
#include "if_name.h"

#include "nsm_firewalld.h"


static int firewall_zone_one_cb(firewall_zone_t *zone, void *vty)
{
	return firewall_rule_show_api(vty, zone, NULL);
}


DEFUN (show_ip_firewall_rule,
		show_ip_firewall_rule_cmd,
		"show ip firewall rule",
		SHOW_STR
		IP_STR
		"Firewalld \n"
		"Rule\n")
{
	int ret = ERROR;
	ret = firewall_zone_foreach_api(firewall_zone_one_cb, vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

void cmd_firewall_init (void)
{
	  install_element (ENABLE_NODE, &show_ip_firewall_rule_cmd);
}
