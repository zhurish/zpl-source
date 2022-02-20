/*
 * os_util.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_UTIL_H__
#define __OS_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OS_PIPE_BASE	SYSRUNDIR
/*
 * thread
 */
extern zpl_pthread_t os_thread_once(int (*entry)(void *), void *p);

extern int os_pipe_create(zpl_char *name, zpl_uint32 mode);
extern int os_pipe_close(int fd);

extern int os_get_blocking(int fd);
extern int os_set_nonblocking(int fd);
extern int os_set_blocking(int fd);


extern void os_log(zpl_char *file, const zpl_char *format, ...);
extern void os_vslog(zpl_char *herd, zpl_char *file, int line, const zpl_char *format, ...);

extern zpl_char * pid2name(zpl_pid_t pid);
extern zpl_pid_t name2pid(const zpl_char *name);

extern zpl_pid_t os_pid_set (const zpl_char *path);
extern zpl_pid_t os_pid_get (const zpl_char *path);


extern int fdprintf ( int fd, const zpl_char *format, ...);

extern int hostname_ipv4_address(zpl_char *hostname, struct in_addr *addr);
extern int hostname_ipv6_address(zpl_char *hostname, struct ipstack_in6_addr *addr);


extern int os_loghex(zpl_char *format, zpl_uint32 size, const zpl_uchar *data, zpl_uint32 len);



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
