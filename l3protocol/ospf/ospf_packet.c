/* ospf_packet.c - OSPF basic process on packet I/O*/

#include "ospf.h"
#include "ospf_nm.h"

/*form ospf header's checksum and authentication field*/
void ospf_auth_fill(struct ospf_hdr *p_packet, struct ospf_if *p_if)

{
	struct ospf_authinfo *p_auth = NULL;
	u_int keyid = p_if->md5id;
	u_int8 auth_type = p_if->authtype;
	u_short len = ntohs(p_packet->len);
	u_int8 md5len = OSPF_MAX_KEY_LEN;
	u_int8 digest[OSPF_MAX_KEY_LEN];
	u_int8 *end = NULL;
	u_int8 *p_key = p_if->key;

	/*decide authtype used*/
	if ((OSPF_AUTH_NONE == p_if->authtype) && (NULL != p_if->p_area))
	{
		auth_type = p_if->p_area->authtype;
		keyid = p_if->p_area->keyid;
		p_key = p_if->p_area->key;
	}

	p_packet->auth_type = auth_type;
	p_packet->checksum = 0;
	memset(p_packet->auth, 0x0, OSPF_KEY_LEN);

	switch (auth_type)
	{
	case OSPF_AUTH_NONE:
		p_packet->checksum = checksum((u_short *) p_packet, len);
		break;

	case OSPF_AUTH_SIMPLE:
		p_packet->checksum = checksum((u_short *) p_packet, len);
		memcpy(p_packet->auth, p_key, OSPF_KEY_LEN);
		break;

	case OSPF_AUTH_MD5:
		p_auth = (struct ospf_authinfo *) p_packet->auth;
		p_auth->len = md5len;
		p_auth->seqnum = htonl(ospf_generate_seq());
		end = ((u_int8 *) p_packet) + len;

		p_auth->keyid = keyid;
		memcpy(end, p_key, md5len);

		MD5((u_int8 *) p_packet, (len + md5len), digest);

		memcpy(end, digest, md5len);
		break;

	default:
		break;
	}
	return;
}

/* section 8.1 of OSPF specification (page 54-56) */
/*send an ospf packet,ospf header will be constructed
 here,and call raw socket to send packet*/
void ospf_output(void *p_msgbuf, struct ospf_if *p_if, u_int dest, u_short len) /*host order*/
{
	struct ospf_hdr *p_packet = (struct ospf_hdr *) p_msgbuf;
	struct ospf_process *p_process = p_if->p_process;
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next_nbr = NULL;

	ospf_logx(ospf_debug_msg, "ospf_output if uint=0x%x, if index=0x%x",
			p_if->ifnet_uint, p_if->ifnet_index);

	//if ((p_if->passive) || (OSPF_IFS_DOWN == p_if->state))
	if ((p_process->packet_block) || (p_if->passive)
			|| (OSPF_IFS_DOWN == p_if->state))
	{
		ospf_logx(ospf_debug_msg, "passive or down if,no send");
		p_packet->len = htons(len);
		return;
	}
	/*20130110 rate limit:delay a tick for dd/update/request packet during busy*/
#if 0
	if ((OSPF_PACKET_HELLO != p_packet->type)
			&& (OSPF_PACKET_ACK != p_packet->type)
			&& (ospf_timer_active(&p_process->ratelimit_timer)))
	{
		ospf_sys_delay(1);
	}
#endif

	/*construct header,type already set*/
	p_packet->version = OSPF_VERSION;
	p_packet->router = htonl(p_process->router_id);
	p_packet->len = htons(len);
	p_packet->area = htonl(p_if->p_area->id);
	p_packet->instance = p_if->instance;

	/*build auth information for packet*/
	ospf_auth_fill(p_packet, p_if);

	/*md5 will use appended 16bytes digest*/
	if (OSPF_AUTH_MD5 == p_packet->auth_type)
	{
		len += OSPF_MD5_KEY_LEN;
	}
	/*NBMA/P2MP interface:must send unicast packet to each nbr*/
	if (((OSPF_IFT_NBMA == p_if->type) || (OSPF_IFT_P2MP == p_if->type))
			&& (OSPF_ADDR_ALLSPF == dest))
	{
		for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
		{
			if (OSPF_NS_INIT < p_nbr->state)
			{
				ospf_socket_send(p_packet, p_if, p_nbr->addr, len);
			}
		}
	}
	else
	{
		ospf_socket_send(p_packet, p_if, dest, len);
	}
	p_if->stat->tx_packet[p_packet->type]++;
	p_if->stat->tx_byte[p_packet->type] += len;
	return;
}

