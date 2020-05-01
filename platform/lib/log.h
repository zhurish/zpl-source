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

#ifndef _ZEBRA_LOG_H
#define _ZEBRA_LOG_H

#include <plconfig.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>



#define ZLOG_TASK_ENABLE
#define ZLOG_TESTING_ENABLE

//#define PL_SYSLOG_MODULE
#ifdef PL_SYSLOG_MODULE
#include "syslogcLib.h"
#endif
/* Here is some guidance on logging levels to use:
 *
 * LOG_DEBUG	- For all messages that are enabled by optional debugging
 *		  features, typically preceded by "if (IS...DEBUG...)"
 * LOG_INFO	- Information that may be of interest, but everything seems
 *		  to be working properly.
 * LOG_NOTICE	- Only for message pertaining to daemon startup or shutdown.
 * LOG_WARNING	- Warning conditions: unexpected events, but the daemon believes
 *		  it can continue to operate correctly.
 * LOG_ERR	- Error situations indicating malfunctions.  Probably require
 *		  attention.
 *
 * Note: LOG_CRIT, LOG_ALERT, and LOG_EMERG are currently not used anywhere,
 * please use LOG_ERR instead.
 */
#define LOG_TRAP		(LOG_DEBUG+1)

typedef enum 
{
  ZLOG_NONE,
  ZLOG_DEFAULT,		//Default
  ZLOG_CONSOLE,		//Console
  ZLOG_TELNET,		//telnet
  ZLOG_HAL,
  ZLOG_PAL,
  ZLOG_NSM,			//route table manage
  ZLOG_RIP,
  ZLOG_BGP,
  ZLOG_OSPF,
  ZLOG_RIPNG,
  ZLOG_BABEL,
  ZLOG_OSPF6,
  ZLOG_ISIS,
  ZLOG_PIM,
  ZLOG_MASC,
  ZLOG_NHRP,
  ZLOG_HSLS, 
  ZLOG_OLSR, 
  ZLOG_VRRP,
  ZLOG_FRP,
  ZLOG_LLDP,
  ZLOG_BFD,
  ZLOG_LDP,
  ZLOG_SNTP,
  ZLOG_IMISH,
  ZLOG_DHCP,
  ZLOG_WIFI,
  ZLOG_MODEM,
  ZLOG_WEB,
  ZLOG_MQTT,
  ZLOG_SIP,
  ZLOG_APP,
  ZLOG_VOIP,
  ZLOG_SOUND,
  ZLOG_UTILS,
  ZLOG_MAX,
} zlog_proto_t;

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
#define ZLOG_DISABLED	(LOG_EMERG-1)

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
	int 	module;
	int 	level;
	int		size;
	char 	log[LOG_MSG_SIZE];
}zbuffer_t;

typedef struct zlog_buffer_s
{
	int max_size;
	int start;
	zbuffer_t 	*buffer;
}zlog_buffer_t;

#ifdef ZLOG_TESTING_ENABLE
typedef struct zlog_testing_s
{
	char 	*filename;
	int		priority;
	FILE 	*fp;
	int 	filesize;
	int 	file_check_interval;
}zlog_testing_t;
#endif

struct zlog 
{
  const char *ident;	/* daemon name (first arg to openlog) */
  zlog_proto_t protocol;
  int maxlvl[ZLOG_NUM_DESTS];	/* maximum priority to send to associated
  				   logging destination */
  int default_lvl[ZLOG_NUM_DESTS];	/* maxlvl to use if none is specified */
  int	trap_lvl;
  FILE *fp;
  char *filename;
  int filesize;
  int file_check_interval;
  int facility;		/* as per syslog facility */
  int record_priority;	/* should messages logged through stdio include the
  			   priority of the message? */
  int syslog_options;	/* 2nd arg to openlog */
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
  int	taskid;
  int	lfd;
  FILE  *lfp;
#endif
#ifdef ZLOG_TESTING_ENABLE
  BOOL	testing;
  zlog_testing_t testlog;
#endif
};
#ifdef ZLOG_TASK_ENABLE
typedef struct zlog_hdr_s
{
	int 	module;
	int 	priority;
	int		len;
	char 	logbuf[LOG_MSG_SIZE];
}zlog_hdr_t;
#endif


typedef int(*zlog_buffer_cb)(zbuffer_t *, void *pVoid);

/* Message structure. */
struct message
{
  int key;
  const char *str;
};

/* Default logging strucutre. */
extern struct zlog *zlog_default;

/* Open zlog function */
extern struct zlog *openzlog (const char *progname, zlog_proto_t protocol,
		              int syslog_options, int syslog_facility);

/* Close zlog function. */
extern void closezlog (struct zlog *zl);

/* GCC have printf type attribute check.  */
#ifdef __GNUC__
#define PRINTF_ATTRIBUTE(a,b) __attribute__ ((__format__ (__printf__, a, b)))
#else
#define PRINTF_ATTRIBUTE(a,b)
#endif /* __GNUC__ */




extern void pl_vzlog(const char *file, const char *func, const int line, struct zlog *zl, int module, int priority, const char *format,
		va_list args);
/* Generic function for zlog. */
extern void pl_zlog (const char *file, const char *func, const int line, int module, int priority, const char *format, ...)
  PRINTF_ATTRIBUTE(6, 7);

