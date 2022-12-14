/*
 * plog.c
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include <sys/stat.h>
#include "process.h"


//#undef ZPL_TOOLS_PROCESS
#ifdef ZPL_TOOLS_PROCESS
int debug = LOG_WARNING;
static FILE * logfp = NULL;
char *logfile = NULL;

static const char *process_log_priority[] = { "emergencies", "alerts", "critical", "errors",
		"warnings", "notifications", "informational", "debugging", "trapping", NULL, };




int open_log(char *file)
{
	char path[256];
	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s", PROCESS_LOG_BASE, file);
	logfp = fopen(path, "a+");
	return OK;
}

static int plog_timestamp(char *buf, size_t buflen)
{

	zpl_time_t ticlock;
	struct tm tm;
	char data[128];
	int len = 0;
	ticlock = time(NULL);
	os_memset(data, 0, sizeof(data));

	//tm = localtime(&ticlock);

	if(os_tmtime_get (OS_TMTIME_LOCAL, ticlock, &tm) != OK)
	{
		return 0;
	}
	len = strftime(data, sizeof(data), "%Y/%m/%d %H:%M:%S",&tm);

	if(buf)
	{
		os_memcpy(buf, data, MIN(buflen, len));
		if (buflen > len)
			buf[len] = '\0';
		return MIN(buflen, len);
	}
	return 0;
}

/* Utility routine for current time printing. */
static void plog_time_print(FILE *fp)
{
	char data[128];
	os_memset(data, 0, sizeof(data));
	if(plog_timestamp(data, sizeof(data)))
		fprintf(fp ? fp:stdout, "%s ", data);
}


void process_log_print(int priority, const char *func, int line, const char *format,...)
{
	/* When zlog_default is also NULL, use stderr for logging. */

	if(logfile)
	{
		char path[256];
		os_memset(path, 0, sizeof(path));
		os_snprintf(path, sizeof(path), "%s/%s", PROCESS_LOG_BASE, logfile);
		if(access(path, 0) != 0)
		{
			if(logfp)
				fclose(logfp);
			logfp = fopen(path, "a+");
		}
	}


	if (logfp == NULL)
	{
		va_list ac;
		va_start(ac, format);
		plog_time_print(NULL);
		fprintf(stdout, "%s: ", "process");
#ifdef _PROCESS_DEBUG
		fprintf(stdout, "%s %d", func, line);
#endif
		vfprintf(stdout, format, ac);
		va_end(ac);
		fprintf(stdout, "\n");
		fflush(stdout);
	}

	/* File output. */
	if (priority <= debug && logfp)
	{
		va_list ac;
		plog_time_print(logfp);
		fprintf(logfp, "%s: ", process_log_priority[priority]);
		fprintf(logfp, "%s: ", PROCESS_MGT_UNIT_NAME);
#ifdef _PROCESS_DEBUG
		fprintf(stdout, "%s %d", func, line);
#endif
		va_start(ac, format);
		vfprintf(logfp, format, ac);
		va_end(ac);
		fprintf(logfp, "\n");
		fflush(logfp);
	}
}




#endif
