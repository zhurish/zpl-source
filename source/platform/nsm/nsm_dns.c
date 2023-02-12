/*
 * nsm_dns.c
 *
 *  Created on: Oct 13, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"

#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "nsm_dns.h"

typedef struct ip_dns_job_s
{
	struct ip_dns_job_s *next;
	dns_cmd_t cmd;
	void *p;
}ip_dns_job_t;

zpl_uint8 _dns_debug = IP_DNS_DEBUG|IP_DNS_EVENT_DEBUG;
static Gip_dns_t gIpdns;

static ip_dns_job_t *head_list = NULL;
static ip_dns_job_t *free_list = NULL;

static int ip_dns_cleanup(dns_class_t type, zpl_bool all);

static int ip_dns_start_job(dns_cmd_t cmd, void *p);

//kernel
static int ip_dns_kernel_add(zpl_char *domain1, zpl_char *domain2, ip_dns_t *dns1, ip_dns_t *dns2, ip_dns_t *dns3);
static int ip_dns_kernel_del(void);
static int ip_host_kernel_load_backup(void);
static int ip_host_kernel_add(ip_host_t *dns1, zpl_char *name);
static int ip_host_kernel_del(ip_host_t *dns1);



int nsm_ip_dns_init(void)
{
	gIpdns.dnsList = malloc(sizeof(LIST));
	gIpdns.mutex = os_mutex_name_create("dns-mutex");
	lstInit(gIpdns.dnsList);
	return OK;
}


int nsm_ip_dns_exit(void)
{
	if(lstCount(gIpdns.dnsList))
	{
		ip_dns_cleanup(0, zpl_true);
		lstFree(gIpdns.dnsList);
		free(gIpdns.dnsList);
		gIpdns.dnsList = NULL;
	}
	if(gIpdns.mutex)
		os_mutex_destroy(gIpdns.mutex);

	ip_host_kernel_load_backup();

	return OK;
}


static int ip_dns_cleanup(dns_class_t type, zpl_bool all)
{
	ip_dns_t *pstNode = NULL;
	NODE index;
	if(!lstCount(gIpdns.dnsList))
		return OK;

	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		//all dynamic dns on interface
		if(type && pstNode && pstNode->type == type)
		{
			lstDelete(gIpdns.dnsList, (NODE*)pstNode);
			if((pstNode->type == IP_HOST_DYNAMIC || pstNode->type == IP_HOST_STATIC))
				XFREE(MTYPE_IP_HOST, pstNode);
			else if((pstNode->type == IP_DNS_DYNAMIC || pstNode->type == IP_DNS_STATIC))
				XFREE(MTYPE_IP_DNS, pstNode);
		}

		//all
		else if(pstNode && all)
		{
			lstDelete(gIpdns.dnsList, (NODE*)pstNode);
			if((pstNode->type == IP_HOST_DYNAMIC || pstNode->type == IP_HOST_STATIC))
				XFREE(MTYPE_IP_HOST, pstNode);
			else if((pstNode->type == IP_DNS_DYNAMIC || pstNode->type == IP_DNS_STATIC))
				XFREE(MTYPE_IP_DNS, pstNode);
		}
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return OK;
}


static int ip_dns_add_node(ip_dns_t *value)
{
	ip_dns_t *node = NULL;
	if((value->type == IP_HOST_DYNAMIC || value->type == IP_HOST_STATIC))
	{
		node = XMALLOC(MTYPE_IP_HOST, sizeof(ip_dns_t));
	}
	else if((value->type == IP_DNS_DYNAMIC || value->type == IP_DNS_STATIC))
	{
		node = XMALLOC(MTYPE_IP_DNS, sizeof(ip_dns_t));
	}
	if(node)
	{
		os_memset(node, 0, sizeof(ip_dns_t));
		os_memcpy(node, value, sizeof(ip_dns_t));
		lstAdd(gIpdns.dnsList, (NODE *)node);
		if((value->type == IP_HOST_STATIC))
			ip_host_kernel_add(node, node->_host_name);
		if((value->type == IP_DNS_DYNAMIC || value->type == IP_DNS_STATIC))
			ip_dns_start_job((value->type == IP_DNS_STATIC) ? IP_DNS_ADD_STATIC:IP_DNS_ADD_DYNAMIC, node);

		if(_dns_debug & IP_DNS_DEBUG)
		{
			zpl_char buf[128];
			union prefix46constptr pu;
			memset(buf, 0, sizeof(buf));
			pu.p = &value->address;
			prefix_2_address_str (pu, buf, sizeof(buf));
			if((value->type == IP_DNS_DYNAMIC || value->type == IP_DNS_STATIC))
				zlog_debug(MODULE_NSM, "add DNS Server %s on %s", buf,
					ifindex2ifname(value->_dns_ifindex));
			else
				zlog_debug(MODULE_NSM, "add host name %s <-> %s", buf,
					value->_host_name);
		}
		return OK;
	}
	return ERROR;
}

static ip_dns_t * ip_dns_lookup_node(struct prefix *address, dns_class_t type)
{
	ip_dns_t *pstNode = NULL;
	NODE index;
	if(!lstCount(gIpdns.dnsList))
		return NULL;
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		//if(host)
		{
			if(prefix_same (&pstNode->address, address) && (pstNode->type == type)
					/*(pstNode->type == IP_HOST_DYNAMIC || pstNode->type == IP_HOST_STATIC)*/)
			{
				return pstNode;
			}
		}
