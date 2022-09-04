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
#include "zpl_type.h"

#define OS_PIPE_BASE	SYSRUNDIR
/*
 * thread
 */
extern zpl_pthread_t os_thread_once(void (*entry)(void *), void *p);

extern int os_pipe_create(zpl_char *name, zpl_uint32 mode);
extern int os_pipe_close(int fd);

extern int os_get_blocking(int fd);
extern int os_set_nonblocking(int fd);
extern int os_set_blocking(int fd);

extern zpl_char * pid2name(zpl_pid_t pid);
extern zpl_pid_t name2pid(const zpl_char *name);

extern zpl_pid_t os_pid_set (const zpl_char *path);
extern zpl_pid_t os_pid_get (const zpl_char *path);


extern int hostname_ipv4_address(zpl_char *hostname, struct in_addr *addr);
extern int hostname_ipv6_address(zpl_char *hostname, struct ipstack_in6_addr *addr);


extern const char *strupr(zpl_char* src);
extern const char *strlwr(zpl_char* src);
extern const char *itoa(int value, int base);
extern const char *ftoa(zpl_float value, zpl_char *fmt);
extern const char *dtoa(zpl_double value, zpl_char *fmt);

extern zpl_uint8 atoascii(int a);
extern zpl_bool is_hex (zpl_char c);


#ifdef __cplusplus
}
#endif

#endif /* __OS_UTIL_H__ */
