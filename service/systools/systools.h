/*
 * systools.h
 *
 *  Created on: Oct 28, 2018
 *      Author: zhurish
 */

#ifndef __SYSTOOLS_H__
#define __SYSTOOLS_H__

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"

#ifndef BUFSIZE
#define BUFSIZE 512
#endif

#define MAX_FILENAME_LENGTH 128
#define MAX_LOGIN_NAME_LEN 	128
#define MAX_DIR_NAME_LEN 	128

#define systools_debug(fmt,...)	zlog_debug(MODULE_UTILS, fmt, ##__VA_ARGS__)
#define systools_warn(fmt,...)		zlog_warn(MODULE_UTILS, fmt, ##__VA_ARGS__)
#define systools_error(fmt,...)	zlog_err(MODULE_UTILS, fmt, ##__VA_ARGS__)


extern int systools_set(void *vty);

extern int systools_printf(const char *format, ...);

extern const char *ftpd_hostname();

extern int systools_task_init (void);
extern int systools_task_exit (void);
extern int systools_module_init (void);
extern int systools_module_exit (void);
extern int systools_cmd_init (void);


#endif /* __SYSTOOLS_H__ */