/*		else
		{
			if(prefix_same (&pstNode->address, address) &&
					(pstNode->type == IP_DNS_DYNAMIC || pstNode->type == IP_DNS_STATIC))
			{
				return pstNode;
			}
		}*/
	}
	return NULL;
}


int nsm_ip_dns_callback_api(ip_dns_cb cb, void *pVoid)
{
	ip_dns_t *pstNode = NULL;
	NODE index;
	if(!lstCount(gIpdns.dnsList))
		return OK;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && (pstNode->type == IP_DNS_DYNAMIC || pstNode->type == IP_DNS_STATIC))
		{
			if(cb)
			{
				(cb)(pstNode, pVoid);
			}
		}
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return OK;
}

int nsm_ip_host_callback_api(ip_dns_cb cb, void *pVoid)
{
	ip_dns_t *pstNode = NULL;
	NODE index;
	if(!lstCount(gIpdns.dnsList))
		return OK;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && (pstNode->type == IP_HOST_DYNAMIC || pstNode->type == IP_HOST_STATIC))
		{
			if(cb)
			{
				(cb)(pstNode, pVoid);
			}
		}
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return OK;
}

ip_dns_t * nsm_ip_dns_lookup_api(struct prefix *address, dns_class_t type)
{
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	ip_dns_t *value = ip_dns_lookup_node(address, type);
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return value;
}

ip_host_t * nsm_ip_host_lookup_api(struct prefix *address, dns_class_t type)
{
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	ip_dns_t *value = ip_dns_lookup_node(address, type);
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return (ip_host_t*)value;
}



int nsm_ip_dns_add(struct prefix *address, ip_dns_opt_t *opt,  zpl_bool	secondly, dns_class_t type)
{
	int ret = ERROR;
	ip_dns_t value;
	ip_dns_t *node = NULL;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	node = ip_dns_lookup_node(address, type);
	if(!node)
	{
		os_memset(&value, 0, sizeof(ip_dns_t));
		prefix_copy (&value.address, address);
		value.type = type;
		if(type == IP_DNS_STATIC)
		{
			value._dns_metric = IP_DNS_METRIC_STATIC_DEFAULT;
		}
		value._dns_secondly = secondly;
		if(opt)
		{
			memcpy(&value.data.dns_opt, opt, sizeof(ip_dns_opt_t));
		}
		ret = ip_dns_add_node(&value);
		//ip_dns_add_job(IP_DNS_ADD_DYNAMIC, NULL);
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);

	return ret;
}

int nsm_ip_dns_add_api(struct prefix *address, zpl_bool	secondly)
{
	return nsm_ip_dns_add(address, NULL, secondly, IP_DNS_STATIC);
}

int nsm_ip_dns_get_api(ifindex_t ifindex, struct prefix *address, zpl_bool	secondly)
{
	ip_dns_t *pstNode = NULL;
	NODE index;
	if(!lstCount(gIpdns.dnsList))
		return ERROR;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->_dns_secondly == secondly && ifindex == pstNode->_dns_ifindex)
		{
			if(address)
				memcpy(address, &pstNode->address, sizeof(struct prefix));
			if(gIpdns.mutex)
				os_mutex_unlock(gIpdns.mutex);
			return OK;
		}
		//if(ifindex == pstNode->_dns_ifindex && (pstNode->type == type))
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return ERROR;
}

int nsm_ip_dns_del_by_ifindex(ifindex_t ifindex, dns_class_t type)
{
	ip_dns_t *pstNode = NULL;
	NODE index;
	if(!lstCount(gIpdns.dnsList))
		return ERROR;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		//if(host)
		{
			if(ifindex == pstNode->_dns_ifindex && (pstNode->type == type))
			{
				if(_dns_debug & IP_DNS_DEBUG)
				{
					zpl_char buf[128];
					union prefix46constptr pu;
					memset(buf, 0, sizeof(buf));
					pu.p = &pstNode->address;
					prefix_2_address_str (pu, buf, sizeof(buf));
					if((pstNode->type == IP_DNS_DYNAMIC || pstNode->type == IP_DNS_STATIC))
						zlog_debug(MODULE_NSM, "add DNS Server %s on %s", buf,
							ifindex2ifname(pstNode->_dns_ifindex));
					else
						zlog_debug(MODULE_NSM, "add host name %s <-> %s", buf,
								pstNode->_host_name);
				}

				if(gIpdns.dns1 == pstNode)
				{
					gIpdns.dns1 = NULL;
				}
				if(gIpdns.dns2 == pstNode)
				{
					gIpdns.dns2 = NULL;
				}
				lstDelete(gIpdns.dnsList, (NODE*)pstNode);
				XFREE(MTYPE_IP_DNS, pstNode);
			}
		}
	}
	ip_dns_start_job(IP_DNS_DEL_DYNAMIC, NULL);

	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return OK;
}

