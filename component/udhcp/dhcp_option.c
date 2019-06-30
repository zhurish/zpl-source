/*
 * dhcp_option.c
 *
 *  Created on: Apr 27, 2019
 *      Author: zhurish
 */

#include "zebra.h"
#include "if.h"
#include "memory.h"
#include "command.h"
#include "prefix.h"
#include "log.h"
#include "eloop.h"

#include "dhcp_def.h"
#include "dhcp_option.h"



static int dhcp_option_packet_end(char *data, int len);
static int dhcp_option_get_end_padding(char *data, int len);
static int dhcp_option_overload_get(char *data);


static const struct dhcp_optflag dhcp_optflag[] = {
	/* flags                                    code */
	{ OPTION_IP                   | OPTION_REQ, 0x01 }, /* DHCP_SUBNET        */
	{ OPTION_S32                              , 0x02 }, /* DHCP_TIME_OFFSET   */
	{ OPTION_IP | OPTION_LIST     | OPTION_REQ, 0x03 }, /* DHCP_ROUTER        */
	{ OPTION_IP | OPTION_LIST                 , 0x04 }, /* DHCP_TIME_SERVER   */
	{ OPTION_IP | OPTION_LIST                 , 0x05 }, /* DHCP_NAME_SERVER   */
	{ OPTION_IP | OPTION_LIST     | OPTION_REQ, 0x06 }, /* DHCP_DNS_SERVER    */
	{ OPTION_IP | OPTION_LIST                 , 0x07 }, /* DHCP_LOG_SERVER    */
	{ OPTION_IP | OPTION_LIST                 , 0x08 }, /* DHCP_COOKIE_SERVER */
	{ OPTION_IP | OPTION_LIST                 , 0x09 }, /* DHCP_LPR_SERVER    */
	{ OPTION_STRING_HOST          | OPTION_REQ, 0x0c }, /* DHCP_HOST_NAME     */
	{ OPTION_U16                              , 0x0d }, /* DHCP_BOOT_SIZE     */
	{ OPTION_STRING_HOST          | OPTION_REQ, 0x0f }, /* DHCP_DOMAIN_NAME   */
	{ OPTION_IP                               , 0x10 }, /* DHCP_SWAP_SERVER   */
	{ OPTION_STRING                           , 0x11 }, /* DHCP_ROOT_PATH     */
	{ OPTION_U8                               , 0x17 }, /* DHCP_IP_TTL        */
	{ OPTION_U16                              , 0x1a }, /* DHCP_MTU           */
	//TODO: why do we request DHCP_BROADCAST? Can't we assume that
	//in the unlikely case it is different from typical N.N.255.255,
	//server would let us know anyway?
	{ OPTION_IP                   | OPTION_REQ, 0x1c }, /* DHCP_BROADCAST     */
	{ OPTION_IP_PAIR | OPTION_LIST            , 0x21 }, /* DHCP_ROUTES        */
	{ OPTION_STRING_HOST                      , 0x28 }, /* DHCP_NIS_DOMAIN    */
	{ OPTION_IP | OPTION_LIST                 , 0x29 }, /* DHCP_NIS_SERVER    */
	{ OPTION_IP | OPTION_LIST     | OPTION_REQ, 0x2a }, /* DHCP_NTP_SERVER    */
	{ OPTION_IP | OPTION_LIST                 , 0x2c }, /* DHCP_WINS_SERVER   */
	{ OPTION_U32                              , 0x33 }, /* DHCP_LEASE_TIME    */
	{ OPTION_IP                               , 0x36 }, /* DHCP_SERVER_ID     */
	{ OPTION_STRING                           , 0x38 }, /* DHCP_ERR_MESSAGE   */
	//TODO: must be combined with 'sname' and 'file' handling:
	{ OPTION_STRING_HOST                      , 0x42 }, /* DHCP_TFTP_SERVER_NAME */
	{ OPTION_STRING                           , 0x43 }, /* DHCP_BOOT_FILE     */
	//TODO: not a string, but a set of LASCII strings:
	{ OPTION_STRING                           , 0x4D }, /* DHCP_USER_CLASS    */
#if ENABLE_FEATURE_UDHCP_RFC3397
	{ OPTION_DNS_STRING | OPTION_LIST         , 0x77 }, /* DHCP_DOMAIN_SEARCH */
	{ OPTION_SIP_SERVERS                      , 0x78 }, /* DHCP_SIP_SERVERS   */
#endif
	{ OPTION_STATIC_ROUTES | OPTION_LIST      , 0x79 }, /* DHCP_STATIC_ROUTES */
#if ENABLE_FEATURE_UDHCP_8021Q
	{ OPTION_U16                              , 0x84 }, /* DHCP_VLAN_ID       */
	{ OPTION_U8                               , 0x85 }, /* DHCP_VLAN_PRIORITY */
#endif
	{ OPTION_STRING                           , 0xd1 }, /* DHCP_PXE_CONF_FILE */
	{ OPTION_STRING                           , 0xd2 }, /* DHCP_PXE_PATH_PREFIX */
	{ OPTION_6RD                              , 0xd4 }, /* DHCP_6RD           */
	{ OPTION_STATIC_ROUTES | OPTION_LIST      , 0xf9 }, /* DHCP_MS_STATIC_ROUTES */
	{ OPTION_STRING                           , 0xfc }, /* DHCP_WPAD          */

	/* Options below have no match in dhcp_option_strings[],
	 * are not passed to dhcpc scripts, and cannot be specified
	 * with "option XXX YYY" syntax in dhcpd config file.
	 * These entries are only used internally by udhcp[cd]
	 * to correctly encode options into packets.
	 */

	{ OPTION_IP                               , 0x32 }, /* DHCP_REQUESTED_IP  */
	{ OPTION_U8                               , 0x35 }, /* DHCP_MESSAGE_TYPE  */
	{ OPTION_U16                              , 0x39 }, /* DHCP_MAX_SIZE      */
	//looks like these opts will work just fine even without these defs:
	{ OPTION_STRING                           , 0x3c }, /* DHCP_VENDOR        */
	//	/* not really a string: */
	{ OPTION_STRING                           , 0x3d }, /* DHCP_CLIENT_ID     */
	{ 0, 0 } /* zeroed terminating entry */
};

