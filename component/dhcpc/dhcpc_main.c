/*
 * dhcpc_main.c
 *
 *  Created on: Sep 2, 2018
 *      Author: zhurish
 */

#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "vrf.h"
#include "command.h"
#include "interface.h"
#include "nsm_dns.h"

#include "arp.h"
#include "bind.h"
#include "dhcpc_config.h"
#include "dhcpc_common.h"
#include "configure.h"
#include "control.h"
#include "dhcpcd.h"
#include "dhcp.h"
#include "dhcpc_eloop.h"
#include "if-options.h"
#include "ipv4ll.h"
#include "ipv6rs.h"
#include "net.h"
#include "dhcpc_util.h"
#include "signals.h"
#include "dhcpc_api.h"


static int dhcpc_task_id = 0;

static unsigned char dhcpc_debug_pri = 7;//LOG_WARNING;

int _dhcp_syslog(int pri, const char *format, ...)
{
	//if(options & DHCPCD_DEBUG)
	{
		if(pri <= dhcpc_debug_pri)
		{
			va_list args;
			va_start(args, format);
			vzlog (NULL, ZLOG_NSM, pri, format, args);
			va_end(args);
		}
	}
	return 0;
}


int dhcpc_hostname(char *name, int len)
{
	os_strcpy(name, "SWPlatformV0.0.3");
	return 0;
}


int dhcpc_interface_add(struct dhcpc_interface *ifp)
{
	struct dhcpc_interface *next = NULL;
	for(next = dhcpc_ifaces_list; next != NULL; next = next->next)
	{
		if(next == ifp)
		{
			return 0;	/* this interface has already exist in this list. */
		}
	}
	ifp->next = dhcpc_ifaces_list;
	dhcpc_ifaces_list = ifp;
	return 0;
}

int dhcpc_interface_del(struct dhcpc_interface *ifp)
{
	struct dhcpc_interface *pTem = NULL;
	pTem	= NULL;
	if(dhcpc_ifaces_list == ifp)
	{
		dhcpc_ifaces_list = ifp->next;
	}
	else
	{
		for(pTem = dhcpc_ifaces_list; pTem != NULL; pTem = pTem->next)
		{
			if(pTem->next == ifp)
			{
				break;
			}
		}
		if(!pTem)
		{
			if(ifp->running == FALSE)
				free_interface(ifp);
			return 0;
		}
		pTem->next= ifp->next;
	}
	if(ifp->running == FALSE)
		free_interface(ifp);
	return 0;
}

struct dhcpc_interface * dhcpc_interface_lookup(const char *ifname, int ifindex)
{
	struct dhcpc_interface *ifp;
	if(!dhcpc_ifaces_list)
		return NULL;
	for (ifp = dhcpc_ifaces_list; ifp != NULL; ifp = ifp->next)
	{
		if (ifname && strcmp(ifp->name, ifname) == 0)
			return ifp;
		else if(ifindex && ifp->kifindex == ifindex)
			return ifp;
	}
	return NULL;
}


struct dhcpc_interface * init_interface(const char *ifname)
{
	struct dhcpc_interface *iface = NULL;
	struct interface *ifp = if_lookup_by_kernel_name (ifname);
	if(!ifp || !ifp->k_name_hash)
		return NULL;

	iface = xzalloc(sizeof(*iface));
	strlcpy(iface->name, ifname, sizeof(iface->name));
	iface->flags = ifp->flags;
	iface->kifindex = ifp->k_ifindex;//if_nametoindex(iface->name);
	iface->family = ARPHRD_ETHER;
	//iface->state->options = if_options_default();
	/* We reserve the 100 range for virtual interfaces, if and when
	 * we can work them out. */
	iface->metric = 200 + iface->kifindex;
	if (getifssid(ifname, iface->ssid) != -1)
	{
		iface->wireless = 1;
		iface->metric += 100;
	}
	snprintf(iface->leasefile, sizeof(iface->leasefile),
	    LEASEFILE, ifname);
	iface->running = FALSE;

	memcpy(iface->hwaddr, ifp->hw_addr, ifp->hw_addr_len);
	iface->hwlen = ifp->hw_addr_len;