int nsm_ip_dns_del(struct prefix *address, dns_class_t type)
{
	int ret = ERROR;
	ip_dns_t *value;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);

	value = ip_dns_lookup_node(address, type);
	if(value && value->type == type)
	{
		//ip_dns_add_job(NULL);
		if(gIpdns.dns1 == value)
		{
			gIpdns.dns1 = NULL;
		}
		if(gIpdns.dns2 == value)
		{
			gIpdns.dns2 = NULL;
		}
		if((value->type == IP_DNS_DYNAMIC || value->type == IP_DNS_STATIC))
			ip_dns_start_job((value->type == IP_DNS_STATIC) ? IP_DNS_DEL_STATIC:IP_DNS_DEL_DYNAMIC, NULL);

		if(_dns_debug & IP_DNS_DEBUG)
		{
			zpl_char buf[128];
			union prefix46constptr pu;
			memset(buf, 0, sizeof(buf));
			pu.p = &value->address;
			prefix_2_address_str (pu, buf, sizeof(buf));
			if((value->type == IP_DNS_DYNAMIC || value->type == IP_DNS_STATIC))
				zlog_debug(MODULE_NSM, "add DNS Server %s on %s", buf,
					ifindex2ifname(value->_dns_ifindex));
			else
				zlog_debug(MODULE_NSM, "add host name %s <-> %s", buf,
					value->_host_name);
		}

		lstDelete(gIpdns.dnsList, (NODE*)value);
		XFREE(MTYPE_IP_DNS, value);
		ret = OK;
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return ret;
}

int nsm_ip_dns_del_api(struct prefix *address)
{
	return nsm_ip_dns_del(address, IP_DNS_STATIC);
}


