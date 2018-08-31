/*
 * modem_string.c
 *
 *  Created on: Aug 4, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"

#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_driver.h"

#include "modem_string.h"







const char * strchr_empty(char *src, const char em)
{
	char *p = src;
	assert(src);
	atcmd_response_t response;
	int i = 0, j = 0, count = os_strlen(src);
	os_memset(&response, 0, sizeof(response));
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
	return (char*)src;
}

const char * strchr_empty_step(char *src, const char em, int step)
{
	char *p = src;
	assert(src);
	atcmd_response_t response;
	int i = 0, j = 0, k = 0, count = os_strlen(src);
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

int strchr_count(char *src, const char em)
{
	char *p = src;
	assert(src);
	int i = 0, j = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			j++;
		}
	}
	return j;
}

int strchr_step(char *src, const char em, int step)
{
	char *p = src;
	assert(src);
	int i = 0, j = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			j++;
			if(j == step)
				break;
		}
	}
	return (i < count)? i:0;
}



char *os_strstr_last(const char *dest,const char *src)
{
	const char *ret=NULL;
	static char *last = NULL;
	assert(dest);
	assert(src);
	if(*src == '\0')
		return (char *)dest;
	while((ret = os_strstr(dest,src)))
	{
		last=ret;
		dest=ret+1;
	}
	return (char *)last;
}


int buffer_isempty(char *dest, int len)
{
	char buf[2048];
	os_memset(buf, 0, sizeof(buf));
	if(os_memcmp(buf, dest, MIN(len, sizeof(buf))) == 0)
		return 1;
	return 0;
}
