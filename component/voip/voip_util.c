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
/*
#include <netinet/in.h>
#include <arpa/inet.h>
*/

#include "voip_def.h"
#include "voip_util.h"


const char *inet_address(u_int32 ip)
{
	static char buf[64];
	memset(buf, 0, sizeof(buf));
	struct in_addr address;
	address.s_addr = htonl(ip);
	//return inet_ntoa(address);
	snprintf(buf, sizeof(buf), "%s", inet_ntoa(address));
	return buf;
}

u_int32 string_to_hex(char * room)
{
	return strtol(room, NULL, 16);
}

char * hex_to_string(u_int32 hex)
{
	static char buf[64];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%x", hex);
	return buf;
}

u_int8 atoascii(int a)
{
	return ((a) - 0x30);
}

int phone_string_to_hex(char * room, u_int8 *phone)
{
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

#if 0
int voip_test()
{
	int n = 0, i = 0;
	u_int8 phone[64];
	u_int8 phonetmp[6] = {0x01, 0x59, 0x22, 0x36, 0x09, 0x51};
	memset(phone, 0, sizeof(phone));
	printf("=====%s atoascii %d\n", __func__, atoascii('6'));

	printf("=====%s string_to_hex %x\n", __func__, string_to_hex("6032"));

	printf("=====%s string_to_hex %s\n", __func__, hex_to_string(0x6032));

	n = phone_string_to_hex("13922360951", phone);
	printf("=====%s phone_string_to_hex: ", __func__);
	for(i = 0; i < n; i++)
	{
		printf("%d", phone[i]);
	}
	printf("\n");

	memset(phone, 0, sizeof(phone));
	n = phone_string_to_compress("13922360951", phone);
	printf("=====%s phone_string_to_compress: ", __func__);
	for(i = 0; i < n; i++)
	{
		printf("%x", phone[i]);
	}
	printf("\n");

	memset(phone, 0, sizeof(phone));
	n = phone_compress_to_uncompress(phonetmp, 6, phone);
	printf("=====%s phone_compress_to_uncompress: ", __func__);
	for(i = 0; i < n; i++)
	{
		printf("%d", phone[i]);
	}
	printf("\n");
	return 0;
}
#endif
