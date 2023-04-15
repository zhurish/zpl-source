/*
 * dhcp_option.c
 *
 *  Created on: Apr 27, 2019
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>

#include "dhcp_def.h"
#include "dhcp_util.h"
#include "dhcp_packet.h"
#include "dhcp_option.h"




static int dhcp_option_packet_end(char *data, zpl_uint32 len);
static zpl_uint32 dhcp_option_get_end_padding(char *data, zpl_uint32 len, zpl_bool get);
static int dhcp_option_overload_get(char *data, zpl_uint32 len);


static const struct dhcp_optflag dhcp_optflag[] = {
	/* flags                                    code */
	{ DHCP_OPTION_TYPE_IP                   | DHCP_OPTION_TYPE_REQ, 0x01 }, /* DHCP_SUBNET        */
	{ DHCP_OPTION_TYPE_S32                              , 0x02 }, /* DHCP_TIME_OFFSET   */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST     | DHCP_OPTION_TYPE_REQ, 0x03 }, /* DHCP_ROUTER        */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST                 , 0x04 }, /* DHCP_TIME_SERVER   */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST                 , 0x05 }, /* DHCP_NAME_SERVER   */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST     | DHCP_OPTION_TYPE_REQ, 0x06 }, /* DHCP_DNS_SERVER    */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST                 , 0x07 }, /* DHCP_LOG_SERVER    */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST                 , 0x08 }, /* DHCP_COOKIE_SERVER */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST                 , 0x09 }, /* DHCP_LPR_SERVER    */
	{ DHCP_OPTION_TYPE_STRING_HOST          | DHCP_OPTION_TYPE_REQ, 0x0c }, /* DHCP_HOST_NAME     */
	{ DHCP_OPTION_TYPE_U16                              , 0x0d }, /* DHCP_BOOT_SIZE     */
	{ DHCP_OPTION_TYPE_STRING_HOST          | DHCP_OPTION_TYPE_REQ, 0x0f }, /* DHCP_DOMAIN_NAME   */
	{ DHCP_OPTION_TYPE_IP                               , 0x10 }, /* DHCP_SWAP_SERVER   */
	{ DHCP_OPTION_TYPE_STRING                           , 0x11 }, /* DHCP_ROOT_PATH     */
	{ DHCP_OPTION_TYPE_U8                               , 0x17 }, /* DHCP_IP_TTL        */
	{ DHCP_OPTION_TYPE_U16                              , 0x1a }, /* DHCP_MTU           */
	//TODO: why do we request DHCP_BROADCAST? Can't we assume that
	//in the unlikely case it is different from typical N.N.255.255,
	//server would let us know anyway?
	{ DHCP_OPTION_TYPE_IP                   | DHCP_OPTION_TYPE_REQ, 0x1c }, /* DHCP_BROADCAST     */
	{ DHCP_OPTION_TYPE_IP_PAIR | DHCP_OPTION_TYPE_LIST            , 0x21 }, /* DHCP_ROUTES        */
	{ DHCP_OPTION_TYPE_STRING_HOST                      , 0x28 }, /* DHCP_NIS_DOMAIN    */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST                 , 0x29 }, /* DHCP_NIS_SERVER    */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST     | DHCP_OPTION_TYPE_REQ, 0x2a }, /* DHCP_NTP_SERVER    */
	{ DHCP_OPTION_TYPE_IP | DHCP_OPTION_TYPE_LIST                 , 0x2c }, /* DHCP_WINS_SERVER   */
	{ DHCP_OPTION_TYPE_U32                              , 0x33 }, /* DHCP_LEASE_TIME    */
	{ DHCP_OPTION_TYPE_IP                               , 0x36 }, /* DHCP_SERVER_ID     */
	{ DHCP_OPTION_TYPE_STRING                           , 0x38 }, /* DHCP_ERR_MESSAGE   */
	//TODO: must be combined with 'sname' and 'file' handling:
	{ DHCP_OPTION_TYPE_STRING_HOST                      , 0x42 }, /* DHCP_TFTP_SERVER_NAME */
	{ DHCP_OPTION_TYPE_STRING                           , 0x43 }, /* DHCP_BOOT_FILE     */
	//TODO: not a string, but a set of LASCII strings:
	{ DHCP_OPTION_TYPE_STRING                           , 0x4D }, /* DHCP_USER_CLASS    */
