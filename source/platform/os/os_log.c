/*
 * os_log.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"


static oslog_callback _oslog_func = NULL;

int os_loghex(zpl_char *format, zpl_uint32 size, const zpl_uchar *data, zpl_uint32 len)
{
	zpl_uint32 i = 0, offset = 0;
	for(i = 0; i < len; i++)
	{
		if((i+1)%16 == 0)
			offset += snprintf(format + offset, size - offset, "\r\n");
		if((size - offset) < 5 )
			offset += snprintf(format + offset, size - offset, "0x%02x ", (zpl_uchar)data[i]);
		else
			return ++i;
	}
	return ++i;
}


void os_vflog(FILE *fp, zpl_char *herd, zpl_char *func, int line, const zpl_char *format, ...)
{
	int len = 0;
	char buftmp[2048];
	va_list args;
	FILE *pfp = fp;
	if(fp == NULL)
		pfp = stdout;
	memset(buftmp, 0, sizeof(buftmp));
	va_start(args, format);

	if(herd)
		len += sprintf(buftmp + len, "%s", herd);
	if(func)
	{
		len += sprintf(buftmp + len, "(%s:%d)", func, line);
	}
	len += sprintf(buftmp + len, "(%s)", os_task_self_name_alisa());
	len += vsprintf(buftmp + len, format, args);
	va_end(args);

	fprintf(pfp, "%s\r\n",buftmp);
	fflush(pfp);
}



static int os_log_file_printf(zpl_char *func, int line, FILE *fp, const zpl_char *buf, va_list args)
{
	if(fp)
	{
		if(func)
		{
			fprintf(fp, "(%s:%d)", func, line);
		}	
		vfprintf(fp, buf, args);
		fprintf(fp, "\n");
		fflush(fp);
	}
	return OK;
}

void os_log_entry(zpl_char *func, int line, zpl_char *file, const zpl_char *format, ...)
{
	FILE *fp = fopen(os_netservice_sockpath_get(file), "a+");
	if(fp)
	{
		va_list args;
		va_start(args, format);
		os_log_file_printf(func, line, fp, format, args);
		va_end(args);
		fclose(fp);
	}
}

int os_fdprintf(zpl_char *func, int line, int fd,const zpl_char *format, ...)
{
	va_list args;
	zpl_char buf[1024];
	zpl_int32 len = 0;
	memset(buf, 0, sizeof(buf));
	va_start(args, format);
	if(func)
	{
		len += sprintf(buf + len, "(%s:%d)", func, line);
	}	
	len += vsnprintf(buf + len, sizeof(buf) - len, format, args);
	va_end(args);

	if(len <= 0)
		return ERROR;
	return write(fd, buf, len);
}

void os_log_func(zpl_char *func, int line, int pri, const zpl_char *format, ...)
{
	if(_oslog_func)
	{
		va_list args;
		zpl_char buf[1024];
		zpl_int32 len = 0;
		memset(buf, 0, sizeof(buf));
		va_start(args, format);
		if(func)
		{
			len += sprintf(buf + len, "(%s:%d)", func, line);
		}
		len += vsnprintf(buf + len, sizeof(buf) - len, format, args);
		va_end(args);
		(_oslog_func)(pri, buf);
	}
}

void os_log_init(oslog_callback func)
{
	_oslog_func = func;
}
