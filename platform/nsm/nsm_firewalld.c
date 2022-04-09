/*
 * nsm_firewalld.c
 *
 *  Created on: 2019年8月31日
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "str.h"
#include "nsm_include.h"
#include "hal_include.h"


struct firewall_service_s
{
	zpl_char 		*name;
	zpl_uint16 	port;
	zpl_uint8		all;
	zpl_uint8		tcp;
	zpl_uint8		udp;	
}firewall_service_default[] = 
{
	//{"FTP-Data", 	20, 0, 1, 0},
	{"FTP", 		21, 0, 1, 0},
	{"SSH", 		22, 0, 1, 0},
	{"Telnet", 		23, 0, 1, 0},
	//{"SMTP", 		25, 0, 1, 0},
	{"DNS", 		53, 0, 1, 1},	
	{"Bootstrap", 	67, 0, 0, 1},
	{"Bootstrap", 	68, 0, 0, 1},
	{"Tftp", 		69, 0, 0, 1},
	{"HTTP", 		80, 0, 1, 0},
	{"SNTP", 		123, 0, 1, 0},	
	//{"SNMP", 		161, 0, 1, 0},
	//{"LDAP",	 	389, 0, 1, 1},	
	{"HTTPS", 		443, 0, 1, 0},
	//{"Login", 		513, 0, 1, 0},
	//{"syslog", 		514, 0, 0, 1},
	//{"LDAPS",	 	636, 0, 1, 1},
	//{"FTPS-Data",	989, 0, 1, 0},	
	//{"FTPS",	 	990, 0, 1, 0},
	{"https", 		8080, 0, 1, 0},
	{"sip", 		5060, 0, 1, 1},
	{"sip-tls", 	5061, 0, 1, 0},		
};

static Gfirewall_t gFirewalld;

static int nsm_halpal_firewall_rule_add_api(firewall_t *value);
static int nsm_halpal_firewall_rule_del_api(firewall_t *value);


const char * nsm_firewall_action_string(firewall_action_t action)
{
	switch (action)
	{
	case FIREWALL_A_DROP:
		return "DROP";
		break;
	case FIREWALL_A_ACCEPT:
		return "ACCEPT";
		break;
	case FIREWALL_A_FORWARD:
		return "FORWARD";
		break;
	case FIREWALL_A_REJECT:
		return "REJECT";
		break;

	case FIREWALL_A_SNAT:
		return "SNAT";
		break;
	case FIREWALL_A_DNAT:
		return "DNAT";
		break;
	case FIREWALL_A_REDIRECT:
		return "REDIRECT";
		break;
	case FIREWALL_A_MARK:
		return "MARK";
		break;
	case FIREWALL_A_LIMIT:
		return "LIMIT";
		break;
	default:
		return "UNKNOW";
		break;
	}
}
const char * nsm_firewall_type_string(firewall_type_t type)
{
	switch (type)
	{
	case FIREWALL_FILTER_INPUT:
		return "INPUT";
		break;
	case FIREWALL_FILTER_OUTPUT:
		return "OUTPUT";
		break;
	case FIREWALL_FILTER_FORWARD:
		return "FORWARD";
		break;
	case FIREWALL_NAT_PREROUTING:
		return "PREROUTING";
		break;
	case FIREWALL_NAT_POSTROUTING:
		return "POSTROUTING";
		break;
	case FIREWALL_NAT_OUTPUT:
		return "OUTPUT";
		break;

	case FIREWALL_MANGLE_PREROUTING:
		return "PREROUTING";
		break;
	case FIREWALL_MANGLE_OUTPUT:
		return "OUTPUT";
		break;
	case FIREWALL_MANGLE_FORWARD:
		return "FORWARD";
		break;
	case FIREWALL_MANGLE_INPUT:
		return "INPUT";
		break;
	case FIREWALL_MANGLE_POSTROUTING:
		return "POSTROUTING";
		break;

	case FIREWALL_RAW_PREROUTING:
		return "PREROUTING";
		break;
	case FIREWALL_RAW_OUTPUT:
		return "OUTPUT";
		break;
	default:
		return "UNKNOW";
		break;
	}
}

const char * nsm_firewall_proto_string(firewall_proto_t type)
{
	switch (type)
	{
	case FIREWALL_P_ALL:
		return "ALL";
		break;
	case FIREWALL_P_TCP:
		return "TCP";
		break;
	case FIREWALL_P_UDP:
		return "UDP";
		break;
	case FIREWALL_P_ICMP:
		return "ICMP";
		break;
	default:
		return "UNKNOW";
		break;
	}
}

static firewall_t * nsm_firewall_rule_add_node(firewall_zone_t *zone, firewall_t *value)
{
	firewall_t *node = XMALLOC(MTYPE_FIREWALL_RULE, sizeof(firewall_t));
	if(node)
	{
		os_memset(node, 0, sizeof(firewall_t));
		os_memcpy(node, value, sizeof(firewall_t));
		lstAdd(zone->zone_list, (NODE *)node);
		//node->ID = gFirewalld.rule_id[node->type] + 1;//lstCount(zone->zone_list);
		//gFirewalld.rule_id[node->type] += 1;	
		return node;
	}
	return NULL;
}

static firewall_t * nsm_firewall_rule_lookup_node(firewall_zone_t *zone, firewall_t *value)
{
	firewall_t *pstNode = NULL;
	NODE index;
	for(pstNode = (firewall_t *)lstFirst(zone->zone_list);
			pstNode != NULL;  pstNode = (firewall_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if( pstNode->class == value->class && os_memcmp(pstNode->name, value->name, FIREWALL_NAME_MAX) == 0)
		{
/*			if( value->family == pstNode->family &&
					value->ID == pstNode->ID &&
					value->type == pstNode->type)
				return pstNode;
			else*/
				return pstNode;
		}
	}
	return NULL;
}


