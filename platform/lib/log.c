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

#define QUAGGA_DEFINE_DESC_TABLE

#include <zebra.h>
#include "os_sem.h"
#include "log.h"
#include "memory.h"
#include "command.h"
#include "eloop.h"

#ifndef SUNOS_5
#include <sys/un.h>
#endif


/* for printstack on solaris */
#ifdef HAVE_UCONTEXT_H
#include <ucontext.h>
#endif

#include "host.h"

static int logfile_fd = -1; /* Used in signal handler. */

struct zlog *zlog_default = NULL;

static const char *zlog_proto_string[] = { "NONE", "DEFAULT", "CONSOLE", "TELNET",/*"ZEBRA",*/ "HAL", "PAL", "NSM", "RIP",
		"BGP", "OSPF", "RIPNG", "BABEL", "OSPF6", "ISIS", "PIM", "MASC", "NHRP",
		"HSLS", "OLSR", "VRRP", "FRP", "LLDP", "BFD", "LDP", "SNTP", "IMISH", "WIFI", "MODEM", "APP", "VOIP", "SOUND",
		"UTILS", NULL, };

static const char *zlog_priority[] = { "emergencies", "alerts", "critical", "errors",
		"warnings", "notifications", "informational", "debugging", "trapping", NULL, };

static const struct facility_map {
	int facility;
	const char *name;
	size_t match;
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
		int module, int level, char *format, va_list args);
//static int zlog_check_file (void);

const char *
zlog_facility_name(int facility) {
	const struct facility_map *fm;

	for (fm = syslog_facilities; fm->name; fm++)
		if (fm->facility == facility)
			return fm->name;
	return "";
}

int zlog_facility_match(const char *str) {
	const struct facility_map *fm;

	for (fm = syslog_facilities; fm->name; fm++)
		if (!strncmp(str, fm->name, fm->match))
			return fm->facility;
	return -1;
}

int zlog_priority_match(const char *s) {
	int level;

	for (level = 0; zlog_priority[level] != NULL; level++)
		if (!strncmp(s, zlog_priority[level], 2))
			return level;
	return ZLOG_DISABLED;
}

const char * zlog_priority_name(int level) {
	if(level >= 0 && level <= LOG_TRAP)
		return zlog_priority[level];
	//if(level == ZLOG_DISABLED)
		return "Unknow";
}

const char * zlog_proto_names(zlog_proto_t module) {
	if(module >= ZLOG_NONE && module <= ZLOG_MAX)
		return zlog_proto_string[module];
	return "Unknow";
}


/* For time string format. */