static int dhcp_option_idx(uint8_t code)
{
	int i = 0;
	for (i = 0; i < sizeof(dhcp_optflag) / sizeof(dhcp_optflag[0]); i++)
	{
		if (dhcp_optflag[i].code == code)
			return i;
	}
	return -1;
}

int dhcp_option_flags(uint8_t code)
{
	int i = 0;
	for (i = 0; i < sizeof(dhcp_optflag) / sizeof(dhcp_optflag[0]); i++)
	{
		if (dhcp_optflag[i].code == code)
			return dhcp_optflag[i].flags;
	}
	return 0;
}

static int dhcp_option_strget(const char *const_str, char *out_str)
{
	int len = 0;
	char *p = const_str;
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



int dhcp_option_string_set(dhcp_option_set_t *option_tbl, uint8_t code,
		const char *const_str)
{
	struct dhcp_optflag *optflag;
	char str[254], *p = NULL;
	int retval = 0, length = 0, optlen = 0, offset = 0;
	uint8_t *val8 = NULL, buffer[512], optindex = 0;
	uint16_t *val16 = NULL;
	uint32_t *val32 = NULL;
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

		switch (optflag->flags & OPTION_TYPE_MASK)
		{
		case OPTION_IP:
			udhcp_str2nip(str, buffer + offset);
			length += 4;
			break;
		case OPTION_IP_PAIR:
			udhcp_str2nip(str, buffer + offset);
			length += 4;
			break;
		case OPTION_STRING:
		case OPTION_STRING_HOST:
			optlen = strnlen(str, 254);
			strncpy(buffer + offset, str, optlen);
			length += optlen;
			break;
	#if ENABLE_FEATURE_UDHCP_RFC3397
		case OPTION_DNS_STRING:
			p = (char *)dname_enc(NULL, 0, str, &optlen);
			if(p)
			{
				strncpy(buffer + offset, p, optlen);
				length += optlen;
			}
	#endif
			break;
		case OPTION_BOOLEAN:
		{
			if(strstr(str, "no"))
				buffer[offset] = 0;
			else if(strstr(str, "yes"))
				buffer[offset] = 1;
			length += 1;
		}
		break;
		case OPTION_U8:
			buffer[offset] = strtoul(str, NULL, 10);
			length += 1;
			break;
			/* htonX are macros in older libc's, using temp var
			 * in code below for safety */
			/* TODO: use bb_strtoX? */
		case OPTION_U16:
		case OPTION_S16:
		{
			val16 = buffer + offset;
			uint32_t tmp = strtoul(str, NULL, 0);
			*val16 = htons(tmp);
			length += 2;
			break;
		}
		case OPTION_U32:
		case OPTION_S32:
		{
			val32 = buffer + offset;
			uint32_t tmp = strtoul(str, NULL, 0);
			*val32 = htonl(tmp);
			length += 4;
			break;
		}
		case OPTION_BIN: /* handled in attach_option() */
			buffer[offset] = str[0];
			length += 1;
			break;
		case OPTION_STATIC_ROUTES:
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
		case OPTION_6RD: /* handled in attach_option() */
			strncpy(buffer + offset, str, optlen);
			length += 12;
			break;
	#if ENABLE_FEATURE_UDHCP_RFC3397
		case OPTION_SIP_SERVERS:
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


int dhcp_option_add(dhcp_option_set_t *option_tbl, uint8_t code, const uint8_t *opt, int len)
{
	if(code > DHCP_OPTION_MAX)
		return ERROR;

	option_tbl[code].data = malloc(len);
	if(option_tbl[code].data)
	{
		memset(option_tbl[code].data, 0, len);
		memcpy(option_tbl[code].data, opt, len);
		option_tbl[code].code = code;
		option_tbl[code].len = len;
		return OK;
	}
	return ERROR;
}

int dhcp_option_del(dhcp_option_set_t *option_tbl, uint8_t code)
{
	if(code > DHCP_OPTION_MAX)
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
	int code = 0;
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

int dhcp_option_lookup(dhcp_option_set_t *option_tbl, uint8_t code)
{
	if(code > DHCP_OPTION_MAX)
		return ERROR;
	if(option_tbl[code].len && option_tbl[code].data)
	{
		return OK;
	}
	return ERROR;
}

int dhcp_option_packet(dhcp_option_set_t *option_tbl, char *data, int len)
{
	int i = 0, offset = 0;//, j = 0;
	//u_int8 parm_lst[256];
	//offset = udhcp_end_option(data);
	offset = dhcp_option_get_end_padding(data, len);
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
			//msg->data = (uint8_t *)(p);
			//memcpy(msg->data, option_tbl[i].data, option_tbl[i].len);
			offset += OPT_DATA + option_tbl[i].len;
/*			if(dhcp_option_flags(msg->code) & OPTION_REQ)
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

/*int FAST_FUNC udhcp_end_option(uint8_t *optionptr)
{
	int i = 0;

	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] != DHCP_PADDING)
			i += optionptr[i + OPT_LEN] + OPT_DATA-1;
		i++;
	}
	return i;
}*/

static int dhcp_option_get_end_padding(char *data, int len)
{
	int offset = 0;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	int overload = dhcp_option_overload_get(data);
	if(msg->code != DHCP_END)//DHCP_PADDING
	{
		if(msg->code != DHCP_PADDING)
			offset += msg->len + OPT_DATA;
		else
			offset++;
		//return offset;
	}
	else
	{
		if(overload == 0)
			return offset;
		if(overload & FILE_FIELD)
		{
			offset += DHCP_FILE_LEN;
		}
		if(overload & SNAME_FIELD)
		{
			offset += DHCP_SNAME_LEN;
		}
	}
	//offset = msg->len + OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_hdr_t *)(data + offset);
		//msg->data = (data + offset + OPT_DATA);
		if(msg->code != DHCP_END)//DHCP_PADDING
		{
			if(msg->code != DHCP_PADDING)
				offset += msg->len + OPT_DATA;
			else
				offset++;
			//return offset;
		}
		else
		{
			if(overload & FILE_FIELD)
			{
				offset += DHCP_FILE_LEN;
				continue;
			}
			if(overload & SNAME_FIELD)
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

int dhcp_option_get_length(char *data)
{
	return dhcp_option_get_end_padding(data, DHCP_OPTIONS_BUFSIZE);
}

/* Return the position of the 'end' option (no bounds checking) */
int udhcp_end_option(uint8_t *optionptr)
{
	int i = 0;

	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] != DHCP_PADDING)
			i += optionptr[i + OPT_LEN] + OPT_DATA-1;
		i++;
	}
	return i;
}

int dhcp_option_message_type(char *data, uint8_t code)
{
	int offset = dhcp_option_get_end_padding(data, DHCP_OPTIONS_BUFSIZE);
	dhcp_option_hdr_t *msg_type = (data + offset);
	msg_type->code = DHCP_MESSAGE_TYPE;
	msg_type->len = 1;
	//msg_type->val.val8 = (uint8_t *)(data + OPT_DATA);
	msg_type->val.val8 = code;
	offset += 3;
	dhcp_option_packet_end(data + offset, DHCP_OPTIONS_BUFSIZE-offset);
	return OPT_DATA + OPT_LEN;
}


int dhcp_option_packet_set_simple(char *data, int len, uint8_t code, uint32_t value)
{
	int offset = 0;//udhcp_end_option(data);
	offset = dhcp_option_get_end_padding(data, len);
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	msg->code = code;
	msg->len = 4;
	msg->val.val32 = value;
	offset += OPT_DATA + 4;
	dhcp_option_packet_end(data + offset, len - offset);
	return offset;
}

int dhcp_option_packet_set_value(char *data, int len, uint8_t code, uint32_t oplen, uint8_t *opt)
{
	int offset = 0;//udhcp_end_option(data);
	offset = dhcp_option_get_end_padding(data, len);
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	msg->code = code;
	msg->len = oplen;
	memcpy(msg->val.pval, opt, oplen);
	offset += OPT_DATA + oplen;
	dhcp_option_packet_end(data + offset, len - offset);
	return offset;
}

int udhcp_add_simple_option(struct dhcp_packet *packet, uint8_t code, uint32_t value)
{
	return dhcp_option_packet_set_simple(packet->options, dhcp_option_get_length(packet->options), code, value);
}

int udhcp_add_simple_option_value(struct dhcp_packet *packet, uint8_t code, uint32_t oplen, uint8_t *opt)
{
	return dhcp_option_packet_set_value(packet->options, dhcp_option_get_length(packet->options), code, oplen, opt);
}

static int dhcp_option_packet_end(char *data, int len)
{
	int offset = 0;//udhcp_end_option(data);
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)(data + offset);
	msg->code = DHCP_END;
	//msg->len = option_tbl[i].len;
	return 1;
}

static int dhcp_option_overload_get(char *data)
{
	int offset = 0;
	int len = dhcp_option_get_end_padding(data, DHCP_OPTIONS_BUFSIZE);
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)data;

	if(msg->code == DHCP_OPTION_OVERLOAD)
	{
		return msg->val.val8;
	}
	offset = msg->len + OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_set_t *)(data + offset);
		if(msg->code == DHCP_OPTION_OVERLOAD)
		{
			return msg->val.val8;
		}
		offset += msg->len + OPT_DATA;
		if(offset >= len)
			return 0;
	}
	return 0;
}

