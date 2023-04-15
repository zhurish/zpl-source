/*
 * Zebra logging funcions.
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

#ifndef __LIB_LOG_H
#define __LIB_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"


#define ZLOG_TASK_ENABLE
#define ZLOG_TESTING_ENABLE

//#define ZPL_SERVICE_SYSLOG
#ifdef ZPL_SERVICE_SYSLOG
#include "syslogcLib.h"
#endif
/* Here is some guidance on logging levels to use:
 *
 * ZLOG_LEVEL_DEBUG	- For all messages that are enabled by optional debugging
 *		  features, typically preceded by "if (IS...DEBUG...)"
 * ZLOG_LEVEL_INFO	- Information that may be of interest, but everything seems
 *		  to be working properly.
 * ZLOG_LEVEL_NOTICE	- Only for message pertaining to daemon startup or shutdown.
 * ZLOG_LEVEL_WARNING	- Warning conditions: unexpected events, but the daemon believes
 *		  it can continue to operate correctly.
 * LOG_ERR	- Error situations indicating malfunctions.  Probably require
 *		  attention.
 *
 * Note: ZLOG_LEVEL_CRIT, ZLOG_LEVEL_ALERT, and ZLOG_LEVEL_EMERG are currently not used anywhere,
 * please use LOG_ERR instead.
 */
typedef enum
{
  ZLOG_LEVEL_EMERG	= 0,	/* system is unusable */
  ZLOG_LEVEL_ALERT	= 1,	/* action must be taken immediately */
  ZLOG_LEVEL_CRIT	= 2,	/* critical conditions */
  ZLOG_LEVEL_ERR		= 3,	/* error conditions */
  ZLOG_LEVEL_WARNING	= 4,	/* warning conditions */
  ZLOG_LEVEL_NOTICE	= 5,	/* normal but significant condition */
  ZLOG_LEVEL_INFO	= 6,	/* informational */
  ZLOG_LEVEL_DEBUG	= 7,	/* debug-level messages */
  ZLOG_LEVEL_TRAP		= 8,
  ZLOG_LEVEL_FORCE_TRAP = 9,
  ZLOG_LEVEL_MAX,
}zlog_level_t;

typedef module_t zlog_proto_t;

#define ZLOG_1M		1024000
#define ZLOG_FILE_SIZE	(2)
#define ZLOG_BUFF_SIZE	512
#define LOG_MSG_SIZE	1024
#define LOG_FILE_CHK_TIME	10

#define ZLOG_REAL_PATH		RSYSLOGDIR"/"
#define ZLOG_VIRTUAL_PATH 	DAEMON_LOG_FILE_DIR"/"

#define ZLOG_FILE_DEFAULT 	"sw-log.log"

#define CRASHLOG_PREFIX 	ZLOG_REAL_PATH"sw."
#define CRASHLOG_SUFFIX 	"crashlog"

/* If maxlvl is set to ZLOG_DISABLED, then no messages will be sent
   to that logging destination. */
#define ZLOG_DISABLED	(ZLOG_LEVEL_EMERG)

typedef enum
{
  ZLOG_DEST_SYSLOG = 0,
  ZLOG_DEST_STDOUT,
  ZLOG_DEST_BUFFER,
  ZLOG_DEST_MONITOR,
  ZLOG_DEST_FILE,
} zlog_dest_t;
#define ZLOG_NUM_DESTS		(ZLOG_DEST_FILE+1)

typedef enum
{
  ZLOG_TIMESTAMP_NONE = 0,
  ZLOG_TIMESTAMP_DATE,
  ZLOG_TIMESTAMP_SHORT,
  ZLOG_TIMESTAMP_BSD,
  ZLOG_TIMESTAMP_ISO,
  ZLOG_TIMESTAMP_RFC3164,
  ZLOG_TIMESTAMP_RFC3339,
} zlog_timestamp_t;


typedef struct zbuffer_s
{
	zpl_uint32 	module;
	zlog_level_t 	level;
	zpl_uint32		size;
	zpl_char 	log[LOG_MSG_SIZE];
}zbuffer_t;

