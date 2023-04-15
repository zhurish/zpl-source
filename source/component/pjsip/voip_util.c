/*
 * voip_util.c
 *
 *  Created on: Dec 8, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"


#include "pjsip_app_api.h"

#include "voip_util.h"




#if 0
int phone_string_to_hex(char * room, zpl_uint8 *phone)
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

int phone_string_to_compress(char * room, zpl_uint8 *phone)
{
	zassert(room != NULL);
	int i = 0, j = 0;
	zpl_uint8 phonetmp[64];
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

int phone_compress_to_uncompress(zpl_uint8 *phonetmp, int len, zpl_uint8 *phone)
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
