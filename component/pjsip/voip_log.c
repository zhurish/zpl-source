/*
 * voip_log.c
 *
 *  Created on: Mar 16, 2019
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
#include "eloop.h"
#include "network.h"
#include "vty.h"


#include "voip_log.h"



#define VOIP_THLOG_FILE	SYSCONFDIR"/thlog.log"

struct thlog_s
{
	FILE 	*thlogfp;
	int		thlog_max;
	int		thlog_index;
};

static struct thlog_s  *thlog_db = NULL;

/*static int voip_thlog_log_file_printf(FILE *fp, const char *buf, va_list args)
{
	if(fp)
	{
		time_print(fp, ZLOG_TIMESTAMP_DATE);
		vfprintf(fp, buf, args);
		fprintf(fp, "\n");
		fflush(fp);
	}
	return OK;
}*/

/*static void voip_thlog_log(struct thlog_s *thlog_db, const char *format, ...)
{
	if(thlog_db->thlogfp)
	{
		va_list args;
		va_start(args, format);
		voip_thlog_log_file_printf(thlog_db->thlogfp, format, args);
		//vzlog(zlog_default, module, priority, format, args);
		va_end(args);
	}
}*/

int voip_thlog_init()
{
	if(!thlog_db)
	{
		thlog_db = malloc(sizeof(struct thlog_s));
		memset(thlog_db, 0, sizeof(struct thlog_s));
	}
	//remove(VOIP_THLOG_FILE);
	//load config
	if(thlog_db->thlogfp == NULL)
		thlog_db->thlogfp = fopen(VOIP_THLOG_FILE, "a+");

#ifdef PL_OPENWRT_UCI
	os_uci_get_integer("product.global.thlog_max", &thlog_db->thlog_max);
	//os_uci_get_integer("product.global.thlog_index", &thlog_db->thlog_index);
#endif
	//zlog_debug(ZLOG_VOIP, "=================%d %d", thlog_db->thlog_max, thlog_db->thlog_index);
	return OK;
}

int voip_thlog_reload()
{
	if(!thlog_db)
		return ERROR;
	if(thlog_db->thlogfp)
	{
		fclose(thlog_db->thlogfp);
		thlog_db->thlogfp = NULL;
	}
	if(thlog_db->thlogfp == NULL)
		thlog_db->thlogfp = fopen(VOIP_THLOG_FILE, "a+");
	return OK;
}

int voip_thlog_close()
{
	if(!thlog_db)
		return ERROR;
	if(thlog_db->thlogfp)
	{
		fflush(thlog_db->thlogfp);
		fclose(thlog_db->thlogfp);
		thlog_db->thlogfp = NULL;
	}
	return OK;
}

int voip_thlog_clean()
{
	if(!thlog_db)
		return ERROR;
	if(thlog_db->thlogfp)
	{
		fflush(thlog_db->thlogfp);
		fclose(thlog_db->thlogfp);
		thlog_db->thlogfp = NULL;
		remove(VOIP_THLOG_FILE);
		sync();
	}
	else
	{
		remove(VOIP_THLOG_FILE);
		sync();
	}
	return OK;
}

static int voip_thlog_check()
{
	if(!thlog_db)
		return ERROR;
	if(access(VOIP_THLOG_FILE, F_OK) != 0)
	{
		if(thlog_db->thlogfp)
		{
			fclose(thlog_db->thlogfp);
			thlog_db->thlogfp = NULL;
		}
		if(thlog_db->thlogfp == NULL)
			thlog_db->thlogfp = fopen(VOIP_THLOG_FILE, "a+");
	}
	return OK;
}


int voip_thlog_log(const char *format, ...)
{
	if(!thlog_db)
		return ERROR;
	if(thlog_db->thlogfp)
	{
		va_list args;
		voip_thlog_check();
		if(thlog_db->thlog_index >= thlog_db->thlog_max)
		{
			thlog_db->thlog_index = 0;
			fseek(thlog_db->thlogfp, 0, SEEK_SET);
		}
		va_start(args, format);
		time_print(thlog_db->thlogfp, ZLOG_TIMESTAMP_DATE);
		vfprintf(thlog_db->thlogfp, format, args);
		fprintf(thlog_db->thlogfp, "\n");
		va_end(args);
		fflush(thlog_db->thlogfp);
		thlog_db->thlog_index++;
		return OK;
	}
	return ERROR;
}