int nsm_ip_host_add(struct prefix *address, zpl_char *name, dns_class_t type)
{
	int ret = ERROR;
	ip_host_t value;
	ip_host_t *node = NULL;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	node = ip_dns_lookup_node(address, type);
	if(!node)
	{
		os_memset(&value, 0, sizeof(ip_dns_t));
		prefix_copy (&value.address, address);
		value.type = type;
		if(name)
			strcpy(value._host_name, name);
		ret = ip_dns_add_node(&value);
	}
	else
	{
		value.type = type;
		if(name)
		{
			os_memset(node->_host_name, 0, sizeof(node->_host_name));
			strcpy(node->_host_name, name);
		}
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return ret;
}

int nsm_ip_host_add_api(struct prefix *address, zpl_char *name)
{
	return nsm_ip_host_add(address, name, IP_HOST_STATIC);
}


int nsm_ip_host_del(struct prefix *address, dns_class_t type)
{
	int ret = ERROR;
	ip_host_t *value;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);

	value = ip_dns_lookup_node(address, type);
	if(value && value->type == type)
	{
		if((value->type == IP_HOST_STATIC))
			ip_host_kernel_del(value);

		if(_dns_debug & IP_DNS_DEBUG)
		{
			zpl_char buf[128];
			union prefix46constptr pu;
			memset(buf, 0, sizeof(buf));
			pu.p = &value->address;
			prefix_2_address_str (pu, buf, sizeof(buf));
			if((value->type == IP_DNS_DYNAMIC || value->type == IP_DNS_STATIC))
				zlog_debug(MODULE_NSM, "del DNS Server %s on %s", buf,
					ifindex2ifname(value->_dns_ifindex));
			else
				zlog_debug(MODULE_NSM, "del host name %s <-> %s", buf,
					value->_host_name);
		}

		lstDelete(gIpdns.dnsList, (NODE*)value);
		XFREE(MTYPE_IP_HOST, value);
		ret = OK;
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return ret;
}

int nsm_ip_host_del_api(struct prefix *address)
{
	return nsm_ip_host_del(address, IP_HOST_STATIC);
}


int nsm_dns_domain_name_add_api(zpl_char *name, zpl_bool secondly)
{
	int ret = ERROR;

	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	if(secondly && name)
	{
		os_memset(gIpdns.domain_name2, 0, sizeof(gIpdns.domain_name2));
		os_strcpy(gIpdns.domain_name2, name);
		ip_dns_start_job(IP_DNS_ADD_DOMAIN, NULL);
	}
	else if(name)
	{
		os_memset(gIpdns.domain_name1, 0, sizeof(gIpdns.domain_name1));
		os_strcpy(gIpdns.domain_name1, name);
		ip_dns_start_job(IP_DNS_ADD_DOMAIN, NULL);
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return ret;
}

int nsm_dns_domain_name_del_api(zpl_bool secondly)
{
	int ret = ERROR;

	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	if(secondly)
	{
		os_memset(gIpdns.domain_name2, 0, sizeof(gIpdns.domain_name2));
		ip_dns_start_job(IP_DNS_DEL_DOMAIN, NULL);
		gIpdns.domain_dynamic2 = zpl_false;
	}
	else
	{
		os_memset(gIpdns.domain_name1, 0, sizeof(gIpdns.domain_name1));
		ip_dns_start_job(IP_DNS_DEL_DOMAIN, NULL);
		gIpdns.domain_dynamic1 = zpl_false;
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return ret;
}

int nsm_dns_domain_name_dynamic_api(zpl_bool dynamic, zpl_bool secondly)
{
	int ret = ERROR;

	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);
	if(secondly)
	{
		gIpdns.domain_dynamic2 = dynamic;
	}
	else
	{
		gIpdns.domain_dynamic1 = dynamic;
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return ret;
}
#ifdef ZPL_SHELL_MODULE
int nsm_ip_dns_host_config(struct vty *vty)
{
	ip_dns_t *pstNode = NULL;
	NODE index;
	zpl_char buf[128];
	union prefix46constptr pu;
	memset(buf, 0, sizeof(buf));
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);

	if(strlen(gIpdns.domain_name1) && !gIpdns.domain_dynamic1)
		vty_out(vty, "dns domain %s%s", gIpdns.domain_name1, VTY_NEWLINE);

	if(strlen(gIpdns.domain_name2) && !gIpdns.domain_dynamic2)
		vty_out(vty, "dns domain %s%s", gIpdns.domain_name2, VTY_NEWLINE);

	if(!lstCount(gIpdns.dnsList))
	{
		if(gIpdns.mutex)
			os_mutex_unlock(gIpdns.mutex);
		return OK;
	}
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(/*pstNode->type == IP_HOST_DYNAMIC || */pstNode->type == IP_HOST_STATIC
					&& strlen(pstNode->_host_name))
			{
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%s", cli_inet_ethernet(pstNode->address.u.prefix_eth.octet));
/*				pu.p = &pstNode->address;
				prefix_2_address_str (pu, buf, sizeof(buf));*/
				vty_out(vty, "ip host %s %s%s", buf,
						pstNode->_host_name, VTY_NEWLINE);
			}
		}
	}
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(/*pstNode->type == IP_DNS_DYNAMIC || */pstNode->type == IP_DNS_STATIC && ifindex2ifname(pstNode->_dns_ifindex))
			{
				memset(buf, 0, sizeof(buf));
				pu.p = &pstNode->address;
				prefix_2_address_str (pu, buf, sizeof(buf));
				vty_out(vty, "dns server %s%s", buf, VTY_NEWLINE);
			}
		}
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return 1;
}