#if DHCP_ENABLE_RFC3397
	{ DHCP_OPTION_TYPE_DNS_STRING | DHCP_OPTION_TYPE_LIST         , 0x77 }, /* DHCP_DOMAIN_SEARCH */
	{ DHCP_OPTION_TYPE_SIP_SERVERS                      , 0x78 }, /* DHCP_SIP_SERVERS   */
#endif
	{ DHCP_OPTION_TYPE_STATIC_ROUTES | DHCP_OPTION_TYPE_LIST      , 0x79 }, /* DHCP_STATIC_ROUTES */
#if DHCP6_ENABLE_8021Q
	{ DHCP_OPTION_TYPE_U16                              , 0x84 }, /* DHCP_VLAN_ID       */
	{ DHCP_OPTION_TYPE_U8                               , 0x85 }, /* DHCP_VLAN_PRIORITY */
#endif
	{ DHCP_OPTION_TYPE_STRING                           , 0xd1 }, /* DHCP_PXE_CONF_FILE */
	{ DHCP_OPTION_TYPE_STRING                           , 0xd2 }, /* DHCP_PXE_PATH_PREFIX */
	{ DHCP_OPTION_TYPE_6RD                              , 0xd4 }, /* DHCP_6RD           */
	{ DHCP_OPTION_TYPE_STATIC_ROUTES | DHCP_OPTION_TYPE_LIST      , 0xf9 }, /* DHCP_MS_STATIC_ROUTES */
	{ DHCP_OPTION_TYPE_STRING                           , 0xfc }, /* DHCP_WPAD          */

	/* Options below have no match in dhcp_option_strings[],
	 * are not passed to dhcpc scripts, and cannot be specified
	 * with "option XXX YYY" syntax in dhcpd config file.
	 * These entries are only used internally by udhcp[cd]
	 * to correctly encode options into packets.
	 */

	{ DHCP_OPTION_TYPE_IP                               , 0x32 }, /* DHCP_REQUESTED_IP  */
	{ DHCP_OPTION_TYPE_U8                               , 0x35 }, /* DHCP_MESSAGE_TYPE  */
	{ DHCP_OPTION_TYPE_U16                              , 0x39 }, /* DHCP_MAX_SIZE      */
	//looks like these opts will work just fine even without these defs:
	{ DHCP_OPTION_TYPE_STRING                           , 0x3c }, /* DHCP_VENDOR        */
	//	/* not really a string: */
	{ DHCP_OPTION_TYPE_STRING                           , 0x3d }, /* DHCP_CLIENT_ID     */
	{ 0, 0 } /* zeroed terminating entry */
};

static int dhcp_option_idx(zpl_uint8 code)
{
	zpl_uint32 i = 0;
	for (i = 0; i < sizeof(dhcp_optflag) / sizeof(dhcp_optflag[0]); i++)
	{
		if (dhcp_optflag[i].code == code)
			return i;
	}
	return -1;
}

int dhcp_option_flags(zpl_uint8 code)
{
	zpl_uint32 i = 0;
	for (i = 0; i < sizeof(dhcp_optflag) / sizeof(dhcp_optflag[0]); i++)
	{
		if (dhcp_optflag[i].code == code)
			return dhcp_optflag[i].flags;
	}
	return 0;
}

static int dhcp_option_strget(const char *const_str, char *out_str)
{
	zpl_uint32 len = 0;
	char *p = const_str;
	if(!const_str)
		return 0;
	char *val = strstr(p, ",");
	if(val)
	{
		len = val - p;
		strncpy(out_str, p, len);
		return len;
	}
	else
	{
		len = strlen(p);
		strncpy(out_str, p, len);
		return len;
	}
	return 0;
}



