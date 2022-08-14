/*
 * Logging of zebra
 * Copyright (C) 1997, 1998, 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#define ROUTE_DEFINE_DESC_TABLE

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "log.h"
#include "host.h"
#include "route_types.h"
#include "vty.h"
#ifndef SUNOS_5
#include <sys/un.h>
#endif


/* for printstack on solaris */
#ifdef HAVE_UCONTEXT_H
#include <ucontext.h>
#endif


static int logfile_fd = -1; /* Used in signal handler. */

struct zlog *zlog_default = NULL;



//static const char *zlog_priority[] = { "emergencies", "alerts", "critical", "errors",
//		"warnings", "notifications", "informational", "debugging", "trapping", "ftrapping", NULL, };
//#define ZLOG_PRI_FMT	"%-8s"
//#define ZLOG_PRO_FMT	"%-8s"
#define ZLOG_PRI_FMT	"%s"
#define ZLOG_PRO_FMT	"%s"
static const char *zlog_priority[] = { "emerg", "alert", "crit", "err",
		"warning", "notice", "info", "debug", "trapping", "focetrap", NULL, };

static const struct facility_map {
	zpl_uint32 facility;
	const char *name;
	zpl_size_t match;
} syslog_facilities[] = { { LOG_KERN, "kern", 1 }, { LOG_USER, "user", 2 }, {
		LOG_MAIL, "mail", 1 }, { LOG_DAEMON, "daemon", 1 }, { LOG_AUTH, "auth",
		1 }, { LOG_SYSLOG, "syslog", 1 }, { LOG_LPR, "lpr", 2 }, { LOG_NEWS,
		"news", 1 }, { LOG_UUCP, "uucp", 2 }, { LOG_CRON, "cron", 1 },
#ifdef LOG_FTP
		{	LOG_FTP, "ftp", 1},
#endif
		{ LOG_LOCAL0, "local0", 6 }, { LOG_LOCAL1, "local1", 6 }, { LOG_LOCAL2,
				"local2", 6 }, { LOG_LOCAL3, "local3", 6 }, { LOG_LOCAL4,
				"local4", 6 }, { LOG_LOCAL5, "local5", 6 }, { LOG_LOCAL6,
				"local6", 6 }, { LOG_LOCAL7, "local7", 6 }, { 0, NULL, 0 }, };


static int zlog_buffer_format(struct zlog *zl, zlog_buffer_t *buffer,
		zpl_uint32 module, zlog_level_t level, zpl_char *format, va_list args);

static int zlog_check_file (struct zlog *zl);
#ifdef ZLOG_TESTING_ENABLE
static int zlog_testing_check_file (struct zlog *zl);
#endif
const char *
zlog_facility_name(zpl_uint32 facility) {
	const struct facility_map *fm;

	for (fm = syslog_facilities; fm->name; fm++)
		if (fm->facility == facility)
			return fm->name;
	return "";
}

zpl_uint32 zlog_facility_match(const char *str) {
	const struct facility_map *fm;

	for (fm = syslog_facilities; fm->name; fm++)
		if (!strncmp(str, fm->name, fm->match))
			return fm->facility;
	return -1;
}

zlog_level_t zlog_priority_match(const char *s) {
	zlog_level_t level;

	for (level = 0; zlog_priority[level] != NULL; level++)
		if (!strncmp(s, zlog_priority[level], 2))
			return level;
	return ZLOG_DISABLED;
}

const char * zlog_priority_name(zlog_level_t level) {
	if(level >= 0 && level <= ZLOG_LEVEL_MAX)
		return zlog_priority[level];
		return "Unknow";
}

const char * zlog_proto_names(zlog_proto_t module) {
	if(module >= MODULE_NONE && module <= MODULE_MAX)
	{
		return module2name(module);
	}
	return "Unknow";
}


/* For time string format. */