uint8_t * dhcp_option_get(char *data, int len, uint8_t code, uint8_t *optlen)
{
	int /*i = 0, */offset = 0;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)data;
	int overload = dhcp_option_overload_get(data);
	if(msg->code == code)
	{
		//msg->data = data + OPT_DATA;
		if(optlen)
			*optlen = msg->len;
		return msg->val.pval;
	}
	offset = msg->len + OPT_DATA;
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
		else if(msg->code == DHCP_END)
		{
			if(overload & FILE_FIELD)
			{
				offset += DHCP_FILE_LEN;
				continue;
			}
			if(overload & SNAME_FIELD)
			{
				offset += DHCP_SNAME_LEN;
				continue;
			}
			return NULL;
		}
		offset += msg->len + OPT_DATA;

		if(offset >= len)
			return NULL;
	}
	return NULL;
}




int dhcp_option_message_type_get(char *data, int len)
{
	int /*i = 0, */offset = 0;
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)data;
	int overload = dhcp_option_overload_get(data);
	if(msg->code == DHCP_MESSAGE_TYPE)
	{
		return msg->val.val8;
	}
	offset = msg->len + OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_set_t *)(data + offset);
		if(msg->code == DHCP_MESSAGE_TYPE)
		{
/*			if(optlen)
				*optlen = msg->len;*/
			return msg->val.val8;
		}
		else if(msg->code == DHCP_END)
		{
			if(overload & FILE_FIELD)
			{
				offset += DHCP_FILE_LEN;
				continue;
			}
			if(overload & SNAME_FIELD)
			{
				offset += DHCP_SNAME_LEN;
				continue;
			}
			return 0;
		}
		offset += msg->len + OPT_DATA;
		if(offset >= len)
			return 0;
	}
	return 0;
}