int dhcp_option_string_set(dhcp_option_set_t *option_tbl, zpl_uint16 code,
		const char *const_str)
{
	struct dhcp_optflag *optflag = NULL;
	char str[254], *p = NULL;
	zpl_uint32 retval = 0, length = 0, optlen = 0, offset = 0;
	zpl_uint8 *val8 = NULL, buffer[512];
	zpl_int32 optindex = 0;
	zpl_uint16 *val16 = NULL;
	zpl_uint32  *val32 = NULL;
	if(!option_tbl || !const_str)
		return 0;
	val8 = buffer;
	val16 = buffer;
	val32 = buffer;
	p = (char *) const_str;
	optindex = dhcp_option_idx(code);
	if (optindex == -1)
		return 0;
	optflag = &dhcp_optflag[optindex];

	memset(buffer, 0, sizeof(buffer));
	do
	{
		memset(str, '\0', sizeof(str));
		if(p)
		{
			if(retval)
				p += retval + 1;
		}
		else
		{
			return dhcp_option_add(option_tbl, code, buffer, length);
			//return _udhcp_option_add(option_list, opc, buffer, length);
		}
		if(p)
			retval = dhcp_option_strget(p, str);
		if(retval == 0)
		{
			return dhcp_option_add(option_tbl, code, buffer, length);
			//return _udhcp_option_add(option_list, opc, buffer, length);
		}
		offset = length;

		switch (optflag->flags & DHCP_OPTION_TYPE_MASK)
		{
		case DHCP_OPTION_TYPE_IP:
			udhcp_str2nip(str, buffer + offset);
			length += 4;
			break;
		case DHCP_OPTION_TYPE_IP_PAIR:
			udhcp_str2nip(str, buffer + offset);
			length += 4;
			break;
		case DHCP_OPTION_TYPE_STRING:
		case DHCP_OPTION_TYPE_STRING_HOST:
			optlen = strnlen(str, 254);
			strncpy(buffer + offset, str, optlen);
			length += optlen;
			break;
	#if DHCP_ENABLE_RFC3397
		case DHCP_OPTION_TYPE_DNS_STRING:
			p = dname_enc(NULL, 0, str, &optlen);
			if(p)
			{
				strncpy(buffer + offset, p, optlen);
				length += optlen;
			}
	#endif
			break;
		case DHCP_OPTION_TYPE_BOOLEAN:
		{
			if(strstr(str, "no"))
				buffer[offset] = 0;
			else if(strstr(str, "yes"))
				buffer[offset] = 1;
			length += 1;
		}
		break;
		case DHCP_OPTION_TYPE_U8:
			buffer[offset] = strtoul(str, NULL, 10);
			length += 1;
			break;
			/* htonX are macros in older libc's, using temp var
			 * in code below for safety */
			/* TODO: use bb_strtoX? */
		case DHCP_OPTION_TYPE_U16:
		case DHCP_OPTION_TYPE_S16:
		{
			val16 = buffer + offset;
			zpl_uint32  tmp = strtoul(str, NULL, 0);
			*val16 = htons(tmp);
			length += 2;
			break;
		}
		case DHCP_OPTION_TYPE_U32:
		case DHCP_OPTION_TYPE_S32:
		{
			val32 = buffer + offset;
			zpl_uint32  tmp = strtoul(str, NULL, 0);
			*val32 = htonl(tmp);
			length += 4;
			break;
		}
		case DHCP_OPTION_TYPE_BIN: /* handled in attach_option() */
			buffer[offset] = str[0];
			length += 1;
			break;
		case DHCP_OPTION_TYPE_STATIC_ROUTES:
		{
			/* Input: "a.b.c.d a.b.c.d" */
			/* Output: mask(1 byte),pfx(0-4 bytes),gw(4 bytes) */
			//unsigned mask;
			char *slash = strchr(str, ' ');
			if (slash)
			{
				*slash = '\0';
				retval = udhcp_str2nip(str, buffer);
				retval = udhcp_str2nip(slash + 1, buffer + 4);
			}
			length += 8;
			break;
		}
		case DHCP_OPTION_TYPE_6RD: /* handled in attach_option() */
			strncpy(buffer + offset, str, optlen);
			length += 12;
			break;
	#if DHCP_ENABLE_RFC3397
		case DHCP_OPTION_TYPE_SIP_SERVERS:
			optlen = strnlen(str, 254);
			strncpy(buffer + offset, str, optlen);
			length += optlen;
			break;
	#endif
		default:
			break;
		}
	}while(1);

	return 0;
}