	dhcp_syslog(LOG_ERR, "HW ADDR: %02x-%02x-%02x-%02x-%02x-%02x", iface->hwaddr[0],iface->hwaddr[1],
			iface->hwaddr[2], iface->hwaddr[3], iface->hwaddr[4], iface->hwaddr[5]);
	//if (dhcpc_if_init(ifp) == -1)
	/* 0 is a valid fd, so init to -1 */
	iface->raw_fd = -1;
	iface->udp_fd = -1;
	iface->arp_fd = -1;

	nsm_dhcp_interface_set_pravite(ifp, DHCP_CLIENT, iface);
	goto exit;

eexit:
	free(iface);
	iface = NULL;
exit:
	return iface;
}


int carrier_status(struct dhcpc_interface *iface)
{
	int ret = 1;
	struct interface *ifp = if_lookup_by_kernel_name (iface->name);
	if(ifp)
	{
		ret = (ifp->flags & IFF_RUNNING) ? 1 : 0;
	}
	ret = (iface->flags & IFF_RUNNING) ? 1 : 0;
	return ret;
}

int up_interface(struct dhcpc_interface *iface)
{
	int ret = 0;
	struct interface *ifp = if_lookup_by_kernel_name (iface->name);
	if(ifp)
	{
		ret = nsm_interface_up_set_api(ifp);
		iface->flags |= ifp->flags;
	}
	iface->flags |= IFF_UP;
	return 0;
}


int do_mtu(const char *ifname, short int mtu)
{
	struct interface *ifp = if_lookup_by_kernel_name (ifname);
	if(ifp)
	{
		int mtu = 0;
		if(nsm_interface_mtu_get_api(ifp, &mtu) == OK)
			return mtu;
	}
	return 1500;
}

int dhcpc_dns_resolv(int cmd, struct dhcpc_interface * iface)
{
	if(cmd)
	{
		if(iface)
		{
			ip_dns_opt_t opt;
			struct prefix address;
			struct if_state *state = iface->state;
			struct if_options *ifo = NULL;
			if(!state)
			{
				return ERROR;
			}
			ifo = state->options;
			if(!ifo)
			{
				return ERROR;
			}

			opt.ifindex = ifkernel2ifindex(iface->kifindex);
			opt.vrfid = 0;
			opt.metric = 0;
			opt.secondly = FALSE;
			opt.active = TRUE;

			if(state->lease.dns1.s_addr)
			{
				prefix_zero(&address);
				address.family = AF_INET;
				address.prefixlen = IPV4_MAX_PREFIXLEN;
				address.u.prefix4 = state->lease.dns1;
				nsm_ip_dns_add(&address, &opt, FALSE, IP_DNS_DYNAMIC);
			}
			if(state->lease.dns2.s_addr)
			{
				prefix_zero(&address);
				address.family = AF_INET;
				address.prefixlen = IPV4_MAX_PREFIXLEN;
				address.u.prefix4 = state->lease.dns2;
				opt.secondly = TRUE;
				nsm_ip_dns_add(&address, &opt, TRUE, IP_DNS_DYNAMIC);
			}
			if(strlen(state->lease.domain))
				nsm_dns_domain_name_add_api(state->lease.domain, FALSE);
			return OK;
		}
	}
	else
	{
		if(iface)
		{
			struct if_state *state = iface->state;
			struct if_options *ifo = NULL;
			if(!state)
			{
				return ERROR;
			}
			ifo = state->options;
			if(!ifo)
			{
				return ERROR;
			}
			nsm_dns_domain_name_del_api(FALSE);
			nsm_ip_dns_del_by_ifindex(ifkernel2ifindex(iface->kifindex), IP_DNS_DYNAMIC);
			return OK;
		}
	}
	return ERROR;
#if 0
	/*
	[zhurish@localhost SWPlatform]$ cat /etc/resolv.conf
	# Generated by NetworkManager
	search www.tendawifi.com lan
	nameserver 192.168.0.1
	nameserver fd04:f2ca:c45b::1
	[zhurish@localhost SWPlatform]$
	*/
	if(iface)
	{
		struct if_state *state = iface->state;
		struct if_options *ifo = NULL;
		if(!state)
		{
			return ERROR;
		}
		ifo = state->options;
		if(!ifo)
		{
			return ERROR;
		}
		if(state->lease.dns1.s_addr)
		{
			FILE *fp = NULL;
			char path[128];
			memset(path, 0, sizeof(path));
			snprintf(path, sizeof(path), "%s/resolv-%s.conf", DAEMON_ENV_DIR, iface->name);

			fp = fopen(path, "w+");
			if(fp)
			{
				fflush(fp);
				fprintf(fp, "# Generated by DHCPC\n");
				if(strlen(state->lease.domain))
					fprintf(fp, "search %s lan\n", state->lease.domain);
				else
					fprintf(fp, "search lan\n");

				fprintf(fp, "nameserver %s\n",inet_ntoa(state->lease.dns1));
				if(state->lease.dns2.s_addr)
					fprintf(fp, "nameserver %s\n",inet_ntoa(state->lease.dns2));
				fflush(fp);
				fclose(fp);

				unlink("/etc/resolv.conf");
				link(path, "/etc/resolv.conf");
				return OK;
			}
		}
	}
	return ERROR;
#endif
}