u_int ospf_packet_dest_select(struct ospf_if *p_if, struct ospf_nbr *p_nbr,
		int type)
{
	u_int dest = 0;

	/*no neighbor exist, select according to interface type and state*/
	switch (p_if->type)
	{
	case OSPF_IFT_BCAST:
		/*if we are in restarting,set destination to 224.0.0.5*/
		if (OSPF_PACKET_HELLO == type)
		{
			return OSPF_ADDR_ALLSPF;
		}
#ifdef OSPF_DCN
		if ((p_if->uiOverlayflag == DCN_OVERLAY_MASTER_ADD)
				|| (p_if->uiOverlayflag == DCN_OVERLAY_SLAVER_ADD))
		{
			if (NULL != p_nbr)
			{
				return p_nbr->addr;
			}
		}
		if(p_if->ulDcnflag == OSPF_DCN_FLAG)
		{
			return OSPF_ADDR_ALLSPF;
		}
		else if(p_if->p_process->process_id == OSPF_DCN_PROCESS)
		{
			return OSPF_ADDR_ALLSPF;
		}
#endif
		/*send directly to nbr*/
		if (NULL != p_nbr)
		{
			//return OSPF_ADDR_ALLSPF;
			return p_nbr->addr;
		}

		if ((OSPF_PACKET_UPDATE == type) || (OSPF_PACKET_ACK == type))
		{
			if ((OSPF_IFS_DR == p_if->state) || (OSPF_IFS_BDR == p_if->state)
					|| p_if->p_process->in_restart)
			{
				return OSPF_ADDR_ALLSPF;
			}
			/*dest to all dr*/
			return OSPF_ADDR_ALLDR;
		}
		break;

	case OSPF_IFT_PPP:
		/*always mcast address on ppp interface*/
		return OSPF_ADDR_ALLSPF;
		break;

	case OSPF_IFT_NBMA:
	case OSPF_IFT_P2MP:
		if (NULL != p_nbr)
		{
			return p_nbr->addr;
		}
		return OSPF_ADDR_ALLSPF;
		break;

	case OSPF_IFT_VLINK:
		p_nbr = ospf_lstfirst(&p_if->nbr_table);
		if (NULL != p_nbr)
		{
			return p_nbr->addr;
		}
		break;

	case OSPF_IFT_SHAMLINK:
		return p_if->nbr;
		break;

	default:
		break;
	}
	return dest;
}

/*decide neighbor for rxd packet*/
struct ospf_nbr *
ospf_nbr_verify(struct ospf_if *p_if, u_int router_id, u_int source)
{
	struct ospf_nbr *p_nbr = NULL;

	switch (p_if->type)
	{
	case OSPF_IFT_BCAST:
	case OSPF_IFT_NBMA:
		/*use address for lookup*/
		return ospf_nbr_lookup(p_if, source);
		break;

	case OSPF_IFT_PPP:
	case OSPF_IFT_P2MP:
	case OSPF_IFT_VLINK:
	case OSPF_IFT_SHAMLINK:
		/*routerid and address must same on these interfaces*/
		p_nbr = ospf_nbr_first(p_if);
		if ((NULL != p_nbr) && (p_nbr->id == router_id)
				&& (p_nbr->addr == source))
		{
			ospf_logx(ospf_debug_msg,
					"#%d,p_if=0x%x,p_if->addr=0x%x,p_if->ifnet_uint=0x%x.\r\n",
					__LINE__, p_if, p_if->addr, p_if->ifnet_uint);
			ospf_logx(ospf_debug_msg,
					"#%d,p_nbr=0x%p,p_nbr->addr=0x%x,router_id=0x%x,p_nbr->p_if=0x%x,p_nbr->p_if->addr=0x%x.\r\n",
					__LINE__, p_nbr, p_nbr->addr, router_id, p_nbr->p_if,
					p_nbr->p_if->addr);
			return p_nbr;
		}
		break;

	default:
		break;
	}
	return NULL;
}

/*check if checksum and auth is correct*/
u_int ospf_auth_verify(struct ospf_hdr *p_header, struct ospf_if *p_if,
		struct ospf_nbr *p_nbr)
{
	struct ospf_authinfo *p_auth = (struct ospf_authinfo *) p_header->auth;
	struct ospf_process *p_process = p_if->p_process;
	u_short len = ntohs(p_header->len);
	u_int auth_type = p_if->authtype;/*init to if auth*/
	u_int keyid = p_if->md5id;/*init to if auth*/
	u_int8 *p_key = p_if->key;/*init to if auth*/
	u_int8 in_digest[OSPF_MAX_KEY_LEN];
	u_int8 digest[OSPF_MAX_KEY_LEN] =
	{ 0 };

	/*decide authtype used*/
	if ((OSPF_AUTH_NONE == p_if->authtype) && (NULL != p_if->p_area))
	{
		auth_type = p_if->p_area->authtype;
		keyid = p_if->p_area->keyid;
		p_key = p_if->p_area->key;
	}