int nsm_firewall_rule_add_api(firewall_zone_t *zone, firewall_t *value)
{
	int ret = ERROR;
	firewall_t *node;
	if(!zone || !zone->zone_list)
		return ERROR;
	if(zone->mutex)
		os_mutex_lock(zone->mutex, OS_WAIT_FOREVER);

	node = nsm_firewall_rule_lookup_node(zone, value);
	if(node == NULL)
	{
#ifdef ZPL_HAL_MODULE
		//ret = hal_mac_add(mac->ifindex, mac->vlan, mac->mac, 0);
#else

#endif
		
		value->ID = gFirewalld.rule_id[value->type] + 1;//lstCount(zone->zone_list);
		ret = nsm_halpal_firewall_rule_add_api(value);

		if(ret == OK && nsm_firewall_rule_add_node(zone, value))
			gFirewalld.rule_id[value->type] += 1;	
		else
			nsm_halpal_firewall_rule_del_api(value);
			
		/*
		node = firewall_rule_add_node(zone, value);
		if(node)
		{
			node->ID = gFirewalld.rule_id[value->type] + 1;
			ret = nsm_halpal_firewall_rule_add_api(node);
			if(ret != OK)
			{
				gFirewalld.rule_id[node->type] -= 1;
				lstDelete(zone->zone_list, (NODE *)node);
				XFREE(MTYPE_FIREWALL_RULE, node);
			}
		}
		*/	
		//TODO
	}
	if(zone->mutex)
		os_mutex_unlock(zone->mutex);
	return ret;
}


int nsm_firewall_rule_del_api(firewall_zone_t *zone, firewall_t *value)
{
	int ret = ERROR;
	firewall_t *node;
	if(!zone || !zone->zone_list)
		return ERROR;
	if(zone->mutex)
		os_mutex_lock(zone->mutex, OS_WAIT_FOREVER);

	node = nsm_firewall_rule_lookup_node(zone, value);
	if(!node)
	{
		ret = ERROR;
	}
	else
	{
		printf("--------------%s-----------------\r\n", __func__);
#ifdef ZPL_HAL_MODULE
		//ret = hal_mac_add(mac->ifindex, mac->vlan, mac->mac, 0);
#else

#endif
		ret = nsm_halpal_firewall_rule_del_api(node);

		if(ret == OK)
		{
			gFirewalld.rule_id[node->type] -= 1;
			lstDelete(zone->zone_list, (NODE *)node);
			XFREE(MTYPE_FIREWALL_RULE, node);
		}

	}
	if(zone->mutex)
		os_mutex_unlock(zone->mutex);
	return ret;
}