typedef struct zlog_buffer_s
{
	zpl_uint32 max_size;
	zpl_uint32 start;
	zbuffer_t 	*buffer;
}zlog_buffer_t;

#ifdef ZLOG_TESTING_ENABLE
typedef struct zlog_testing_s
{
	zpl_char 	*filename;
	zlog_level_t		priority;
	FILE 	*fp;
	zpl_uint32 	filesize;
	zpl_uint32 	file_check_interval;
}zlog_testing_t;
#endif

struct zlog 
{
  const char *ident;	/* daemon name (first arg to openlog) */
  zlog_proto_t protocol;
  zlog_level_t maxlvl[ZLOG_NUM_DESTS];	/* maximum priority to send to associated
  				   logging destination */
  zlog_level_t default_lvl[ZLOG_NUM_DESTS];	/* maxlvl to use if none is specified */
  zpl_bool	trap_lvl;
  FILE *fp;
  zpl_char *filename;
  zpl_uint32 filesize;
  zpl_uint32 file_check_interval;
  zpl_uint32 facility;		/* as per syslog facility */
  zpl_uint32 record_priority;	/* should messages logged through stdio include the
  			   priority of the message? */
  zpl_uint32 syslog_options;	/* 2nd arg to openlog */
  zlog_timestamp_t timestamp;	/* # of digits of subsecond precision */

  zlog_buffer_t	log_buffer;

  void *mutex;
  enum
  {
	  ZLOG_DEPTH_NONE,		//not enable
	  ZLOG_DEPTH_LEVEL1,	//FILE LINE
	  ZLOG_DEPTH_LEVEL2,	//LWP FILE LINE
	  ZLOG_DEPTH_LEVEL3,	//LWP(name) FILE FUNC LINE
  }depth_debug;
#define ZLOG_DEPTH_DEBUG_DEFAULT ZLOG_DEPTH_LEVEL2
#ifdef ZLOG_TASK_ENABLE
  zpl_taskid_t	taskid;
  int	lfd;
  FILE  *lfp;
#endif
#ifdef ZLOG_TESTING_ENABLE
  zpl_bool	testing;
  zlog_testing_t testlog;
#endif
};
#ifdef ZLOG_TASK_ENABLE
typedef struct zlog_hdr_s
{
	zpl_uint32 	module;
	zlog_level_t 	priority;
	zpl_uint32		len;
	zpl_char 	logbuf[LOG_MSG_SIZE];
}zlog_hdr_t;
#endif



typedef int(*zlog_buffer_cb)(zbuffer_t *, void *pVoid);

/* Message structure. */
struct message
{
  zpl_uint32 key;
  const char *str;
};

/* Default logging strucutre. */
extern struct zlog *zlog_default;

/* Open zlog function */
extern struct zlog *openzlog (const char *progname, zlog_proto_t protocol,
		              zpl_uint32 syslog_options, zpl_uint32 syslog_facility);

/* Close zlog function. */
extern void closezlog (struct zlog *zl);
extern void openzlog_start(struct zlog *zl);

/* GCC have printf type attribute check.  */
#ifdef __GNUC__
#define PRINTF_ATTRIBUTE(a,b) __attribute__ ((__format__ (__printf__, a, b)))
#else
#define PRINTF_ATTRIBUTE(a,b)
#endif /* __GNUC__ */


extern int zlog_depth_debug_detail(FILE *fp, zpl_char *buf, zpl_uint32 depth, const char *file,
		const char *func, const zpl_uint32 line);

extern void pl_vzlog(const char *file, const char *func, const zpl_uint32 line, struct zlog *zl, zpl_uint32 module, zlog_level_t priority, const char *format,
		va_list args);
/* Generic function for zlog. */
extern void pl_zlog (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, zlog_level_t priority, const char *format, ...)
  PRINTF_ATTRIBUTE(6, 7);

/* Handy zlog functions. */

