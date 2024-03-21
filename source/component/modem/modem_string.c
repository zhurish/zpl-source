/*
 * modem_string.c
 *
 *  Created on: Aug 4, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zpl_type.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_driver.h"

#include "modem_string.h"







const char * strchr_empty(char *src, const char em)
{
	char *p = src;
	assert(src);
	atcmd_response_t response;
	zpl_uint32 i = 0, j = 0, count = os_strlen(src);
	os_memset(&response, '\0', sizeof(response));
	response.len = count;

	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			;//response.buf[i++] = *p;
		}
		else
			response.buf[j++] = p[i];
	}

	os_memset(src, 0, response.len);
	response.len = j;
	os_memcpy(src, response.buf, response.len);
	src[response.len] = '\0';
	return (char*)src;
}

const char * strchr_empty_step(char *src, const char em, zpl_uint32 step)
{
	char *p = src;
	assert(src);
	atcmd_response_t response;
	zpl_uint32 i = 0, j = 0, k = 0, count = os_strlen(src);
	os_memset(&response, 0, sizeof(response));
	response.len = count;

	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			//response.buf[i++] = *p;
			k++;
			if(k == step)
				;
			else
				response.buf[j++] = p[i];
		}
		else
			response.buf[j++] = p[i];
	}

	os_memset(src, 0, response.len);
	response.len = j;
	os_memcpy(src, response.buf, response.len);
	return (char*)src;
}