int nsm_ip_dns_host_show(struct vty *vty)
{
	ip_dns_t *pstNode = NULL;
	NODE index;
	zpl_char tmp[20], flag = 0;
	zpl_char buf[128];
	union prefix46constptr pu;
	memset(buf, 0, sizeof(buf));

	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);

	vty_out(vty, "Current config  : %s", VTY_NEWLINE);
	vty_out(vty, " DNS domain(%s %s)          : %s %s %s", gIpdns.domain_dynamic1 ? "D":"S",
			gIpdns.domain_dynamic2 ? "D":"S",
			gIpdns.domain_name1, gIpdns.domain_name2, VTY_NEWLINE);
	if(gIpdns.dns1)
	{
		pu.p = &gIpdns.dns1->address;
		prefix_2_address_str (pu, buf, sizeof(buf));
		vty_out(vty, " DNS Server           : %s%s", buf, VTY_NEWLINE);
	}
	if(gIpdns.dns2)
	{
		memset(buf, 0, sizeof(buf));
		pu.p = &gIpdns.dns2->address;
		prefix_2_address_str (pu, buf, sizeof(buf));
		vty_out(vty, " DNS Server           : %s%s", buf, VTY_NEWLINE);
	}
	if(gIpdns.dns3)
	{
		memset(buf, 0, sizeof(buf));
		pu.p = &gIpdns.dns3->address;
		prefix_2_address_str (pu, buf, sizeof(buf));
		vty_out(vty, " DNS Server           : %s%s", buf, VTY_NEWLINE);
	}

	if(!lstCount(gIpdns.dnsList))
	{
		if(gIpdns.mutex)
			os_mutex_unlock(gIpdns.mutex);
		return OK;
	}

	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && ifindex2ifname(pstNode->_dns_ifindex))
		{
			if(pstNode->type == IP_DNS_DYNAMIC || pstNode->type == IP_DNS_STATIC)
			{
				if(flag == 0)
				{
					flag = 1;

					vty_out(vty, "%-20s %-20s %-16s %-16s %-4s%s", "--------------------", "--------------------",
							"----------------", "----------------", "----", VTY_NEWLINE);
					vty_out(vty, "%-20s %-20s %-16s %-16s %-4s%s", "DNS Server", "Interface", "VPN/METRIC", "STATE/BACKUP", "FLAG",VTY_NEWLINE);
					vty_out(vty, "%-20s %-20s %-16s %-16s %-4s%s", "--------------------", "--------------------",
							"----------------", "----------------", "----",VTY_NEWLINE);
				}
				memset(buf, 0, sizeof(buf));
				pu.p = &pstNode->address;
				prefix_2_address_str (pu, buf, sizeof(buf));
				vty_out(vty, "%-20s ", buf);
				vty_out(vty, "%-20s ", ifindex2ifname(pstNode->_dns_ifindex));
				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "%d/%d", pstNode->_dns_vrfid, pstNode->_dns_metric);
				vty_out(vty, "%-16s ", tmp);

				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "%s/%s", pstNode->_dns_active ? "UP":"DOWN",
						pstNode->_dns_secondly ? "zpl_true":"zpl_false");
				vty_out(vty, "%-16s %-4s%s", tmp, (pstNode->type == IP_DNS_DYNAMIC)? "D":"S",VTY_NEWLINE);
			}
		}
	}
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return OK;
}
#endif

static int ip_dns_static_update_interface(ip_dns_t *value)
{
	//nexthop_active_update
	if(value->_dns_ifindex)
		return OK;
	else
	{
		struct interface *ifp = if_lookup_prefix(&value->address);
		if(ifp)
		{
			value->_dns_ifindex = ifp->ifindex;
			if(_dns_debug & IP_DNS_EVENT_DEBUG)
			{
				zpl_char buf[128];
				union prefix46constptr pu;
				memset(buf, 0, sizeof(buf));
				pu.p = &value->address;
				prefix_2_address_str (pu, buf, sizeof(buf));
				zlog_debug(MODULE_NSM, "update DNS Server %s on %s", buf,
						ifindex2ifname(value->_dns_ifindex));
			}
		}
	}
	return OK;
}

static int ip_dns_static_check_active(ip_dns_t *value)
{
	struct interface *ifp = NULL;
	if(value->_dns_ifindex)
		ifp = if_lookup_by_index(value->_dns_ifindex);
	else
		ifp = if_lookup_prefix(&value->address);
	{
		//struct interface *ifp = if_lookup_prefix(&value->address);
		if(ifp)
		{
			zpl_bool old = value->_dns_active;
			if(if_is_operative(ifp))
				value->_dns_active = zpl_true;
			else
				value->_dns_active = zpl_false;

			if((_dns_debug & IP_DNS_EVENT_DEBUG) && (old != value->_dns_active))
			{
				zpl_char buf[128];
				union prefix46constptr pu;
				memset(buf, 0, sizeof(buf));
				pu.p = &value->address;
				prefix_2_address_str (pu, buf, sizeof(buf));
				zlog_debug(MODULE_NSM, "DNS Server %s on %s state channel %s -> %s",
						buf,
						ifindex2ifname(value->_dns_ifindex),
						old ? "UP":"DOWN", value->_dns_active ? "UP":"DOWN");
			}
			return OK;
		}
		//return OK;
	}
	return ERROR;
}

static int ip_dns_update_active(ip_dns_t *value)
{
	ip_dns_static_update_interface(value);
	ip_dns_static_check_active(value);
	return OK;
}

static int ip_dns_choose_best_metric(ip_dns_t *value)
{
	if(value->_dns_secondly)
	{
		if(gIpdns.dns2 == NULL)
			gIpdns.dns2 = value;
		else
		{
			if(gIpdns.dns2->_dns_active == zpl_true)
			{
				if(value->_dns_metric < gIpdns.dns2->_dns_metric)
					gIpdns.dns2 = value;
			}
			else
				gIpdns.dns2 = value;
		}
	}
	else
	{
		if(gIpdns.dns1 == NULL)
			gIpdns.dns1 = value;
		else
		{
			if(gIpdns.dns1->_dns_active == zpl_true)
			{
				if(value->_dns_metric < gIpdns.dns1->_dns_metric)
					gIpdns.dns1 = value;
			}
			else
				gIpdns.dns1 = value;
		}
	}
	return OK;
}