firewall_t * nsm_firewall_rule_lookup_api(firewall_zone_t *zone, firewall_t *value)
{
	int ret = ERROR;
	firewall_t *node;
	if(!zone || !zone->zone_list)
		return NULL;
	if(zone->mutex)
		os_mutex_lock(zone->mutex, OS_WAIT_FOREVER);
	node = nsm_firewall_rule_lookup_node(zone, value);
	if(!node)
	{
		ret = 0;
	}
	if(zone->mutex)
		os_mutex_unlock(zone->mutex);
	return node;
}
#ifdef ZPL_SHELL_MODULE
int nsm_firewall_rule_show_api(struct vty *vty, firewall_zone_t *zone, zpl_char * intype)
{
	firewall_t *pstNode = NULL;
	NODE index;
	zpl_char id[16];
	zpl_char type[16];
	zpl_char action[16];
	zpl_char proto[16];
	zpl_char source[64];
	zpl_char destination[64];
	zpl_char s_port[16];
	zpl_char d_port[16];
	zpl_char s_ifindex[64];
	zpl_char d_ifindex[64];
	zpl_char s_mac[16];
	zpl_char d_mac[16];
	int head = 0, found = 0;
	if(!zone || !zone->zone_list)
		return ERROR;
	for(pstNode = (firewall_t *)lstFirst(zone->zone_list);
			pstNode != NULL;  pstNode = (firewall_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;

		if(intype)
		{
			if(strcmp(intype, "filter"))
			{
				if(pstNode && (pstNode->class == FIREWALL_C_FILTER))
					found = 1;
			}
			else if(strcmp(intype, "snat"))
			{
				if(pstNode && (pstNode->class == FIREWALL_C_SNAT))
					found = 1;
			}
			else if(strcmp(intype, "dnat"))
			{
				if(pstNode && (pstNode->class == FIREWALL_C_DNAT))
					found = 1;
			}
			else if(strcmp(intype, "mangle"))
			{
				if(pstNode && (pstNode->class == FIREWALL_C_MANGLE))
					found = 1;
			}
			else if(strcmp(intype, "raw"))
			{
				if(pstNode && (pstNode->class == FIREWALL_C_RAW))
					found = 1;
			}
			else if(strcmp(intype, "port"))
			{
				if(pstNode && (pstNode->class == FIREWALL_C_PORT))
					found = 1;
			}
		}
		else
			found = 1;

		if(pstNode && found)
		{
			memset(id, 0, sizeof(id));
			memset(type, 0, sizeof(type));
			memset(action, 0, sizeof(action));
			memset(proto, 0, sizeof(proto));
			memset(source, 0, sizeof(source));
			memset(destination, 0, sizeof(destination));
			memset(s_port, 0, sizeof(s_port));
			memset(d_port, 0, sizeof(d_port));
			memset(s_ifindex, 0, sizeof(s_ifindex));
			memset(d_ifindex, 0, sizeof(d_ifindex));
			memset(s_mac, 0, sizeof(s_mac));
			memset(d_mac, 0, sizeof(d_mac));
			sprintf(id, "%d", pstNode->ID);
			sprintf(type, "%s", nsm_firewall_type_string(pstNode->type));
			sprintf(action, "%s", nsm_firewall_action_string(pstNode->action));
			sprintf(proto, "%s", nsm_firewall_proto_string(pstNode->proto));

			if(pstNode->source.family)
			{
				zpl_char tmp[64];
				union prefix46constptr pa;
				pa.p = &pstNode->source;
				sprintf(source, "%s", prefix2str (pa, tmp, sizeof(tmp)));
			}
			if(pstNode->destination.family)
			{
				zpl_char tmp[64];
				union prefix46constptr pa;
				pa.p = &pstNode->destination;
				sprintf(destination, "%s", prefix2str (pa, tmp, sizeof(tmp)));
			}

			if(pstNode->s_port)
				sprintf(s_port, "%d", pstNode->s_port);
			if(pstNode->d_port)
				sprintf(d_port, "%d", pstNode->d_port);
			if(pstNode->s_ifindex)
				sprintf(s_ifindex, "%s", ifindex2ifname(pstNode->s_ifindex));
			if(pstNode->d_ifindex)
				sprintf(d_ifindex, "%s", ifindex2ifname(pstNode->d_ifindex));
			if(!str_isempty(pstNode->s_mac, sizeof(pstNode->s_mac)))
				sprintf(s_mac, "%s", inet_ethernet(pstNode->s_mac));
			if(!str_isempty(pstNode->d_mac, sizeof(pstNode->d_mac)))
				sprintf(d_mac, "%s", inet_ethernet(pstNode->d_mac));

			if(head == 0)
			{
				head++;

				if(pstNode && (pstNode->class == FIREWALL_C_FILTER))
				{
					vty_out(vty, " Filter Table:%s",VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-18s %-18s %-5s %-5s %-16s %-16s %s",
							"---", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----","----------------", "----------------",
							VTY_NEWLINE);
					//iptables -A OUTPUT -p tcp --sport 22 -j ACCEPT
					vty_out(vty, " %-3s %-16s %-16s %-18s %-18s %-5s %-5s %-16s %-16s %s",
							"ID", "Name", "Type", "Source", "Destination", "S-PORT", "D-PORT", "Proto", "Action", VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-18s %-18s %-5s %-5s %-16s %-16s %s",
							"---", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----","----------------", "----------------",
							VTY_NEWLINE);
				}
				else if(pstNode && (pstNode->class == FIREWALL_C_SNAT))
				{
					vty_out(vty, " SNAT Table:%s",VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
					//iptables –t nat –A POSTROUTING –s 192.168.10.10 –o eth1 –j SNAT --to-source 111.196.221.212
					//iptables -t nat -A POSTROUTING -s 192.168.10.0/24 -j MASQUERADE
					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"ID", "Name", "Type", "Action", "Proto",
							"Source", "Destination", "S-PORT", "D-PORT",
							"Source Mac", "Dest Mac", "Source Inface", "Dest Inface", VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
				}
				else if(pstNode && (pstNode->class == FIREWALL_C_DNAT))
				{
					vty_out(vty, " DNAT Table:%s",VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
					//iptables –t nat –A PREROUTING –i eth1 –d 61.240.149.149 –p tcp –dport 80 –j DNAT --to-destination 192.168.10.6:8
					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"ID", "Name", "Type", "Action", "Proto",
							"Source", "Destination", "S-PORT", "D-PORT",
							"Source Mac", "Dest Mac", "Source Inface", "Dest Inface", VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
				}
				else if(pstNode && (pstNode->class == FIREWALL_C_MANGLE))
				{
					vty_out(vty, " Mangle Table:%s",VTY_NEWLINE);
					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"ID", "Name", "Type", "Action", "Proto",
							"Source", "Destination", "S-PORT", "D-PORT",
							"Source Mac", "Dest Mac", "Source Inface", "Dest Inface", VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
				}
				else if(pstNode && (pstNode->class == FIREWALL_C_RAW))
				{
					vty_out(vty, " Raw Table:%s",VTY_NEWLINE);
					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"ID", "Name", "Type", "Action", "Proto",
							"Source", "Destination", "S-PORT", "D-PORT",
							"Source Mac", "Dest Mac", "Source Inface", "Dest Inface", VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
				}
				else if(pstNode && (pstNode->class == FIREWALL_C_PORT))
				{
					vty_out(vty, " Port-Map Table:%s",VTY_NEWLINE);
/*					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
					//iptables -t nat -A PREROUTING -d 192.168.10.88 -p tcp --dport 80 -j
					// DNAT --to-destination 192.168.10.88:8080
					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"ID", "Name", "Type", "Action", "Proto",
							"Source", "Destination", "S-PORT", "D-PORT",
							"Source Mac", "Dest Mac", "Source Inface", "Dest Inface", VTY_NEWLINE);
					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);*/

					vty_out(vty, " %-3s %-16s %-16s %-18s %-18s %-5s %-5s %-16s %-16s %s",
							"---", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----","----------------", "----------------",
							VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-18s %-18s %-5s %-5s %-16s %-16s %s",
							"ID", "Name", "Type", "Source", "Destination", "S-PORT", "D-PORT", "Proto", "Action", VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-18s %-18s %-5s %-5s %-16s %-16s %s",
							"---", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----","----------------", "----------------",
							VTY_NEWLINE);
				}
				else
				{
					vty_out(vty, " All Table:%s",VTY_NEWLINE);
					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"ID", "Name", "Type", "Action", "Proto",
							"Source", "Destination", "S-PORT", "D-PORT",
							"Source Mac", "Dest Mac", "Source Inface", "Dest Inface", VTY_NEWLINE);

					vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
							"---", "----------------", "----------------", "----------------", "----------------",
							"------------------", "------------------", "-----", "-----",
							"------------------", "------------------", "------------------", "------------------",
							VTY_NEWLINE);
				}
			}
			else
				head++;

			if(pstNode && (pstNode->class == FIREWALL_C_FILTER))
			{
				//iptables -A OUTPUT -p tcp --sport 22 -j ACCEPT
				vty_out(vty, " %-3s %-16s %-16s %-18s %-18s %-5s %-5s %-16s %-16s %s",
						id, pstNode->name, type, source, destination,
						s_port, d_port, proto, action, VTY_NEWLINE);
			}
			else if(pstNode && (pstNode->class == FIREWALL_C_SNAT))
			{
				//iptables –t nat –A POSTROUTING –s 192.168.10.10 –o eth1 –j SNAT --to-source 111.196.221.212
				//iptables -t nat -A POSTROUTING -s 192.168.10.0/24 -j MASQUERADE
				vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
						id, pstNode->name, type, action, proto, source, destination,
						s_port, d_port, s_mac, d_mac, s_ifindex, d_ifindex, VTY_NEWLINE);
			}
			else if(pstNode && (pstNode->class == FIREWALL_C_DNAT))
			{
				//iptables –t nat –A PREROUTING –i eth1 –d 61.240.149.149 –p tcp –dport 80 –j DNAT --to-destination 192.168.10.6:8
				vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
						id, pstNode->name, type, action, proto, source, destination,
						s_port, d_port, s_mac, d_mac, s_ifindex, d_ifindex, VTY_NEWLINE);
			}
			else if(pstNode && (pstNode->class == FIREWALL_C_MANGLE))
			{
				vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
						id, pstNode->name, type, action, proto, source, destination,
						s_port, d_port, s_mac, d_mac, s_ifindex, d_ifindex, VTY_NEWLINE);
			}
			else if(pstNode && (pstNode->class == FIREWALL_C_RAW))
			{
				vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
						id, pstNode->name, type, action, proto, source, destination,
						s_port, d_port, s_mac, d_mac, s_ifindex, d_ifindex, VTY_NEWLINE);
			}
			else if(pstNode && (pstNode->class == FIREWALL_C_PORT))
			{
				//iptables -t nat -A PREROUTING -d 192.168.10.88 -p tcp --dport 80 -j
				// DNAT --to-destination 192.168.10.88:8080
/*				vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
						id, pstNode->name, type, action, proto, source, destination,
						s_port, d_port, s_mac, d_mac, s_ifindex, d_ifindex, VTY_NEWLINE);*/

				vty_out(vty, " %-3s %-16s %-16s %-18s %-18s %-5s %-5s %-16s %-16s %s",
						id, pstNode->name, type, source, destination,
						s_port, d_port, proto, action, VTY_NEWLINE);
			}
			else
			{
				vty_out(vty, " %-3s %-16s %-16s %-16s %-6s %-18s %-18s %-5s %-5s %-18s %-18s %-18s %-18s%s",
						id, pstNode->name, type, action, proto, source, destination,
						s_port, d_port, s_mac, d_mac, s_ifindex, d_ifindex, VTY_NEWLINE);
			}
		}
	}
	vty_out(vty, "%s",VTY_NEWLINE);
	return OK;
}
#endif

