/*
 * pal_driver.c
 *
 *  Created on: Jan 28, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zebra_event.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "host.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"

#include "nsm_include.h"
#include "pal_include.h"
#include "pal_global.h"
#include "linux_driver.h"

pal_stack_t pal_stack;

static zpl_uint32 kernel_task_id = 0;
static void *master_eloop_kernel = NULL;
static int pal_module_task_init(void);

struct module_list module_list_pal = 
{ 
	.module=MODULE_PAL, 
	.name="PAL\0", 
	.module_init=pal_module_init, 
	.module_exit=NULL, 
	.module_task_init=pal_module_task_init, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.taskid=0,
	.flags = 0,
};


/***************************************************************/
static int pal_main_task(void *argv)
{
	module_setup_task(MODULE_PAL, os_task_id_self());
	host_waitting_loadconfig();
	eloop_mainloop(master_eloop_kernel);
	return OK;
}


static int kernel_task_init (void)
{
	if(master_eloop_kernel == NULL)
		master_eloop_kernel = eloop_master_module_create(MODULE_PAL);

	if(kernel_task_id == 0)
		kernel_task_id = os_task_create("kernelTask", OS_TASK_DEFAULT_PRIORITY,
	               0, pal_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if(kernel_task_id)
	{
		module_setup_task(MODULE_PAL, kernel_task_id);
		return OK;
	}
	return ERROR;
}

int pal_module_init(void)
{
	if(master_eloop_kernel == NULL)
		master_eloop_kernel = eloop_master_module_create(MODULE_PAL);
	iplinux_stack_init();
	#ifdef ZPL_NSM_ARP
	ip_arp_stack_init();
	#endif

	return 0;;
}

static int pal_module_task_init(void)
{
	return kernel_task_init ();
}


#ifdef ZPL_NSM_FIREWALLD
/*
 * 端口映射
 */
int pal_firewall_portmap_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_portmap_rule_set)
		return pal_stack.ip_stack_firewall_portmap_rule_set(rule, action);
    return OK;
}
/*
 * 端口开放
 */
int pal_firewall_port_filter_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_port_filter_rule_set)
		return pal_stack.ip_stack_firewall_port_filter_rule_set(rule, action);
    return OK;
}

int pal_firewall_mangle_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_mangle_rule_set)
		return pal_stack.ip_stack_firewall_mangle_rule_set(rule, action);
    return OK;
}

int pal_firewall_raw_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_raw_rule_set)
		return pal_stack.ip_stack_firewall_raw_rule_set(rule, action);
    return OK;
}

int pal_firewall_snat_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_snat_rule_set)
		return pal_stack.ip_stack_firewall_snat_rule_set(rule, action);
    return OK;
}

int pal_firewall_dnat_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_dnat_rule_set)
		return pal_stack.ip_stack_firewall_dnat_rule_set(rule, action);
    return OK;
}
#endif