static int ip_dns_choose_best(ip_dns_t *value)
{
	if(value->_dns_active == zpl_true)
	{
		if(value->type == IP_DNS_STATIC)
		{
			if(value->_dns_secondly)
			{
				if(!gIpdns.dns2)
					gIpdns.dns2 = value;
				if(gIpdns.dns2 && gIpdns.dns2 != value && gIpdns.dns2->type == IP_DNS_STATIC)
				{
					ip_dns_choose_best_metric(value);
				}
				if(gIpdns.dns2 && gIpdns.dns2 != value && gIpdns.dns2->type == IP_DNS_DYNAMIC)
				{
					gIpdns.dns2 = value;
				}
			}
			else
			{
				if(!gIpdns.dns1)
					gIpdns.dns1 = value;
				if(gIpdns.dns1 && gIpdns.dns1 != value && gIpdns.dns1->type == IP_DNS_STATIC)
				{
					ip_dns_choose_best_metric(value);
				}
				if(gIpdns.dns1 && gIpdns.dns1 != value && gIpdns.dns1->type == IP_DNS_DYNAMIC)
				{
					gIpdns.dns1 = value;
				}
			}
		}
		if(value->type == IP_DNS_DYNAMIC)
		{
			if(value->_dns_secondly)
			{
				if(!gIpdns.dns2)
					gIpdns.dns2 = value;
				if(gIpdns.dns2 && gIpdns.dns2 != value && gIpdns.dns2->type == IP_DNS_DYNAMIC)
				{
					ip_dns_choose_best_metric(value);
				}
				if(gIpdns.dns2 && gIpdns.dns2 != value && gIpdns.dns2->type == IP_DNS_STATIC)
				{
					//gIpdns.dns2 = value;
				}
			}
			else
			{
				if(!gIpdns.dns1)
					gIpdns.dns1 = value;
				if(gIpdns.dns1 && gIpdns.dns1 != value && gIpdns.dns1->type == IP_DNS_DYNAMIC)
				{
					ip_dns_choose_best_metric(value);
				}
				if(gIpdns.dns1 && gIpdns.dns1 != value && gIpdns.dns1->type == IP_DNS_STATIC)
				{
					//gIpdns.dns1 = value;
				}
			}
		}
	}
	if((_dns_debug & IP_DNS_EVENT_DEBUG))
	{
		zpl_char buf[128];
		union prefix46constptr pu;
		memset(buf, 0, sizeof(buf));
		if(gIpdns.dns1)
		{
			pu.p = &gIpdns.dns1->address;
			prefix_2_address_str (pu, buf, sizeof(buf));
			zlog_debug(MODULE_NSM, "Choose best: DNS Server %s on %s",
					buf,
					ifindex2ifname(gIpdns.dns1->_dns_ifindex));
		}
		if(gIpdns.dns2)
		{
			pu.p = &gIpdns.dns2->address;
			prefix_2_address_str (pu, buf, sizeof(buf));
			zlog_debug(MODULE_NSM, "Choose best: DNS Server(seondly) %s on %s",
					buf,
					ifindex2ifname(gIpdns.dns2->_dns_ifindex));
		}
	}
	return OK;
}


static int ip_dns_active_process_update(dns_cmd_t cmd, void *p)
{
	NODE index;
	ip_dns_t *pstNode = NULL;
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(pstNode->type == IP_DNS_STATIC || pstNode->type == IP_DNS_DYNAMIC)
			{
				switch(cmd)
				{
				case IP_DNS_UP_IF:
				case IP_DNS_ADD_IF:
				case IP_DNS_ADD_ADDRESS:
				case IP_DNS_ADD_ROUTE:
				case IP_DNS_ADD_STATIC:
				case IP_DNS_ADD_DYNAMIC:
					ip_dns_update_active(pstNode);
					ip_dns_choose_best(pstNode);
					break;
				default:
					break;
				}
			}
		}
	}
	return OK;
}

static int ip_dns_inactive_process_update(dns_cmd_t cmd, void *p)
{
	NODE index;
	ip_dns_t *pstNode = NULL;
	for(pstNode = (ip_dns_t *)lstFirst(gIpdns.dnsList);
			pstNode != NULL;  pstNode = (ip_dns_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(pstNode->type == IP_DNS_STATIC || pstNode->type == IP_DNS_DYNAMIC)
			{
				switch(cmd)
				{
				case IP_DNS_DOWN_IF:
				case IP_DNS_DEL_IF:
				case IP_DNS_DEL_ADDRESS:
				case IP_DNS_DEL_ROUTE:
				case IP_DNS_DEL_STATIC:
				case IP_DNS_DEL_DYNAMIC:
					ip_dns_update_active(pstNode);

					if(gIpdns.dns1 == pstNode)
					{
						if(gIpdns.dns1->_dns_active == zpl_false)
							gIpdns.dns1 = NULL;
					}
					if(gIpdns.dns2 == pstNode)
					{
						if(gIpdns.dns2->_dns_active == zpl_false)
							gIpdns.dns2 = NULL;
					}
					ip_dns_choose_best(pstNode);
					break;
				default:
					break;
				}
			}
		}
	}
	return OK;
}