int dhcp_option_add(dhcp_option_set_t *option_tbl, zpl_uint16 code, const zpl_uint8 *opt, zpl_uint32 len)
{
	zpl_uint32 inlen = len;
	zpl_uint32 type = 0;
	if(code >= DHCP_OPTION_MAX)
		return ERROR;
	if(!option_tbl || !opt)
		return ERROR;
	if(code == DHCP_OPTION_CLIENT_ID)
	{
		if(len == 6)
		{
			inlen += 1;
			type = DHCP_OPTION_61_MAC;
		}
		else if(len > 6 && len <= 36)
		{
			inlen += 1;
			type = DHCP_OPTION_61_UUID;
		}
		else// if(len == 6)
		{
			inlen += 1;
			type = DHCP_OPTION_61_IAID;
		}
	}
	option_tbl[code].data = malloc(inlen);
	if(option_tbl[code].data)
	{
		memset(option_tbl[code].data, 0, inlen);
		if(code == DHCP_OPTION_CLIENT_ID)
		{
			memcpy(option_tbl[code].data + 1, opt, inlen - 1);
			option_tbl[code].data[0] = type;
		}
		else
			memcpy(option_tbl[code].data, opt, inlen);
		option_tbl[code].code = code;
		option_tbl[code].len = inlen;
		return OK;
	}
	return ERROR;
}

int dhcp_option_add_hex(dhcp_option_set_t *option_tbl, zpl_uint16 code, const zpl_uint32  value, zpl_uint32 len)
{
	zpl_uint32  invalue32 = value;
	zpl_uint16 invalue16 = value;
	zpl_uint8 invalue8 = value;
	if(code >= DHCP_OPTION_MAX)
		return ERROR;
	if(!option_tbl)
		return ERROR;
	option_tbl[code].data = malloc(len);
	if(option_tbl[code].data)
	{
		memset(option_tbl[code].data, 0, len);
		if(len == 1)
			memcpy(option_tbl[code].data, &invalue8, len);
		else if(len == 2)
			memcpy(option_tbl[code].data, &invalue16, len);
		else if(len == 4)
			memcpy(option_tbl[code].data, &invalue32, len);
		option_tbl[code].code = code;
		option_tbl[code].len = len;
		return OK;
	}
	return ERROR;
}

int dhcp_option_del(dhcp_option_set_t *option_tbl, zpl_uint16 code)
{
	if(code >= DHCP_OPTION_MAX)
		return ERROR;
	if(!option_tbl)
		return ERROR;
	if(option_tbl[code].len)
	{
		if(option_tbl[code].data)
			free(option_tbl[code].data);
		option_tbl[code].data = NULL;
		option_tbl[code].code = 0;
		option_tbl[code].len = 0;
		return OK;
	}
	return ERROR;
}

int dhcp_option_clean(dhcp_option_set_t *option_tbl)
{
	zpl_uint32 code = 0;
	if(!option_tbl)
		return ERROR;
	for(code = 0; code < DHCP_OPTION_MAX; code++)
	{
		if(option_tbl[code].data)
			free(option_tbl[code].data);
		option_tbl[code].data = NULL;
		option_tbl[code].code = 0;
		option_tbl[code].len = 0;
	}
	return OK;
}

int dhcp_option_lookup(dhcp_option_set_t *option_tbl, zpl_uint16 code)
{
	if(code >= DHCP_OPTION_MAX)
		return ERROR;
	if(!option_tbl)
		return ERROR;
	if(option_tbl[code].len && option_tbl[code].data)
	{
		return OK;
	}
	return ERROR;
}