	/*compare auth type*/
	if (auth_type != p_header->auth_type)
	{

		ospf_logx(ospf_debug_msg, "authtype mismatch,local:%d, rcv:%d",
				auth_type, p_header->auth_type);

		p_process->trap_error = ERROR_AUTH_TYPE;
		p_process->trap_packet = p_header->type;

		if (OSPF_IFT_VLINK != p_if->type)
		{
			ospf_trap_ifautherror(ERROR_AUTH_TYPE, p_header->type,
					p_process->trap_source, p_if);
		}
		else
		{
			ospf_trap_vifautherror(ERROR_AUTH_TYPE, p_header->type, p_if);
		}
		p_if->stat->error[OSPF_IFERR_AUTHTYPE]++;
		p_if->stat->error_data[OSPF_IFERR_AUTHTYPE] = p_header->auth_type;
		return FALSE;
	}

	switch (auth_type)
	{
	case OSPF_AUTH_NONE:
		/*verify checksum only*/
		if (checksum((u_short *) p_header, ntohs(p_header->len)))
		{
			goto AUTH_ERROR;
		}
		break;

	case OSPF_AUTH_SIMPLE:
		/*verify password*/
		if (memcmp(p_header->auth, p_key, OSPF_KEY_LEN))
		{
			ospf_logx(ospf_debug_msg, "password mismatchs");
			goto AUTH_ERROR;
		}
		/*verify checksum*/
		memset(p_header->auth, 0x0, OSPF_KEY_LEN);
		if (checksum((u_short *) p_header, ntohs(p_header->len)))
		{
			goto AUTH_ERROR;
		}
		break;

	case OSPF_AUTH_MD5:
		/*no checksum need*/
		if (0 != p_header->checksum)
		{
			ospf_logx(ospf_debug_msg, "no checksum");
			goto AUTH_ERROR;
		}
		/*rxd md5 seq must >= last rxd md5 seq*/
		if ((NULL != p_nbr)
				&& (ntohl(p_auth->seqnum) < p_nbr->auth_seqnum[p_header->type]))
		{
			/* md5*/
			if (!p_nbr->in_restart)
			{
				ospf_logx(ospf_debug_msg, "md5 seq is not increase");
				goto AUTH_ERROR;
			}
		}
		/*md5 key id must same*/
		if (keyid != p_auth->keyid)
		{
			ospf_logx(ospf_debug_msg, "keyid mismatch");
			goto AUTH_ERROR;
		}

		/*save input digest*/
		memcpy(in_digest, ((u_int8 *) p_header) + len, OSPF_MAX_KEY_LEN);

		/*calculate md5 digest,and compre result against rxd value,if not same,auth failed*/
		memcpy(((u_int8 *) p_header) + len, p_key, OSPF_MAX_KEY_LEN);
		MD5((u_int8 *) p_header, (len + OSPF_MAX_KEY_LEN), digest);
		if (memcmp(digest, in_digest, OSPF_MAX_KEY_LEN))
		{
			ospf_logx(ospf_debug_msg, "digest mismatch");
			goto AUTH_ERROR;
		}
		/*record rxd md5 seqnum*/
		if (NULL != p_nbr)
		{
			p_nbr->auth_seqnum[p_header->type] = ntohl(p_auth->seqnum);
		}
		break;

	default:
		break;
	}
	return TRUE;

	AUTH_ERROR: p_process->trap_error = ERROR_AUTH;
	p_process->trap_packet = p_header->type;

	if (OSPF_IFT_VLINK != p_if->type)
	{
		ospf_trap_ifautherror(ERROR_AUTH, p_header->type,
				p_process->trap_source, p_if);
	}
	else
	{
		ospf_trap_vifautherror(ERROR_AUTH, p_header->type, p_if);
	}
	p_if->stat->error[OSPF_IFERR_AUTH]++;
	return FALSE;
}

/*check lsa type in dd packet*/
u_int ospf_dd_verify(struct ospf_hdr *ospfh)
{
	struct ospf_dd_msg *p_dbd = (struct ospf_dd_msg *) ospfh;
	struct ospf_lshdr *p_lshdr = p_dbd->lshdr;
	u_int i = 0;
	u_int len = ntohs(p_dbd->h.len) - OSPF_PACKET_HLEN - OSPF_DBD_HLEN;
	u_int count = len / OSPF_DBD_UNIT_LEN;

	/*null dd pkt is valid*/
	if (0 >= len)
	{
		return TRUE;
	}
	/*if any lsa header is invalid,discard full packet*/
	for (i = 0; i < count; i++, p_lshdr++)
	{
		if (!ospf_lshdr_is_valid(p_lshdr))
		{
			ospf_logx(ospf_debug_msg, "lshdr is invalild in dd pkt,ignore");
			return FALSE;
		}
	}
	return TRUE;
}