static int add_options_api(struct if_options *ifo, BOOL set, enum DHO option, char *argv)
{
	char *p = NULL, *np = NULL;
	int s = 0, i = 0;
	struct in_addr addr;
	switch (option)
	{
	case DHO_PAD:
		break;
	case DHO_SUBNETMASK:
	case DHO_ROUTER:
	case DHO_DNSSERVER:
		if (set)
		{
			add_option_mask(ifo->requestmask, option);
			if(option == DHO_ROUTER)
				ifo->options |= DHCPCD_GATEWAY;
		}
		else
		{
			if(option == DHO_ROUTER)
				ifo->options &= ~(DHCPCD_GATEWAY);
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_HOSTNAME:
		if (set && argv)
		{
			s = dhcpc_parse_string(ifo->hostname,
					HOSTNAME_MAX_LEN, argv);
			if (s == -1)
			{
				dhcp_syslog(LOG_ERR, "hostname: %m");
				return -1;
			}
			if (s != 0 && ifo->hostname[0] == '.')
			{
				dhcp_syslog(LOG_ERR,
				    "hostname cannot begin with .");
				return -1;
			}
			ifo->hostname[s] = '\0';
			ifo->options |= DHCPCD_HOSTNAME;
		}
		if(!set)
		{
			os_memset(ifo->hostname, 0, sizeof(ifo->hostname));
			ifo->options &= ~DHCPCD_HOSTNAME;
		}
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_DNSDOMAIN:
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_MTU:
		break;
	case DHO_BROADCAST:
		break;
	case DHO_STATICROUTE:
	case DHO_NISDOMAIN:
	case DHO_NISSERVER:
	case DHO_NTPSERVER:
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_VENDOR:
		p = strchr(argv, ',');
		if (!p || !p[1]) {
			dhcp_syslog(LOG_ERR, "invalid vendor format");
			return -1;
		}

		/* If vendor starts with , then it is not encapsulated */
		if (p == argv) {
			argv++;
			s = dhcpc_parse_string((char *)ifo->vendor + 1, VENDOR_MAX_LEN, argv);
			if (s == -1) {
				dhcp_syslog(LOG_ERR, "vendor: %m");
				return -1;
			}
			ifo->vendor[0] = (uint8_t)s;
			ifo->options |= DHCPCD_VENDORRAW;
			if (set)
				add_option_mask(ifo->requestmask, option);
			else
			{
				del_option_mask(ifo->requestmask, option);
				del_option_mask(ifo->requiremask, option);
				add_option_mask(ifo->nomask, option);
			}
			break;
		}

		/* Encapsulated vendor options */
		if (ifo->options & DHCPCD_VENDORRAW) {
			ifo->options &= ~DHCPCD_VENDORRAW;
			ifo->vendor[0] = 0;
		}

		*p = '\0';
		i = atoint(argv);
		argv = p + 1;
		if (i < 1 || i > 254) {
			dhcp_syslog(LOG_ERR, "vendor option should be between"
			    " 1 and 254 inclusive");
			return -1;
		}
		s = VENDOR_MAX_LEN - ifo->vendor[0] - 2;
		if (inet_aton(argv, &addr) == 1) {
			if (s < 6) {
				s = -1;
				errno = ENOBUFS;
			} else
				memcpy(ifo->vendor + ifo->vendor[0] + 3,
				    &addr.s_addr, sizeof(addr.s_addr));
		} else {
			s = dhcpc_parse_string((char *)ifo->vendor +
			    ifo->vendor[0] + 3, s, argv);
		}
		if (s == -1) {
			dhcp_syslog(LOG_ERR, "vendor: %m");
			return -1;
		}
		if (s != 0) {
			ifo->vendor[ifo->vendor[0] + 1] = i;
			ifo->vendor[ifo->vendor[0] + 2] = s;
			ifo->vendor[0] += s + 2;
		}
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_IPADDRESS:
		if (argv && set)
		{
			if (parse_addr(&ifo->req_addr, NULL, argv) != 0)
				return -1;
			ifo->options |= DHCPCD_REQUEST;
			ifo->req_mask.s_addr = 0;
		}
		else
		{
			ifo->options |= DHCPCD_REQUEST;
			ifo->req_addr.s_addr = 0;
			ifo->req_mask.s_addr = 0;
		}
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_LEASETIME:
		if (argv && set)
		{
			ifo->leasetime = (uint32_t)strtol(argv, NULL, 0);
			ifo->options |= DHCPCD_DUMPLEASE;
		}
		else
		{
			ifo->leasetime = 0;
			ifo->options &= ~DHCPCD_DUMPLEASE;
		}
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_OPTIONSOVERLOADED:
		break;
	case DHO_MESSAGETYPE:
		break;
	case DHO_SERVERID:
		break;
	case DHO_PARAMETERREQUESTLIST:
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_MESSAGE:
		break;
	case DHO_MAXMESSAGESIZE:
		break;
	case DHO_RENEWALTIME:
		break;
	case DHO_REBINDTIME:
		break;
	case DHO_VENDORCLASSID:
		if (argv && set)
		{
			s = dhcpc_parse_string((char *)ifo->vendorclassid + 1,
					VENDORCLASSID_MAX_LEN, argv);
			ifo->vendorclassid[0] = (uint8_t)s;
		}
		else
		{
			os_memset(ifo->vendorclassid, 0, sizeof(ifo->vendorclassid));
		}
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_CLIENTID:
		/* Strings have a type of 0 */;
		ifo->clientid[1] = 0;
		if (set && argv)
		{
			s = parse_string_hwaddr((char *)ifo->clientid + 1,
			    CLIENTID_MAX_LEN, argv, 1);
			if (s == -1) {
				dhcp_syslog(LOG_ERR, "clientid: %m");
				return -1;
			}
			ifo->options |= DHCPCD_CLIENTID;
			ifo->clientid[0] = (uint8_t)s;
		}
		else
		{
			s = 0;
			ifo->options &= ~DHCPCD_CLIENTID;
			ifo->clientid[0] = (uint8_t)s;
		}
		if(set)
			ifo->options |= DHCPCD_CLIENTID;// | DHCPCD_DUID;
		else
			ifo->options &= ~(DHCPCD_CLIENTID);// | DHCPCD_DUID);
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_USERCLASS:/* RFC 3004 */
		if(set)
		{
			s = 0;//USERCLASS_MAX_LEN - ifo->userclass[0] - 1;
			s = dhcpc_parse_string(ifo->userclass[0] + 1, s, argv);
			if (s == -1) {
				dhcp_syslog(LOG_ERR, "userclass: %m");
				return -1;
			}
			if (s != 0) {
				//ifo->userclass[ifo->userclass[0] + 1] = s;
				ifo->userclass[0] = (uint8_t)s;
			}
		}
		else
		{
			os_memset(ifo->userclass, 0, sizeof(ifo->userclass));
		}
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_FQDN:
		if (!set)
		{
			ifo->fqdn = FQDN_BOTH;
			break;
		}
		else
		{
			if (strcmp(argv, "none") == 0)
				ifo->fqdn = FQDN_NONE;
			else if (strcmp(argv, "ptr") == 0)
				ifo->fqdn = FQDN_PTR;
			else if (strcmp(argv, "both") == 0)
				ifo->fqdn = FQDN_BOTH;
			else if (strcmp(argv, "disable") == 0)
				ifo->fqdn = FQDN_DISABLE;
			else {
				dhcp_syslog(LOG_ERR, "invalid value `%s' for FQDN", argv);
				return -1;
			}
		}
		if (set)
			add_option_mask(ifo->requestmask, option);
		else
		{
			del_option_mask(ifo->requestmask, option);
			del_option_mask(ifo->requiremask, option);
			add_option_mask(ifo->nomask, option);
		}
		break;
	case DHO_DNSSEARCH:/* RFC 3397 */
		break;
	case DHO_CSR: /* RFC 3442 */
		break;
	case DHO_SIXRD: /* RFC 5969 */
		break;
	case DHO_MSCSR: /* MS code for RFC 3442 */
		break;
	case DHO_END:
		break;
	default:
		return ERROR;
	}
	return OK;
}

static int add_options_other_api(struct if_options *ifo, BOOL set, int option, char *argv)
{
	switch((option - DHO_END))
	{
	case 'a':
		if(set)
		{
			ifo->options &= ~DHCPCD_ARP;
			/* IPv4LL requires ARP */
			ifo->options &= ~DHCPCD_IPV4LL;
		}
		else
		{
			ifo->options |= DHCPCD_ARP;
			/* IPv4LL requires ARP */
			ifo->options |= DHCPCD_IPV4LL;
		}
		break;
	case 'm':
		if(set)
			ifo->metric = atoi(argv);
		else
			ifo->metric = 1000;
		break;

	case 'd':
		if(set)
			ifo->options |= DHCPCD_CLIENTID | DHCPCD_DUID;
		else
			ifo->options &= ~(DHCPCD_CLIENTID | DHCPCD_DUID);
		break;
	case 'G':
		if(set)
			ifo->options |= DHCPCD_GATEWAY;
		else
			ifo->options &= ~(DHCPCD_GATEWAY);
		break;
	case 'D':
		if(set)
			ifo->options |= DHCPCD_XID_HWADDR;
		else
			ifo->options &= ~(DHCPCD_XID_HWADDR);
		break;
	case 'B':
		if(set)
			ifo->options |= DHCPCD_BROADCAST;
		else
			ifo->options &= ~(DHCPCD_BROADCAST);
		break;
	case 'K':
		if(set)
			ifo->options |= DHCPCD_LINK;
		else
			ifo->options &= ~(DHCPCD_LINK);
		break;
	case 'L':
		if(set)
			ifo->options |= DHCPCD_IPV4LL;
		else
			ifo->options &= ~(DHCPCD_IPV4LL);
		break;
	}
	return OK;
}

int control_handle_args(struct fd_list *fd, char *buf)
{
	struct dhcpc_interface *dhcpifp = NULL;
	struct interface *ifp = NULL;
	dhcpc_ctrl_head *head = (dhcpc_ctrl_head *) buf;
	ifp = if_lookup_by_kernel_index(head->kifindex);
	if(!ifp || !ifp->k_name_hash)
		return -1;
	DHCP_DEBUG("%s \r\n", __func__);
/*	if ((ifi->ifi_flags & IFF_MASTER) && !(ifi->ifi_flags & IFF_LOWER_UP)) {
		handle_interface(-1, ifn);
		return 1;
	}

	handle_carrier(ifi->ifi_flags & IFF_RUNNING ? 1 : -1,
	    ifi->ifi_flags, ifn);*/
	switch (head->action)
	{
	case DHCPC_CLIENT_ADD_IF:
		dhcpifp = dhcpc_interface_lookup(NULL, head->kifindex);
		if(!dhcpifp)
		{
			dhcpc_socket();
			dhcpifp = init_interface(ifp->k_name);
			if(dhcpifp)
			{
				dhcpc_interface_add(dhcpifp);
				init_state(dhcpifp);
			//	dhcpifp->state->options = if_options_default();
				add_options_api(dhcpifp->state->options, TRUE, DHO_CLIENTID, "interface eth0/0/1");
			//	add_options_api(dhcpifp->state->options, TRUE, DHO_USERCLASS, "interface giteth0/0/1");
				add_options_api(dhcpifp->state->options, TRUE, DHO_VENDORCLASSID, "deth0/0/1");
				DHCP_DEBUG("%s add interface %s\r\n", __func__, ifp->k_name);
				if(dhcpifp->running == TRUE)
					start_interface(dhcpifp);
			}
		}
		break;
	case DHCPC_CLIENT_DEL_IF:
		dhcpifp = find_interface(ifp->k_name);
		//dhcpifp = dhcpc_interface_lookup(NULL, head->kifindex);
		if(dhcpifp)
		{
			if(dhcpifp->running == TRUE)
			{
				//send_release(dhcpifp);
				stop_interface(dhcpifp);
			}
			DHCP_DEBUG("%s stop interface %s\r\n", __func__, ifp->k_name);
			dhcpc_interface_del(dhcpifp);
		}
		break;
	case DHCPC_CLIENT_START_IF:
		dhcpifp = dhcpc_interface_lookup(NULL, head->kifindex);
		if(dhcpifp)
		{
			if(dhcpifp->running == FALSE)
			{
				dhcpifp->running = TRUE;
				start_interface(dhcpifp);
				DHCP_DEBUG("%s start interface %s\r\n", __func__, ifp->k_name);
			}
		}
		break;
	case DHCPC_CLIENT_STOP_IF:
		dhcpifp = dhcpc_interface_lookup(NULL, head->kifindex);
		if(dhcpifp)
		{
			if(dhcpifp->running == TRUE)
			{
				//send_release(dhcpifp);
				stop_interface(dhcpifp);
			}
		}
		break;
	case DHCPC_CLIENT_RESTART_IF:
		dhcpifp = dhcpc_interface_lookup(NULL, head->kifindex);
		if(dhcpifp)
		{
			if_reboot(dhcpifp);
/*			if(dhcpifp->running == TRUE)
			{
				send_release(dhcpifp);
				stop_interface(dhcpifp);
			}
			dhcpifp->running = TRUE;
			start_discover(dhcpifp);*/
			//start_renew(dhcpifp);
		}
		break;
	case DHCPC_CLIENT_FREE_IF:
		dhcpifp = dhcpc_interface_lookup(NULL, head->kifindex);
		if(dhcpifp)
		{
			if(dhcpifp->running == TRUE)
			{
				//send_release(dhcpifp);
				stop_interface(dhcpifp);
			}
			dhcpc_interface_del(dhcpifp);
		}
		break;

	case DHCPC_CLIENT_ADD_OPTION_IF:
		dhcpifp = dhcpc_interface_lookup(NULL, head->kifindex);
		if(dhcpifp)
		{
			if(dhcpifp && dhcpifp->state && dhcpifp->state->options)
			{
				if(head->option <= DHO_END)
					add_options_api(dhcpifp->state->options, TRUE, head->option, head->value);
				else
					add_options_other_api(dhcpifp->state->options, FALSE, head->option, head->value);
			}
			if_reboot(dhcpifp);
/*			if(dhcpifp->running == TRUE)
			{
				send_release(dhcpifp);
				stop_interface(dhcpifp);
			}
			dhcpifp->running = TRUE;
			start_discover(dhcpifp);*/
		}
		break;
	case DHCPC_CLIENT_DEL_OPTION_IF:
		dhcpifp = dhcpc_interface_lookup(NULL, head->kifindex);
		if(dhcpifp)
		{
			if(dhcpifp && dhcpifp->state && dhcpifp->state->options)
			{
				if(head->option <= DHO_END)
					add_options_api(dhcpifp->state->options, FALSE, head->option, head->value);
				else
					add_options_other_api(dhcpifp->state->options, FALSE, head->option, head->value);
			}
			if_reboot(dhcpifp);
/*			if(dhcpifp->running == TRUE)
			{
				send_release(dhcpifp);
				stop_interface(dhcpifp);
			}
			dhcpifp->running = TRUE;
			start_discover(dhcpifp);*/
		}
		break;
	case DHCPC_CLIENT_METRIC_IF:
/*		dhcpifp = find_interface(head->name);
		if(dhcpifp)
		{
			if(dhcpifp && dhcpifp->state && dhcpifp->state->options)
				dhcpc_interface_metric(dhcpifp, head->option ? TRUE:FALSE, head->option);
		}*/
		break;
	default:
		break;
	}
	return 0;
}

static int dhcpc_task(void *arg)
{
/*
 * dhcpcd -b -d -h ada
 */
	int argc;
	char **argv[] = {"-b", "-d", "-h", "router"};
	os_sleep(5);
	while(1)
	{
		dhcpc_main(argc, argv);
	}
	return 0;
}



int dhcpc_module_init ()
{
	/* Make master thread emulator. */
	//master_thread[MODULE_DHCP] = thread_master_module_create (MODULE_DHCP);
	//master_thread[MODULE_DHCP];
#ifdef OS_ELOOP_THREAD
	start_eloop_init();
#endif
	return 0;

}

int dhcpc_task_init ()
{
	if(dhcpc_task_id == 0)
		dhcpc_task_id = os_task_create("dhcpcTask", OS_TASK_DEFAULT_PRIORITY,
	               0, dhcpc_task, NULL, OS_TASK_DEFAULT_STACK);
	if(dhcpc_task_id)
		return OK;
	return ERROR;

}

int dhcpc_task_exit ()
{
	if(dhcpc_task_id)
		os_task_destroy(dhcpc_task_id);
	dhcpc_task_id = 0;
	return OK;
}

int dhcpc_module_exit ()
{
	struct dhcpc_interface *iface = NULL;
	start_eloop_exit();
	cleanup();
	while (dhcpc_ifaces_list) {
		iface = dhcpc_ifaces_list;
		if(iface)
		{
			stop_interface(iface);
			dhcpc_ifaces_list = iface->next;
			free_interface(iface);
		}
	}
	close_link_socket();
	return OK;
}




/*
 * ./dhcpcd -f /home/test/dhcpcd.conf br-lan   0x51d888
 *
# Inform the DHCP server of our hostname for DDNS.
hostname
# To share the DHCP lease across OSX and Windows a ClientID is needed.
# Enabling this may get a different lease than the kernel DHCP client.
# Some upstream DHCP servers may also require a ClientID, such as FRITZ!Box.
#clientid

# A list of options to request from the DHCP server.
option domain_name_servers, domain_name, domain_search, host_name
option classless_static_routes
# Most distributions have NTP support.
option ntp_servers
# Respect the network MTU.
option interface_mtu
# A ServerID is required by RFC2131.
require dhcp_server_identifier

# A hook script is provided to lookup the hostname if not set by the DHCP
# server, but it should not be run by default.
nohook lookup-hostname

 *
HCP Client 命令 ....................................................................................................................... 16
2.1 ip address dhcp ............................................................................................................................................. 16
2.2 management ip address dhcp ........................................................................................................................ 17
2.3 dhcp client request ........................................................................................................................................ 18
2.4 dhcp client client-id ...................................................................................................................................... 19
2.5 dhcp client class-id ....................................................................................................................................... 21
2.6 dhcp client lease ........................................................................................................................................... 22
2.7 dhcp client hostname .................................................................................................................................... 23
2.8 dhcp client default-router distance ............................................................................................................... 24
2.9 dhcp client broadcast-flag ............................................................................................................................. 25
2.10 debug dhcp client ........................................................................................................................................ 26
2.11 show dhcp client ......................................................................................................................................... 27
2.12 show dhcp client statistics .......................................................................................................................... 28
2.13 clear dhcp client statistics ........................................................................................................................... 29
DHCPC_CLIENT_ADD_IF,
DHCPC_CLIENT_DEL_IF,
DHCPC_CLIENT_START_IF,
DHCPC_CLIENT_STOP_IF,
DHCPC_CLIENT_RESTART_IF,
DHCPC_CLIENT_FREE_IF,
DHCPC_CLIENT_MAX,
*/