int dhcp_option_packet(dhcp_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	zpl_int32 i = 0, offset = 0;//, j = 0;
	//zpl_uint8 parm_lst[256];
	if(!option_tbl || !data)
		return 0;
	//offset = dhcp_end_option(data);
	offset = dhcp_option_get_end_padding(data, len, zpl_false);
	if(offset < 0)
		return 0;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	//char *p = (char *)msg;
	for(i = 0; i <= DHCP_OPTION_MAX; i++)
	{
		if(option_tbl[i].len && option_tbl[i].data)
		{
			msg = (dhcp_option_hdr_t *)(data + offset);
			//p = (char *)(data + offset + OPT_DATA);
			msg->code = option_tbl[i].code;
			msg->len = option_tbl[i].len;
			memcpy(msg->val.pval, option_tbl[i].data, option_tbl[i].len);
			//msg->data = (zpl_uint8 *)(p);
			//memcpy(msg->data, option_tbl[i].data, option_tbl[i].len);
			offset += DHCP_OPT_DATA + option_tbl[i].len;
/*			if(dhcp_option_flags(msg->code) & DHCP_OPTION_TYPE_REQ)
				parm_lst[j++] = msg->code;*/
		}
	}
/*	msg = (dhcp_option_hdr_t *)(data + offset);
	msg->code = DHCP_PARAM_REQ;
	msg->len = j;
	memcpy(msg->val.pval, parm_lst, j);
	offset += OPT_DATA + j;*/
	dhcp_option_packet_end(data + offset, len - offset);
	return offset;
}

/*int  dhcp_end_option(zpl_uint8 *optionptr)
{
	int i = 0;

	while (optionptr[i] != DHCP_OPTION_END) {
		if (optionptr[i] != DHCP_PADDING)
			i += optionptr[i + OPT_LEN] + OPT_DATA-1;
		i++;
	}
	return i;
}*/

static zpl_uint32 dhcp_option_get_end_padding(char *data, zpl_uint32 len, zpl_bool get)
{
	zpl_uint32 offset = 0;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	if(!data || len == 0)
		return -1;
	int overload = dhcp_option_overload_get(data, len);
/*	if(get)
		overload = dhcp_option_overload_get(data, len);*/
	if(overload == ERROR)
		overload = 0;
	if(msg->code != DHCP_OPTION_END)//DHCP_PADDING
	{
		if(msg->code != DHCP_OPTION_PADDING)
			offset += msg->len + DHCP_OPT_DATA;
		else
			offset++;
		//return offset;
	}
	else
	{
		if(overload == 0)
			return offset;
		if(overload & DHCP_FILE_FIELD)
		{
			offset += DHCP_FILE_LEN;
		}
		if(overload & DHCP_SNAME_FIELD)
		{
			offset += DHCP_SNAME_LEN;
		}
	}
	//offset = msg->len + OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_hdr_t *)(data + offset);
		//msg->data = (data + offset + OPT_DATA);
		if(msg->code != DHCP_OPTION_END)//DHCP_PADDING
		{
			if(msg->code != DHCP_OPTION_PADDING)
				offset += msg->len + DHCP_OPT_DATA;
			else
				offset++;
			//return offset;
		}
		else
		{
			if(overload & DHCP_FILE_FIELD)
			{
				offset += DHCP_FILE_LEN;
				continue;
			}
			if(overload & DHCP_SNAME_FIELD)
			{
				offset += DHCP_SNAME_LEN;
				continue;
			}
			break;
		}
		if(offset >= len)
			break;
		//offset += msg->len + OPT_DATA;
		//if(offset >= len)
		//	return 0;
	}
	return offset;
}

zpl_uint32 dhcp_option_get_length(char *data)
{
	if(!data)
		return 0;
	return dhcp_option_get_end_padding(data, DHCP_OPTIONS_BUFSIZE, zpl_true);
}

