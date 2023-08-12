/*
 * pjsua_app_cb.c
 *
 *  Created on: Jun 22, 2019
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "pjsip_util.h"



static voip_payload_t _voip_payload_table[] =
{
	{"PCMU", 				1, "pcmu", 			1},
	{"PCMA", 				1, "pcma", 			2},
	{"l016/8000", 			0, "l016", 			3},
	{"lpc/8000", 			0, "lpc", 			4},
	{"G729/8000", 			0, "g729", 			5},
	{"G7231/8000", 			0, "g7231", 		6},
	{"G7221/8000", 			0, "g7221", 		7},
	{"G726-40/8000", 		0, "g726-40", 		8},
	{"G726-32/8000", 		0, "g726-32", 		9},
	{"G726-24/8000", 		0, "g726-24", 		10},
	{"G726-16/8000", 		0, "g726-16", 		11},
	{"AAL2-G726-40/8000", 	0, "aal2-g726-40", 	12},
	{"AAL2-G726-32/8000", 	0, "aal2-g726-32", 	13},
	{"AAL2-G726-24/8000", 	0, "aal2-g726-24", 	14},
	{"AAL2-G726-16/8000", 	0, "aal2-g726-16", 	15},

	{"speex/8000", 			1, "speex-nb", 		16},
	{"speex/16000", 		0, "speex-wb", 		17},
	{"iLBC/8000", 			1, "ilbc", 			18},
	{"l015/8000", 			0, "l015", 			19},
	{"AMR/8000", 			0, "amr-nb", 		20},
	{"G722", 			    1, "g722", 			21},
	{"GSM", 				1, "gsm", 			22},
	{"opus/8000", 			0, "opus", 			23},
	{"custom/8000", 		0, "custom", 		24},
};


int codec_payload_index(char *cmdname)
{
	int i = 0;
	char tmp[PJSIP_NUMBER_MAX];
	zassert(cmdname != NULL);
	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, cmdname);
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].active > 0)
		{
			if(strncasecmp(tmp, _voip_payload_table[i].cmdname, PJSIP_NUMBER_MAX) == 0)
			{
				return (int)_voip_payload_table[i].index;
			}
		}
	}
	return -1;
}

const char * codec_payload_name(int index)
{
	int i = 0;
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].active >= 0 && _voip_payload_table[i].index == index)
		{
			return _voip_payload_table[i].payload_name;
		}
	}
	return NULL;
}

const char * codec_cmdname(int index)
{
	int i = 0;
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].active >= 0 && _voip_payload_table[i].index == index)
		{
			return _voip_payload_table[i].cmdname;
		}
	}
	return "Unknown";
}
/*************************************************************************/
/*************************************************************************/

int voip_get_address(ifindex_t ifindex)
{
	struct interface * ifp = (struct interface *)if_lookup_by_index (ifindex);
	if(ifp)
	{
		
		struct prefix address;
		if(nsm_interface_address_get_api(ifp, &address) == OK)
		{
			//zlog_debug(MODULE_VOIP, "============ %s :%s -> %s = %x", __func__, ifp->name, ifp->k_name, address.u.prefix4.s_addr);
			if(address.family == IPSTACK_AF_INET)
				return ntohl(address.u.prefix4.s_addr);
		}
		//zlog_debug(MODULE_VOIP, "============ %s :%s -> %s = %x", __func__, ifp->name, ifp->k_name, address.u.prefix4.s_addr);
	}
	return 0;
}