int dhcp_option_get_simple(const char *data, u_int32 *output, uint8_t code, uint8_t optlen)
{
	int /*i = 0, */offset = 0;
	int len = dhcp_option_get_end_padding(data, DHCP_OPTIONS_BUFSIZE);
	dhcp_option_hdr_t *msg = (dhcp_option_hdr_t *)data;
	int overload = dhcp_option_overload_get(data);
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
	offset = msg->len + OPT_DATA;
	while(1)
	{
		msg = (dhcp_option_set_t *)(data + offset);
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
		else if(msg->code == DHCP_END)
		{
			if(overload & FILE_FIELD)
			{
				offset += DHCP_FILE_LEN;
				continue;
			}
			if(overload & SNAME_FIELD)
			{
				offset += DHCP_SNAME_LEN;
				continue;
			}
			return ERROR;
		}
		offset += msg->len + OPT_DATA;
		if(offset >= len)
			return ERROR;
	}
	return ERROR;
}

uint8_t* udhcp_get_option(struct dhcp_packet *packet, int code, int *optlen)
{
	uint8_t *optionptr = NULL;
	int len = 0;
	/* option bytes: [code][len][data1][data2]..[dataLEN] */
	optionptr = packet->options;
	//len = sizeof(packet->options);
	len = dhcp_option_get_end_padding(optionptr, DHCP_OPTIONS_BUFSIZE);
	return dhcp_option_get(optionptr,  len,  code, optlen);
}