/* Return the position of the 'end' option (no bounds checking) */
int dhcp_end_option(zpl_uint8 *optionptr)
{
	int i = 0;
	if(!optionptr)
		return 0;
	while (optionptr[i] != DHCP_OPTION_END) {
		if (optionptr[i] != DHCP_OPTION_PADDING)
			i += optionptr[i + DHCP_OPT_LEN] + DHCP_OPT_DATA-1;
		i++;
	}
	return i;
}

int dhcp_option_message_type(char *data, zpl_uint8 code)
{
	if(!data)
		return 0;
	zpl_int32 offset = dhcp_option_get_end_padding(data, DHCP_OPTIONS_BUFSIZE, zpl_false);
	if(offset < 0)
		return 0;
	dhcp_option_hdr_t *msg_type = (data + offset);
	msg_type->code = DHCP_OPTION_MESSAGE_TYPE;
	msg_type->len = 1;
	//msg_type->val.val8 = (zpl_uint8 *)(data + OPT_DATA);
	msg_type->val.val8 = code;
	offset += 3;
	dhcp_option_packet_end(data + offset, DHCP_OPTIONS_BUFSIZE-offset);
	return DHCP_OPT_DATA + DHCP_OPT_LEN;
}


int dhcp_option_packet_set_simple(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint32  value)
{
	if(!data)
		return 0;
	zpl_int32 offset = 0;//dhcp_end_option(data);
	offset = dhcp_option_get_end_padding(data, len, zpl_false);
	if(offset < 0)
		return 0;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	msg->code = code;
	msg->len = 4;
	msg->val.val32 = value;
	offset += DHCP_OPT_DATA + 4;
	dhcp_option_packet_end(data + offset, len - offset);
	return offset;
}

int dhcp_option_packet_set_value(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint32  oplen, zpl_uint8 *opt)
{
	if(!data || opt)
		return 0;
	zpl_int32 offset = 0;//dhcp_end_option(data);
	offset = dhcp_option_get_end_padding(data, len, zpl_false);
	if(offset < 0)
		return 0;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	msg->code = code;
	msg->len = oplen;
	memcpy(msg->val.pval, opt, oplen);
	offset += DHCP_OPT_DATA + oplen;
	dhcp_option_packet_end(data + offset, len - offset);
	return offset;
}

int dhcp_add_simple_option(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32  value)
{
	return dhcp_option_packet_set_simple(packet->options, dhcp_option_get_length(packet->options), code, value);
}

int dhcp_add_simple_option_value(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32  oplen, zpl_uint8 *opt)
{
	return dhcp_option_packet_set_value(packet->options, dhcp_option_get_length(packet->options), code, oplen, opt);
}

static int dhcp_option_packet_end(char *data, zpl_uint32 len)
{
	if(!data)
		return 0;
	zpl_uint32 offset = 0;//dhcp_end_option(data);
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	msg->code = DHCP_OPTION_END;
	//msg->len = option_tbl[i].len;
	return 1;
}

static int dhcp_option_overload_get(char *data, zpl_uint32 len)
{
	zpl_uint32 offset = 0;
	if(!data)
		return ERROR;
/*	int len = dhcp_option_get_end_padding(data, DHCP_OPTIONS_BUFSIZE);
	if(!len)
		return ERROR;*/
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)data;

	if(msg->code == DHCP_OPTION_OVERLOAD)
	{
		return msg->val.val8;
	}
	offset = msg->len + DHCP_OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_hdr_t *)(data + offset);
		if(msg->code == DHCP_OPTION_OVERLOAD)
		{
			return msg->val.val8;
		}
		offset += msg->len + DHCP_OPT_DATA;
		if(offset >= len)
			return ERROR;
	}
	return ERROR;
}