static int ip_dns_job_work(void *p)
{
	ip_dns_job_t *tmp = NULL;
	ip_dns_job_t *job = p;
	os_msleep(100);
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);

	if((_dns_debug & IP_DNS_EVENT_DEBUG))
	{
		zlog_debug(MODULE_NSM, "start running work job %d # %p", job->cmd, job->p);
	}

	switch(job->cmd)
	{
	case IP_DNS_UP_IF:
	case IP_DNS_ADD_IF:
	case IP_DNS_ADD_ADDRESS:
	case IP_DNS_ADD_ROUTE:
	case IP_DNS_ADD_STATIC:
	case IP_DNS_ADD_DYNAMIC:
	case IP_DNS_ADD_DOMAIN:
		ip_dns_active_process_update(job->cmd, job->p);
		break;
	case IP_DNS_DOWN_IF:
	case IP_DNS_DEL_IF:
	case IP_DNS_DEL_ADDRESS:
	case IP_DNS_DEL_ROUTE:
	case IP_DNS_DEL_STATIC:
	case IP_DNS_DEL_DYNAMIC:
	case IP_DNS_DEL_DOMAIN:
		ip_dns_inactive_process_update(job->cmd, job->p);
		break;
	default:
		break;
	}

	if(gIpdns.dns1 || gIpdns.dns2)
	{
		if((_dns_debug & IP_DNS_EVENT_DEBUG))
		{
			zlog_debug(MODULE_NSM, "add/update DNS Server under work job %d # %p", job->cmd, job->p);
		}
		ip_dns_kernel_add(gIpdns.domain_name1, gIpdns.domain_name2,
			gIpdns.dns1, gIpdns.dns2, gIpdns.dns3);
	}
	else
	{
		if((_dns_debug & IP_DNS_EVENT_DEBUG))
		{
			zlog_debug(MODULE_NSM, "delete DNS Server under work job %d # %p", job->cmd, job->p);
		}
		ip_dns_kernel_del();
	}
	if(head_list == job)
	{
		head_list = job->next;
	}
	else
	{
		for(tmp = head_list; tmp; tmp = tmp->next)
		{
			if(tmp && tmp->next && tmp->next == job)
			{
				//ip_dns_job_t *tmp1 = job->next;
				tmp->next = job->next;
				break;
			}
		}
	}

	tmp = free_list;
	free_list = job;
	free_list->next = tmp;

	job->p = NULL;
	job->cmd = IP_DNS_EV_NONE;
	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return OK;
}

static int ip_dns_start_job(dns_cmd_t cmd, void *p)
{
	ip_dns_job_t *node = NULL;
	if(free_list)
	{
		node = free_list;
		free_list = node->next;
	}
	else
	{
		node = XMALLOC(MTYPE_EVENT, sizeof(ip_dns_job_t));
	}
	if(node)
	{
		ip_dns_job_t *tmp = NULL;
		node->cmd = cmd;
		node->p = p;
		node->next = NULL;
		if(head_list == NULL)
			head_list = node;
		else
		{
			for(tmp = head_list; tmp; tmp = tmp->next)
			{
				if(tmp && tmp->next == NULL)
				{
					tmp->next = node;
					node->next = NULL;
					break;
				}
			}
		}
		if((_dns_debug & IP_DNS_EVENT_DEBUG))
		{
			zlog_debug(MODULE_NSM, "add work job %d # %p", node->cmd, node->p);
		}
		os_job_add(OS_JOB_NONE,ip_dns_job_work, node);
		return OK;
	}
	return ERROR;
}

int ip_dns_add_job(dns_cmd_t cmd, void *p)
{
	int ret = ERROR;
	if(gIpdns.mutex)
		os_mutex_lock(gIpdns.mutex, OS_WAIT_FOREVER);

	ret = ip_dns_start_job(cmd, p);

	if(gIpdns.mutex)
		os_mutex_unlock(gIpdns.mutex);
	return ret;
}



/*
 * KERNEL
 */