/* Handy zlog functions. */
extern void pl_zlog_err (const char *file, const char *func, const int line, int module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_warn (const char *file, const char *func, const int line, int module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_info (const char *file, const char *func, const int line, int module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_notice (const char *file, const char *func, const int line, int module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_debug (const char *file, const char *func, const int line, int module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);
extern void pl_zlog_trap (const char *file, const char *func, const int line, int module, const char *format, ...) PRINTF_ATTRIBUTE(5, 6);


#define vzlog(obj, module, pri, format, arg) 		pl_vzlog (__FILE__, __FUNCTION__, __LINE__, obj, module, pri, format, arg)
#define vzlog_other(obj, module, pri, format, arg) 		pl_vzlog (NULL, NULL, NULL, obj, module, pri, format, arg)
#define zlog(module, pri, format, ...) 				pl_zlog (__FILE__, __FUNCTION__, __LINE__, module, pri, format, ##__VA_ARGS__)
#define zlog_other(module, pri, format, ...) 		pl_zlog (NULL, NULL, NULL, module, pri, format, ##__VA_ARGS__)

#define zlog_err(module, format, ...) 				pl_zlog_err (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_warn(module, format, ...) 				pl_zlog_warn (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_info(module, format, ...) 				pl_zlog_info (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_notice(module, format, ...) 			pl_zlog_notice (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_debug(module, format, ...) 			pl_zlog_debug (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)
#define zlog_trap(module, format, ...) 				pl_zlog_trap (__FILE__, __FUNCTION__, __LINE__, module, format, ##__VA_ARGS__)


#if 0

extern void vzlog(struct zlog *zl, int module, int priority, const char *format,
		va_list args);

/* Generic function for zlog. */
extern void zlog (int module, int priority, const char *format, ...)
  PRINTF_ATTRIBUTE(3, 4);

/* Handy zlog functions. */
extern void zlog_err (int module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_warn (int module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_info (int module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_notice (int module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_debug (int module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void zlog_trap (int module, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
#endif

/* Set logging level for the given destination.  If the log_level
   argument is ZLOG_DISABLED, then the destination is disabled.
   This function should not be used for file logging (use zlog_set_file
   or zlog_reset_file instead). */
extern void zlog_set_level (zlog_dest_t, int log_level);
extern void zlog_get_level(zlog_dest_t dest, int *log_level);
/* Set logging to the given filename at the specified level. */
extern int zlog_set_file (const char *filename, int log_level);
extern int zlog_get_file(const char *filename, int *log_level);
extern int zlog_set_file_size (int filesize);
extern int zlog_get_file_size (int *filesize);
extern int zlog_reset_file (BOOL bOpen);
extern int zlog_close_file();
extern int zlog_file_save (void);
//extern int zlog_check_file (void);
#ifdef ZLOG_TESTING_ENABLE
extern int zlog_testing_enable (BOOL);
extern BOOL zlog_testing_enabled (void);
extern int zlog_testing_file (char * file);
extern int zlog_testing_priority (int priority);
#endif
extern int zlog_set_buffer_size (int size);
extern int zlog_get_buffer_size (int *size);
extern int zlog_buffer_reset(void);
extern int zlog_buffer_save(void);
extern int zlog_buffer_callback_api (zlog_buffer_cb cb, void *pVoid);

/* Rotate log. */
extern int zlog_rotate ();

extern void zlog_set_facility(int facility);
extern void zlog_get_facility(int *facility);

extern void zlog_set_timestamp(zlog_timestamp_t value);
extern void zlog_get_timestamp(zlog_timestamp_t *value);

extern void zlog_set_record_priority(int record_priority);
extern void zlog_get_record_priority(int *record_priority);

extern void zlog_set_trap(int level);
extern void zlog_get_trap(int *level);

extern const char *zlog_facility_name(int facility);
extern int zlog_facility_match(const char *str) ;
extern int zlog_priority_match(const char *s);
extern const char *zlog_priority_name(int level);

extern const char * zlog_proto_names(zlog_proto_t module);

/* For hackey message lookup and check */
#define LOOKUP_DEF(x, y, def) mes_lookup(x, x ## _max, y, def, #x)
#define LOOKUP(x, y) LOOKUP_DEF(x, y, "(no item found)")

extern const char *lookup (const struct message *, int);
extern const char *mes_lookup (const struct message *meslist, 
                               int max, int index,
                               const char *no_item, const char *mesname);

/* Safe version of strerror -- never returns NULL. */
extern const char *safe_strerror(int errnum);

/* To be called when a fatal signal is caught. */
extern void zlog_signal(int signo, const char *action
#ifdef SA_SIGINFO
			, siginfo_t *siginfo, void *program_counter
#endif
		       );

/* Log a backtrace. */
extern void zlog_backtrace(int priority);

/* Log a backtrace, but in an async-signal-safe way.  Should not be
   called unless the program is about to exit or abort, since it messes
   up the state of zlog file pointers.  If program_counter is non-NULL,
   that is logged in addition to the current backtrace. */
extern void zlog_backtrace_sigsafe(int priority, void *program_counter);

/* Puts a current timestamp in buf and returns the number of characters
   written (not including the terminating NUL).  The purpose of
   this function is to avoid calls to localtime appearing all over the code.
   It caches the most recent localtime result and can therefore
   avoid multiple calls within the same second.  If buflen is too small,
   *buf will be set to '\0', and 0 will be returned. */
#define QUAGGA_TIMESTAMP_LEN 40
extern size_t quagga_timestamp(zlog_timestamp_t timestamp /* # subsecond digits */,
			       char *buf, size_t buflen);

extern void time_print(FILE *fp, zlog_timestamp_t ctl);

extern void zlog_hexdump(void *mem, unsigned int len);

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

#endif /* _ZEBRA_LOG_H */