size_t quagga_timestamp(zlog_timestamp_t timestamp, char *buf, size_t buflen) {

	time_t clock;
	struct tm *tm;
	char data[128];
	int len = 0;
	clock = os_time(NULL);
	os_memset(data, 0, sizeof(data));
	switch(timestamp)
	{
	case ZLOG_TIMESTAMP_NONE:
		return 0;
		break;
		//UTC :Wed Apr 18 05:19:00 UTC 2018
	case ZLOG_TIMESTAMP_BSD:
		tm = localtime(&clock);
		len = strftime(data, sizeof(data), "%b %e %T",tm);
		break;
	case ZLOG_TIMESTAMP_DATE:
		tm = localtime(&clock);
		len = strftime(data, sizeof(data), "%Y/%m/%d %H:%M:%S",tm);
		break;
	case ZLOG_TIMESTAMP_SHORT:
		tm = localtime(&clock);
		len = strftime(data, sizeof(data), "%m/%d %H:%M:%S",tm);
		break;
	case ZLOG_TIMESTAMP_ISO:
		tm = gmtime(&clock);
		len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S+08:00",tm);
		break;

	case ZLOG_TIMESTAMP_RFC3164:
		//Mmm dd hh:mm:ss
		//Oct 11 22:14:15
		//1990 Oct 22 10:52:01 TZ-6
		tm = localtime(&clock);
		len = strftime(data, sizeof(data), "%b %d %T",tm);
		break;
		//rfc2822
		//Mon, 07 Aug 2006 12:34:56 -0600
	case ZLOG_TIMESTAMP_RFC3339:
		tm = gmtime(&clock);
		len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S-08:00",tm);
		break;
	default:
		return 0;
	}
	//char * asctime(const struct tm * timeptr);
	//char * ctime(const time_t *timer);

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
static void time_print(FILE *fp, zlog_timestamp_t ctl)
{
	char data[128];
	os_memset(data, 0, sizeof(data));
	if(quagga_timestamp(ctl, data, sizeof(data)))
		fprintf(fp, "%s ", data);
}

#ifdef ZLOG_TASK_ENABLE
/* va_list version of zlog. */
static void vzlog_output(struct zlog *zl, int module, int priority, const char *format,
		va_list args) {
	int protocol = 0;
	int original_errno = errno;

	/* If zlog is not specified, use default one. */
	if (zl == NULL)
		zl = zlog_default;
	if (zl->mutex)
		os_mutex_lock(zl->mutex, OS_WAIT_FOREVER);
	if (module != ZLOG_NONE) {
		protocol = zl->protocol;
		zl->protocol = module;
	}
	/* When zlog_default is also NULL, use stderr for logging. */
	if (zl == NULL) {
		time_print(stderr, zl->timestamp);
		fprintf(stderr, "%s: ", "unknown");
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
		fflush(stderr);

		/* In this case we return at here. */
		errno = original_errno;
		if (module != ZLOG_NONE) {
			zl->protocol = protocol;
		}
		if (zl->mutex)
			os_mutex_unlock(zl->mutex);
		return;
	}

	/* Syslog output */
	if (priority <= zl->maxlvl[ZLOG_DEST_SYSLOG]) {
		va_list ac;
		va_copy(ac, args);
#ifndef SYSLOG_CLIENT
		vsyslog(priority | zlog_default->facility, format, ac);
#else
		vsysclog(priority, format, args);
#endif
		va_end(ac);
	}

	/* File output. */
	if ((priority <= zl->maxlvl[ZLOG_DEST_FILE]) && zl->fp) {
		va_list ac;
		time_print(zl->fp, zl->timestamp);
		if (zl->record_priority)
			fprintf(zl->fp, "%s: ", zlog_priority[priority]);
		fprintf(zl->fp, "%s: ", zlog_proto_string[zl->protocol]);
		va_copy(ac, args);
		vfprintf(zl->fp, format, ac);
		va_end(ac);
		fprintf(zl->fp, "\n");
		fflush(zl->fp);
	}

	/* stdout output. */
	if (priority <= zl->maxlvl[ZLOG_DEST_STDOUT]) {
		va_list ac;
		time_print(stdout, zl->timestamp);
		if (zl->record_priority)
			fprintf(stdout, "%s: ", zlog_priority[priority]);
		fprintf(stdout, "%s: ", zlog_proto_string[zl->protocol]);
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
	/* Terminal monitor. */
	if (priority <= zl->maxlvl[ZLOG_DEST_MONITOR])
	{
		vty_log((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_string[zl->protocol], format, zl->timestamp, args);
	}
	//trapping
	if (priority == zl->trap_lvl)
		vty_trap_log((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_string[zl->protocol], format, zl->timestamp, args);

	if (module != ZLOG_NONE) {
		zl->protocol = protocol;
	}
	errno = original_errno;
	if (zl->mutex)
		os_mutex_unlock(zl->mutex);
}

static void zlog_out(int module, int priority, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vzlog_output(zlog_default, module, priority, format, args);
	va_end(args);
}

void vzlog(struct zlog *zl, int module, int priority, const char *format,
		va_list args)
{
	int protocol = 0;

	/* If zlog is not specified, use default one. */
	if (zl == NULL)
		zl = zlog_default;

	if (zl->mutex)
		os_mutex_lock(zl->mutex, OS_WAIT_FOREVER);
	if (module != ZLOG_NONE)
	{
		protocol = zl->protocol;
		zl->protocol = module;
	}
	if (zl == NULL)
	{
		time_print(stderr, zl->timestamp);
		fprintf(stderr, "%s: ", "unknown");
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
		fflush(stderr);

		if (module != ZLOG_NONE) {
			zl->protocol = protocol;
		}
		if (zl->mutex)
			os_mutex_unlock(zl->mutex);
		return;
	}
	if (priority <= zl->maxlvl[ZLOG_DEST_SYSLOG] ||
			priority <= zl->maxlvl[ZLOG_DEST_BUFFER] ||
			((priority <= zl->maxlvl[ZLOG_DEST_FILE]) && zl->fp) ||
			priority <= zl->maxlvl[ZLOG_DEST_MONITOR] ||
			priority == zl->trap_lvl ||
			priority <= zl->maxlvl[ZLOG_DEST_STDOUT])
	{
		va_list ac;
		zlog_hdr_t loghdr;
		memset(&loghdr, 0, sizeof(loghdr));
		loghdr.module = module;
		loghdr.priority = priority;
		//loghdr.len;
		//loghdr.logbuf[LOG_MSG_SIZE];

		va_copy(ac, args);
		loghdr.len = vsnprintf(loghdr.logbuf, sizeof(loghdr.logbuf), format, ac);
		va_end(ac);
		if(zl->lfd)
			write(zl->lfd, &loghdr, sizeof(loghdr));
	}
	if (zl->mutex)
		os_mutex_unlock(zl->mutex);
	return ;
}

static int zlog_task(struct zlog *zl)
{
	int retval = 0, interval = 0;
	fd_set rfds;
	zlog_hdr_t loghdr;
	os_sleep(1);
	memset(&loghdr, 0, sizeof(loghdr));
	FD_ZERO (&rfds);
	//FD_SET (zl->lfd, &rfds);
	while (1)
	{
		if(zl->lfd == 0)
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

		interval++;

		if(interval >= 10)
		{
			interval = 0;
			zlog_check_file();
		}

		if (retval == 0)
			continue;

		if (!FD_ISSET(zl->lfd, &rfds))
			continue;
		memset(&loghdr, 0, sizeof(loghdr));
		if (read(zl->lfd, &loghdr, sizeof(loghdr)) < 0)
		{
			if (errno == EPIPE || errno == EBADF || errno == EIO /*ECONNRESET ECONNABORTED ENETRESET ECONNREFUSED*/)
			{
				close(zl->lfd);
				zl->lfd = os_pipe_create("zlog.p", O_RDWR);
				if(zl->lfd)
				{
					os_set_nonblocking(zl->lfd);
				}
				continue;
			}
			continue;
		}
		if(loghdr.len)
			zlog_out(loghdr.module, loghdr.priority, loghdr.logbuf);
	}
	return OK;
}

static int zlog_task_init(struct zlog *zl)
{
	if(zl->lfd)
	{
		close(zl->lfd);
		zl->lfd = 0;
	}
	zl->lfd = os_pipe_create("zlog.p", O_RDWR);
	if(zl->lfd)
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
	if(zl->lfd)
	{
		close(zl->lfd);
		zl->lfd = 0;
	}
	return OK;
}
#else
/* va_list version of zlog. */
void vzlog(struct zlog *zl, int module, int priority, const char *format,
		va_list args) {
	int protocol = 0;
	int original_errno = errno;

	/* If zlog is not specified, use default one. */
	if (zl == NULL)
		zl = zlog_default;
	if (zl->mutex)
		os_mutex_lock(zl->mutex, OS_WAIT_FOREVER);
	if (module != ZLOG_NONE) {
		protocol = zl->protocol;
		zl->protocol = module;
	}
	/* When zlog_default is also NULL, use stderr for logging. */
	if (zl == NULL) {
		time_print(stderr, zl->timestamp);
		fprintf(stderr, "%s: ", "unknown");
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
		fflush(stderr);

		/* In this case we return at here. */
		errno = original_errno;
		if (module != ZLOG_NONE) {
			zl->protocol = protocol;
		}
		if (zl->mutex)
			os_mutex_unlock(zl->mutex);
		return;
	}

	/* Syslog output */
	if (priority <= zl->maxlvl[ZLOG_DEST_SYSLOG]) {
		va_list ac;
		va_copy(ac, args);
#ifndef SYSLOG_CLIENT
		vsyslog(priority | zlog_default->facility, format, ac);
#else
		vsysclog(priority, format, args);
#endif
		va_end(ac);
	}

	/* File output. */
	if ((priority <= zl->maxlvl[ZLOG_DEST_FILE]) && zl->fp) {
		va_list ac;
		time_print(zl->fp, zl->timestamp);
		if (zl->record_priority)
			fprintf(zl->fp, "%s: ", zlog_priority[priority]);
		fprintf(zl->fp, "%s: ", zlog_proto_string[zl->protocol]);
		va_copy(ac, args);
		vfprintf(zl->fp, format, ac);
		va_end(ac);
		fprintf(zl->fp, "\n");
		fflush(zl->fp);
	}

	/* stdout output. */
	if (priority <= zl->maxlvl[ZLOG_DEST_STDOUT]) {
		va_list ac;
		time_print(stdout, zl->timestamp);
		if (zl->record_priority)
			fprintf(stdout, "%s: ", zlog_priority[priority]);
		fprintf(stdout, "%s: ", zlog_proto_string[zl->protocol]);
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
	/* Terminal monitor. */
	if (priority <= zl->maxlvl[ZLOG_DEST_MONITOR])
	{
		vty_log((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_string[zl->protocol], format, zl->timestamp, args);
	}
	//trapping
	if (priority == zl->trap_lvl)
		vty_trap_log((zl->record_priority ? zlog_priority[priority] : NULL),
				zlog_proto_string[zl->protocol], format, zl->timestamp, args);

	if (module != ZLOG_NONE) {
		zl->protocol = protocol;
	}
	errno = original_errno;
	if (zl->mutex)
		os_mutex_unlock(zl->mutex);
}
#endif

static char *
str_append(char *dst, int len, const char *src) {
	while ((len-- > 0) && *src)
		*dst++ = *src++;
	return dst;
}

static char *
num_append(char *s, int len, u_long x) {
	char buf[30];
	char *t;

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
static char *
hex_append(char *s, int len, u_long x)
{
	char buf[30];
	char *t;

	if (!x)
	return str_append(s,len,"0");
	*(t = &buf[sizeof(buf)-1]) = '\0';
	while (x && (t > buf))
	{
		u_int cc = (x % 16);
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
	char *s;
	struct sockaddr_un addr;

	if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
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
	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}
	return fd;
#endif
}

static void syslog_sigsafe(int priority, const char *msg, size_t msglen) {
	static int syslog_fd = -1;
	char buf[sizeof("<1234567890>ripngd[1234567890]: ") + msglen + 50];
	char *s;

	if ((syslog_fd < 0) && ((syslog_fd = syslog_connect()) < 0))
		return;

#define LOC s,buf+sizeof(buf)-s
	s = buf;
	s = str_append(LOC, "<");
	s = num_append(LOC, priority);
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
		size_t ilen;

		for (p = zlog_default->ident, ilen = 0; *p; p++)
			ilen++;
		{
			char buf[sizeof(CRASHLOG_PREFIX) + ilen + sizeof(CRASHLOG_SUFFIX)
					+ 3];
			char *s = buf;
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
	time_t now;
	char buf[sizeof("DEFAULT: Received signal S at T (si_addr 0xP, PC 0xP); aborting...")
			+ 100];
	char *s = buf;
	char *msgstart = buf;
#define LOC s,buf+sizeof(buf)-s

	time(&now);
	if (zlog_default) {
		s = str_append(LOC, zlog_proto_string[zlog_default->protocol]);
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
	if (s < buf + sizeof(buf))
		*s++ = '\n';

	/* N.B. implicit priority is most severe */
#define PRI LOG_CRIT

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
		if (PRI <= zlog_default->maxlvl[ZLOG_DEST_MONITOR])
			vty_log_fixed(buf, s - buf);
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
	{
		s = str_append(LOC, "in thread ");
		s = str_append(LOC, zlog_backtrace_funcname());
		s = str_append(LOC, " scheduled from ");
		s = str_append(LOC, zlog_backtrace_schedfrom());
		s = str_append(LOC, ":");
		s = num_append(LOC, zlog_backtrace_schedfrom_line());
		s = str_append(LOC, "\n");
	}


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
		if (PRI <= zlog_default->maxlvl[ZLOG_DEST_MONITOR])
			vty_log_fixed(buf, s - buf);
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
void zlog_backtrace_sigsafe(int priority, void *program_counter)
{
#ifdef HAVE_STACK_TRACE
	static const char pclabel[] = "Program counter: ";
	void *array[64];
	int size;
	char buf[100];
	char *s, **bt = NULL;
#define LOC s,buf+sizeof(buf)-s

#ifdef HAVE_GLIBC_BACKTRACE
	size = backtrace(array, array_size(array));
	if (size <= 0 || (size_t)size > array_size(array))
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
		if (priority <= zlog_default->maxlvl[ZLOG_DEST_MONITOR])
			vty_log_fixed(buf,s-buf);
		if (priority <= zlog_default->maxlvl[ZLOG_DEST_SYSLOG])
			syslog_sigsafe(priority|zlog_default->facility,buf,s-buf);
		{
			int i;
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
				if (priority <= zlog_default->maxlvl[ZLOG_DEST_MONITOR])
					vty_log_fixed(buf,s-buf);
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

void zlog_backtrace(int priority)
{
#ifndef HAVE_GLIBC_BACKTRACE
	zlog(NULL, priority, "No backtrace available on this platform.");
#else
	void *array[20];
	int size, i;
	char **strings;
	size = backtrace(array, array_size(array));
	if (size <= 0 || (size_t)size > array_size(array))
	{
		zlog_err(ZLOG_DEFAULT, "Cannot get backtrace, returned invalid # of frames %d "
				"(valid range is between 1 and %lu)",
				size, (unsigned long)(array_size(array)));
		return;
	}
	zlog(ZLOG_DEFAULT, priority, "Backtrace for %d stack frames:", size);
	if (!(strings = backtrace_symbols(array, size)))
	{
		zlog_err(ZLOG_DEFAULT, "Cannot get backtrace symbols (out of memory?)");
		for (i = 0; i < size; i++)
			zlog(ZLOG_DEFAULT, priority, "[bt %d] %p",i,array[i]);
	}
	else
	{
		for (i = 0; i < size; i++)
			zlog(ZLOG_DEFAULT, priority, "[bt %d] %s",i,strings[i]);
		free(strings);
	}
#endif /* HAVE_GLIBC_BACKTRACE */
}

void zlog(int module, int priority, const char *format, ...) {
	va_list args;
	va_start(args, format);
	vzlog(zlog_default, module, priority, format, args);
	va_end(args);
}

#define ZLOG_FUNC(FUNCNAME,PRIORITY) \
void \
FUNCNAME(int module, const char *format, ...) \
{ \
  va_list args; \
  va_start(args, format); \
  vzlog (zlog_default, module, PRIORITY, format, args); \
  va_end(args); \
}

ZLOG_FUNC(zlog_err, LOG_ERR)

ZLOG_FUNC(zlog_warn, LOG_WARNING)

ZLOG_FUNC(zlog_info, LOG_INFO)

ZLOG_FUNC(zlog_notice, LOG_NOTICE)

ZLOG_FUNC(zlog_debug, LOG_DEBUG)

ZLOG_FUNC(zlog_trap, LOG_TRAP)

#undef ZLOG_FUNC

static void zlog_thread_info(int log_level)
{
	zlog(ZLOG_DEFAULT, log_level,
		"Current thread/eloop function %s, scheduled from "
				"file %s, line %u", zlog_backtrace_funcname(),
				zlog_backtrace_schedfrom(), zlog_backtrace_schedfrom_line());
}

void _zlog_assert_failed(const char *assertion, const char *file,
		unsigned int line, const char *function)
{
	/* Force fallback file logging? */
	if (zlog_default && !zlog_default->fp
			&& ((logfile_fd = open_crashlog()) >= 0) && ((zlog_default->fp =
					fdopen(logfile_fd, "w")) != NULL))
		zlog_default->maxlvl[ZLOG_DEST_FILE] = LOG_ERR;
	zlog(ZLOG_DEFAULT, LOG_CRIT,
			"Assertion `%s' failed in file %s, line %u, function %s", assertion,
			file, line, (function ? function : "?"));
	zlog_backtrace(LOG_CRIT);
	zlog_thread_info(LOG_CRIT);
	abort();
}

/* Open log stream */
struct zlog *
openzlog(const char *progname, zlog_proto_t protocol, int syslog_flags,
		int syslog_facility) {
	struct zlog *zl;
	u_int i;

	zl = XCALLOC(MTYPE_ZLOG, sizeof(struct zlog));

	zl->mutex = os_mutex_init();
	zl->ident = progname;
	zl->protocol = protocol;
	zl->facility = syslog_facility;
	zl->syslog_options = syslog_flags;

	/* Set default logging levels. */
	for (i = 0; i < array_size(zl->maxlvl); i++)
	{
		zl->maxlvl[i] = ZLOG_DISABLED;
		zl->default_lvl[i] = ZLOG_DISABLED;
	}

	zl->default_lvl[ZLOG_DEST_SYSLOG] = LOG_WARNING;
	zl->default_lvl[ZLOG_DEST_STDOUT] = LOG_ERR;
	zl->default_lvl[ZLOG_DEST_BUFFER] = LOG_NOTICE;
	zl->default_lvl[ZLOG_DEST_MONITOR] = LOG_NOTICE;
	zl->default_lvl[ZLOG_DEST_FILE] = LOG_ERR;

	zl->trap_lvl = LOG_TRAP;

	openlog(progname, syslog_flags, zl->facility);
#ifdef SYSLOG_CLIENT
	syslogc_lib_init(NULL, progname);
#endif
	//zl->maxlvl[ZLOG_DEST_STDOUT] = zl->default_lvl;

	zl->timestamp = ZLOG_TIMESTAMP_DATE;
	zl->log_buffer.max_size = ZLOG_BUFF_SIZE;
	zl->log_buffer.start = 0;

	zlog_default = zl;

	zlog_buffer_reset ();
	zlog_set_level (ZLOG_DEST_STDOUT, LOG_DEBUG);
#ifdef ZLOG_TASK_ENABLE
	zlog_task_init(zlog_default);
#endif
	return zl;
}

void closezlog(struct zlog *zl) {
	closelog();

	if (zl->fp != NULL)
		fclose(zl->fp);

	if (zl->filename != NULL)
		free(zl->filename);
	if (zl->mutex)
		os_mutex_exit(zl->mutex);
#ifdef ZLOG_TASK_ENABLE
	zlog_task_exit(zlog_default);
#endif
	XFREE(MTYPE_ZLOG, zl);
#ifdef SYSLOG_CLIENT
	syslogc_lib_uninit();
#endif
}

/* Called from command.c. */

void zlog_set_trap(int level) {

	int i = 0;
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


void zlog_get_trap(int *level)
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

void zlog_set_facility(int facility) {

	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->facility = facility;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


void zlog_get_facility(int *facility)
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

void zlog_set_record_priority(int record_priority) {

	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->record_priority = record_priority;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


void zlog_get_record_priority(int *record_priority)
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

void zlog_set_level(zlog_dest_t dest, int log_level) {

	if (zlog_default == NULL)
		return;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	zlog_default->maxlvl[dest] = log_level;
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
}


void zlog_get_level(zlog_dest_t dest, int *log_level)
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
int zlog_reset_file(int bOpen)
{

	char filetmp[256];
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
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	if(bOpen)
		return zlog_set_file(zlog_default->filename, zlog_default->maxlvl[ZLOG_DEST_FILE]);
	return OK;
}

int zlog_set_file(const char *filename, int log_level)
{
	FILE *fp;
	mode_t oldumask;
	char filetmp[256];
	char *file_name = NULL;
	const char *default_file = ZLOG_FILE_DEFAULT;
	if (zlog_default == NULL)
		return ERROR;
	if(filename)
		file_name = (char *)filename;
	else
		file_name = (char *)default_file;

	zlog_reset_file(FALSE);

	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);

/*	if(nsmlog.t_time)
	{
		eloop_cancel(nsmlog.t_time);
		nsmlog.t_time = NULL;
	}*/
	/* Open file. */
	memset(filetmp, 0, sizeof(filetmp));
	sprintf(filetmp, "%s%s", ZLOG_VIRTUAL_PATH, filename);
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
	zlog_default->filename = strdup(filename);
	zlog_default->maxlvl[ZLOG_DEST_FILE] = log_level;
	zlog_default->fp = fp;
	logfile_fd = fileno(fp);

	if (host_config_set_api(API_SET_LOGFILE_CMD, filename) != OK) {
		if (zlog_default->mutex)
			os_mutex_unlock(zlog_default->mutex);
		return ERROR;
	}

	//nsmlog.t_time = eloop_add_timer (nsmlog.master, zlog_file_time_thread, NULL, LOG_FILE_CHK_TIME);
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}

int zlog_get_file(const char *filename, int *log_level)
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

int zlog_close_file()
{

	//char filetmp[256];
	if (zlog_default == NULL)
		return 0;

	zlog_reset_file(FALSE);

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
int
zlog_set_file_size (int filesize)
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
zlog_get_file_size (int *filesize)
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


static int zlog_file_move(const char *src, const char *dest)
{
	char buff[1024];
	int len;
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
	char filetmp[256];
	char filereal[256];
	char filereal2[256];
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
		sync();
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}
/* Check log filesize. */
#if 1
int zlog_check_file (void)
{
	struct stat fsize;
	char filetmp[256];
	char filereal[256];
	char filereal2[256];
	if (zlog_default == NULL)
		return ERROR;
	memset (filetmp, 0, sizeof(filetmp));
	memset (filereal, 0, sizeof(filereal));
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	sprintf (filetmp, "%s%s", ZLOG_VIRTUAL_PATH, zlog_default->filename);
	if (access (filetmp, F_OK) >= 0)
	{
		memset (&fsize, 0, sizeof(fsize));
		(void) stat (filetmp, &fsize);

		if (fsize.st_size < (zlog_default->filesize * ZLOG_1M))
		{
			if (zlog_default->mutex)
				os_mutex_unlock(zlog_default->mutex);
			return OK;
		}
		if (zlog_default->fp)
		{
			fflush (zlog_default->fp);
			fclose (zlog_default->fp);
		}
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
		sync();
		zlog_default->fp = fopen (filetmp, "a");
		if (zlog_default->fp == NULL)
		{
			if (zlog_default->mutex)
				os_mutex_unlock(zlog_default->mutex);
			return ERROR;
		}
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}
#endif
/******************************************************/
/*static int zlog_buffer_insert_one(zlog_buffer_t *buffer, int module, int level, int len, const char *log)
{
	int index = buffer->start;

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
		int module, int level, char *format, va_list args)
{
	va_list ac;
	int len;
	int index = buffer->start;
	if(index == buffer->max_size)
		index = 0;

	os_memset(buffer->buffer[index].log, 0, LOG_MSG_SIZE);
	len = quagga_timestamp(zl->timestamp, buffer->buffer[index].log, sizeof(buffer->buffer[index].log));

	buffer->buffer[index].log[len++] = ' ';
	buffer->buffer[index].log[len++] = '\0';

	if (zl->record_priority)
		len += sprintf(buffer->buffer[index].log + len, "%s: ", zlog_priority[level]);
	len += sprintf(buffer->buffer[index].log + len, "%s: ", zlog_proto_string[zl->protocol]);
	va_copy(ac, args);
	len += vsprintf(buffer->buffer[index].log + len, format, ac);
	va_end(ac);
	len += sprintf(buffer->buffer[index].log + len, "\n");

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
	int size = sizeof(zbuffer_t)*zlog_default->log_buffer.max_size;
	if(zlog_default->log_buffer.buffer)
		XFREE(MTYPE_ZLOG, zlog_default->log_buffer.buffer);
	zlog_default->log_buffer.start = 0;
	zlog_default->log_buffer.buffer = XMALLOC(MTYPE_ZLOG, size);
	memset(zlog_default->log_buffer.buffer, 0 , size);
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return OK;
}


int zlog_set_buffer_size (int size)
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

int zlog_get_buffer_size (int *size)
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
	int i = 0;
	FILE	 *fp = NULL;
	char filereal[256];
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
	if(zlog_default->maxlvl[ZLOG_DEST_BUFFER])
	{
		sprintf (filereal, "%s%s", ZLOG_REAL_PATH, zlog_default->filename);

		//if(zlog_default->fp)
		//	vos_wdog_cancel(zlog_default->zlog_wdog);

		fp = fopen (filereal, "a");
		if (fp == NULL)
		{
			if (zlog_default->mutex)
					os_mutex_unlock(zlog_default->mutex);
			return ERROR;
		}
		if(fp)
		{
			for(i = zlog_default->log_buffer.start; i < zlog_default->log_buffer.max_size; i++)
			{
				if(zlog_default->log_buffer.buffer[i].size)
					fprintf (fp, "%s: ", zlog_default->log_buffer.buffer[i].log);
				fflush (fp);
			}
			for(i = 0; i < zlog_default->log_buffer.start; i++)
			{
				if(zlog_default->log_buffer.buffer[i].size)
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
	int i = 0;
	int ret = 0;
	if (zlog_default == NULL)
		return 0;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);
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
int zlog_rotate()
{
	int level;

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

	if (zlog_default->filename) {
		mode_t oldumask;
		int save_errno;

		oldumask = umask(0777 & ~LOGFILE_MASK);
		zlog_default->fp = fopen(zlog_default->filename, "a");
		save_errno = errno;
		umask(oldumask);
		if (zlog_default->fp == NULL) {
			zlog_err(ZLOG_DEFAULT,
					"Log rotate failed: cannot open file %s for append: %s",
					zlog_default->filename, safe_strerror(save_errno));
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
lookup(const struct message *mes, int key) {
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
mes_lookup(const struct message *meslist, int max, int index, const char *none,
		const char *mesname) {
	int pos = index - meslist[0].key;

	/* first check for best case: index is in range and matches the key
	 * value in that slot.
	 * NB: key numbering might be offset from 0. E.g. protocol constants
	 * often start at 1.
	 */
	if ((pos >= 0) && (pos < max) && (meslist[pos].key == index))
		return meslist[pos].str;

	/* fall back to linear search */
	{
		int i;

		for (i = 0; i < max; i++, meslist++) {
			if (meslist->key == index) {
				const char *str = (meslist->str ? meslist->str : none);

				zlog_debug(ZLOG_DEFAULT,
						"message index %d [%s] found in %s at position %d (max is %d)",
						index, str, mesname, i, max);
				return str;
			}
		}
	}
	zlog_err(ZLOG_DEFAULT, "message index %d not found in %s (max is %d)",
			index, mesname, max);
	assert(none);
	return none;
}

/* Wrapper around strerror to handle case where it returns NULL. */
const char *
safe_strerror(int errnum) {
	const char *s = strerror(errnum);
	return (s != NULL) ? s : "Unknown error";
}

#define DESC_ENTRY(T) [(T)] = { (T), (#T), '\0' }
static const struct zebra_desc_table command_types[] = {
DESC_ENTRY (ZEBRA_INTERFACE_ADD),
DESC_ENTRY (ZEBRA_INTERFACE_DELETE),
DESC_ENTRY (ZEBRA_INTERFACE_ADDRESS_ADD),
DESC_ENTRY (ZEBRA_INTERFACE_ADDRESS_DELETE),
DESC_ENTRY (ZEBRA_INTERFACE_UP),
DESC_ENTRY (ZEBRA_INTERFACE_DOWN),
DESC_ENTRY (ZEBRA_IPV4_ROUTE_ADD),
DESC_ENTRY (ZEBRA_IPV4_ROUTE_DELETE),
DESC_ENTRY (ZEBRA_IPV6_ROUTE_ADD),
DESC_ENTRY (ZEBRA_IPV6_ROUTE_DELETE),
DESC_ENTRY (ZEBRA_REDISTRIBUTE_ADD),
DESC_ENTRY (ZEBRA_REDISTRIBUTE_DELETE),
DESC_ENTRY (ZEBRA_REDISTRIBUTE_DEFAULT_ADD),
DESC_ENTRY (ZEBRA_REDISTRIBUTE_DEFAULT_DELETE),
DESC_ENTRY (ZEBRA_IPV4_NEXTHOP_LOOKUP),
DESC_ENTRY (ZEBRA_IPV6_NEXTHOP_LOOKUP),
DESC_ENTRY (ZEBRA_IPV4_IMPORT_LOOKUP),
DESC_ENTRY (ZEBRA_IPV6_IMPORT_LOOKUP),
DESC_ENTRY (ZEBRA_INTERFACE_RENAME),
DESC_ENTRY (ZEBRA_ROUTER_ID_ADD),
DESC_ENTRY (ZEBRA_ROUTER_ID_DELETE),
DESC_ENTRY (ZEBRA_ROUTER_ID_UPDATE),
DESC_ENTRY (ZEBRA_NEXTHOP_REGISTER),
DESC_ENTRY (ZEBRA_NEXTHOP_UNREGISTER),
DESC_ENTRY (ZEBRA_NEXTHOP_UPDATE), };
#undef DESC_ENTRY

static const struct zebra_desc_table unknown = { 0, "unknown", '?' };

static const struct zebra_desc_table *
zroute_lookup(u_int zroute) {
	u_int i;

	if (zroute >= array_size(route_types)) {
		zlog_err(ZLOG_DEFAULT, "unknown zebra route type: %u", zroute);
		return &unknown;
	}
	if (zroute == route_types[zroute].type)
		return &route_types[zroute];
	for (i = 0; i < array_size(route_types); i++) {
		if (zroute == route_types[i].type) {
			zlog_warn(ZLOG_DEFAULT,
					"internal error: route type table out of order "
							"while searching for %u, please notify developers",
					zroute);
			return &route_types[i];
		}
	}
	zlog_err(ZLOG_DEFAULT,
			"internal error: cannot find route type %u in table!", zroute);
	return &unknown;
}

const char *
zebra_route_string(u_int zroute) {
	return zroute_lookup(zroute)->string;
}

char zebra_route_char(u_int zroute) {
	return zroute_lookup(zroute)->chr;
}

const char *
zserv_command_string(unsigned int command) {
	if (command >= array_size(command_types)) {
		zlog_err(ZLOG_DEFAULT, "unknown zserv command type: %u", command);
		return unknown.string;
	}
	return command_types[command].string;
}

int proto_name2num(const char *s) {
	unsigned i;

	for (i = 0; i < array_size(route_types); ++i)
		if (strcasecmp(s, route_types[i].string) == 0)
			return route_types[i].type;
	return -1;
}

int proto_redistnum(int afi, const char *s) {
	if (!s)
		return -1;
	if (afi == AFI_IP) {
		if (strncmp(s, "sy", 2) == 0)
			return ZEBRA_ROUTE_SYSTEM;
		else if (strncmp(s, "ke", 2) == 0)
			return ZEBRA_ROUTE_KERNEL;
		else if (strncmp(s, "co", 2) == 0)
			return ZEBRA_ROUTE_CONNECT;
		else if (strncmp(s, "st", 2) == 0)
			return ZEBRA_ROUTE_STATIC;
		else if (strncmp(s, "ri", 2) == 0)
			return ZEBRA_ROUTE_RIP;
		else if (strncmp(s, "os", 2) == 0)
			return ZEBRA_ROUTE_OSPF;
		else if (strncmp(s, "is", 2) == 0)
			return ZEBRA_ROUTE_ISIS;
		else if (strncmp(s, "bg", 2) == 0)
			return ZEBRA_ROUTE_BGP;
		else if (strncmp(s, "pi", 2) == 0)
			return ZEBRA_ROUTE_PIM;
		else if (strncmp(s, "hs", 2) == 0)
			return ZEBRA_ROUTE_HSLS;
		else if (strncmp(s, "ol", 2) == 0)
			return ZEBRA_ROUTE_OLSR;
		else if (strncmp(s, "ba", 2) == 0)
			return ZEBRA_ROUTE_BABEL;
		else if (strncmp(s, "n", 1) == 0)
			return ZEBRA_ROUTE_NHRP;
		else if (strncmp(s, "vr", 2) == 0)
			return ZEBRA_ROUTE_VRRP;
		else if (strncmp(s, "fr", 2) == 0)
			return ZEBRA_ROUTE_FRP;
	}
	if (afi == AFI_IP6) {
		if (strncmp(s, "sy", 2) == 0)
			return ZEBRA_ROUTE_SYSTEM;
		else if (strncmp(s, "ke", 2) == 0)
			return ZEBRA_ROUTE_KERNEL;
		else if (strncmp(s, "co", 2) == 0)
			return ZEBRA_ROUTE_CONNECT;
		else if (strncmp(s, "st", 2) == 0)
			return ZEBRA_ROUTE_STATIC;
		else if (strncmp(s, "ri", 2) == 0)
			return ZEBRA_ROUTE_RIPNG;
		else if (strncmp(s, "os", 2) == 0)
			return ZEBRA_ROUTE_OSPF6;
		else if (strncmp(s, "is", 2) == 0)
			return ZEBRA_ROUTE_ISIS;
		else if (strncmp(s, "bg", 2) == 0)
			return ZEBRA_ROUTE_BGP;
		else if (strncmp(s, "pi", 2) == 0)
			return ZEBRA_ROUTE_PIM;
		else if (strncmp(s, "hs", 2) == 0)
			return ZEBRA_ROUTE_HSLS;
		else if (strncmp(s, "ol", 2) == 0)
			return ZEBRA_ROUTE_OLSR;
		else if (strncmp(s, "ba", 2) == 0)
			return ZEBRA_ROUTE_BABEL;
		else if (strncmp(s, "n", 1) == 0)
			return ZEBRA_ROUTE_NHRP;
		else if (strncmp(s, "vr", 2) == 0)
			return ZEBRA_ROUTE_VRRP;
		else if (strncmp(s, "fr", 2) == 0)
			return ZEBRA_ROUTE_FRP;
	}
	return -1;
}

void zlog_hexdump(void *mem, unsigned int len) {
	unsigned long i = 0;
	unsigned int j = 0;
	unsigned int columns = 8;
	char buf[(len * 4) + ((len / 4) * 20) + 30];
	char *s = buf;

	for (i = 0; i < len + ((len % columns) ? (columns - len % columns) : 0);
			i++) {
		/* print offset */
		if (i % columns == 0)
			s += sprintf(s, "0x%016lx: ", (unsigned long) mem + i);

		/* print hex data */
		if (i < len)
			s += sprintf(s, "%02x ", 0xFF & ((char*) mem)[i]);

		/* end of block, just aligning for ASCII dump */
		else
			s += sprintf(s, "   ");

		/* print ASCII dump */
		if (i % columns == (columns - 1)) {
			for (j = i - (columns - 1); j <= i; j++) {
				if (j >= len) /* end of block, not really printing */
					s += sprintf(s, " ");

				else if (isprint((int) ((char*) mem)[j])) /* printable char */
					s += sprintf(s, "%c", 0xFF & ((char*) mem)[j]);

				else
					/* other char */
					s += sprintf(s, ".");
			}
			s += sprintf(s, "\n");
		}
	}
	zlog_debug(ZLOG_DEFAULT, "\n%s", buf);
}
