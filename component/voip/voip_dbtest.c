/*
 * voip_dbtest.c
 *
 *  Created on: 2019年1月8日
 *      Author: DELL
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
#include "eloop.h"
#include "network.h"
#include "vty.h"

#include "voip_def.h"
#include "voip_task.h"
#include "voip_event.h"
#include "voip_app.h"
#include "voip_api.h"
#include "voip_sip.h"
//#include "estate_mgt.h"
#include "application.h"
#include "voip_stream.h"
#include "voip_dbtest.h"





static room_phone_dbtest_t _phone_dbtest[VOIP_ROOM_PHONE_DBTEST_MAX + 2];


static BOOL _v_dbtest = 1;


int voip_dbtest_enable(BOOL enable)
{
	_v_dbtest = enable;
	return OK;
}


BOOL voip_dbtest_isenable()
{
	return _v_dbtest;
}



int voip_dbtest_add(room_phone_dbtest_t one)
{
	int i = 0;
	for(i = 0; i <= VOIP_ROOM_PHONE_DBTEST_MAX; i++)
	{
		if(!_phone_dbtest[i].use)
		{
			memset(&_phone_dbtest[i], 0, sizeof(room_phone_dbtest_t));
			_phone_dbtest[i].use = 1;
			_phone_dbtest[i].room = one.room;
			memcpy(_phone_dbtest[i].phone, one.phone, sizeof(one.phone));
			return OK;
		}
	}
	return ERROR;
}

int voip_dbtest_del(room_phone_dbtest_t one)
{
	int i = 0;
	for(i = 0; i <= VOIP_ROOM_PHONE_DBTEST_MAX; i++)
	{
		if(_phone_dbtest[i].use &&
			_phone_dbtest[i].room == one.room)
		{
			memset(&_phone_dbtest[i], 0, sizeof(room_phone_dbtest_t));
			return OK;
		}
	}
	return ERROR;
}

int voip_dbtest_lookup(char *room, room_phone_dbtest_t *out)
{
	int i = 0;
	for(i = 0; i <= VOIP_ROOM_PHONE_DBTEST_MAX; i++)
	{
		if(_phone_dbtest[i].use &&
			_phone_dbtest[i].room == string_to_hex(room))
		{
			memcpy(out, &_phone_dbtest[i], sizeof(room_phone_dbtest_t));
			return OK;
		}
	}
	return ERROR;
}

int voip_dbtest_getphone(char *room, char *phone)
{
	int i = 0;
	for(i = 0; i <= VOIP_ROOM_PHONE_DBTEST_MAX; i++)
	{
		if(_phone_dbtest[i].use &&
			_phone_dbtest[i].room == string_to_hex(room))
		{
			if(phone)
				strcpy(phone, _phone_dbtest[i].phone);
			return OK;
		}
	}
	return ERROR;
}

int voip_dbtest_getremote(char *room, char *address, u_int16 *port)
{
	int i = 0;
	for(i = 0; i <= VOIP_ROOM_PHONE_DBTEST_MAX; i++)
	{
		if(_phone_dbtest[i].use &&
			_phone_dbtest[i].room == string_to_hex(room))
		{
			if(address)
				strcpy(address, _phone_dbtest[i].address);
			if(port)
				*port = _phone_dbtest[i].port;
			return OK;
		}
	}
	return ERROR;
}

int voip_dbtest_show(struct vty *vty, int detail)
{
	int i = 0, cnt = 0;
	char room[64];//, phone[64];
	memset(room, 0, sizeof(room));
	//memset(phone, 0, sizeof(phone));
	for(i = 0; i <= VOIP_ROOM_PHONE_DBTEST_MAX; i++)
	{
		if(_phone_dbtest[i].use)
		{
			if(cnt==0)
			{
				vty_out(vty, " %-8s %-12s %s",  "--------", "------------", VTY_NEWLINE);
				vty_out(vty, " %-8s %-12s %s",  "ROOM", "Phone", VTY_NEWLINE);
				vty_out(vty, " %-8s %-12s %s",  "--------", "------------", VTY_NEWLINE);
				cnt = 1;
			}
			snprintf(room, sizeof(room), "%x", _phone_dbtest[i].room);
			vty_out(vty, " %-8s %-12s %s",  room, _phone_dbtest[i].phone, VTY_NEWLINE);
		}
	}
	return OK;
}

static int voip_dbtest_add_sync(char *room, char *phone, char *address, int port)
{
	room_phone_dbtest_t one;
	memset(&one, 0, sizeof(one));
	one.room = string_to_hex(room);

	//one.plen = phone_string_to_compress(phone, one.phone);
	one.plen = phone_string_to_hex(phone, one.phone);

	os_strcpy(one.address, address);
	one.port = port;

	return voip_dbtest_add(one);
}


static int voip_dbtest_phone_load(FILE *fp)
{
	int cnt = 0;
	char buf[512];
	char room[64], phone[64], address[64];
	if (fp)
	{
		// 8001:13922360951:192.168.1.1:6666
		char *s = NULL, *p = NULL;
		os_memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), fp))
		{
			os_memset(room, 0, sizeof(room));
			os_memset(phone, 0, sizeof(phone));
			os_memset(address, 0, sizeof(address));
			s = strstr(buf, ":");
			if(s)
			{
				os_memcpy(room, buf, s - buf);//8001
				s++;
				p = strstr(s, ":");
				if(p)
				{
					os_strncpy(phone, s, p-s);//13922360951
				}
				s = p+1;
				p = strstr(s, ":");
				if(p)
				{
					os_strncpy(address, s, p-s);//192.168.1.1
				}
				p++;
				if(os_strlen(room) && os_strlen(phone)&& os_strlen(address) && p)
				{
					cnt += (voip_dbtest_add_sync(room, phone, address, atoi(p)) == OK) ? 1:0;
				}
			}
			else
				break;
		}
	}
	return cnt;
}


int voip_dbtest_load()
{
	FILE *fp = fopen(VOIP_ROOM_PHONE_DBTEST, "r");
	if (fp)
	{
		if(voip_dbtest_phone_load(fp) > 0)
			_v_dbtest = TRUE;
		fclose(fp);
	}
	return 0;
}