int nsm_firewall_rule_foreach_api(firewall_zone_t *zone, int(*cb)(firewall_t *, void *), void *p)
{
	firewall_t *pstNode = NULL;
	NODE index;
	if(!zone || !zone->zone_list)
		return ERROR;

	for(pstNode = (firewall_t *)lstFirst(zone->zone_list);
			pstNode != NULL;  pstNode = (firewall_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && cb)
		{
			(cb)(pstNode, p);
		}
	}
	return OK;
}
/***********************************************************************/
static int nsm_firewall_rule_cleanup(firewall_zone_t *zone, firewall_t *value)
{
	firewall_t *pstNode = NULL;
	NODE index;
	if(!zone || !zone->zone_list)
		return ERROR;
	if(zone->mutex)
		os_mutex_lock(zone->mutex, OS_WAIT_FOREVER);
	for(pstNode = (firewall_t *)lstFirst(zone->zone_list);
			pstNode != NULL;  pstNode = (firewall_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(value && pstNode && value->s_ifindex && pstNode->s_ifindex == value->s_ifindex)
		{
			gFirewalld.rule_id[pstNode->type] -= 1;
			nsm_halpal_firewall_rule_del_api(pstNode);
			lstDelete(zone->zone_list, (NODE*)pstNode);
			XFREE(MTYPE_FIREWALL_RULE, pstNode);
		}
		else if(value &&  pstNode && value->d_ifindex && pstNode->d_ifindex == value->d_ifindex)
		{
			gFirewalld.rule_id[pstNode->type] -= 1;
			nsm_halpal_firewall_rule_del_api(pstNode);
			lstDelete(zone->zone_list, (NODE*)pstNode);
			XFREE(MTYPE_FIREWALL_RULE, pstNode);
		}

		else if(value && pstNode && value->proto && pstNode->proto == value->proto)
		{
			gFirewalld.rule_id[pstNode->type] -= 1;
			nsm_halpal_firewall_rule_del_api(pstNode);
			lstDelete(zone->zone_list, (NODE*)pstNode);
			XFREE(MTYPE_FIREWALL_RULE, pstNode);
		}
		else if(pstNode)
		{
			gFirewalld.rule_id[pstNode->type] -= 1;
			nsm_halpal_firewall_rule_del_api(pstNode);
			lstDelete(zone->zone_list, (NODE*)pstNode);
			XFREE(MTYPE_FIREWALL_RULE, pstNode);
		}
	}

	if(zone->mutex)
		os_mutex_unlock(zone->mutex);
	return OK;
}
/***********************************************************************/
firewall_zone_t * nsm_firewall_zone_add(zpl_int8 	*zonename)
{
	if(!gFirewalld.init)
		return NULL;
	firewall_zone_t *node = XMALLOC(MTYPE_FIREWALL_ZONE, sizeof(firewall_zone_t));
	if(node)
	{
		os_memset(node, 0, sizeof(firewall_zone_t));
		os_strcpy(node->zonename, zonename);
		node->zone_list = malloc(sizeof(LIST));
		node->mutex = os_mutex_init();
		lstInit(node->zone_list);
		lstAdd(gFirewalld.firewall_list, (NODE *)node);
		return node;
	}
	return NULL;
}

firewall_zone_t * nsm_firewall_zone_lookup(zpl_int8 	*zonename)
{
	firewall_zone_t *pstNode = NULL;
	NODE index;
	if(!gFirewalld.init)
		return NULL;
	if(gFirewalld.mutex)
		os_mutex_lock(gFirewalld.mutex, OS_WAIT_FOREVER);
	for(pstNode = (firewall_zone_t *)lstFirst(gFirewalld.firewall_list);
			pstNode != NULL;  pstNode = (firewall_zone_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && strlen(pstNode->zonename) && strcmp(pstNode->zonename, zonename) == 0)
		{
			break;
		}
	}
	if(gFirewalld.mutex)
		os_mutex_unlock(gFirewalld.mutex);
	return pstNode;
}

int nsm_firewall_zone_del(zpl_int8 	*zonename)
{
	if(!gFirewalld.init)
		return ERROR;
	firewall_zone_t *pstNode = nsm_firewall_zone_lookup(zonename);
	if(pstNode)
	{
		if(gFirewalld.mutex)
			os_mutex_lock(gFirewalld.mutex, OS_WAIT_FOREVER);
		lstDelete(gFirewalld.firewall_list, (NODE*)pstNode);

		nsm_firewall_rule_cleanup(pstNode, NULL);

		if(lstCount(pstNode->zone_list))
		{
			lstFree(pstNode->zone_list);
			free(pstNode->zone_list);
			pstNode->zone_list = NULL;
		}
		if(pstNode->mutex)
			os_mutex_exit(pstNode->mutex);

		XFREE(MTYPE_FIREWALL_ZONE, pstNode);
		if(gFirewalld.mutex)
			os_mutex_unlock(gFirewalld.mutex);
		return OK;
	}
	return ERROR;
}

static int nsm_firewall_zone_cleanup(zpl_int8 	*zonename)
{
	firewall_zone_t *pstNode = NULL;
	NODE index;
	if(zonename)
	{
		return nsm_firewall_zone_del(zonename);
	}
	if(gFirewalld.mutex)
		os_mutex_lock(gFirewalld.mutex, OS_WAIT_FOREVER);
	for(pstNode = (firewall_zone_t *)lstFirst(gFirewalld.firewall_list);
			pstNode != NULL;  pstNode = (firewall_zone_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(gFirewalld.firewall_list, (NODE*)pstNode);
			nsm_firewall_rule_cleanup(pstNode, NULL);
			XFREE(MTYPE_FIREWALL_ZONE, pstNode);
		}
	}
	if(gFirewalld.mutex)
		os_mutex_unlock(gFirewalld.mutex);
	return OK;
}

int nsm_firewall_zone_foreach_api(int(*cb)(firewall_zone_t *, void *), void *p)
{
	firewall_zone_t *pstNode = NULL;
	NODE index;
	if(!gFirewalld.init)
		return ERROR;
	if(gFirewalld.mutex)
		os_mutex_lock(gFirewalld.mutex, OS_WAIT_FOREVER);
	for(pstNode = (firewall_zone_t *)lstFirst(gFirewalld.firewall_list);
			pstNode != NULL;  pstNode = (firewall_zone_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && cb)
		{
			(cb)(pstNode, p);
		}
	}
	if(gFirewalld.mutex)
		os_mutex_unlock(gFirewalld.mutex);
	return OK;
}
/***********************************************************************/
/***********************************************************************/
static int nsm_firewall_tcpudp_dport_default(void)
{
	zpl_uint32 i = 0, j = 0;
	//开放的TCP/UDP目的端口
	/*for(i = 0; i < sizeof(firewall_service_default)/sizeof(firewall_service_default[0]); i++)
	{
		if(firewall_service_default[i].all && j < FIREWALL_DEFAULT_PORT_MAX)
		{
			gFirewalld.all_port.port[j++] = firewall_service_default[i].port;
		}
	}
	*/
	j = 0;
	//开放的TCP目的端口
	for(i = 0; i < sizeof(firewall_service_default)/sizeof(firewall_service_default[0]); i++)
	{
		if(firewall_service_default[i].tcp == 1 && j < FIREWALL_DEFAULT_PORT_MAX)
		{
			gFirewalld.tcp_port.port[j++] = firewall_service_default[i].port;
		}
	}	
	j = 0;
	//开放的UDP目的端口
	for(i = 0; i < sizeof(firewall_service_default)/sizeof(firewall_service_default[0]); i++)
	{
		if(firewall_service_default[i].udp == 1 && j < FIREWALL_DEFAULT_PORT_MAX)
		{
			gFirewalld.udp_port.port[j++] = firewall_service_default[i].port;
		}
	}
	return OK;
}
/***********************************************************************/
static int nsm_firewall_action_dport_default(void)
{
	//iptables -t filter -A INPUT -p tcp -m multiport --dport 21,22,23,25,53,67,68,69,80,110,161,443,513,2610,8080 -j ACCEPT
	zpl_uint32 i = 0, action = 0;
	zpl_char cmd[512];
	zpl_char tmp[32];
	/*
	memset(cmd, 0, sizeof(cmd));
	strcpy(cmd, "iptables -t filter -A INPUT  -p tcp -m multiport --dport ");
	for(i = 0; i < FIREWALL_DEFAULT_PORT_MAX; i++)
	{
		if(gFirewalld.all_port.port[i])
		{
			if(action != 0)
				strcat(cmd, ",");
			memset(tmp, 0, sizeof(tmp));
			//snprintf(cmd + strlen(cmd), sizeof(cmd)- strlen(cmd) - 1, "%d", gFirewalld.all_port.port[i]);
			//strcat(cmd, itoa(gFirewalld.all_port.port[i], 10));
			sprintf(tmp, "%d", gFirewalld.all_port.port[i]);
			strcat(cmd, tmp);
			action++;
		}
	}	
	strcat(cmd, " -j ACCEPT");
	if(action)
		super_system(cmd);
	*/
	action = 0;
	memset(cmd, 0, sizeof(cmd));
	strcpy(cmd, "iptables -t filter -A INPUT -p tcp -m multiport --dport ");
	for(i = 0; i < FIREWALL_DEFAULT_PORT_MAX; i++)
	{
		if(gFirewalld.tcp_port.port[i] > 0)
		{
			if(action != 0)
				strcat(cmd, ",");
				
			memset(tmp, 0, sizeof(tmp));
			//snprintf(cmd + strlen(cmd), sizeof(cmd)- strlen(cmd) - 1, "%d", gFirewalld.tcp_port.port[i]);
			//strcat(cmd, itoa(gFirewalld.tcp_port.port[i], 10));
			sprintf(tmp, "%d", gFirewalld.tcp_port.port[i]);
			strcat(cmd, tmp);	
			action++;
		}
	}	
	strcat(cmd, " -j ACCEPT");
	printf("---%s-tcp--:%s\r\n", __func__, cmd);
	if(action)
		super_system(cmd);
	
	action = 0;
	memset(cmd, 0, sizeof(cmd));
	strcpy(cmd, "iptables -t filter -A INPUT -p udp -m multiport --dport ");
	for(i = 0; i < FIREWALL_DEFAULT_PORT_MAX; i++)
	{
		if(gFirewalld.udp_port.port[i] > 0)
		{
			if(action != 0)
				strcat(cmd, ",");

			memset(tmp, 0, sizeof(tmp));
			//snprintf(cmd + strlen(cmd), sizeof(cmd)- strlen(cmd) - 1, "%d", gFirewalld.udp_port.port[i]);
			//strcat(cmd, itoa(gFirewalld.udp_port.port[i], 10));
			sprintf(tmp, "%d", gFirewalld.udp_port.port[i]);
			strcat(cmd, tmp);
			action++;
		}
	}	
	strcat(cmd, " -j ACCEPT");
	printf("---%s-udp--:%s\r\n", __func__, cmd);
	if(action)
		super_system(cmd);

	return OK;
}
/***********************************************************************/
static int nsm_firewall_default(void)
{
	firewall_zone_t *zone = NULL;
	/*
	iptables -t filter -N wan_input	//创建一条新的链
	iptables -t filter -A INPUT -j wan_input	//把INPUT链过滤的转到 wan_input 链
	*/
#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
	return OK;
#endif

	return OK;
	//清空链
	super_system("iptables -t filter -F INPUT");
	super_system("iptables -t filter -F OUTPUT");
	super_system("iptables -t filter -F FORWARD");

	super_system("iptables -t nat -F");//##清空nat表所有规则
	super_system("iptables -t nat -X");//##删除nat表中所有自定义的空链
	//super_system("iptables -t nat -F");

	//设定预设规则
	super_system("iptables -t filter -P INPUT ACCEPT");
	super_system("iptables -t filter -P OUTPUT ACCEPT");
	super_system("iptables -t filter -P FORWARD DROP");

	return OK;
	//super_system("iptables -t nat -P INPUT DROP");
	//super_system("iptables -t nat -P OUTPUT ACCEPT");
	//super_system("iptables -t nat -P FORWARD DROP");

/*	[root@m01 ~]# iptables -F                             <- 清空iptables所有规则信息（清除filter）
	[root@m01 ~]# iptables -X                             <- 清空iptables自定义链配置（清除filter）
	[root@m01 ~]# iptables -Z                              <- 清空iptables计数器信息（清除filter）
	*/
	gFirewalld.rule_id[FIREWALL_FILTER_INPUT] = 3;
	super_system("iptables -t filter -I INPUT 1 -p icmp -j ACCEPT");
	nsm_firewall_tcpudp_dport_default();
	nsm_firewall_action_dport_default();

	zone = nsm_firewall_zone_lookup("zones");
	if(!zone)
		zone = nsm_firewall_zone_add("zones");
	if(zone)
	{
		firewall_t value;
		memset(&value, 0, sizeof(value));
		strcpy(value.name, "cli");
		value.class = FIREWALL_C_FILTER;
		value.type = FIREWALL_FILTER_INPUT;
		value.action = FIREWALL_A_ACCEPT;
		value.proto = FIREWALL_P_TCP;
		value.d_port = 2610;
		nsm_firewall_rule_add_api(zone, &value);

		memset(&value, 0, sizeof(value));
		strcpy(value.name, "app");
		value.class = FIREWALL_C_FILTER;
		value.type = FIREWALL_FILTER_INPUT;
		value.action = FIREWALL_A_ACCEPT;
		value.proto = FIREWALL_P_UDP;
		value.d_port = 9527;
		nsm_firewall_rule_add_api(zone, &value);
	}

	return OK;
}
/***********************************************************************/
int nsm_firewall_init(void)
{
	memset(&gFirewalld, 0, sizeof(Gfirewall_t));
	gFirewalld.firewall_list = malloc(sizeof(LIST));
	gFirewalld.mutex = os_mutex_init();
	lstInit(gFirewalld.firewall_list);
	gFirewalld.init = zpl_true;
	nsm_firewall_default();
	return OK;
}

int nsm_firewall_exit(void)
{
	if(lstCount(gFirewalld.firewall_list))
	{
		nsm_firewall_zone_cleanup(NULL);
		lstFree(gFirewalld.firewall_list);
		free(gFirewalld.firewall_list);
		gFirewalld.firewall_list = NULL;
		gFirewalld.init = zpl_false;
	}
	if(gFirewalld.mutex)
		os_mutex_exit(gFirewalld.mutex);
	memset(&gFirewalld, 0, sizeof(Gfirewall_t));
	return OK;
}

/***********************************************************************/
/***********************************************************************/
static int nsm_halpal_firewall_rule_add_api(firewall_t *value)
{
	int ret = ERROR;

	if (value->class == FIREWALL_C_PORT)
	{
		ret = nsm_halpal_firewall_portmap_rule_set(value, 1);
	}
	else if (value->class == FIREWALL_C_FILTER)
	{
		ret = nsm_halpal_firewall_port_filter_rule_set(value, 1);
	}
	else if (value->class == FIREWALL_C_DNAT)
	{
		ret = nsm_halpal_firewall_dnat_rule_set(value, 1);
	}
	else if (value->class == FIREWALL_C_SNAT)
	{
		ret = nsm_halpal_firewall_snat_rule_set(value, 1);
	}
	else if (value->class == FIREWALL_C_MANGLE)
	{
		ret = nsm_halpal_firewall_mangle_rule_set(value, 1);
	}
	else if (value->class == FIREWALL_C_RAW)
	{
		ret = nsm_halpal_firewall_raw_rule_set(value, 1);
	}
	else
		ret = ERROR;
	return ret;
}


static int nsm_halpal_firewall_rule_del_api(firewall_t *value)
{
	int ret = ERROR;
	if (value->class == FIREWALL_C_PORT)
	{
		ret = nsm_halpal_firewall_portmap_rule_set(value, 0);
	}
	else if (value->class == FIREWALL_C_FILTER)
	{
		ret = nsm_halpal_firewall_port_filter_rule_set(value, 0);
	}
	else if (value->class == FIREWALL_C_DNAT)
	{
		ret = nsm_halpal_firewall_dnat_rule_set(value, 0);
	}
	else if (value->class == FIREWALL_C_SNAT)
	{
		ret = nsm_halpal_firewall_snat_rule_set(value, 0);
	}
	else if (value->class == FIREWALL_C_MANGLE)
	{
		ret = nsm_halpal_firewall_mangle_rule_set(value, 0);
	}
	else if (value->class == FIREWALL_C_RAW)
	{
		ret = nsm_halpal_firewall_raw_rule_set(value, 0);
	}
	else
		ret = ERROR;
	return ret;
}
/***********************************************************************/
