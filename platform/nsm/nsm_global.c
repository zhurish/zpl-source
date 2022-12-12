
#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "nsm_include.h"
#include "hal_include.h"



static nsm_global_t nsm_global;

#define NSM_GLOBAL_JUMBO_DEFAULT		8192
#define NSM_GLOBAL_FORWARD_DEFAULT	            zpl_true
#define NSM_GLOBAL_UNICAST_FLOOD_DEFAULT		zpl_false
#define NSM_GLOBAL_MULTICAST_FLOOD_DEFAULT	    zpl_false
#define NSM_GLOBAL_MULTICAST_LEARNING_DEFAULT	zpl_false
#define NSM_GLOBAL_BPDU_DEFAULT	                zpl_false

static int bulid_global_config(struct vty *vty, void *p)
{
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
  //vty_out(vty, "=============bulid_global_config%s", VTY_NEWLINE);

	if(nsm_global.cpu_rate)
	{
		vty_out(vty, "cpu rate limit %d%s", nsm_global.cpu_rate, VTY_NEWLINE);
	}
	if(nsm_global.qinq_tpid)
			vty_out(vty, "system dot1q-tpid %x%s", nsm_global.qinq_tpid, VTY_NEWLINE);
	if(nsm_global.global_jumbo_size && nsm_global.global_jumbo_size != NSM_GLOBAL_JUMBO_DEFAULT)
			vty_out(vty, "system jumboframe size %d%s", nsm_global.global_jumbo_size, VTY_NEWLINE);
	if(nsm_global.switch_forward != NSM_GLOBAL_FORWARD_DEFAULT)
  {
      if(nsm_global.switch_forward)
			  vty_out(vty, "system switch forward%s", VTY_NEWLINE);
      else
			  vty_out(vty, "no system switch forward%s", VTY_NEWLINE);
  }
	if(nsm_global.unicast_flood != NSM_GLOBAL_UNICAST_FLOOD_DEFAULT)
  {
    if(nsm_global.unicast_flood)
			vty_out(vty, "system unicast flood%s", VTY_NEWLINE);
    else
      vty_out(vty, "no system unicast flood%s", VTY_NEWLINE);
  }
	if(nsm_global.multicast_flood != NSM_GLOBAL_MULTICAST_FLOOD_DEFAULT)
  {
    if(nsm_global.multicast_flood)
			vty_out(vty, "system multicast flood%s", VTY_NEWLINE);
    else
      vty_out(vty, "no system multicast flood%s", VTY_NEWLINE);
  }
	if(nsm_global.multicast_learning != NSM_GLOBAL_MULTICAST_LEARNING_DEFAULT)
  {
    if(nsm_global.multicast_learning)
			vty_out(vty, "system multicast learning%s", VTY_NEWLINE);
    else
      vty_out(vty, "no system multicast learning%s", VTY_NEWLINE);
  }
	if(nsm_global.bpdu_enable != NSM_GLOBAL_BPDU_DEFAULT)
  {
    if(nsm_global.bpdu_enable)
			vty_out(vty, "system l2 bpdu enable%s", VTY_NEWLINE);
    else
      vty_out(vty, "no system l2 bpdu enable%s", VTY_NEWLINE);
  }
#ifdef ZPL_NSM_IGMP
  if(nsm_global.snoop_proto.igmp_snoop)
		vty_out(vty, "igmp snooping enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.igmp_proxy)
		vty_out(vty, "igmp proxy enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.igmpqry_snoop)
		vty_out(vty, "igmp qry snooping enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.igmpqry_proxy)
		vty_out(vty, "igmp qry proxy enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.igmpunknow_snoop)
		vty_out(vty, "igmp unknow snooping enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.igmpunknow_proxy)
		vty_out(vty, "igmp unknow proxy enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.mld_snoop)
		vty_out(vty, "mld snooping enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.mld_proxy)
		vty_out(vty, "mld proxy enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.mldqry_snoop)
		vty_out(vty, "mld qry snooping enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.mldqry_proxy)
		vty_out(vty, "mld qry proxy enable%s", VTY_NEWLINE);

  if(nsm_global.snoop_proto.arp_snoop)
		vty_out(vty, "ip arp snooping enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.rarp_snoop)
		vty_out(vty, "ip rarp snooping enable%s", VTY_NEWLINE);
  if(nsm_global.snoop_proto.dhcp_snoop)
		vty_out(vty, "ip dhcp snooping enable%s", VTY_NEWLINE);
#endif
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return 0;
}

int nsm_global_init(void)
{
  template_t * temp = NULL;
  os_memset(&nsm_global, 0, sizeof(nsm_global_t));
	nsm_global.mutex = os_mutex_name_init("nsmgl");
	temp = lib_template_new (zpl_true);
	if(temp)
	{
		temp->module = 0;
		strcpy(temp->name, "global");
		temp->write_template = bulid_global_config;
		temp->pVoid = NULL;
		lib_template_config_list_install(temp, 0);
	}

	return OK;
}

int nsm_global_exit(void)
{
	if(nsm_global.mutex)
		os_mutex_exit(nsm_global.mutex);
	return OK;
}

int nsm_global_start(void)
{
    int ret = 0;
  //hal_igmp_snooping_init();
	ret |= nsm_global_switch_forward_set(zpl_true);
  ret |= nsm_global_jumbo_size_set(NSM_GLOBAL_JUMBO_DEFAULT);
  return ret;
}


int nsm_cpu_rate_set_api(zpl_uint32	cpu_rate)
{
	if (nsm_global.cpu_rate != cpu_rate)
	{
		if(hal_qos_cpu_rate_limit(cpu_rate, 0) == OK)
		{
			nsm_global.cpu_rate = cpu_rate;
			return OK;
		}
		return ERROR;
	}
	return OK;
}

int nsm_global_jumbo_size_set(zpl_uint32 value)
{
	int ret = ERROR;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
		ret = hal_jumbo_size_set(value);
#else
		ret = OK;
#endif
		if(ret == OK)
			nsm_global.global_jumbo_size = value;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}
int nsm_global_jumbo_size_get(zpl_uint32 *value)
{
	int ret = OK;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
	if(value)
			*value = nsm_global.global_jumbo_size;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}


int nsm_global_switch_forward_set(zpl_bool value)
{
	int ret = ERROR;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
		ret = hal_switch_forward(value);
#else
		ret = OK;
#endif
		if(ret == OK)
			nsm_global.switch_forward = value;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}
int nsm_global_switch_forward_get(zpl_bool *value)
{
	int ret = OK;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
	if(value)
			*value = nsm_global.switch_forward;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}


int nsm_global_multicast_flood_set(zpl_bool value)
{
	int ret = ERROR;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
		ret = hal_multicast_flood(value);
#else
		ret = OK;
#endif
		if(ret == OK)
			nsm_global.multicast_flood = value;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}
int nsm_global_multicast_flood_get(zpl_bool *value)
{
	int ret = OK;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
	if(value)
			*value = nsm_global.multicast_flood;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}

int nsm_global_unicast_flood_set(zpl_bool value)
{
	int ret = ERROR;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
		ret = hal_unicast_flood(value);
#else
		ret = OK;
#endif
		if(ret == OK)
			nsm_global.unicast_flood = value;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}
int nsm_global_unicast_flood_get(zpl_bool *value)
{
	int ret = OK;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
	if(value)
			*value = nsm_global.unicast_flood;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}
int nsm_global_multicast_learning_set(zpl_bool value)
{
	int ret = ERROR;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
		ret = hal_multicast_learning(value);
#else
		ret = OK;
#endif
		if(ret == OK)
			nsm_global.multicast_learning = value;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}
int nsm_global_multicast_learning_get(zpl_bool *value)
{
	int ret = OK;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
	if(value)
			*value = nsm_global.multicast_learning;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}

//全局使能接收BPDU报文
int nsm_global_bpdu_set(zpl_bool value)
{
	int ret = ERROR;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
		ret = hal_global_bpdu_enable(value);
#else
		ret = OK;
#endif
		if(ret == OK)
			nsm_global.bpdu_enable = value;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}

int nsm_global_bpdu_get(zpl_bool *value)
{
	int ret = OK;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
	if(value)
			*value = nsm_global.bpdu_enable;
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}


#ifdef ZPL_NSM_IGMP
int nsm_snooping_proto_set(enum nsm_snoop_proto_type type, enum nsm_proto_action action, zpl_bool value)
{
	int ret = ERROR;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
  switch(type)
  {      	
    case NSM_SNOOP_PROTO_IGMP:
      {
#ifdef ZPL_HAL_MODULE
		  ret = hal_igmp_snooping_set(action, value);
#else
		  ret = OK;
#endif
      	if(ret == OK)
        {
			    if(action == NSM_PKT_SNOOPING)
            nsm_global.snoop_proto.igmp_snoop = value;
          else
            nsm_global.snoop_proto.igmp_proxy = value;
        }
      }
      break;
    case NSM_SNOOP_PROTO_IGMPQRY:
      {
#ifdef ZPL_HAL_MODULE
		  ret = hal_igmpqry_snooping_set(action, value);
#else
		  ret = OK;
#endif
      	if(ret == OK)
        {
			    if(action == NSM_PKT_SNOOPING)
            nsm_global.snoop_proto.igmpqry_snoop = value;
          else
            nsm_global.snoop_proto.igmpqry_proxy = value;
        }
      }
      break;
    case NSM_SNOOP_PROTO_IGMPUNKNOW:
      {
#ifdef ZPL_HAL_MODULE
		  ret = hal_igmpunknow_snooping_set(action, value);
#else
		  ret = OK;
#endif
      	if(ret == OK)
        {
			    if(action == NSM_PKT_SNOOPING)
            nsm_global.snoop_proto.igmpunknow_snoop = value;
          else
            nsm_global.snoop_proto.igmpunknow_proxy = value;
        }
      }
      break;
    case NSM_SNOOP_PROTO_MLD:
      {
#ifdef ZPL_HAL_MODULE
		  ret = hal_mld_snooping_set(action, value);
#else
		  ret = OK;
#endif
      	if(ret == OK)
        {
			    if(action == NSM_PKT_SNOOPING)
            nsm_global.snoop_proto.mld_snoop = value;
          else
            nsm_global.snoop_proto.mld_proxy = value;
        }
      }
      break;
    case NSM_SNOOP_PROTO_MLDQRY:
      {
#ifdef ZPL_HAL_MODULE
		  ret = hal_mldqry_snooping_set(action, value);
#else
		  ret = OK;
#endif
      	if(ret == OK)
        {
			    if(action == NSM_PKT_SNOOPING)
            nsm_global.snoop_proto.mldqry_snoop = value;
          else
            nsm_global.snoop_proto.mldqry_proxy = value;
        }
      }
      break;
    case NSM_SNOOP_PROTO_ARP:
      {
#ifdef ZPL_HAL_MODULE
		  ret = hal_arp_snooping_set(value);
#else
		  ret = OK;
#endif
      	if(ret == OK)
        {
            nsm_global.snoop_proto.arp_snoop = value;
        }
      }
      break;
    case NSM_SNOOP_PROTO_RARP:
      {
#ifdef ZPL_HAL_MODULE
		  ret = hal_rarp_snooping_set(value);
#else
		  ret = OK;
#endif
      	if(ret == OK)
        {
            nsm_global.snoop_proto.rarp_snoop = value;
        }
      }
      break;
    case NSM_SNOOP_PROTO_DHCP:
      {
#ifdef ZPL_HAL_MODULE
		  ret = hal_dhcp_snooping_set(value);
#else
		  ret = OK;
#endif
      	if(ret == OK)
        {
            nsm_global.snoop_proto.dhcp_snoop = value;
        }
      }
      break;
    default:
      break;  
  }  
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}

int nsm_snooping_proto_get(enum nsm_snoop_proto_type type, enum nsm_proto_action action, zpl_bool *value)
{
	int ret = OK;
	if(nsm_global.mutex)
		os_mutex_lock(nsm_global.mutex, OS_WAIT_FOREVER);
  switch(type)
  {      	
    case NSM_SNOOP_PROTO_IGMP:
      {
      	if(value)
			    *value = (action == NSM_PKT_SNOOPING)?nsm_global.snoop_proto.igmp_snoop:nsm_global.snoop_proto.igmp_proxy;
      }
      break;
    case NSM_SNOOP_PROTO_IGMPQRY:
      {
      	if(value)
			    *value = (action == NSM_PKT_SNOOPING)?nsm_global.snoop_proto.igmpqry_snoop:nsm_global.snoop_proto.igmpqry_proxy;
      }
      break;
    case NSM_SNOOP_PROTO_IGMPUNKNOW:
      {
      	if(value)
			    *value = (action == NSM_PKT_SNOOPING)?nsm_global.snoop_proto.igmpunknow_snoop:nsm_global.snoop_proto.igmpunknow_proxy;
      }
      break;
    case NSM_SNOOP_PROTO_MLD:
      {
      	if(value)
			    *value = (action == NSM_PKT_SNOOPING)?nsm_global.snoop_proto.mld_snoop:nsm_global.snoop_proto.mld_proxy;
      }
      break;
    case NSM_SNOOP_PROTO_MLDQRY:
      {
      	if(value)
			    *value = (action == NSM_PKT_SNOOPING)?nsm_global.snoop_proto.mldqry_snoop:nsm_global.snoop_proto.mldqry_proxy;
      }
      break;
    case NSM_SNOOP_PROTO_ARP:
      {
      	if(value)
			    *value = nsm_global.snoop_proto.arp_snoop;
      }
      break;
    case NSM_SNOOP_PROTO_RARP:
      {
      	if(value)
			    *value = nsm_global.snoop_proto.rarp_snoop;
      }
      break;
    case NSM_SNOOP_PROTO_DHCP:
      {
      	if(value)
			    *value = nsm_global.snoop_proto.dhcp_snoop;
      }
      break;
    default:
      break;  
  }  
	if(nsm_global.mutex)
		os_mutex_unlock(nsm_global.mutex);
	return ret;
}


#endif