static int ip_dns_kernel_add(zpl_char *domain1, zpl_char *domain2, ip_dns_t *dns1, ip_dns_t *dns2, ip_dns_t *dns3)
{
	FILE *fp = NULL;
	zpl_char path[128];
	zpl_char buf[128];
	union prefix46constptr pu;
	memset(buf, 0, sizeof(buf));
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/resolv-nsm.conf", DAEMON_ENV_DIR);

	fp = fopen(path, "w+");
	if(fp)
	{
		fflush(fp);
		fprintf(fp, "# Generated by NSM-DNS\n");
		if(strlen(domain1))
		{
			fprintf(fp, "search %s lan\n", domain1);
			if(strlen(domain2))
				fprintf(fp, "search %s lan\n", domain2);
		}
		else
		{
			if(strlen(domain2))
				fprintf(fp, "search %s lan\n", domain2);
			else
				fprintf(fp, "search lan\n");
		}
		if(dns1 && dns1->address.u.prefix4.s_addr)
		{
			pu.p = &dns1->address;
			prefix_2_address_str (pu, buf, sizeof(buf));
			fprintf(fp, "nameserver %s\n",buf);
		}
		if(dns2 && dns2->address.u.prefix4.s_addr)
		{
			pu.p = &dns2->address;
			prefix_2_address_str (pu, buf, sizeof(buf));
			fprintf(fp, "nameserver %s\n",buf);
		}

		if(dns3 && dns3->address.u.prefix4.s_addr)
		{
			pu.p = &dns3->address;
			prefix_2_address_str (pu, buf, sizeof(buf));
			fprintf(fp, "nameserver %s\n",buf);
		}

		fflush(fp);
		fclose(fp);

/*		unlink("/etc/resolv.conf");
		link(path, "/etc/resolv.conf");*/
		return OK;
	}
	return ERROR;
}

static int ip_dns_kernel_del(void)
{
	FILE *fp = NULL;
	zpl_char path[128];
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/resolv-nsm.conf", DAEMON_ENV_DIR);

	fp = fopen(path, "w+");
	if(fp)
	{
		fflush(fp);
		fprintf(fp, "# Generated by NSM-DNS\n");
		fprintf(fp, "search lan\n");
		fprintf(fp, "nameserver 127.0.0.1\n");
		fflush(fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}



static int ip_host_kernel_make_backup(void)
{
/*	FILE *fp = NULL;
	zpl_char path[128];
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "/etc/.hosts-back");*/
	if(access("/etc/.hosts-back", F_OK) == 0)
	{
		return OK;
	}
	super_system("cp /etc/hosts /etc/.hosts-back");
	if(access("/etc/.hosts-back", F_OK) == 0)
	{
		return OK;
	}
	return ERROR;
}

static int ip_host_kernel_load_backup(void)
{
/*	FILE *fp = NULL;
	zpl_char path[128];
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "/etc/.hosts-back");*/
	if(access("/etc/.hosts-back", F_OK) != 0)
	{
		return ERROR;
	}
	super_system("cp /etc/.hosts-back /etc/hosts");
	if(access("/etc/hosts", F_OK) == 0)
	{
		return OK;
	}
	return OK;
}

static int ip_host_kernel_add(ip_host_t *dns1, zpl_char *name)
{
	FILE *fp = NULL;
	zpl_char buf[128];
	union prefix46constptr pu;
	memset(buf, 0, sizeof(buf));
	if(ip_host_kernel_make_backup() != OK)
		return ERROR;
	fp = fopen("/etc/hosts", "a+");
	if(fp)
	{
		pu.p = &dns1->address;
		prefix_2_address_str (pu, buf, sizeof(buf));
		if((_dns_debug & IP_DNS_EVENT_DEBUG))
		{
			zlog_debug(MODULE_NSM, "kernel add host: host %s -> %s", buf, name);
		}
		fprintf(fp, "# Generated by NSM-DNS\n");
		fprintf(fp, "%s %s\n", buf, name);
		fflush(fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

static int ip_host_kernel_del(ip_host_t *dns1)
{
	zpl_char buf[512];
	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	zpl_char tmp[128];
	union prefix46constptr pu;
	memset(tmp, 0, sizeof(tmp));
	if(ip_host_kernel_make_backup() != OK)
		return ERROR;
	fp1 = fopen("/etc/hosts", "r");
	fp2 = fopen("/etc/hosts-tmp", "r");

	if((_dns_debug & IP_DNS_EVENT_DEBUG))
	{
		zlog_debug(MODULE_NSM, "kernel del host: host %s", buf);
	}
	if(fp1 && fp2)
	{
		zpl_char *s = NULL;
		memset(buf, 0, sizeof(buf));
		pu.p = &dns1->address;
		prefix_2_address_str (pu, tmp, sizeof(tmp));
		while (fgets(buf, sizeof(buf), fp1))
		{
			for (s = buf + strlen(buf);
					(s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';

			if(strstr(buf, tmp))
			{
				;
			}
			else
			{
				fputs(buf, fp2);
			}
		}
		fflush(fp2);
		fclose(fp2);
		fclose(fp1);
		remove("/etc/hosts");
		rename("/etc/hosts-tmp", "/etc/hosts");
		sync();
		return OK;
	}
	if(fp1)
		fclose(fp1);
	if(fp2)
		fclose(fp2);
	return ERROR;
}

