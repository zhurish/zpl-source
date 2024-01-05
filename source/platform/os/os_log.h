/*
 * os_log.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_LOG_H__
#define __OS_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"


#define OSLOG_COLOR_BLACK		"\033[01;30m"     /* black color */
#define OSLOG_COLOR_RED      "\033[01;31m"     /* red color */
#define OSLOG_COLOR_GREEN		"\033[01;32m"     /* green color */
#define OSLOG_COLOR_YELLOW		"\033[01;33m"     /* yellow color */
#define OSLOG_COLOR_BLUE     "\033[01;34m"     /* blue color */
#define OSLOG_COLOR_MAGENTA	"\033[01;35m"     /* magenta color */
#define OSLOG_COLOR_CYAN     "\033[01;36m"     /* cyan color */
#define OSLOG_COLOR_WHITE		"\033[01;37m"     /* white color */
#define OSLOG_COLOR_NONE     "\033[00m"        /* default console color */

/* 16进制格式化 */
extern int os_loghex(zpl_char *format, zpl_uint32 size, const zpl_uchar *data, zpl_uint32 len);

extern int os_fdprintf(zpl_char *func, int line, int fd, const zpl_char *format, ...);
#define fdprintf(fd,fmt,...) os_fdprintf(__func__, __LINE__, fd, fmt, ##__VA_ARGS__)

extern void os_vflog(FILE *fp, zpl_char *herd, zpl_char *func, int line, const zpl_char *format, ...);

#define os_vslog(h,fmt,...)   os_vflog(NULL, h, __func__, __LINE__,fmt, ##__VA_ARGS__)

extern void os_log_entry(zpl_char *func, int line, zpl_char *file, const zpl_char *format, ...);
#define os_log(f,fmt,...)   os_log_entry(__func__, __LINE__, f, fmt, ##__VA_ARGS__)


/* OS LIB 模块使用 LIB 模块的 log 系统 */

typedef void (*oslog_callback)(const char *file, const char *func, const zpl_uint32 line, int pri, const zpl_char *format, ...);

void os_log_init(oslog_callback func);
extern void os_log_func(const char *file, const char *func, const zpl_uint32 line, int pri, const zpl_char *format, ...);
#define os_log_err(fmt,...)   os_log_func(__FILE__, __FUNCTION__, __LINE__, 3, fmt, ##__VA_ARGS__)
#define os_log_info(fmt,...)   os_log_func(__FILE__, __FUNCTION__, __LINE__, 6, fmt, ##__VA_ARGS__)
#define os_log_warn(fmt,...)   os_log_func(__FILE__, __FUNCTION__, __LINE__, 4, fmt, ##__VA_ARGS__)
#define os_log_debug(fmt,...)   os_log_func(__FILE__, __FUNCTION__, __LINE__, 7, fmt, ##__VA_ARGS__)
#define os_log_trap(fmt,...)   os_log_func(__FILE__, __FUNCTION__, __LINE__, 8, fmt, ##__VA_ARGS__)

//#define __OS_DEBUG_ENABLE  

#ifdef __OS_DEBUG_ENABLE
#define _OS_DEBUG_DETAIL(fmt,...)	os_vslog("DETAIL", __func__, __LINE__,fmt, ##__VA_ARGS__)
#define _OS_DEBUG(fmt,...)	os_vslog("DEBUG", __func__, __LINE__,fmt, ##__VA_ARGS__)
#define _OS_ERROR(fmt,...)	os_vslog("ERROR", __func__, __LINE__,fmt, ##__VA_ARGS__)
#define _OS_WARN(fmt,...)	os_vslog("WARNNING", __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define _OS_DEBUG(fmt,...)	
#define _OS_ERROR(fmt,...)	
#define _OS_WARN(fmt,...)	
#define _OS_DEBUG_DETAIL(fmt,...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __OS_UTIL_H__ */