zpl_uint8 * dhcp_option_get(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint8 *optlen)
{
	zpl_uint32 /*i = 0, */offset = 0;
	if(!data)
		return NULL;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)data;
	int overload = dhcp_option_overload_get(data, len);
	if(overload == ERROR)
		return NULL;
	if(msg->code == code)
	{
		//msg->data = data + OPT_DATA;
		if(optlen)
			*optlen = msg->len;
		return msg->val.pval;
	}
	offset = msg->len + DHCP_OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_hdr_t *)(data + offset);
		//msg->data = (data + offset + OPT_DATA);
		if(msg->code == code)
		{
			if(optlen)
				*optlen = msg->len;
			return msg->val.pval;//msg->data;
		}
		else if(msg->code == DHCP_OPTION_END)
		{
			if(overload & DHCP_FILE_FIELD)
			{
				offset += DHCP_FILE_LEN;
				continue;
			}
			if(overload & DHCP_SNAME_FIELD)
			{
				offset += DHCP_SNAME_LEN;
				continue;
			}
			return NULL;
		}
		offset += msg->len + DHCP_OPT_DATA;

		if(offset >= len)
			return NULL;
	}
	return NULL;
}




int dhcp_option_message_type_get(char *data, zpl_uint32 len)
{
	zpl_uint32 /*i = 0, */offset = 0;
	if(!data)
		return 0;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)data;
	int overload = dhcp_option_overload_get(data, len);
	if(overload == ERROR)
		return 0;
	if(msg->code == DHCP_OPTION_MESSAGE_TYPE)
	{
		return msg->val.val8;
	}
	offset = msg->len + DHCP_OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_hdr_t *)(data + offset);
		if(msg->code == DHCP_OPTION_MESSAGE_TYPE)
		{
/*			if(optlen)
				*optlen = msg->len;*/
			return msg->val.val8;
		}
		else if(msg->code == DHCP_OPTION_END)
		{
			if(overload & DHCP_FILE_FIELD)
			{
				offset += DHCP_FILE_LEN;
				continue;
			}
			if(overload & DHCP_SNAME_FIELD)
			{
				offset += DHCP_SNAME_LEN;
				continue;
			}
			return 0;
		}
		offset += msg->len + DHCP_OPT_DATA;
		if(offset >= len)
			return 0;
	}
	return 0;
}

int dhcp_option_get_simple(const char *data, zpl_uint32 *output, zpl_uint8 code, zpl_uint8 optlen)
{
	zpl_int32 /*i = 0, */offset = 0;
	if(!data)
		return ERROR;
	zpl_int32 len = dhcp_option_get_end_padding(data, DHCP_OPTIONS_BUFSIZE, zpl_true);
	if(!data || len < 0)
		return ERROR;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)data;
	int overload = dhcp_option_overload_get(data, DHCP_OPTIONS_BUFSIZE);
	if(overload == ERROR)
		return ERROR;
	if(msg->code == code)
	{
		if(optlen == 1 && output)
			*output = msg->val.val8;
		if(optlen == 2 && output)
			*output = msg->val.val16;
		if(optlen == 4 && output)
			*output = msg->val.val32;
		return OK;
	}
	offset = msg->len + DHCP_OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_hdr_t *)(data + offset);
		if(msg->code == code)
		{
			if(optlen == 1 && output)
				*output = msg->val.val8;
			if(optlen == 2 && output)
				*output = msg->val.val16;
			if(optlen == 4 && output)
				*output = msg->val.val32;
			return OK;
		}
		else if(msg->code == DHCP_OPTION_END)
		{
			if(overload & DHCP_FILE_FIELD)
			{
				offset += DHCP_FILE_LEN;
				continue;
			}
			if(overload & DHCP_SNAME_FIELD)
			{
				offset += DHCP_SNAME_LEN;
				continue;
			}
			return ERROR;
		}
		offset += msg->len + DHCP_OPT_DATA;
		if(offset >= len)
			return ERROR;
	}
	return ERROR;
}

zpl_uint8* udhcp_get_option(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32 *optlen)
{
	zpl_uint8 *optionptr = NULL;
	zpl_int32 len = 0;
	if(!packet)
		return NULL;
	/* option bytes: [code][len][data1][data2]..[dataLEN] */
	optionptr = packet->options;
	//len = sizeof(packet->options);
	len = dhcp_option_get_end_padding(optionptr, DHCP_OPTIONS_BUFSIZE, zpl_true);
	if(len < 0)
		return NULL;
	return dhcp_option_get(optionptr,  len,  code, optlen);
}