zpl_size_t os_timestamp(zlog_timestamp_t timestamp, zpl_char *buf, zpl_size_t buflen)
{
	zpl_uint32 len = 0;
	zpl_char data[128];
	zpl_time_t clock = os_time(NULL);;
	os_memset(data, 0, sizeof(data));
	switch(timestamp)
	{
	case ZLOG_TIMESTAMP_NONE:
		return 0;
		break;
		//UTC :Wed Apr 18 05:19:00 UTC 2018
	case ZLOG_TIMESTAMP_BSD:
		len = snprintf(data, sizeof(data), "%s", os_time_fmt ("bsd", clock));
		break;
	case ZLOG_TIMESTAMP_DATE:
		len = snprintf(data, sizeof(data), "%s", os_time_fmt ("date", clock));
		break;
	case ZLOG_TIMESTAMP_SHORT:
		len = snprintf(data, sizeof(data), "%s", os_time_fmt ("zpl_int16", clock));
		break;
	case ZLOG_TIMESTAMP_ISO:
		len = snprintf(data, sizeof(data), "%s", os_time_fmt ("iso", clock));
		break;

	case ZLOG_TIMESTAMP_RFC3164:
		//Mmm dd hh:mm:ss
		//Oct 11 22:14:15
		//1990 Oct 22 10:52:01 TZ-6
		len = snprintf(data, sizeof(data), "%s", os_time_fmt ("rfc3164", clock));
		break;
		//rfc2822
		//Mon, 07 Aug 2006 12:34:56 -0600
	case ZLOG_TIMESTAMP_RFC3339:
		len = snprintf(data, sizeof(data), "%s", os_time_fmt ("rfc3339", clock));
		break;
	default:
		return 0;
	}

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
void time_print(FILE *fp, zlog_timestamp_t ctl)
{
	zpl_char data[128];
	os_memset(data, 0, sizeof(data));
	if(os_timestamp(ctl, data, sizeof(data)))
		fprintf(fp, "%s ", data);
}

int zlog_depth_debug_detail(FILE *fp, zpl_char *buf, zpl_uint32 depth, const char *file,
		const char *func, const zpl_uint32 line)
{
	if(depth != ZLOG_DEPTH_NONE)
	{
		if(file)
		{
			zpl_char *bk = strrchr(file, '/');
			if(buf)
			{
				zpl_uint32 offset = 0;
				if(depth == ZLOG_DEPTH_LEVEL1)
				{
					if(bk)
						sprintf(buf, "(%s ", ++bk);
					else
						sprintf(buf, "(%s ", file);
					offset = strlen(buf);
					sprintf(buf + offset, "line %d:)", line);
					return strlen(buf);
				}
				else if(depth == ZLOG_DEPTH_LEVEL2)
				{
					if(bk)
						sprintf(buf, "[LWP=%d](%s ", os_task_gettid(), ++bk);
					else
						sprintf(buf, "[LWP=%d](%s ", os_task_gettid(), file);
					offset = strlen(buf);
					sprintf(buf + offset, "line %d:)", line);
					return strlen(buf);
				}
				else if(depth == ZLOG_DEPTH_LEVEL3)
				{
					if(bk)
						sprintf(buf, "%s(%s ", os_task_self_name_alisa(), ++bk);
					else
						sprintf(buf, "%s(%s ", os_task_self_name_alisa(), file);

					if(func)
					{
						strcat(buf, "->");
						strcat(buf, func);
					}
					offset = strlen(buf);
					sprintf(buf + offset, " line %d:)", line);
					return strlen(buf);
				}
				return 0;
			}
			if(fp)
			{
				if(depth == ZLOG_DEPTH_LEVEL1)
				{
					if(bk)
						fprintf(fp, "(%s ", ++bk);
					else
						fprintf(fp, "(%s ", file);
					fprintf(fp, "line %d:)", line);
				}
				else if(depth == ZLOG_DEPTH_LEVEL2)
				{
					if(bk)
						fprintf(fp, "[LWP=%d](%s ", os_task_gettid(), ++bk);
					else
						fprintf(fp, "[LWP=%d](%s ", os_task_gettid(), file);
					fprintf(fp, "line %d:)", line);
				}
				else if(depth == ZLOG_DEPTH_LEVEL3)
				{
					if(bk)
						fprintf(fp, "%s(%s ", os_task_self_name_alisa(), ++bk);
					else
						fprintf(fp, "%s(%s ", os_task_self_name_alisa(), file);

					if(func)
						fprintf(fp, "->%s ", func);
					fprintf(fp, "line %d:)", line);
				}
			}
		}
	}
	return 0;
}


#ifdef ZLOG_TASK_ENABLE
/* va_list version of zlog. */
static void vzlog_output(struct zlog *zl, zpl_uint32 module, zlog_level_t priority, const char *format,
		va_list args) {
	//zpl_uint16 protocol = 0;
	int original_errno = ipstack_errno;
	/* If zlog is not specified, use default one. */
	if (zl == NULL)
		zl = zlog_default;
	
		/* When zlog_default is also NULL, use stderr for logging. */
	if (zl == NULL) {
		time_print(stderr, ZLOG_TIMESTAMP_DATE);
		fprintf(stderr, ZLOG_PRI_FMT": ", "unknown");
		vfprintf(stderr, format, args);
		fprintf(stderr, "\r\n");
		fflush(stderr);

		/* In this case we return at here. */
		ipstack_errno = original_errno;
		return;
	}
	
#ifdef ZLOG_TESTING_ENABLE
	if (zl->testing && (priority <= zl->testlog.priority) && zl->testlog.fp)
	{
		va_list ac;
		time_print(zl->testlog.fp, zl->timestamp);
		fprintf(zl->testlog.fp, ZLOG_PRI_FMT": ", zlog_priority[priority]);
		fprintf(zl->testlog.fp, ZLOG_PRO_FMT": ", zlog_proto_names(module));
		va_copy(ac, args);
		vfprintf(zl->testlog.fp, format, ac);
		va_end(ac);
		fprintf(zl->testlog.fp, "\n");
		fflush(zl->testlog.fp);
		zl->testlog.file_check_interval++;
	}
#endif
	/* Syslog output */
	if (priority <= zl->maxlvl[ZLOG_DEST_SYSLOG]) {
		va_list ac;
		va_copy(ac, args);
#ifndef ZPL_SERVICE_SYSLOG
		vsyslog(priority | zl->facility, format, ac);
#else
		vsysclog(priority, zl->facility, format, args);
#endif
		va_end(ac);
	}

	/* File output. */
	if ((priority <= zl->maxlvl[ZLOG_DEST_FILE]) && zl->fp) {
		va_list ac;
		time_print(zl->fp, zl->timestamp);
		if (zl->record_priority)
			fprintf(zl->fp, ZLOG_PRI_FMT": ", zlog_priority[priority]);
		fprintf(zl->fp, ZLOG_PRO_FMT": ", zlog_proto_names(module));
		va_copy(ac, args);
		vfprintf(zl->fp, format, ac);
		va_end(ac);
		fprintf(zl->fp, "\r\n");
		fflush(zl->fp);
		zl->file_check_interval++;
	}

	/* stdout output. */
	if (priority <= zl->maxlvl[ZLOG_DEST_STDOUT]) {
		va_list ac;
		time_print(stdout, zl->timestamp);
		if (zl->record_priority)
			fprintf(stdout, ZLOG_PRI_FMT": ", zlog_priority[priority]);
		fprintf(stdout, ZLOG_PRO_FMT": ", zlog_proto_names(module));
		va_copy(ac, args);
		vfprintf(stdout, format, ac);
		va_end(ac);
		fprintf(stdout, "\r\n");
		fflush(stdout);
	}
	/* logbuff output. */
	if (priority <= zl->maxlvl[ZLOG_DEST_BUFFER]) {
		zlog_buffer_format(zl, &zl->log_buffer,
					 module, priority, format,  args);
	}
	#ifdef ZPL_SHELL_MODULE
	/* Terminal monitor. */
	if (priority <= zl->maxlvl[ZLOG_DEST_MONITOR])
	{
		vty_log((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_names(module), format, zl->timestamp, args);
	}
	//trapping
	if ((priority == ZLOG_LEVEL_TRAP || priority == ZLOG_LEVEL_FORCE_TRAP) && zl->trap_lvl)
	{
		if(vty_trap_log((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_names(module), format, zl->timestamp, args) != OK)
		{
			va_list ac;
			time_print(stdout, zl->timestamp);
			if (zl->record_priority)
				fprintf(stdout, ZLOG_PRI_FMT": ", zlog_priority[priority]);
			fprintf(stdout, ZLOG_PRO_FMT": ", zlog_proto_names(module));
			va_copy(ac, args);
			vfprintf(stdout, format, ac);
			va_end(ac);
			fprintf(stdout, "\r\n");
			fflush(stdout);
		}
	}
	#endif
	/*if (module != MODULE_NONE) {
		zl->protocol = protocol;
	}*/
	ipstack_errno = original_errno;
}

static void zlog_out(struct zlog *zl, zpl_uint32 module, zlog_level_t priority, const char *format, ...)
{
	va_list args;
	if (zl && zl->mutex)
		os_mutex_lock(zl->mutex, OS_WAIT_FOREVER);	
	va_start(args, format);
	vzlog_output(zl, module, priority, format, args);
	va_end(args);
	if (zl && zl->mutex)
		os_mutex_unlock(zl->mutex);
}

void pl_vzlog(const char *file, const char *func, const zpl_uint32 line,
		struct zlog *zl, zpl_uint32 module, zlog_level_t priority, const char *format,
		va_list args)
{
	/* If zlog is not specified, use default one. */
	if (zl == NULL)
		zl = zlog_default;
	if (zl == NULL)
	{
		time_print(stderr, ZLOG_TIMESTAMP_DATE);
		fprintf(stderr, ZLOG_PRI_FMT": ", "unknown");
		zlog_depth_debug_detail(stderr, NULL, ZLOG_DEPTH_LEVEL2, file, func, line);

		vfprintf(stderr, format, args);
		fprintf(stderr, "\r\n");
		fflush(stderr);
		return;
	}
	if (zl->mutex)
		os_mutex_lock(zl->mutex, OS_WAIT_FOREVER);
/*		protocol = zl->protocol;
	if (module != MODULE_NONE)
	{
		protocol = zl->protocol;
		zl->protocol = module;
	}
*/
	if (priority <= zl->maxlvl[ZLOG_DEST_SYSLOG] ||
			priority <= zl->maxlvl[ZLOG_DEST_BUFFER] ||
			((priority <= zl->maxlvl[ZLOG_DEST_FILE]) && zl->fp) ||
			priority <= zl->maxlvl[ZLOG_DEST_MONITOR] ||
			((priority == ZLOG_LEVEL_TRAP || priority == ZLOG_LEVEL_FORCE_TRAP) && zl->trap_lvl) ||
#ifdef ZLOG_TESTING_ENABLE
			(zl->testing && (priority <= zl->testlog.priority) && zl->testlog.fp) ||
#endif
			priority <= zl->maxlvl[ZLOG_DEST_STDOUT])
	{
		zpl_uint32 offset = 0;
		va_list ac;
		zlog_hdr_t loghdr;
		memset(&loghdr, 0, sizeof(loghdr));
		loghdr.module = module;
		loghdr.priority = priority;
		loghdr.len = 0;
		offset = zlog_depth_debug_detail(NULL, loghdr.logbuf, zl->depth_debug, file, func, line);
		va_copy(ac, args);
		loghdr.len += vsnprintf(loghdr.logbuf + offset, sizeof(loghdr.logbuf) - offset, format, ac);
		va_end(ac);
		if(priority == ZLOG_LEVEL_FORCE_TRAP)
		{
			if (zl->mutex)
				os_mutex_unlock(zl->mutex);
			zlog_out(zl, loghdr.module, loghdr.priority, loghdr.logbuf);
			//zl->protocol = protocol;
			return;
		}

		if(zl->lfd > 0 && zl->taskid)
		{
			write(zl->lfd, &loghdr, sizeof(loghdr));
		}
		else
		{
			if (zl->mutex)
				os_mutex_unlock(zl->mutex);
			zlog_out(zl, loghdr.module, loghdr.priority, loghdr.logbuf);
			//zl->protocol = protocol;
			return;
		}
	}
	if (zl->mutex)
		os_mutex_unlock(zl->mutex);
	//zl->protocol = protocol;
	return ;
}


static int zlog_task(struct zlog *zl)
{
	zpl_uint32 retval = 0;
	fd_set rfds;
	zlog_hdr_t loghdr;
	os_msleep(10);
	memset(&loghdr, 0, sizeof(loghdr));
	FD_ZERO (&rfds);
	//FD_SET (zl->lfd, &rfds);
	while (1)
	{
		if(zl->lfd <= 0)
		{
			os_sleep(1);
			continue;
		}
/*		if(zl->lfd == -1)
		{
			break;
		}*/
		FD_SET (zl->lfd, &rfds);

		retval = os_select_wait(zl->lfd + 1, &rfds, NULL,  5000);

		if(zl->file_check_interval >= 50 && zl->fp)
		{
			zl->file_check_interval = 0;
			zlog_check_file(zl);
		}
#ifdef ZLOG_TESTING_ENABLE
		if(zl->testlog.file_check_interval >= 50 && zl->testlog.fp)
		{
			zl->testlog.file_check_interval = 0;
			zlog_testing_check_file(zl);
		}
#endif
		if (retval == OS_TIMEOUT)
			continue;
		if (retval < 0)
		{
			close(zl->lfd);
			zl->lfd = os_pipe_create("zlog.p", O_RDWR);
			if(zl->lfd > 0)
			{
				os_set_nonblocking(zl->lfd);
				continue;
			}
		}
		if (FD_ISSET(zl->lfd, &rfds))
		{
			memset(&loghdr, 0, sizeof(loghdr));
			if (read(zl->lfd, &loghdr, sizeof(loghdr)) < 0)
			{
				if (ipstack_errno == IPSTACK_ERRNO_EPIPE || ipstack_errno == IPSTACK_ERRNO_EBADF || ipstack_errno == IPSTACK_ERRNO_EIO /*IPSTACK_ERRNO_ECONNRESET IPSTACK_ERRNO_ECONNABORTED IPSTACK_ERRNO_ENETRESET IPSTACK_ERRNO_ECONNREFUSED*/)
				{
					close(zl->lfd);
					zl->lfd = os_pipe_create("zlog.p", O_RDWR);
					if(zl->lfd > 0)
					{
						os_set_nonblocking(zl->lfd);
					}
					continue;
				}
				continue;
			}
			if(loghdr.len)
			{
				zlog_out(zl, loghdr.module, loghdr.priority, loghdr.logbuf);
			}
		}
	}
	return OK;
}

static int zlog_task_init(struct zlog *zl)
{
	if(zl->lfd > 0)
	{
		close(zl->lfd);
		zl->lfd = 0;
	}
	zl->lfd = os_pipe_create("zlog.p", O_RDWR);
	if(zl->lfd > 0)
	{
		os_set_nonblocking(zl->lfd);
	}
	if(zl->taskid)
		return OK;
	zl->taskid = os_task_create("logTask", OS_TASK_DEFAULT_PRIORITY,
	               0, zlog_task, zl, OS_TASK_DEFAULT_STACK);   
	return OK;
}

static int zlog_task_exit(struct zlog *zl)
{
	if(zl->taskid)
	{
		os_task_destroy(zl->taskid);
		zl->taskid = 0;
	}
	if(zl->lfd > 0)
	{
		close(zl->lfd);
		zl->lfd = 0;
	}
	return OK;
}
#else
/* va_list version of zlog. */
void vzlog(const char *file, const char *func, const zpl_uint32 line,
		struct zlog *zl, zpl_uint32 module, zlog_level_t priority, const char *format,
		va_list args) {
	zpl_uint16 protocol = 0;
	int original_errno = ipstack_errno;
	/* If zlog is not specified, use default one. */
	if (zl == NULL)
		zl = zlog_default;
	if (zl->mutex)
		os_mutex_lock(zl->mutex, OS_WAIT_FOREVER);
	if (module != MODULE_NONE) {
		protocol = zl->protocol;
		zl->protocol = module;
	}
	/* When zlog_default is also NULL, use stderr for logging. */
	if (zl == NULL) {
		time_print(stderr, zl->timestamp);
		fprintf(stderr, ZLOG_PRI_FMT": ", "unknown");
		zlog_depth_debug_detail(stderr, NULL, ZLOG_DEPTH_LEVEL2, file, func, line);
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
		fflush(stderr);

		/* In this case we return at here. */
		ipstack_errno = original_errno;
		if (module != MODULE_NONE) {
			zl->protocol = protocol;
		}
		if (zl->mutex)
			os_mutex_unlock(zl->mutex);
		return;
	}
#ifdef ZLOG_TESTING_ENABLE
	if (zl->testing && (priority <= zl->testlog.priority) && zl->testlog.fp)
	{
		va_list ac;
		time_print(zl->testlog.fp, zl->timestamp);
		fprintf(zl->testlog.fp, ZLOG_PRI_FMT": ", zlog_priority[priority]);
		fprintf(zl->testlog.fp, ZLOG_PRO_FMT": ", zlog_proto_names(zl->protocol));
		zlog_depth_debug_detail(zl->testlog.fp, NULL, zl->depth_debug, file, func, line);
		va_copy(ac, args);
		vfprintf(zl->testlog.fp, format, ac);
		va_end(ac);
		fprintf(zl->testlog.fp, "\n");
		fflush(zl->testlog.fp);
		zl->testlog.file_check_interval++;
		if(zl->testlog.file_check_interval >= 50 && zl->testlog.fp)
		{
			zl->testlog.file_check_interval = 0;
			zlog_testing_check_file();
		}
	}
#endif
	/* Syslog output */
	if (priority <= zl->maxlvl[ZLOG_DEST_SYSLOG]) {
		va_list ac;
		va_copy(ac, args);
#ifndef ZPL_SERVICE_SYSLOG
		vsyslog(priority | zlog_default->facility, format, ac);
#else
		vsysclog(priority, zl->facility, format, args);
#endif
		va_end(ac);
	}

	/* File output. */
	if ((priority <= zl->maxlvl[ZLOG_DEST_FILE]) && zl->fp) {
		va_list ac;
		time_print(zl->fp, zl->timestamp);
		if (zl->record_priority)
			fprintf(zl->fp, ZLOG_PRI_FMT": ", zlog_priority[priority]);
		fprintf(zl->fp, ZLOG_PRO_FMT": ", zlog_proto_names(zl->protocol));
		zlog_depth_debug_detail(zl->fp, NULL, zl->depth_debug, file, func, line);
		va_copy(ac, args);
		vfprintf(zl->fp, format, ac);
		va_end(ac);
		fprintf(zl->fp, "\n");
		fflush(zl->fp);
		zl->file_check_interval++;
		if(zl->file_check_interval >= 50 && zl->fp)
		{
			zl->file_check_interval = 0;
			zlog_check_file();
		}
	}

	/* stdout output. */
	if (priority <= zl->maxlvl[ZLOG_DEST_STDOUT]) {
		va_list ac;
		time_print(stdout, zl->timestamp);
		if (zl->record_priority)
			fprintf(stdout, "%-2s: ", zlog_priority[priority]);
		fprintf(stdout, "%-8s: ", zlog_proto_names(zl->protocol));
		zlog_depth_debug_detail(stdout, NULL, zl->depth_debug, file, func, line);
		va_copy(ac, args);
		vfprintf(stdout, format, ac);
		va_end(ac);
		fprintf(stdout, "\n");
		fflush(stdout);
	}
	/* logbuff output. */
	if (priority <= zl->maxlvl[ZLOG_DEST_BUFFER]) {

		zlog_buffer_format(zl, &zl->log_buffer,
					 module, priority, format,  args);
	}
	#ifdef ZPL_SHELL_MODULE
	/* Terminal monitor. */
	if (priority <= zl->maxlvl[ZLOG_DEST_MONITOR])
	{
		if(zl->depth_debug != ZLOG_DEPTH_NONE)
			vty_log_debug((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_names(zl->protocol), format, zl->timestamp, args, file, func, line);
		else
			vty_log((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_names(zl->protocol), format, zl->timestamp, args);
	}
	//trapping
	if ((priority == ZLOG_LEVEL_TRAP) && zl->trap_lvl)
		vty_trap_log((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_names(zl->protocol), format, zl->timestamp, args);
	#endif
	if (module != MODULE_NONE) {
		zl->protocol = protocol;
	}
	ipstack_errno = original_errno;
	if (zl->mutex)
		os_mutex_unlock(zl->mutex);
}
#endif

static zpl_char *
str_append(zpl_char *dst, zpl_uint32 len, const char *src) {
	while ((len-- > 0) && *src)
		*dst++ = *src++;
	return dst;
}

static zpl_char *
num_append(zpl_char *s, zpl_uint32 len, u_long x) {
	zpl_char buf[30];
	zpl_char *t;

	if (!x)
		return str_append(s, len, "0");
	*(t = &buf[sizeof(buf) - 1]) = '\0';
	while (x && (t > buf)) {
		*--t = '0' + (x % 10);
		x /= 10;
	}
	return str_append(s, len, t);
}

#if defined(SA_SIGINFO) || defined(HAVE_STACK_TRACE)
static zpl_char *
hex_append(zpl_char *s, zpl_uint32 len, u_long x)
{
	zpl_char buf[30];
	zpl_char *t;

	if (!x)
	return str_append(s,len,"0");
	*(t = &buf[sizeof(buf)-1]) = '\0';
	while (x && (t > buf))
	{
		zpl_uint32 cc = (x % 16);
		*--t = ((cc < 10) ? ('0'+cc) : ('a'+cc-10));
		x /= 16;
	}
	return str_append(s,len,t);
}
#endif

/* Needs to be enhanced to support Solaris. */
static int syslog_connect(void) {
#ifdef SUNOS_5
	return -1;
#else
	int fd;
	zpl_char *s;
	struct ipstack_sockaddr_un addr;

	if ((fd = socket(AF_UNIX, IPSTACK_SOCK_DGRAM, 0)) < 0)
		return -1;
	addr.sun_family = AF_UNIX;
#ifdef _PATH_LOG
#define SYSLOG_SOCKET_PATH _PATH_LOG
#else
#define SYSLOG_SOCKET_PATH "/dev/log"
#endif
	s = str_append(addr.sun_path, sizeof(addr.sun_path), SYSLOG_SOCKET_PATH);
#undef SYSLOG_SOCKET_PATH
	*s = '\0';
	if (connect(fd, (struct ipstack_sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}
	return fd;
#endif
}


static void syslog_sigsafe(zpl_uint32 flags, const char *msg, zpl_size_t msglen) {
	static int syslog_fd = -1;
	zpl_char buf[strlen("<1234567890>ripngd[1234567890]: ") + msglen + 50];
	zpl_char *s;

	if ((syslog_fd < 0) && ((syslog_fd = syslog_connect()) < 0))
		return;

#define LOC s,buf+sizeof(buf)-s
	s = buf;
	s = str_append(LOC, "<");
	s = num_append(LOC, flags);
	s = str_append(LOC, ">");
	/* forget about the timestamp, too difficult in a signal handler */
	s = str_append(LOC, zlog_default->ident);
	if (zlog_default->syslog_options & LOG_PID) {
		s = str_append(LOC, "[");
		s = num_append(LOC, getpid());
		s = str_append(LOC, "]");
	}
	s = str_append(LOC, ": ");
	s = str_append(LOC, msg);
	write(syslog_fd, buf, s - buf);
#undef LOC
}

static int open_crashlog(void) {

	if (zlog_default && zlog_default->ident) {
		/* Avoid strlen since it is not async-signal-safe. */
		const char *p;
		zpl_size_t ilen;

		for (p = zlog_default->ident, ilen = 0; *p; p++)
			ilen++;
		{
			zpl_char buf[sizeof(CRASHLOG_PREFIX) + ilen + sizeof(CRASHLOG_SUFFIX)
					+ 3];
			zpl_char *s = buf;
#define LOC s,buf+sizeof(buf)-s
			s = str_append(LOC, CRASHLOG_PREFIX);
			s = str_append(LOC, zlog_default->ident);
			s = str_append(LOC, ".");
			s = str_append(LOC, CRASHLOG_SUFFIX);
#undef LOC
			*s = '\0';
			return open(buf, O_WRONLY | O_CREAT | O_EXCL, LOGFILE_MASK);
		}
	}
	return open(CRASHLOG_PREFIX CRASHLOG_SUFFIX, O_WRONLY | O_CREAT | O_EXCL,
			LOGFILE_MASK);
#undef CRASHLOG_SUFFIX
#undef CRASHLOG_PREFIX
}


/* Note: the goal here is to use only async-signal-safe functions. */
void zlog_signal(int signo, const char *action
#ifdef SA_SIGINFO
		, siginfo_t *siginfo, void *program_counter
#endif
		) {
	zpl_time_t now;
	zpl_char buf[strlen("DEFAULT: Received signal S at T (si_addr 0xP, PC 0xP); aborting...")
			+ 200];
	zpl_char *s = buf;
	zpl_char *msgstart = buf;
#define LOC s,buf+sizeof(buf)-s

	time(&now);
	s = str_append(LOC, "(");
	s = str_append(LOC, os_task_self_name_alisa());
#ifdef SA_SIGINFO
	s = str_append(LOC, "[pid=");
	s = num_append(LOC, siginfo->si_pid);
	s = str_append(LOC, " uid=");
	s = num_append(LOC, siginfo->si_uid);
	s = str_append(LOC, "]");
#endif
	s = str_append(LOC, ")");

	if (zlog_default) {
		s = str_append(LOC, zlog_proto_names(zlog_default->protocol));
		*s++ = ':';
		*s++ = ' ';
		msgstart = s;
	}
	s = str_append(LOC, "Received signal ");
	s = num_append(LOC, signo);
	s = str_append(LOC, " at ");
	s = num_append(LOC, now);
#ifdef SA_SIGINFO
	s = str_append(LOC," (si_addr 0x");
	s = hex_append(LOC,(u_long)(siginfo->si_addr));
	if (program_counter)
	{
		s = str_append(LOC,", PC 0x");
		s = hex_append(LOC,(u_long)program_counter);
	}
	s = str_append(LOC,"); ");
#else /* SA_SIGINFO */
	s = str_append(LOC, "; ");
#endif /* SA_SIGINFO */
	s = str_append(LOC, action);
	s = str_append(LOC, "Current:");
	s = str_append(LOC, zpl_backtrace_symb_info());
	if (s < buf + sizeof(buf))
		*s++ = '\n';
		
	/* N.B. implicit priority is most severe */
#define PRI ZLOG_LEVEL_CRIT

#define DUMP(FD) write(FD, buf, s-buf);
	/* If no file logging configured, try to write to fallback log file. */
	if ((logfile_fd >= 0) || ((logfile_fd = open_crashlog()) >= 0))
		DUMP(logfile_fd)
	if (!zlog_default)
		DUMP(STDERR_FILENO)
	else {
		if (PRI <= zlog_default->maxlvl[ZLOG_DEST_STDOUT])
			DUMP(STDOUT_FILENO)
		/* Remove trailing '\n' for monitor and syslog */
		*--s = '\0';
		#ifdef ZPL_SHELL_MODULE
		if (PRI <= zlog_default->maxlvl[ZLOG_DEST_MONITOR])
			vty_log_fixed(buf, s - buf);
		#endif	
		if (PRI <= zlog_default->maxlvl[ZLOG_DEST_SYSLOG])
			syslog_sigsafe(PRI | zlog_default->facility, msgstart,
					s - msgstart);
	}
#undef DUMP

	zlog_backtrace_sigsafe(PRI,
#ifdef SA_SIGINFO
			program_counter
#else
			NULL
#endif
			);

	s = buf;

#define DUMP(FD) write(FD, buf, s-buf);
	/* If no file logging configured, try to write to fallback log file. */
	if (logfile_fd >= 0)
		DUMP(logfile_fd)
	if (!zlog_default)
		DUMP(STDERR_FILENO)
	else {
		if (PRI <= zlog_default->maxlvl[ZLOG_DEST_STDOUT])
			DUMP(STDOUT_FILENO)
		/* Remove trailing '\n' for monitor and syslog */
		*--s = '\0';
		#ifdef ZPL_SHELL_MODULE
		if (PRI <= zlog_default->maxlvl[ZLOG_DEST_MONITOR])
			vty_log_fixed(buf, s - buf);
		#endif	
		if (PRI <= zlog_default->maxlvl[ZLOG_DEST_SYSLOG])
			syslog_sigsafe(PRI | zlog_default->facility, msgstart,
					s - msgstart);
	}
#undef DUMP

#undef PRI
#undef LOC
}

/* Log a backtrace using only async-signal-safe functions.
 Needs to be enhanced to support syslog logging. */
void zlog_backtrace_sigsafe(zlog_level_t priority, void *program_counter)
{
#ifdef HAVE_STACK_TRACE
	static const char pclabel[] = "Program counter: ";
	void *array[64];
	zpl_uint32 size;
	zpl_char buf[200];
	zpl_char *s, **bt = NULL;
#define LOC s,buf+sizeof(buf)-s

#ifdef HAVE_GLIBC_BACKTRACE
	size = backtrace(array, array_size(array));
	if (size <= 0 || (zpl_size_t)size > array_size(array))
	return;

#define DUMP(FD) { \
  if (program_counter) \
    { \
      write(FD, pclabel, sizeof(pclabel)-1); \
      backtrace_symbols_fd(&program_counter, 1, FD); \
    } \
  write(FD, buf, s-buf);	\
  backtrace_symbols_fd(array, size, FD); \
}
#elif defined(HAVE_PRINTSTACK)
#define DUMP(FD) { \
  if (program_counter) \
    write((FD), pclabel, sizeof(pclabel)-1); \
  write((FD), buf, s-buf); \
  printstack((FD)); \
}
#endif /* HAVE_GLIBC_BACKTRACE, HAVE_PRINTSTACK */

	s = buf;
	s = str_append(LOC,"Backtrace for ");
	s = num_append(LOC,size);
	s = str_append(LOC," stack frames:\n");

	if ((logfile_fd >= 0) || ((logfile_fd = open_crashlog()) >= 0))
		DUMP(logfile_fd)
	if (!zlog_default)
		DUMP(STDERR_FILENO)
	else
	{
		if (priority <= zlog_default->maxlvl[ZLOG_DEST_STDOUT])
			DUMP(STDOUT_FILENO)
		/* Remove trailing '\n' for monitor and syslog */
		*--s = '\0';
		#ifdef ZPL_SHELL_MODULE
		if (priority <= zlog_default->maxlvl[ZLOG_DEST_MONITOR])
			vty_log_fixed(buf,s-buf);
		#endif	
		if (priority <= zlog_default->maxlvl[ZLOG_DEST_SYSLOG])
			syslog_sigsafe(priority|zlog_default->facility,buf,s-buf);
		{
			zpl_uint32 i;
#ifdef HAVE_GLIBC_BACKTRACE
			bt = backtrace_symbols(array, size);
#endif
			/* Just print the function addresses. */
			for (i = 0; i < size; i++)
			{
				s = buf;
				if (bt)
				s = str_append(LOC, bt[i]);
				else
				{
					s = str_append(LOC,"[bt ");
					s = num_append(LOC,i);
					s = str_append(LOC,"] 0x");
					s = hex_append(LOC,(u_long)(array[i]));
				}
				*s = '\0';
				#ifdef ZPL_SHELL_MODULE
				if (priority <= zlog_default->maxlvl[ZLOG_DEST_MONITOR])
					vty_log_fixed(buf,s-buf);
				#endif	
				if (priority <= zlog_default->maxlvl[ZLOG_DEST_SYSLOG])
					syslog_sigsafe(priority|zlog_default->facility,buf,s-buf);
			}
			if (bt)
				free(bt);
		}
	}
#undef DUMP
#undef LOC
#endif /* HAVE_STRACK_TRACE */
}

void zlog_backtrace(zlog_level_t priority)
{
#ifndef HAVE_GLIBC_BACKTRACE
	zlog(NULL, priority, "No backtrace available on this platform.");
#else
	void *array[20];
	zpl_uint32 size, i;
	zpl_char **strings;
	size = backtrace(array, array_size(array));
	if (size <= 0 || (zpl_size_t)size > array_size(array))
	{
		zlog_err(MODULE_DEFAULT, "Cannot get backtrace, returned invalid # of frames %d "
				"(valid range is between 1 and %lu)",
				size, (zpl_ulong)(array_size(array)));
		return;
	}
	zlog(MODULE_DEFAULT, priority, "Backtrace for %d stack frames:", size);
	if (!(strings = backtrace_symbols(array, size)))
	{
		zlog_err(MODULE_DEFAULT, "Cannot get backtrace symbols (out of memory?)");
		for (i = 0; i < size; i++)
			zlog(MODULE_DEFAULT, priority, "[bt %d] %p",i,array[i]);
	}
	else
	{
		for (i = 0; i < size; i++)
			zlog(MODULE_DEFAULT, priority, "[bt %d] %s",i,strings[i]);
		free(strings);
	}
#endif /* HAVE_GLIBC_BACKTRACE */
}


void pl_zlog(const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, zlog_level_t priority, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	pl_vzlog(file, func, line, zlog_default, module, priority, format, args);
	va_end(args);
}

#define ZLOG_FUNC(FUNCNAME,PRIORITY) \
void \
FUNCNAME(const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) \
{ \
  va_list args; \
  va_start(args, format); \
  pl_vzlog (file, func, line, zlog_default, module, PRIORITY, format, args); \
  va_end(args); \
}

ZLOG_FUNC(pl_zlog_err, ZLOG_LEVEL_ERR)

ZLOG_FUNC(pl_zlog_warn, ZLOG_LEVEL_WARNING)

ZLOG_FUNC(pl_zlog_info, ZLOG_LEVEL_INFO)

ZLOG_FUNC(pl_zlog_notice, ZLOG_LEVEL_NOTICE)

ZLOG_FUNC(pl_zlog_debug, ZLOG_LEVEL_DEBUG)

ZLOG_FUNC(pl_zlog_trap, ZLOG_LEVEL_TRAP)

#undef ZLOG_FUNC

#if 0
void pl_zlog(zpl_uint32 module, zlog_level_t priority, const char *format, ...) {
	va_list args;
	va_start(args, format);
	pl_vzlog(zlog_default, module, priority, format, args);
	va_end(args);
}

#define ZLOG_FUNC(FUNCNAME,PRIORITY) \
void \
FUNCNAME(zpl_uint32 module, const char *format, ...) \
{ \
  va_list args; \
  va_start(args, format); \
  pl_vzlog (zlog_default, module, PRIORITY, format, args); \
  va_end(args); \
}

ZLOG_FUNC(pl_zlog_err, ZLOG_LEVEL_ERR)

ZLOG_FUNC(pl_zlog_warn, ZLOG_LEVEL_WARNING)

ZLOG_FUNC(pl_zlog_info, ZLOG_LEVEL_INFO)

ZLOG_FUNC(pl_zlog_notice, ZLOG_LEVEL_NOTICE)

ZLOG_FUNC(pl_zlog_debug, ZLOG_LEVEL_DEBUG)

ZLOG_FUNC(pl_zlog_trap, LOG_TRAP)

#undef ZLOG_FUNC
#endif


static void zlog_thread_info(zlog_level_t log_level)
{
	zlog(MODULE_DEFAULT, log_level,"Current %s", zpl_backtrace_symb_info());
}


void _zlog_assert_failed(const char *assertion, const char *file,
		zpl_uint32  line, const char *function)
{
	/* Force fallback file logging? */
	if (zlog_default && !zlog_default->fp
			&& ((logfile_fd = open_crashlog()) >= 0) && ((zlog_default->fp =
					fdopen(logfile_fd, "w")) != NULL))
		zlog_default->maxlvl[ZLOG_DEST_FILE] = ZLOG_LEVEL_ERR;
	zlog(MODULE_DEFAULT, ZLOG_LEVEL_CRIT,
			"Assertion `%s' failed in file %s, line %u, function %s", assertion,
			file, line, (function ? function : "?"));
	zlog_backtrace(ZLOG_LEVEL_CRIT);
	zlog_thread_info(ZLOG_LEVEL_CRIT);
	abort();
}

/* Open log stream */
struct zlog *
openzlog(const char *progname, zlog_proto_t protocol, zpl_uint32 syslog_flags,
		zpl_uint32 syslog_facility) {
	struct zlog *zl;
	zpl_uint32 i;

	zl = XCALLOC(MTYPE_ZLOG, sizeof(struct zlog));

	zl->mutex = os_mutex_name_init("logmutex");
	zl->ident = progname;
	zl->protocol = protocol;
	zl->facility = syslog_facility;
	zl->syslog_options = syslog_flags;
	zl->filesize = ZLOG_FILE_SIZE;
	zl->record_priority = zpl_true;
	/* Set default logging levels. */
	for (i = 0; i < array_size(zl->maxlvl); i++)
	{
		zl->maxlvl[i] = ZLOG_DISABLED;
		zl->default_lvl[i] = ZLOG_DISABLED;
	}

	zl->default_lvl[ZLOG_DEST_SYSLOG] = ZLOG_LEVEL_WARNING;
	zl->default_lvl[ZLOG_DEST_STDOUT] = ZLOG_LEVEL_MAX;
	zl->default_lvl[ZLOG_DEST_BUFFER] = ZLOG_LEVEL_NOTICE;
	zl->default_lvl[ZLOG_DEST_MONITOR] = ZLOG_LEVEL_MAX;
	zl->default_lvl[ZLOG_DEST_FILE] = ZLOG_LEVEL_ERR;

	zl->trap_lvl = zpl_true;//ZLOG_LEVEL_TRAP;

	openlog(progname, syslog_flags, zl->facility);
#ifdef ZPL_SERVICE_SYSLOG
	syslogc_lib_init(NULL, progname);
#endif
	//zl->maxlvl[ZLOG_DEST_STDOUT] = zl->default_lvl;
	zl->timestamp = ZLOG_TIMESTAMP_DATE;
	zl->log_buffer.max_size = ZLOG_BUFF_SIZE;
	zl->log_buffer.start = 0;

	zlog_default = zl;

	zlog_buffer_reset ();
	zlog_set_level (ZLOG_DEST_STDOUT, ZLOG_LEVEL_DEBUG);
#ifdef ZLOG_TASK_ENABLE
	zlog_task_init(zlog_default);
#endif
	zl->depth_debug = ZLOG_DEPTH_DEBUG_DEFAULT;
#ifdef ZLOG_TESTING_ENABLE
	zl->testing = zpl_false;
	zl->testlog.priority = ZLOG_LEVEL_WARNING;
	zl->testlog.fp = NULL;
	zl->testlog.filesize = 8;
	zl->testlog.file_check_interval = 0;
#endif
	return zl;
}

void openzlog_start(struct zlog *zl)
{
	zpl_uint32 i;
	struct zlog *zlp = zl;
	if(zlp == NULL)
		zlp = zlog_default;
	for (i = 0; i < array_size(zlp->maxlvl); i++)
	{
		zlp->maxlvl[i] = ZLOG_DISABLED;
		zlp->default_lvl[i] = ZLOG_DISABLED;
	}

	zlp->default_lvl[ZLOG_DEST_SYSLOG] = ZLOG_LEVEL_WARNING;
	zlp->default_lvl[ZLOG_DEST_STDOUT] = ZLOG_LEVEL_ERR;
	zlp->default_lvl[ZLOG_DEST_BUFFER] = ZLOG_LEVEL_NOTICE;
	zlp->default_lvl[ZLOG_DEST_MONITOR] = ZLOG_LEVEL_MAX;
	zlp->default_lvl[ZLOG_DEST_FILE] = ZLOG_LEVEL_ERR;
	zlp->trap_lvl = zpl_true;//ZLOG_LEVEL_TRAP;
	return OK;
}

void closezlog(struct zlog *zl) {
	closelog();

	if (zl->fp != NULL)
		fclose(zl->fp);

	if (zl->filename != NULL)
		free(zl->filename);
	if (zl->mutex)
		os_mutex_exit(zl->mutex);
#ifdef ZLOG_TESTING_ENABLE
	if(zl->testlog.fp)
	{
		fflush(zl->testlog.fp);
		fclose(zl->testlog.fp);
		zl->testlog.fp = NULL;
	}
#endif

#ifdef ZLOG_TASK_ENABLE
	zlog_task_exit(zlog_default);
#endif
	XFREE(MTYPE_ZLOG, zl);
#ifdef ZPL_SERVICE_SYSLOG
	syslogc_lib_uninit();
#endif
}

/* Called from command.c. */

void zlog_set_trap(zpl_bool level) {

	zpl_uint32 i = 0;
	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->trap_lvl = level;
	for (i = 0; i < ZLOG_NUM_DESTS; i++)
		if (zlog_default->maxlvl[i] != ZLOG_DISABLED)
			zlog_default->maxlvl[i] = level;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


void zlog_get_trap(zpl_bool *level)
{
	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(level)
		*level = zlog_default->trap_lvl;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}

void zlog_set_facility(zpl_uint32 facility) {

	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->facility = facility;
#ifdef ZPL_SERVICE_SYSLOG
	syslogc_facility_set(zlog_default->facility);
#endif
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


void zlog_get_facility(zpl_uint32 *facility)
{
	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(facility)
		*facility = zlog_default->facility;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}

void zlog_set_record_priority(zpl_uint32 record_priority) {

	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->record_priority = record_priority;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


void zlog_get_record_priority(zpl_uint32 *record_priority)
{
	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(record_priority)
		*record_priority = zlog_default->record_priority;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}

void zlog_set_timestamp(zlog_timestamp_t value) {

	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(zlog_default->timestamp != value)
		zlog_default->timestamp = value;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


void zlog_get_timestamp(zlog_timestamp_t *value)
{
	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = zlog_default->timestamp;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}

void zlog_set_level(zlog_dest_t dest, zlog_level_t log_level) {

	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->maxlvl[dest] = log_level;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


void zlog_get_level(zlog_dest_t dest, zlog_level_t *log_level)
{
	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(log_level)
		*log_level = zlog_default->maxlvl[dest];
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


/* Reset opend file. */
int zlog_reset_file(zpl_bool bOpen)
{
	zpl_char filetmp[256];
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
/*	if(nsmlog.t_time)
	{
		eloop_cancel(nsmlog.t_time);
		nsmlog.t_time = NULL;
	}*/
	if (zlog_default->fp)
	{
		fflush (zlog_default->fp);
		fclose(zlog_default->fp);
		zlog_default->fp = NULL;
	}
	if(bOpen)
	{
		memset (filetmp, 0, sizeof(filetmp));
		sprintf (filetmp, "%s%s", ZLOG_VIRTUAL_PATH, zlog_default->filename);
		if (access (filetmp, F_OK) >= 0)
		{
			remove(filetmp);
			sync();
		}
		memset (filetmp, 0, sizeof(filetmp));
		sprintf (filetmp, "%s%s", ZLOG_REAL_PATH, zlog_default->filename);
		if (access (filetmp, F_OK) >= 0)
		{
			remove(filetmp);
			sync();
		}
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	if(bOpen)
		return zlog_set_file(zlog_default->filename, zlog_default->maxlvl[ZLOG_DEST_FILE]);
	return OK;
}

int zlog_set_file(const char *filename, zlog_level_t log_level)
{
	FILE *fp;
	mode_t oldumask;
	zpl_char filetmp[256];
	zpl_char *file_name = NULL;
	const char *default_file = ZLOG_FILE_DEFAULT;
	if (zlog_default == NULL)
		return ERROR;
	if(filename)
		file_name = (zpl_char *)filename;
	else
		file_name = (zpl_char *)default_file;

	zlog_reset_file(zpl_false);

	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);

/*	if(nsmlog.t_time)
	{
		eloop_cancel(nsmlog.t_time);
		nsmlog.t_time = NULL;
	}*/
	/* Open file. */
	memset(filetmp, 0, sizeof(filetmp));
	sprintf(filetmp, "%s%s", ZLOG_VIRTUAL_PATH, file_name);
	oldumask = umask(0777 & ~LOGFILE_MASK);
	fp = fopen (filetmp, "a");
	umask(oldumask);
	if (fp == NULL)
	{
		if (zlog_default->mutex)
			os_mutex_unlock(zlog_default->mutex);
		return ERROR;
	}
	/* Set flags. */
	if (zlog_default->filename)
		free(zlog_default->filename);
	zlog_default->filename = strdup(file_name);
	zlog_default->maxlvl[ZLOG_DEST_FILE] = log_level;
	zlog_default->fp = fp;
	logfile_fd = fileno(fp);

	if (host_config_set_api(API_SET_LOGFILE_CMD, file_name) != OK) {
		if (zlog_default->mutex)
			os_mutex_unlock(zlog_default->mutex);
		return ERROR;
	}

	//nsmlog.t_time = eloop_add_timer (nsmlog.master, zlog_file_time_thread, NULL, LOG_FILE_CHK_TIME);
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

int zlog_get_file(const char *filename, zlog_level_t *log_level)
{
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);

	if(filename && zlog_default->filename)
	{
		strcpy(filename, zlog_default->filename);
	}
	if(log_level)
		*log_level = zlog_default->maxlvl[ZLOG_DEST_FILE];

	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

int zlog_close_file(void)
{

	//zpl_char filetmp[256];
	if (zlog_default == NULL)
		return 0;

	zlog_reset_file(zpl_false);

	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
/*	if(nsmlog.t_time)
	{
		eloop_cancel(nsmlog.t_time);
		nsmlog.t_time = NULL;
	}*/
	zlog_default->fp = NULL;
	logfile_fd = -1;
	zlog_default->maxlvl[ZLOG_DEST_FILE] = ZLOG_DISABLED;

	if (zlog_default->filename)
		free(zlog_default->filename);
	zlog_default->filename = NULL;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}


int zlog_set_file_size (zpl_uint32 filesize)
{
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(filesize == 0)
		zlog_default->filesize = ZLOG_FILE_SIZE;
	else
		zlog_default->filesize = filesize;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

int
zlog_get_file_size (zpl_uint32 *filesize)
{
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(filesize)
		*filesize = zlog_default->filesize;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

#ifdef ZLOG_TESTING_ENABLE

int zlog_testing_enable (zpl_bool enable)
{
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->testing = enable;
	if(enable == zpl_false)
	{
		if(zlog_default->testlog.fp)
		{
			fflush(zlog_default->testlog.fp);
			fclose(zlog_default->testlog.fp);
			zlog_default->testlog.fp = NULL;
		}
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

zpl_bool zlog_testing_enabled (void)
{
	if (zlog_default == NULL)
		return 0;
	return zlog_default->testing;
}

int zlog_testing_priority (zlog_level_t priority)
{
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->testlog.priority = priority;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

int zlog_testing_file (zpl_char * filename)
{
	zpl_char filetmp[256];
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);

	if(zlog_default->testlog.filename)
	{
		if(zlog_default->testlog.fp)
		{
			fclose(zlog_default->testlog.fp);
			zlog_default->testlog.fp = NULL;
		}
		memset(filetmp, 0, sizeof(filetmp));
		sprintf (filetmp, "%s%s", ZLOG_VIRTUAL_PATH, zlog_default->testlog.filename);
		remove(filetmp);
		sync();
		free(zlog_default->testlog.filename);
		zlog_default->testlog.filename = NULL;
		if(filename == NULL)
		{
			if (zlog_default->mutex)
				os_mutex_unlock(zlog_default->mutex);
			return OK;
		}
	}
	if(filename)
	{
		zlog_default->testlog.filename = strdup(filename);

		memset(filetmp, 0, sizeof(filetmp));
		sprintf (filetmp, "%s%s", ZLOG_VIRTUAL_PATH, filename);

		zlog_default->testlog.fp = fopen(filetmp, "a+");
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	if(filename && zlog_default->testlog.fp)
		return OK;
	return ERROR;
}
static int zlog_testing_check_file (struct zlog *zl)
{
	struct stat fsize;
	zpl_char filetmp[256];
	if (zlog_default == NULL)
		return ERROR;
	memset (filetmp, 0, sizeof(filetmp));
	if(!zl->testlog.filename)
		return ERROR;
	if (zl->mutex)
		os_mutex_lock(zl->mutex, OS_WAIT_FOREVER);
	sprintf (filetmp, "%s%s", ZLOG_VIRTUAL_PATH, zl->testlog.filename);
	if (access (filetmp, F_OK) >= 0)
	{
		memset (&fsize, 0, sizeof(fsize));
		(void) stat (filetmp, &fsize);
		//zlog_debug(MODULE_DEFAULT, "========%s:%d %p", __func__, fsize.st_size, zl->testlog.filesize * ZLOG_1M);
		if (fsize.st_size < (zl->testlog.filesize * ZLOG_1M))
		{
			if (zl->mutex)
				os_mutex_unlock(zl->mutex);
			return OK;
		}
		fseek(zl->testlog.fp, 0, SEEK_SET);
	}
	if (zl->mutex)
		os_mutex_unlock(zl->mutex);
	return OK;
}
#endif

static int zlog_file_move(const char *src, const char *dest)
{
	zpl_char buff[1024];
	zpl_uint32 len;
	FILE *in,*out;
	in = fopen(src, "r+");
	out = fopen(dest, "w+");
	if(in == NULL || out == NULL)
		return ERROR;
	while((len = fread(buff, 1, sizeof(buff), in)))
	{
		fwrite(buff,1,len,out);
	}
	fflush (in);
	fflush (out);
	fclose(in);
	fclose(out);
	return OK;
}

int zlog_file_save (void)
{
	zpl_char filetmp[256];
	zpl_char filereal[256];
	zpl_char filereal2[256];
	if (zlog_default == NULL)
		return ERROR;
	memset (filetmp, 0, sizeof(filetmp));
	memset (filereal, 0, sizeof(filereal));
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	sprintf (filetmp, "%s%s", ZLOG_VIRTUAL_PATH, zlog_default->filename);
	if (access (filetmp, F_OK) >= 0)
	{
		if (zlog_default->fp)
		{
			fflush (zlog_default->fp);
			fclose (zlog_default->fp);
		}
#ifdef ZLOG_TESTING_ENABLE
		if(zlog_default->testing == zpl_false)
#endif
		{
			sprintf (filereal, "%s%s", ZLOG_REAL_PATH, zlog_default->filename);

			if (access (filereal, F_OK) >= 0)
			{
				sprintf (filereal2, "%s%s.old", ZLOG_REAL_PATH, zlog_default->filename);
				if (access (filereal2, F_OK) >= 0)
					remove(filereal2);
				sync();
				zlog_file_move(filereal, filereal2);
			}
			zlog_file_move(filetmp, filereal);
			remove(filetmp);
		}
		sync();
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

/* Check log filesize. */
static int zlog_check_file (struct zlog *zl)
{
	struct stat fsize;
	zpl_char filetmp[256];
	if (zlog_default == NULL)
		return ERROR;
	memset (filetmp, 0, sizeof(filetmp));
	//memset (filereal, 0, sizeof(filereal));
	if (zl->mutex)
		os_mutex_lock(zl->mutex, OS_WAIT_FOREVER);
	sprintf (filetmp, "%s%s", ZLOG_VIRTUAL_PATH, zl->filename);
	if (access (filetmp, F_OK) >= 0)
	{
		memset (&fsize, 0, sizeof(fsize));
		(void) stat (filetmp, &fsize);

		//zlog_debug(MODULE_DEFAULT, "========%s:%d %d", __func__, fsize.st_size, zl->filesize * ZLOG_1M);
		if (fsize.st_size < (zl->filesize * ZLOG_1M))
		{
			if (zl->mutex)
				os_mutex_unlock(zl->mutex);
			return OK;
		}
		if (zl->fp)
		{
			fflush (zl->fp);
			fclose (zl->fp);
		}
#ifdef ZLOG_TESTING_ENABLE
		if(zl->testing == zpl_false)
#endif
		{
			zpl_char filereal[256];
			zpl_char filereal2[256];
			memset (filereal, 0, sizeof(filereal));
			memset (filereal2, 0, sizeof(filereal2));
			sprintf (filereal, "%s%s", ZLOG_REAL_PATH, zl->filename);

			if (access (filereal, F_OK) >= 0)
			{
				sprintf (filereal2, "%s%s.old", ZLOG_REAL_PATH, zl->filename);
				if (access (filereal2, F_OK) >= 0)
					remove(filereal2);
				sync();
				zlog_file_move(filereal, filereal2);
			}
			zlog_file_move(filetmp, filereal);
			remove(filetmp);
		}
		sync();
		zl->fp = fopen (filetmp, "a");
		if (zl->fp == NULL)
		{
			if (zl->mutex)
				os_mutex_unlock(zl->mutex);
			return ERROR;
		}
	}
	if (zl->mutex)
		os_mutex_unlock(zl->mutex);
	return OK;
}
/******************************************************/
/*static int zlog_buffer_insert_one(zlog_buffer_t *buffer, zpl_uint32 module, zpl_uint32 level, zpl_uint32 len, const char *log)
{
	zpl_uint32 index = buffer->start;

	if(index == buffer->max_size)
		index = 0;
	memset(buffer->buffer[index].log, 0, LOG_MSG_SIZE);
	memcpy(buffer->buffer[index].log, log, MIN(LOG_MSG_SIZE, strlen(log)));
	buffer->buffer[index].module = module;
	buffer->buffer[index].level = level;
	buffer->buffer[index].size = len;
	index++;
	buffer->start = index;
	return 0;
}*/

static int zlog_buffer_format(struct zlog *zl, zlog_buffer_t *buffer,
		zpl_uint32 module, zlog_level_t level, zpl_char *format, va_list args)
{
	va_list ac;
	zpl_uint32 len;
	zpl_uint32 index = buffer->start;
	if(index == buffer->max_size)
		index = 0;

	os_memset(buffer->buffer[index].log, 0, LOG_MSG_SIZE);
	len = os_timestamp(zl->timestamp, buffer->buffer[index].log, sizeof(buffer->buffer[index].log));

	buffer->buffer[index].log[len++] = ' ';
	buffer->buffer[index].log[len++] = '\0';

	if (zl->record_priority)
		len += sprintf(buffer->buffer[index].log + len, "%s: ", zlog_priority[level]);
	len += sprintf(buffer->buffer[index].log + len, "%s: ", zlog_proto_names(zl->protocol));
	va_copy(ac, args);
	len += vsprintf(buffer->buffer[index].log + len, format, ac);
	va_end(ac);
	len += sprintf(buffer->buffer[index].log + len, "\r\n");

	buffer->buffer[index].module = module;
	buffer->buffer[index].level = level;
	buffer->buffer[index].size = len;
	index++;
	buffer->start = index;
	return OK;//zlog_buffer_insert_one(buffer, module, level, len, format);
}

int zlog_buffer_reset(void)
{
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zpl_uint32 size = sizeof(zbuffer_t)*zlog_default->log_buffer.max_size;
	if(zlog_default->log_buffer.buffer)
		XFREE(MTYPE_ZLOG, zlog_default->log_buffer.buffer);
	zlog_default->log_buffer.start = 0;
	if(size)
	{
		zlog_default->log_buffer.buffer = XMALLOC(MTYPE_ZLOG, size);
		if(zlog_default->log_buffer.buffer)
			memset(zlog_default->log_buffer.buffer, 0 , size);
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}


int zlog_set_buffer_size (zpl_uint32 size)
{
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(size == 0)
		zlog_default->log_buffer.max_size = ZLOG_BUFF_SIZE;
	else
		zlog_default->log_buffer.max_size = size;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);

	return zlog_buffer_reset();
}

int zlog_get_buffer_size (zpl_uint32 *size)
{
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(size)
		*size = zlog_default->log_buffer.max_size;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

int zlog_buffer_save(void)
{
	zpl_uint32 i = 0;
	FILE	 *fp = NULL;
	zpl_char filereal[256];
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(zlog_default->maxlvl[ZLOG_DEST_BUFFER] > ZLOG_DISABLED)
	{
#ifdef ZLOG_TESTING_ENABLE
		if(zlog_default->testing == zpl_false)
		{
			sprintf (filereal, "%s%s", ZLOG_REAL_PATH, zlog_default->filename);
		}
		else
#endif
		{
			sprintf (filereal, "%s%s", ZLOG_VIRTUAL_PATH, zlog_default->filename);
		}
		fp = fopen (filereal, "a");
		if (fp == NULL)
		{
			if (zlog_default->mutex)
					os_mutex_unlock(zlog_default->mutex);
			return ERROR;
		}
		if(fp && zlog_default->log_buffer.buffer)
		{
			for(i = zlog_default->log_buffer.start; i < zlog_default->log_buffer.max_size; i++)
			{
				if(zlog_default->log_buffer.buffer[i].size > 0)
					fprintf (fp, "%s: ", zlog_default->log_buffer.buffer[i].log);
				fflush (fp);
			}
			for(i = 0; i < zlog_default->log_buffer.start; i++)
			{
				if(zlog_default->log_buffer.buffer[i].size > 0)
					fprintf (fp, "%s: ", zlog_default->log_buffer.buffer[i].log);
				fflush (fp);
			}
		}
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

int zlog_buffer_callback_api (zlog_buffer_cb cb, void *pVoid)
{
	zpl_uint32 i = 0;
	int ret = 0;
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(zlog_default->maxlvl[ZLOG_DEST_BUFFER] > ZLOG_DISABLED && zlog_default->log_buffer.buffer)	
	{
		for(i = zlog_default->log_buffer.start; i >= 0; i--)
		{
			if(zlog_default->log_buffer.buffer[i].size)
			{
				if(cb)
					ret = (cb)(&zlog_default->log_buffer.buffer[i], pVoid);
				if(ret != 0)
				{
					if (zlog_default->mutex)
						os_mutex_unlock(zlog_default->mutex);
					return OK;
				}
			}
		}
		for(i = zlog_default->log_buffer.max_size; i > zlog_default->log_buffer.start; i--)
		{
			if(zlog_default->log_buffer.buffer[i].size)
			{
				if(cb)
					ret = (cb)(&zlog_default->log_buffer.buffer[i], pVoid);
				if(ret != 0)
				{
					if (zlog_default->mutex)
						os_mutex_unlock(zlog_default->mutex);
					return OK;
				}
			}

		}
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}


/* Reopen log file. */
int zlog_rotate(void)
{
	zpl_uint32 level;

	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if (zlog_default->fp)
		fclose(zlog_default->fp);
	zlog_default->fp = NULL;
	logfile_fd = -1;
	level = zlog_default->maxlvl[ZLOG_DEST_FILE];
	zlog_default->maxlvl[ZLOG_DEST_FILE] = ZLOG_DISABLED;

	if (zlog_default->filename)
	{
		zpl_char filetmp[128];
		mode_t oldumask;
		int save_errno;
		memset(filetmp, 0, sizeof(filetmp));
		snprintf (filetmp, sizeof(filetmp), "%s%s", ZLOG_VIRTUAL_PATH, zlog_default->filename);
		oldumask = umask(0777 & ~LOGFILE_MASK);
		zlog_default->fp = fopen(filetmp, "a");
		save_errno = ipstack_errno;
		umask(oldumask);
		if (zlog_default->fp == NULL)
		{
			zlog_err(MODULE_DEFAULT,
					"Log rotate failed: cannot open file %s for append: %s",
					zlog_default->filename, ipstack_strerror(save_errno));
			if (zlog_default->mutex)
				os_mutex_unlock(zlog_default->mutex);
			return -1;
		}
		logfile_fd = fileno(zlog_default->fp);
		zlog_default->maxlvl[ZLOG_DEST_FILE] = level;
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return 1;
}

/* Message lookup function. */
const char *
lookup(const struct message *mes, zpl_uint32 key) {
	const struct message *pnt;

	for (pnt = mes; pnt->key != 0; pnt++)
		if (pnt->key == key)
			return pnt->str;

	return "";
}

/* Older/faster version of message lookup function, but requires caller to pass
 * in the array size (instead of relying on a 0 key to terminate the search). 
 *
 * The return value is the message string if found, or the 'none' pointer
 * provided otherwise.
 */
const char *
message_lookup(const struct message *meslist, zpl_uint32 max, zpl_uint32 index, const char *none,
		const char *mesname) {
	zpl_uint32 pos = index - meslist[0].key;

	/* first check for best case: index is in range and matches the key
	 * value in that slot.
	 * NB: key numbering might be offset from 0. E.g. protocol constants
	 * often start at 1.
	 */
	if ((pos >= 0) && (pos < max) && (meslist[pos].key == index))
		return meslist[pos].str;

	/* fall back to linear search */
	{
		zpl_uint32 i;

		for (i = 0; i < max; i++, meslist++) {
			if (meslist->key == index) {
				const char *str = (meslist->str ? meslist->str : none);

				zlog_debug(MODULE_DEFAULT,
						"message index %d [%s] found in %s at position %d (max is %d)",
						index, str, mesname, i, max);
				return str;
			}
		}
	}
	zlog_err(MODULE_DEFAULT, "message index %d not found in %s (max is %d)",
			index, mesname, max);
	assert(none);
	return none;
}





static const struct rttype_desc_table unknown = { 0, "unknown", '?' };

static const struct rttype_desc_table *
zroute_lookup(zpl_uint32 zroute) {
	zpl_uint32 i;

	if (zroute >= array_size(route_types)) {
		zlog_err(MODULE_DEFAULT, "unknown zebra route type: %u", zroute);
		return &unknown;
	}
	if (zroute == route_types[zroute].type)
		return &route_types[zroute];
	for (i = 0; i < array_size(route_types); i++) {
		if (zroute == route_types[i].type) {
			zlog_warn(MODULE_DEFAULT,
					"internal error: route type table out of order "
							"while searching for %u, please notify developers",
					zroute);
			return &route_types[i];
		}
	}
	zlog_err(MODULE_DEFAULT,
			"internal error: cannot find route type %u in table!", zroute);
	return &unknown;
}

const char *
zroute_string(zpl_uint32 zroute) {
	return zroute_lookup(zroute)->string;
}

zpl_char zroute_keychar(zpl_uint32 zroute) {
	return zroute_lookup(zroute)->chr;
}


zpl_proto_t proto_name2num(const char *s) {
	unsigned i;

	for (i = 0; i < array_size(route_types); ++i)
		if (strcasecmp(s, route_types[i].string) == 0)
			return route_types[i].type;
	return -1;
}

zpl_proto_t proto_redistnum(zpl_uint16 afi, const char *s) {
	if (!s)
		return -1;
	if (afi == AFI_IP) {
		if (strncmp(s, "sy", 2) == 0)
			return ZPL_ROUTE_PROTO_SYSTEM;
		else if (strncmp(s, "ke", 2) == 0)
			return ZPL_ROUTE_PROTO_KERNEL;
		else if (strncmp(s, "co", 2) == 0)
			return ZPL_ROUTE_PROTO_CONNECT;
		else if (strncmp(s, "st", 2) == 0)
			return ZPL_ROUTE_PROTO_STATIC;
		else if (strncmp(s, "ri", 2) == 0)
			return ZPL_ROUTE_PROTO_RIP;
		else if (strncmp(s, "os", 2) == 0)
			return ZPL_ROUTE_PROTO_OSPF;
		else if (strncmp(s, "is", 2) == 0)
			return ZPL_ROUTE_PROTO_ISIS;
		else if (strncmp(s, "bg", 2) == 0)
			return ZPL_ROUTE_PROTO_BGP;
		else if (strncmp(s, "pi", 2) == 0)
			return ZPL_ROUTE_PROTO_PIM;
		else if (strncmp(s, "hs", 2) == 0)
			return ZPL_ROUTE_PROTO_HSLS;
		else if (strncmp(s, "ol", 2) == 0)
			return ZPL_ROUTE_PROTO_OLSR;
		else if (strncmp(s, "ba", 2) == 0)
			return ZPL_ROUTE_PROTO_BABEL;
		else if (strncmp(s, "n", 1) == 0)
			return ZPL_ROUTE_PROTO_NHRP;
		else if (strncmp(s, "vr", 2) == 0)
			return ZPL_ROUTE_PROTO_VRRP;
		else if (strncmp(s, "fr", 2) == 0)
			return ZPL_ROUTE_PROTO_FRP;
	}
	if (afi == AFI_IP6) {
		if (strncmp(s, "sy", 2) == 0)
			return ZPL_ROUTE_PROTO_SYSTEM;
		else if (strncmp(s, "ke", 2) == 0)
			return ZPL_ROUTE_PROTO_KERNEL;
		else if (strncmp(s, "co", 2) == 0)
			return ZPL_ROUTE_PROTO_CONNECT;
		else if (strncmp(s, "st", 2) == 0)
			return ZPL_ROUTE_PROTO_STATIC;
		else if (strncmp(s, "ri", 2) == 0)
			return ZPL_ROUTE_PROTO_RIPNG;
		else if (strncmp(s, "os", 2) == 0)
			return ZPL_ROUTE_PROTO_OSPF6;
		else if (strncmp(s, "is", 2) == 0)
			return ZPL_ROUTE_PROTO_ISIS;
		else if (strncmp(s, "bg", 2) == 0)
			return ZPL_ROUTE_PROTO_BGP;
		else if (strncmp(s, "pi", 2) == 0)
			return ZPL_ROUTE_PROTO_PIM;
		else if (strncmp(s, "hs", 2) == 0)
			return ZPL_ROUTE_PROTO_HSLS;
		else if (strncmp(s, "ol", 2) == 0)
			return ZPL_ROUTE_PROTO_OLSR;
		else if (strncmp(s, "ba", 2) == 0)
			return ZPL_ROUTE_PROTO_BABEL;
		else if (strncmp(s, "n", 1) == 0)
			return ZPL_ROUTE_PROTO_NHRP;
		else if (strncmp(s, "vr", 2) == 0)
			return ZPL_ROUTE_PROTO_VRRP;
		else if (strncmp(s, "fr", 2) == 0)
			return ZPL_ROUTE_PROTO_FRP;
	}
	return -1;
}



void zlog_hexdump(void *mem, zpl_uint32  len) {
	zpl_ulong i = 0;
	zpl_uint32  j = 0;
	zpl_uint32  columns = 8;
	zpl_char buf[(len * 4) + ((len / 4) * 20) + 30];
	zpl_char *s = buf;

	for (i = 0; i < len + ((len % columns) ? (columns - len % columns) : 0);
			i++) {
		/* print offset */
		if (i % columns == 0)
			s += sprintf(s, "0x%016lx: ", (zpl_ulong) mem + i);

		/* print hex data */
		if (i < len)
			s += sprintf(s, "%02x ", 0xFF & ((zpl_char*) mem)[i]);

		/* end of block, just aligning for ASCII dump */
		else
			s += sprintf(s, "   ");

		/* print ASCII dump */
		if (i % columns == (columns - 1)) {
			for (j = i - (columns - 1); j <= i; j++) {
				if (j >= len) /* end of block, not really printing */
					s += sprintf(s, " ");

				else if (isprint((int) ((zpl_char*) mem)[j])) /* printable zpl_char */
					s += sprintf(s, "%c", 0xFF & ((zpl_char*) mem)[j]);

				else
					/* other zpl_char */
					s += sprintf(s, ".");
			}
			s += sprintf(s, "\n");
		}
	}
	zlog_debug(MODULE_DEFAULT, "\n%s", buf);
}