int voip_thlog_log1(u_int8 building, u_int8 unit, u_int16 room, char *id,
					char *phone, const char *format, ...)
{
	if(!thlog_db)
		return ERROR;
	if(thlog_db->thlogfp)
	{
		// time : type: result: ID
		va_list args;
		voip_thlog_check();
		if(thlog_db->thlog_index >= thlog_db->thlog_max)
		{
			thlog_db->thlog_index = 0;
			fseek(thlog_db->thlogfp, 0, SEEK_SET);
		}
		va_start(args, format);
		time_print(thlog_db->thlogfp, ZLOG_TIMESTAMP_DATE);
		if(id)
			fprintf(thlog_db->thlogfp, ": type=%s result=%s ID:%s, Room->%d#%d@%d, phone->%s,",
					"Phone", "OK", id, building, unit, room, phone);
		else
			fprintf(thlog_db->thlogfp, ": type=%s result=%s ID: Room->%d#%d@%d, phone->%s,",
					"Phone", "OK", building, unit, room, phone);
		//fprintf(thlog_db->thlogfp, "Pbuilding:%d unit:%d room:%d phone:%s", building, unit, room, phone);
		vfprintf(thlog_db->thlogfp, format, args);
		fprintf(thlog_db->thlogfp, "\n");
		va_end(args);
		fflush(thlog_db->thlogfp);
		thlog_db->thlog_index++;
		return OK;
	}
	return ERROR;
}


int voip_thlog_log2(const char *format, ...)
{
	if(!thlog_db)
		return ERROR;
	if(thlog_db->thlogfp)
	{
		va_list args;
		voip_thlog_check();
		if(thlog_db->thlog_index >= thlog_db->thlog_max)
		{
			thlog_db->thlog_index = 0;
			fseek(thlog_db->thlogfp, 0, SEEK_SET);
		}
		va_start(args, format);
		vfprintf(thlog_db->thlogfp, format, args);
		fprintf(thlog_db->thlogfp, "\n");
		va_end(args);
		fflush(thlog_db->thlogfp);
		thlog_db->thlog_index++;
		return OK;
	}
	return ERROR;
}

int voip_thlog_log3(char *type, char *result, const char *format, ...)
{
	if(!thlog_db)
		return ERROR;
	if(thlog_db->thlogfp)
	{
		// time : type: result: ID
		va_list args;
		voip_thlog_check();
		if(thlog_db->thlog_index >= thlog_db->thlog_max)
		{
			thlog_db->thlog_index = 0;
			fseek(thlog_db->thlogfp, 0, SEEK_SET);
		}
		va_start(args, format);
		time_print(thlog_db->thlogfp, ZLOG_TIMESTAMP_DATE);
		fprintf(thlog_db->thlogfp, ": type=%s result=%s ID:", type, result);
		vfprintf(thlog_db->thlogfp, format, args);
		fprintf(thlog_db->thlogfp, "\n");
		va_end(args);
		fflush(thlog_db->thlogfp);
		thlog_db->thlog_index++;
		return OK;
	}
	return ERROR;
}

int voip_thlog_log4(const time_t ti, char *type, char *result, const char *format, ...)
{
	if(!thlog_db)
		return ERROR;
	if(thlog_db->thlogfp)
	{
		// time : type: result: ID
		va_list args;
		voip_thlog_check();
		if(thlog_db->thlog_index >= thlog_db->thlog_max)
		{
			thlog_db->thlog_index = 0;
			fseek(thlog_db->thlogfp, 0, SEEK_SET);
		}
		fprintf(thlog_db->thlogfp, "%s ", os_time_fmt("/",ti));
		va_start(args, format);
		fprintf(thlog_db->thlogfp, ": type=%s result=%s ID:", type, result);
		vfprintf(thlog_db->thlogfp, format, args);
		fprintf(thlog_db->thlogfp, "\n");
		va_end(args);
		fflush(thlog_db->thlogfp);
		thlog_db->thlog_index++;
		return OK;
	}
	return ERROR;
}
