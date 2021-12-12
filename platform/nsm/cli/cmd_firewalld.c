/*
 * cmd_firewalld.c
 *
 *  Created on: 2019年8月31日
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"


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
	  install_element (ENABLE_NODE, CMD_VIEW_LEVEL, &show_ip_firewall_rule_cmd);
}
