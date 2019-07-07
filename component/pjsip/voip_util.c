/*
 * voip_util.c
 *
 *  Created on: Dec 8, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "if.h"
#include "interface.h"

#include "pjsip_app_api.h"

#include "voip_util.h"



static voip_payload_t _voip_payload_table[] =
{
	{"0 PCMU/8000", 			20, "PCMU", 			0},
	{"8 PCMA/8000", 			-1, "PCMA", 			8},
	{"1 l016/8000", 			-1, "L016", 			1},
	{"7 lpc/8000", 				20, "LPC", 				7},
	{"18 G729/8000", 			-1, "G729", 			18},
	{"4 G7231/8000", 			-1, "G7231", 			4},
	{"18 G7221/8000", 			-1, "G7221", 			-1},
	{"18 G726-40/8000", 		-1, "G726-40", 			-1},
	{"18 G726-32/8000", 		-1, "G726-32", 			-1},
	{"18 G726-24/8000", 		-1, "G726-24", 			-1},
	{"18 G726-16/8000", 		-1, "G726-16", 			-1},
	{"18 AAL2-G726-40/8000", 	-1, "AAL2-G726-40", 	-1},
	{"18 AAL2-G726-32/8000", 	-1, "AAL2-G726-32", 	-1},
	{"18 AAL2-G726-24/8000", 	-1, "AAL2-G726-24", 	-1},
	{"18 AAL2-G726-16/8000", 	-1, "AAL2-G726-16", 	-1},

	{"112 speex/8000", 			-1, "SPEEX-NB", 		110},
	{"112 speex/16000", 		-1, "SPEEX-WB", 		111},
	{"112 iLBC/8000", 			30, "iLBC", 			112},
	{"115 l015/8000", 			-1, "L015", 			115},
	{"113 AMR/8000", 			-1, "AMR", 				113},
	{"9 G722/8000", 			-1, "G722", 			9},
	{"105 opus/8000", 			-1, "OPUS", 			105},
	{"114 custom/8000", 		-1, "CUSTOM", 			114},
};


/*int voip_sip_get_payload_index(void)
{
	zassert(pl_pjsip != NULL);
	//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d", __func__, sip_config->payload);
	return pl_pjsip->payload;
}*/

int voip_sip_payload_index(char *name)
{
	int i = 0;
	char tmp[PJSIP_NUMBER_MAX];
	zassert(name != NULL);
	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, name);
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].payload >= 0)
		{
			if(strncasecmp(tmp, _voip_payload_table[i].name, PJSIP_NUMBER_MAX) == 0)
			{
				//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d %s", __func__, _voip_payload_table[i].payload, tmp);
				return (int)_voip_payload_table[i].payload;
			}
		}
	}
	return -1;
}

char * voip_sip_payload_name(int index)
{
	int i = 0;
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].payload >= 0 && _voip_payload_table[i].payload == index)
		{
			//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d %s", __func__, _voip_payload_table[i].payload,
			//		_voip_payload_table[i].name);
			return _voip_payload_table[i].name;
		}
	}
	return NULL;
}

char * voip_sip_payload_rtpmap(int index)
{
	int i = 0;
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].payload >= 0 && _voip_payload_table[i].payload == index)
		{
			//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d %s rtpmap=%s", __func__, _voip_payload_table[i].payload,
			//		_voip_payload_table[i].name, _voip_payload_table[i].rtpmap);
			return _voip_payload_table[i].rtpmap;
		}
	}
	return NULL;
}

int voip_sip_payload_ptime(int index)
{
	int i = 0;
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].payload >= 0 && _voip_payload_table[i].payload == index)
		{
			//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d %s ptime=%d", __func__, _voip_payload_table[i].payload,
			//		_voip_payload_table[i].name, _voip_payload_table[i].ptime);
			return (int)_voip_payload_table[i].ptime;
		}
	}
	return -1;
}
/*************************************************************************/
/*************************************************************************/

u_int32 voip_get_address(u_int32 ifindex)
{
	struct interface * ifp = if_lookup_by_index (ifindex);
	if(ifp)
	{
		struct prefix address;
		if(nsm_interface_address_get_api(ifp, &address) == OK)
		{
			zlog_debug(ZLOG_VOIP, "============ %s :%s -> %s = %x", __func__, ifp->name, ifp->k_name, address.u.prefix4.s_addr);
			if(address.family == AF_INET)
				return ntohl(address.u.prefix4.s_addr);
		}
		zlog_debug(ZLOG_VOIP, "============ %s :%s -> %s = %x", __func__, ifp->name, ifp->k_name, address.u.prefix4.s_addr);
	}
	return 0;
}

#if 0
int phone_string_to_hex(char * room, u_int8 *phone)
{
	zassert(room != NULL);
	int i = 0, ln = strlen(room);
	char *src = room;
	while(ln)
	{
		phone[i++] = atoascii(*src);
		src++;
		ln--;
	}
	return i;
}

int phone_string_to_compress(char * room, u_int8 *phone)
{
	zassert(room != NULL);
	int i = 0, j = 0;
	u_int8 phonetmp[64];
	memset(phonetmp, 0, sizeof(phonetmp));
	int n = phone_string_to_hex(room, phonetmp);
	if(n & 0x01)
	{
		phone[j++] = (phonetmp[0] & 0x0f);
		n -= 1;
		for(i = 1; i < n; i+=2)
		{
			phone[j++] = ((phonetmp[i] & 0x0f)<<4)|(phonetmp[i+1] & 0x0f);
		}
	}
	else
	{
		for(i = 0; i < n; i+=2)
		{
			phone[j++] = ((phonetmp[i] & 0x0f)<<4)|(phonetmp[i+1] & 0x0f);
		}
	}
	return j;
}

int phone_compress_to_uncompress(u_int8 *phonetmp, int len, u_int8 *phone)
{
	int i = 0, j = 0;
	if(len & 0x01)
	{
		phone[j++] = (phonetmp[0] & 0xf0)>>4;
		if(phone[0] == 0)
			j = 0;
		phone[j++] = (phonetmp[0] & 0x0f);

		len -= 1;
		for(i = 1; i < len; i++)
		{
			phone[j++] = (phonetmp[i] & 0xf0)>>4;
			phone[j++] = (phonetmp[i] & 0x0f);
		}
	}
	else
	{
		for(i = 0; i < len; i++)
		{
			phone[j++] = (phonetmp[i] & 0xf0)>>4;
			if(phone[0] == 0)
				j = 0;
			phone[j++] = (phonetmp[i] & 0x0f);
		}
	}
	return j;
}

#endif