extern void pl_zlog_emergencies (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_alerts (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_critical (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_err (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_warn (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_info (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_notice (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_debug (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_trap (const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);


#define vzlog(obj, module, pri, format, arg) 		pl_vzlog (__FILE__, __FUNCTION__, __LINE__, obj, module, pri, format, arg)
#define vzlog_other(obj, module, pri, format, arg) 		pl_vzlog (NULL, NULL, NULL, obj, module, pri, format, arg)
#define zlog(module, pri, format, ...) 				pl_zlog (__FILE__, __FUNCTION__, __LINE__, module, pri, format, ##__VA_ARGS__)
#define zlog_other(module, pri, format, ...) 		pl_zlog (NULL, NULL, 0, module, pri, format, ##__VA_ARGS__)


#define zlog_emergencies(module, format, ...) 	pl_zlog_emergencies (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_alerts(module, format, ...) 				pl_zlog_alerts (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_critical(module, format, ...) 			pl_zlog_critical (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_err(module, format, ...) 				pl_zlog_err (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_warn(module, format, ...) 				pl_zlog_warn (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_info(module, format, ...) 				pl_zlog_info (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_notice(module, format, ...) 			pl_zlog_notice (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_debug(module, format, ...) 			pl_zlog_debug (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_trap(module, format, ...) 				pl_zlog_trap (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_force_trap(module, format, ...) 	pl_zlog (__FILE__, __FUNCTION__, __LINE__, module, ZLOG_LEVEL_FORCE_TRAP, format, ##__VA_ARGS__)
#define liblog_trap(format, ...) 	            pl_zlog_trap (__FILE__, __FUNCTION__, __LINE__, MODULE_LIB, format, ##__VA_ARGS__)

#if 0

extern void vzlog(struct zlog *zl, zpl_uint32 module, zpl_uint32 priority, const char *format,
		va_list args);

/* Generic function for zlog. */
extern void zlog (zpl_uint32 module, zpl_uint32 priority, const char *format, ...)
  PRINTF_ATTRIBUTE(3, 4);

/* Handy zlog functions. */
extern void zlog_err (zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_warn (zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_info (zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_notice (zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_debug (zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_trap (zpl_uint32 module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
#endif

/* Set logging level for the given destination.  If the log_level
   argument is ZLOG_DISABLED, then the destination is disabled.
   This function should not be used for file logging (use zlog_set_file
   or zlog_reset_file instead). */
extern void zlog_set_level (zlog_dest_t, zlog_level_t log_level);
extern void zlog_get_level(zlog_dest_t dest, zlog_level_t *log_level);
/* Set logging to the given filename at the specified level. */
extern int zlog_set_file (const char *filename, zlog_level_t log_level);
extern int zlog_get_file(const char *filename, zlog_level_t *log_level);
extern int zlog_set_file_size (zpl_uint32 filesize);
extern int zlog_get_file_size (zpl_uint32 *filesize);
extern int zlog_reset_file (zpl_bool bOpen);
extern int zlog_close_file(void);
extern int zlog_file_save (void);
//extern zpl_uint32 zlog_check_file (void);
#ifdef ZLOG_TESTING_ENABLE
extern int zlog_testing_enable (zpl_bool);
extern zpl_bool zlog_testing_enabled (void);
extern int zlog_testing_file (zpl_char * file);
extern int zlog_testing_priority (zlog_level_t priority);
#endif
extern int zlog_set_buffer_size (zpl_uint32 size);
extern int zlog_get_buffer_size (zpl_uint32 *size);
extern int zlog_buffer_reset(void);
extern int zlog_buffer_save(void);
extern int zlog_buffer_callback_api (zlog_buffer_cb cb, void *pVoid);

/* Rotate log. */
extern int zlog_rotate (void);

extern void zlog_set_facility(zpl_uint32 facility);
extern void zlog_get_facility(zpl_uint32 *facility);

extern void zlog_set_timestamp(zlog_timestamp_t value);
extern void zlog_get_timestamp(zlog_timestamp_t *value);

extern void zlog_set_record_priority(zpl_uint32 record_priority);
extern void zlog_get_record_priority(zpl_uint32 *record_priority);

extern void zlog_set_trap(zpl_bool level);
extern void zlog_get_trap(zpl_bool *level);

extern const char *zlog_facility_name(zpl_uint32 facility);
extern zpl_uint32 zlog_facility_match(const char *str) ;
extern zlog_level_t zlog_priority_match(const char *s);
extern const char *zlog_priority_name(zlog_level_t level);

extern const char * zlog_proto_names(zlog_proto_t module);

extern int cmd_log_init(void);

/* For hackey message lookup and check */
#define LOOKUP_DEF(x, y, def) message_lookup(x, x ## _max, y, def, #x)
#define LOOKUP(x, y) LOOKUP_DEF(x, y, "(no item found)")

extern const char *lookup (const struct message *, zpl_uint32);
extern const char *message_lookup (const struct message *meslist, 
                               zpl_uint32 max, zpl_uint32 index,
                               const char *no_item, const char *mesname);

/* To be called when a fatal signal is caught. */
extern void zlog_signal(int signo, const char *action
#ifdef SA_SIGINFO
			, siginfo_t *siginfo, void *program_counter
#endif
		       );

/* Log a backtrace. */
extern void zlog_backtrace(zlog_level_t priority);

/* Log a backtrace, but in an async-signal-safe way.  Should not be
   called unless the program is about to exit or abort, since it messes
   up the state of zlog file pointers.  If program_counter is non-NULL,
   that is logged in addition to the current backtrace. */
extern void zlog_backtrace_sigsafe(zlog_level_t priority, void *program_counter);

/* Puts a current timestamp in buf and returns the number of characters
   written (not including the terminating NUL).  The purpose of
   this function is to avoid calls to localtime appearing all over the code.
   It caches the most recent localtime result and can therefore
   avoid multiple calls within the same second.  If buflen is too small,
   *buf will be set to '\0', and 0 will be returned. */
#define LOG_TIMESTAMP_LEN 40
extern zpl_size_t os_timestamp(zlog_timestamp_t timestamp /* # subsecond digits */,
			       zpl_char *buf, zpl_size_t buflen);

extern void time_print(FILE *fp, zlog_timestamp_t ctl);

extern void zlog_hexdump(void *mem, zpl_uint32  len);

/* Map a route type to a string.  For example, ZPL_ROUTE_PROTO_RIPNG -> "ripng". */
extern const char *zroute_string(zpl_uint32 route_type);
/* Map a route type to a char.  For example, ZPL_ROUTE_PROTO_RIPNG -> 'R'. */
extern char zroute_keychar(zpl_uint32 route_type);
/* Map a zserv command type to the same string, 
 * e.g. NSM_EVENT_INTERFACE_ADD -> "NSM_EVENT_INTERFACE_ADD" */
/* Map a protocol name to its number. e.g. ZPL_ROUTE_PROTO_BGP->9*/
extern zpl_proto_t proto_name2num(const char *s);
/* Map redistribute X argument to protocol number.
 * unlike proto_name2num, this accepts zpl_int16hands and takes
 * an AFI value to restrict input */
extern zpl_proto_t proto_redistnum(zpl_uint16 afi, const char *s);


/* Defines for use in command construction: */

#define LOG_LEVELS "(emergencies|alerts|critical|errors|warnings|notifications|informational|debugging)"

#define LOG_LEVEL_DESC \
  "System is unusable\n" \
  "Immediate action needed\n" \
  "Critical conditions\n" \
  "Error conditions\n" \
  "Warning conditions\n" \
  "Normal but significant conditions\n" \
  "Informational messages\n" \
  "Debugging messages\n"

#define LOG_FACILITIES "(kern|user|mail|daemon|auth|syslog|lpr|news|uucp|cron|local0|local1|local2|local3|local4|local5|local6|local7)"

#define LOG_FACILITY_DESC \
       "Kernel\n" \
       "User process\n" \
       "Mail system\n" \
       "System daemons\n" \
       "Authorization system\n" \
       "Syslog itself\n" \
       "Line printer system\n" \
       "USENET news\n" \
       "Unix-to-Unix copy system\n" \
       "Cron/at facility\n" \
       "Local use\n" \
       "Local use\n" \
       "Local use\n" \
       "Local use\n" \
       "Local use\n" \
       "Local use\n" \
       "Local use\n" \
       "Local use\n"
 

#include "zassert.h"

#ifdef __cplusplus
}
#endif

#endif /* __LIB_LOG_H */
