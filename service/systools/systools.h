/*
 * systools.h
 *
 *  Created on: Oct 28, 2018
 *      Author: zhurish
 */

#ifndef __SYSTOOLS_H__
#define __SYSTOOLS_H__

#ifndef BUFSIZE
#define BUFSIZE 512
#endif

#define MAX_FILENAME_LENGTH 128
#define MAX_LOGIN_NAME_LEN 	128
#define MAX_DIR_NAME_LEN 	128

#define systools_debug(fmt,...)	zlog_debug(ZLOG_UTILS, fmt, ##__VA_ARGS__)
#define systools_warn(fmt,...)		zlog_warn(ZLOG_UTILS, fmt, ##__VA_ARGS__)
#define systools_error(fmt,...)	zlog_err(ZLOG_UTILS, fmt, ##__VA_ARGS__)


extern int systools_set(void *vty);

extern int systools_printf(const char *format, ...);

extern const char *ftpd_hostname();

extern int systools_task_init ();
extern int systools_task_exit ();
extern int systools_module_init ();
extern int systools_module_exit ();
extern int systools_cmd_init ();


#endif /* __SYSTOOLS_H__ */