/*check lsa header in request packet*/
u_int ospf_req_verify(struct ospf_hdr *ospfh)
{
	struct ospf_request_msg *p_req = (struct ospf_request_msg *) ospfh;
	struct ospf_request_unit *p_requnit = p_req->lsa;
	u_int len = ntohs(p_req->h.len) - OSPF_PACKET_HLEN;
	u_int count = len / OSPF_REQ_UNIT_LEN;
	u_int i;

	/*req must have lsa*/
	if (0 >= len)
	{
		return FALSE;
	}
	/*only check lsa type*/
	for (i = 0; i < count; i++, p_requnit++)
	{
		if ((OSPF_LS_ROUTER > p_requnit->type)
				|| ((OSPF_LS_TYPE_11 < p_requnit->type)))
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*check lsa header in ack packet*/
u_int ospf_ack_verify(struct ospf_hdr *ospfh)
{
	struct ospf_ack_msg *p_ack = (struct ospf_ack_msg *) ospfh;
	struct ospf_lshdr *p_lshdr = NULL;
	u_int len = (ntohs(p_ack->h.len) - OSPF_PACKET_HLEN);
	u_int count = (len / OSPF_LSA_HLEN);
	u_int i = 0;

	/*ack must have lsa*/
	if (0 >= len)
	{
		return FALSE;
	}
	/*if any lsa header is invalid,discard full packet*/
	for (i = 0, p_lshdr = p_ack->lshdr; i < count; i++, p_lshdr++)
	{
		if (!ospf_lshdr_is_valid(p_lshdr))
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*check if update packet's length is valid*/
u_int ospf_update_length_verify(struct ospf_hdr *ospfh)
{
	struct ospf_update_msg *p_update = (struct ospf_update_msg *) ospfh;
	struct ospf_lshdr *p_lshdr = NULL;
	struct ospf_lshdr *p_next_lshdr = NULL;
	u_int ospflen = ntohs(ospfh->len);
	u_int i = 0;
	u_int lslen = 0;
	u_int readlen = 0;

	/*include header 4 bytes*/
	if (OSPF_UPDATE_HLEN > ospflen)
	{
		ospf_logx(ospf_debug_msg, "update length too short %d", ospflen);
		return FALSE;
	}

	ospflen -= (OSPF_PACKET_HLEN + OSPF_UPDATE_HLEN);
	/*validate each lsa in update*/
	p_lshdr = p_update->lshdr;
	for (i = 0; i < ntohl(p_update->lscount); i++, p_lshdr = p_next_lshdr)
	{
		/*total lsa length exceed limit*/
		if (readlen >= ospflen)
		{
			ospf_logx(ospf_debug_msg, "lsa total len %d exceed ospflen",
					readlen);
			return FALSE;
		}
		/*if any lsa header invalid,discard full packet*/
		if (!ospf_lshdr_is_valid(p_lshdr))
		{
			return FALSE;
		}
		lslen = ntohs(p_lshdr->len);
		readlen += lslen;
		p_next_lshdr = (struct ospf_lshdr *) ((u_int8 *) p_lshdr + lslen);

		/*must contain 20 bytes header*/
		if (OSPF_LSA_HLEN > lslen)
		{
			ospf_logx(ospf_debug_msg, "lsa len %d too short", lslen);
			return FALSE;
		}
		lslen -= OSPF_LSA_HLEN;
		/*special for lsa type*/
		switch (p_lshdr->type)
		{
		case OSPF_LS_ROUTER:
		{
			struct ospf_router_link *link = NULL;
			struct ospf_router_lsa *router = (struct ospf_router_lsa *) p_lshdr;
			u_int linkcnt = 0;
			u_int linklen = 0;
			u_int total_linklen = 0;

			if (4 > lslen)
			{
				ospf_logx(ospf_debug_msg, "router lsa len too short");
				return FALSE;
			}

			lslen -= 4;

			/*rest length is sum of link's length*/
			linkcnt = ntohs(router->link_count);
			link = router->link;

			/*check each link*/
			while (0 < linkcnt)
			{
				linklen = 12 + (link->tos_count * 4);

				total_linklen += linklen;

				link = (struct ospf_router_link *) ((u_int8 *) link + linklen);

				linkcnt--;
			}

			/*total link length must same as lsa length*/
			if (lslen != total_linklen)
			{
				ospf_logx(ospf_debug_msg, "router lsa len mismatch");
				return FALSE;
			}
		}
			break;

		case OSPF_LS_NETWORK:
			if ((4 > lslen) || (lslen % 4))
			{
				return FALSE;
			}
			break;

		case OSPF_LS_SUMMARY_NETWORK:
		case OSPF_LS_SUMMARY_ASBR:
			if ((8 > lslen) || (lslen % 4))
			{
				return FALSE;
			}
			break;

		case OSPF_LS_AS_EXTERNAL:
		case OSPF_LS_TYPE_7:
			if (16 > lslen)
			{
				return FALSE;
			}
			lslen -= 16;
			if (lslen % 12)
			{
				return FALSE;
			}
			break;

		default:
			break;
		}
	}

	/*packet length must same as the sum of all lsa's length*/
	if (readlen != ospflen)
	{
		return FALSE;
	}
	return TRUE;
}

/*validate length of input packet,called before processing,if 
 failed, do not process packet, discard it*/
u_int ospf_packet_length_verify(u_int8 *buf, u_int len)
{
	struct ospf_hdr *ospfh = NULL;
	struct iphdr *iph = NULL;
	u_int iphlen = 0;
	u_int iplen = 0;
	u_int ospflen = 0;
	u_int auth_len = 0;

	/*obtain ip header length*/
	iphlen = (*buf) & 0x0f;
	iphlen *= 4;

	/*obtain total ip length*/
	iph = (struct ip *) buf;
	iplen = ntohs((u_short) iph->tot_len);

	/*obtain ospf length*/
	ospfh = (struct ospf_hdr *) (buf + iphlen);
	ospflen = ntohs(ospfh->len);

	/*decide appended auth length,if md5 used, there be 16 bytes appended field*/
	if (OSPF_AUTH_MD5 == ospfh->auth_type)
	{
		auth_len = OSPF_MD5_KEY_LEN;
	}

	/*length must at least contain the whole packet,auth filed do not included in ospf length*/
	if (len < (iphlen + ospflen + auth_len))
	{
		ospf_logx(ospf_debug_msg, "len mismatch,iplen=%d,ospflen=%d", iphlen,
				ospflen);
		return FALSE;
	}

	/*min ospf packet contain header*/
	if (OSPF_PACKET_HLEN > ospflen)
	{
		ospf_logx(ospf_debug_msg, "ospf len too short");
		return FALSE;
	}

	/*get remained length exclude header*/
	ospflen -= (OSPF_PACKET_HLEN);

	/*special for message type*/
	switch (ospfh->type)
	{
	case OSPF_PACKET_HELLO:
		/*hello has fixed length 20bytes*/
		if (OSPF_HELLO_HLEN > ospflen)
		{
			return FALSE;
		}
		ospflen -= OSPF_HELLO_HLEN;

		/*rest length must be multiple of 4bytes*/
		if (ospflen % 4)
		{
			return FALSE;
		}
		break;

	case OSPF_PACKET_DBD:
		/*dbd has fixed length 8 bytes*/
		if (OSPF_DBD_HLEN > ospflen)
		{
			return FALSE;
		}
		ospflen -= OSPF_DBD_HLEN;
		/*multiple of lsa header*/
		if (ospflen % OSPF_LSA_HLEN)
		{
			return FALSE;
		}
		return ospf_dd_verify(ospfh);
		break;

	case OSPF_PACKET_REQUEST:
		/*rest length must be multiple of 12 bytes*/
		if (ospflen % 12)
		{
			return FALSE;
		}
		return ospf_req_verify(ospfh);
		break;

	case OSPF_PACKET_UPDATE:
		return ospf_update_length_verify(ospfh);
		break;

	case OSPF_PACKET_ACK:
		/*multiple of lsa header*/
		if (ospflen % OSPF_LSA_HLEN)
		{
			return FALSE;
		}
		return ospf_ack_verify(ospfh);
		break;

	default:
		return FALSE;
		break;
	}
	/*accept it*/
	return TRUE;
}

/*get rxd interface from packet,may be virtual link*/
struct ospf_if *
ospf_rxif_verify(struct ospf_hdr *p_header, u_int source, u_int dest,
		u_int ifindex)
{
	struct ospf_if start_if;
	struct ospf_if *p_if = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	u_int area_id = ntohl(p_header->area);
	u_int router_id = ntohl(p_header->router);
	u_int find_if = FALSE;
	u_int ulLoopType = 0;
	int iRtv = 0;
	u_int ucPort = 0;
	u_int ulIp = 0;
	u_int ulDcnindex = 0;

	/*get real interface according to ifindex*/
	start_if.ifnet_index = 0;
	start_if.addr = 0;

	for_each_node_noless(&ospf.real_if_table, p_if, &start_if)
	{

		/*interface index match or do not care*/
		if (ifindex && (p_if->ifnet_uint == ifindex))
		{

			/*ifindex match,try to check network*/
			if ((ospf_netmatch(source, p_if->addr, p_if->mask)
					|| ospf_netmatch(dest, p_if->addr, p_if->mask))
					&& (p_if->instance == p_header->instance))
			{
				find_if = TRUE;
				break;
			}
		}
		ospf_logx(ospf_debug_msg/*1*/,
				"%s:%d,instance %s, source %s,source %x,addr %x,mask %x",
				p_if->instance, p_header->instance,
				(p_if->instance == p_header->instance) ? "TRUE" : "FALSE",
				ospf_netmatch(source, p_if->addr, p_if->mask) ? "TRUE" : "FALSE",
				source, p_if->addr, p_if->mask);
	}

	if (TRUE != find_if)
	{
		for_each_node_noless(&ospf.real_if_table, p_if, &start_if)
		{

			/*interface index match or do not care*/
			if (ifindex && (p_if->ifnet_uint == ifindex))
			{
#ifdef OSPF_DCN
				/*dcn 无需校验网段*/
				if(p_if->ulDcnflag == OSPF_DCN_FLAG)
				{
					find_if = TRUE;
					break;
				}
#endif
			}
		}
	}

	if (TRUE != find_if)
	{
		if (NULL == p_if)
		{
			ospf_logx(ospf_debug_msg/*1*/,
					"#%d,TRUE != find_if,(NULL == p_if).\r\n", __LINE__);
		}
		else
		{
			ospf_logx(ospf_debug_msg/*1*/,"#%d,TRUE != find_if,mask=%d.\r\n",
					__LINE__, p_if->mask);
		}
		p_if = NULL;
	}

	if ((NULL == p_if) && (!IN_MULTICAST(dest)))
	{
		/*maybe shanlink packet,search shamlink using dest and source address*/
		for_each_ospf_process(p_process, p_next_process)
		{
			p_if = ospf_shamlinkif_lookup(p_process, dest, source);
			if ((NULL != p_if) && (p_if->p_area)
					&& (p_if->p_area->id == area_id))
			{
				break;
			}
		}
	}
	if ((NULL == p_if) || (NULL == p_if->p_area))
	{
		ospf_logx(ospf_debug_msg, "no interface found");
		return NULL;
	}
	/*ignore packet on passive interface*/
	if (p_if->passive)
	{
		ospf_logx(ospf_debug_msg, "discard packet on passive iface");
		return NULL;
	}
	/*ignore loobacked packet from self*/
	if (p_if->addr == source)
	{
		ospf_logx(ospf_debug_msg, "recv packet from self");
		return NULL;
	}
	if ((OSPF_IFT_NBMA == p_if->type) && (IN_MULTICAST(dest)))
	{
		ospf_logx(ospf_debug_msg, "NBMA interface can't rcv multicast pkt");
		return NULL;
	}
	p_process = p_if->p_process;

	/*dest is all dr:dr,bdr or restarted node accept it */
	if (OSPF_ADDR_ALLDR == dest)
	{
		if ((OSPF_IFS_DR != p_if->state) && (OSPF_IFS_BDR != p_if->state)
				&& !(p_process->in_restart
						&& ((p_if->dr == p_if->addr)
								|| (p_if->bdr == p_if->addr))))
		{
			return NULL;
		}
	}
	/*area match,this is real interface*/
	if (area_id == p_if->p_area->id)
	{
		/*20120724 shamlink*/
		if (OSPF_IFT_SHAMLINK == p_if->type)
		{
			return p_if;
		}
		/*check if from same network*/
		//if(ulLoopType == INVALID)
#ifdef OSPF_DCN
		if(p_if->ulDcnflag != OSPF_DCN_FLAG)
#endif
		{
			if (!ospf_netmatch(source, p_if->addr, p_if->mask))
			{
				ospf_logx(ospf_debug_msg,
						"source not in ifnetwork,source=%x,if_addr=%x, if_mask=%x",
						source, p_if->addr, p_if->mask);

				p_if->stat->error[OSPF_IFERR_SOURCE]++;

				ospf_trap_iferror(ERROR_NETMASK, p_header->type, source, p_if);

				ospf_logx(ospf_debug_msg, "#%d\r\n", __LINE__);
				return NULL;
			}
		}

		if ((OSPF_IFT_BCAST == p_if->type) && (!IN_MULTICAST(dest))
				&& (OSPF_PACKET_HELLO == p_header->type))
		{
			ospf_logx(ospf_debug_msg,
					"Broadcast interface only rcv multicast hello");
			return NULL;
		}
	}/*area mismatch,and packet belong to backbone,search vlink*/
	else if (OSPF_BACKBONE == area_id)
	{
		p_if = ospf_vif_lookup(p_process, p_if->p_area->id, router_id);
		if (NULL == p_if)
		{
			ospf_logx(ospf_debug_msg, "can not find virtual neighbor");
		}
	}
	else
	{
		ospf_trap_iferror(ERROR_AREA, p_header->type, source, p_if);
		p_if = NULL;
	}
	return p_if;
}

u_int ospf_dcn_creat_nbr(struct ospf_if *p_if, u_int source)
{
	struct ospf_nbr *p_nbr = NULL;

	p_nbr = ospf_nbr_create(p_if, source);

	if (NULL != p_nbr)
	{
		p_nbr->dr = 0;
		p_nbr->bdr = 0;
		p_nbr->priority = 1;
		p_nbr->id = source;

		/*add for smart-discover,prepare fast hello*/
		ospf_nsm(p_nbr, OSPF_NE_HELLO);
		//   ospf_nsm(p_nbr, OSPF_NE_2WAY);
		ospf_hello_output(p_if, FALSE, FALSE);
		ospf_timer_start(&p_if->hello_timer, 5);
		//  ospf_timer_start(&p_if->hello_timer,ospf_rand(p_if->hello_interval * OSPF_TICK_PER_SECOND));

	}
}

/*packet input,start from ip header*/
void ospf_input(u_int8 *p_buf, u_int len, u_int ifindex)
{
	struct ospf_hdr *p_header = NULL;
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_process *p_process = NULL;
	struct iphdr *ip = (struct ip *) p_buf;
	u_int iphlen = ((*p_buf) & 0x0f) << 2;
	u_int source = ntohl(ip->saddr);
	u_int dest = ntohl(ip->daddr);
	u_int ttl = ip->ttl;
	u_int error_type = 0;
	u_int8 nbrstr[64];
	u_int8 nbrstr1[64];
	int cnt = 0;
	u_short usChgFlag = 0;

	/*skip ip header + options*/
	p_header = (struct ospf_hdr *) (p_buf + iphlen);
	ospf_logx(ospf_debug_msg, "ospf_input source=%x,dest=%x,if_index=%x,len=%d",
			source, dest, ifindex, len);
	/*decide rx interface,including vlink*/

	p_if = ospf_rxif_verify(p_header, source, dest, ifindex);
	if (NULL == p_if)
	{
		ospf_logx(ospf_debug_msg, "failed to check iface,discard");
		ospf.stat.discard_packet++;
		return;
	}
#ifdef OSPF_DCN /*caoyong delete 2017.9.20*//*dcn暂不考虑*/
	if(p_if->p_process->process_id == OSPF_DCN_PROCESS)
	{
		if (ERR == ospf_hello_overlay_check (p_if,source,ntohl(p_header->router)))
		{

			ospf_logx(ospf_debug_hello,"ospf_hello_input ospf_hello_overlay_check err\r\n");
			return;
		}
	}
#endif

	ospf_set_context(p_if->p_process);
	/*check if packet length valid,if not, discard it*/
	if (ospf_packet_length_verify(p_buf, len) != TRUE)
	{
		ospf_logx(ospf_debug_msg, "packet length invalid, discard it");

		if (OSPF_IFT_VLINK != p_if->type)
		{
			ospf_trap_ifbadpacket(p_header->type, source, p_if);
		}
		else
		{
			ospf_trap_vifbadpacket(p_header->type, p_if);
		}
		ospf.stat.discard_packet++;
		return;
	}

	p_process = p_if->p_process;
	ospf_set_context(p_process);

	/*discard packet with self id*/
	if (ntohl(p_header->router) == p_process->router_id)
	{
		ospf_logx(ospf_debug_msg, "ospf_input router id is same as self");
		p_if->stat->error[OSPF_IFERR_OWN]++;

		error_type = ERROR_ROUTERID;
		goto CONFIG_ERROR;
	}

	/*GTSM ttl checking*/
	if (p_process->valid_hops && (ttl < (255 - p_process->valid_hops + 1)))
	{
		ospf_logx(ospf_debug_msg,
				"ospf_input ttl is not valid,rcv ttl %d,vallid ttl %d", ttl,
				p_process->valid_hops);
		ospf.stat.discard_packet++;
		return;
	}
	p_process->trap_source = source;

	/*version must be 2*/
	if (OSPF_VERSION != p_header->version)
	{
		ospf_logx(ospf_debug_msg, "ospf_input ospf_input invalid version %d",
				p_header->version);
		p_process->trap_error = ERROR_VERSION;
		p_process->trap_packet = p_header->type;

		p_if->stat->error[OSPF_IFERR_VERSION]++;

		error_type = ERROR_VERSION;
		goto CONFIG_ERROR;
	}

	/*decide nbr for packet,nbr can be null only for hello packet*/
	p_nbr = ospf_nbr_verify(p_if, ntohl(p_header->router), source);
	if ((NULL == p_nbr) && (OSPF_PACKET_HELLO != p_header->type))
	{
		ospf_logx(ospf_debug_msg, "ospf_input no nbr for non-hello packet");
		p_if->stat->error[OSPF_IFERR_NONBR]++;

		if (OSPF_IFT_NBMA == p_if->type)
		{
			error_type = ERROR_UNKOWN_NBMANBR;
		}
		else if (OSPF_IFT_VLINK == p_if->type)
		{
			error_type = ERROR_UNKOWN_VNBR;
		}
		goto CONFIG_ERROR;
	}

	/*special for interface with only one nbr:ppp/vlink/shamlink
	 if input nbr is null,but there is already another nbr on
	 this interface,reject this packet.20130807
	 */
	//   cnt = ospf_lstcnt(&p_if->nbr_table);
	cnt = ospf_nbr_search(p_if);
	if ((NULL == p_nbr) && cnt
			&& ((OSPF_IFT_PPP == p_if->type) || (OSPF_IFT_VLINK == p_if->type)
					|| (OSPF_IFT_SHAMLINK == p_if->type)))
	{

		/*DCN允许多个邻居*/
#ifdef OSPF_DCN
		if(p_if->p_process->process_id != OSPF_DCN_PROCESS)
#else
		if (p_if->p_process->process_id)
#endif
		{
			ospf_logx(ospf_debug_msg,
					"ospf_input invalid nbr on ppp interface");
			p_if->stat->error[OSPF_IFERR_NONBR]++;
			ospf.stat.discard_packet++;
			return;
		}
		else
		{
		}
	}
	/* forth and fifth bullet items under verifying the OSPF packet header (page 58) */
	if (!ospf_auth_verify(p_header, p_if, p_nbr))
	{
		ospf_logx(ospf_debug_msg, "ospf_input authentication mismatchs");
		ospf.stat.discard_packet++;
		return;
	}

	/*restart neighbor hold timer for non-hello packet*/
	if ((NULL != p_nbr) && (OSPF_PACKET_HELLO != p_header->type))
	{
#if 1
		p_nbr->rcv_pkt_time = os_system_tick();
#else
		ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
#endif
	}

	if ((NULL != p_nbr) && (p_nbr->id != ntohl(p_header->router)))
	{
		ospf_logx(ospf_debug_msg, "nbr id changed from %s to %s",
				ospf_inet_ntoa(nbrstr, p_nbr->id),
				ospf_inet_ntoa(nbrstr1, ntohl(p_header->router)));
	}
	/*stat*/
	p_if->stat->rx_packet[p_header->type]++;
	p_if->stat->rx_byte[p_header->type] += len;

	if ((OSPF_ADDR_ALLDR == dest) || (OSPF_ADDR_ALLSPF == dest))
	{
		p_if->stat->rx_muti_pkt++;
	}
	else
	{
		p_if->stat->rx_unicast_pkt++;
	}

	/*nbr is in rejected state,discard packet*/
	if ((NULL != p_nbr) && (ospf_timer_active(&p_nbr->reject_nbr_timer)))
	{
		ospf_logx(ospf_debug_msg, "ospf_input reject nbr %x msg\r\n",
				p_nbr->id);
		return;
	}

	if (NULL != p_nbr)
	{
	}
	/*processing according to type*/
	ospf_logx(ospf_debug_msg, "222 ospf_input p_header->type=%x msg\r\n",
			p_header->type);
	switch (p_header->type)
	{
	case OSPF_PACKET_HELLO:
		ospf_hello_input(p_header, p_nbr, p_if, source);
		break;

	case OSPF_PACKET_DBD:
		ospf_dd_input(p_nbr, p_header);
		break;

	case OSPF_PACKET_REQUEST:
		ospf_request_input(p_nbr, p_header);
		break;

	case OSPF_PACKET_UPDATE:
		ospf_update_input(p_nbr, p_header);
		break;

	case OSPF_PACKET_ACK:
		ospf_ack_input(p_nbr, p_header);
		break;

	default:
	{
		p_process->trap_packet = p_header->type;

		if (OSPF_IFT_VLINK != p_if->type)
		{
			ospf_trap_ifbadpacket(p_header->type, source, p_if);
		}
		else
		{
			ospf_trap_vifbadpacket(p_header->type, p_if);
		}
		break;
	}

	}
	return;

	CONFIG_ERROR: ospf.stat.discard_packet++;
	if (OSPF_IFT_VLINK != p_if->type)
	{
		ospf_trap_iferror(error_type, p_header->type, source, p_if);
	}
	else
	{
		ospf_trap_viferror(error_type, p_header->type, p_if);
	}
	return;
}

int ospf_nbr_search(struct ospf_if *p_if)
{
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next = NULL;
	u_int uiCnt = 0;

	for_each_ospf_nbr(p_if, p_nbr, p_next)
	{
		return 1;
	}

	return 0;

}
