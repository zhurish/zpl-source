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


typedef void (*oslog_callback)(int pri, const zpl_char *format, ...);


/* 16进制格式化 */
extern int os_loghex(zpl_char *format, zpl_uint32 size, const zpl_uchar *data, zpl_uint32 len);

extern int os_fdprintf(zpl_char *func, int line, int fd, const zpl_char *format, ...);
#define fdprintf(fd,fmt,...) os_fdprintf(__func__, __LINE__, fd, fmt, ##__VA_ARGS__)

extern void os_vflog(FILE *fp, zpl_char *herd, zpl_char *func, int line, const zpl_char *format, ...);

#define os_vslog(h,fmt,...)   os_vflog(NULL, h, __func__, __LINE__,fmt, ##__VA_ARGS__)

extern void os_log_entry(zpl_char *func, int line, zpl_char *file, const zpl_char *format, ...);
#define os_log(f,fmt,...)   os_log_entry(__func__, __LINE__, f, fmt, ##__VA_ARGS__)

void os_log_init(oslog_callback func);
extern void os_log_func(zpl_char *func, int line, int pri, const zpl_char *format, ...);


#define os_log_err(fmt,...)   os_log_func(__func__, __LINE__, 3,fmt, ##__VA_ARGS__)
#define os_log_info(fmt,...)   os_log_func(__func__, __LINE__, 6, fmt, ##__VA_ARGS__)
#define os_log_warn(fmt,...)   os_log_func(__func__, __LINE__, 4, fmt, ##__VA_ARGS__)
#define os_log_debug(fmt,...)   os_log_func(__func__, __LINE__, 7, fmt, ##__VA_ARGS__)
#define os_log_trap(fmt,...)   os_log_func(__func__, __LINE__, 8, fmt, ##__VA_ARGS__)

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
