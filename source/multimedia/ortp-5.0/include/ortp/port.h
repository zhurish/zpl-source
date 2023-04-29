/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of oRTP 
 * (see https://gitlab.linphone.org/BC/public/ortp).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/* this file is responsible of the portability of the stack */

#ifndef ORTP_PORT_H
#define ORTP_PORT_H

#if __APPLE__
#include "TargetConditionals.h"
#endif

#if !defined(_WIN32) && !defined(_WIN32_WCE)
/********************************/
/* definitions for UNIX flavour */
/********************************/

#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __linux__
#include <stdint.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if defined(_XOPEN_SOURCE_EXTENDED) || !defined(__hpux)
#include <arpa/inet.h>
#endif

#include <sys/time.h>

#include <netdb.h>
//#include <ortp/bctport.h>
typedef int ortp_socket_t;
typedef pthread_t ortp_thread_t;
typedef pthread_mutex_t ortp_mutex_t;
typedef pthread_cond_t ortp_cond_t;

#ifdef __INTEL_COMPILER
#pragma warning(disable : 111)		// statement is unreachable
#pragma warning(disable : 181)		// argument is incompatible with corresponding format string conversion
#pragma warning(disable : 188)		// enumerated type mixed with another type
#pragma warning(disable : 593)		// variable "xxx" was set but never used
#pragma warning(disable : 810)		// conversion from "int" to "unsigned short" may lose significant bits
#pragma warning(disable : 869)		// parameter "xxx" was never referenced
#pragma warning(disable : 981)		// operands are evaluated in unspecified order
#pragma warning(disable : 1418)		// external function definition with no prior declaration
#pragma warning(disable : 1419)		// external declaration in primary source file
#pragma warning(disable : 1469)		// "cc" clobber ignored
#endif

#define ORTP_PUBLIC 
#define ORTP_INLINE			inline
#ifndef ORTP_DEPRECATED
#if defined(_MSC_VER)
#define ORTP_DEPRECATED __declspec(deprecated)
#else
#define ORTP_DEPRECATED __attribute__((deprecated))
#endif
#endif
#ifdef __cplusplus
extern "C"
{
#endif

int __ortp_thread_join(ortp_thread_t thread, void **ptr);
int __ortp_thread_create(ortp_thread_t *thread, pthread_attr_t *attr, void * (*routine)(void*), void *arg);
unsigned long __ortp_thread_self(void);

#ifdef __cplusplus
}
#endif

#define ortp_thread_create	__ortp_thread_create
#define ortp_thread_join	__ortp_thread_join
#define ortp_thread_self	__ortp_thread_self
#define ortp_thread_exit	pthread_exit
#define ortp_mutex_init		pthread_mutex_init
#define ortp_mutex_lock		pthread_mutex_lock
#define ortp_mutex_unlock	pthread_mutex_unlock
#define ortp_mutex_destroy	pthread_mutex_destroy
#define ortp_cond_init		pthread_cond_init
#define ortp_cond_signal	pthread_cond_signal
#define ortp_cond_broadcast	pthread_cond_broadcast
#define ortp_cond_wait		pthread_cond_wait
#define ortp_cond_destroy	pthread_cond_destroy

#define SOCKET_OPTION_VALUE	void *
#define SOCKET_BUFFER		void *

#define getSocketError() strerror(errno)
#define getSocketErrorCode() (errno)
#define ortp_gettimeofday(tv,tz) gettimeofday(tv,tz)
#define ortp_log10f(x)	log10f(x)

#define ORTP_UNUSED(x)		x
#else
/*********************************/
/* definitions for WIN32 flavour */
/*********************************/

#include <stdio.h>
#define _CRT_RAND_S
#include <stdlib.h>
#include <stdarg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#if defined(__MINGW32__) || !defined(WINAPI_FAMILY_PARTITION) || !defined(WINAPI_PARTITION_DESKTOP)
#define ORTP_WINDOWS_DESKTOP 1
#elif defined(WINAPI_FAMILY_PARTITION)
// See bctoolbox/include/port.h for WINAPI_PARTITION checker
#if defined(WINAPI_PARTITION_DESKTOP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define ORTP_WINDOWS_DESKTOP 1
#elif defined (WINAPI_PARTITION_PC_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PC_APP)
#define ORTP_WINDOWS_DESKTOP 1
#define ORTP_WINDOWS_UWP 1
#elif defined(WINAPI_PARTITION_PHONE_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
#define ORTP_WINDOWS_PHONE 1
#elif defined(WINAPI_PARTITION_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define ORTP_WINDOWS_UNIVERSAL 1
#endif
#endif

#ifdef _MSC_VER
#ifdef ORTP_STATIC
#define ORTP_PUBLIC
#else
#ifdef ORTP_EXPORTS
#define ORTP_PUBLIC	__declspec(dllexport)
#else
#define ORTP_PUBLIC	__declspec(dllimport)
#endif
#endif
#pragma push_macro("_WINSOCKAPI_")
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

typedef  unsigned __int64 uint64_t;
typedef  __int64 int64_t;
typedef  unsigned short uint16_t;
typedef  unsigned int uint32_t;
typedef  int int32_t;
typedef  unsigned char uint8_t;
typedef __int16 int16_t;
#else
#include <stdint.h> /*provided by mingw32*/
#include <io.h>
#define ORTP_PUBLIC
ORTP_PUBLIC char* strtok_r(char *str, const char *delim, char **nextp);
#endif

#define vsnprintf _vsnprintf

typedef SOCKET ortp_socket_t;
#ifdef ORTP_WINDOWS_DESKTOP
typedef HANDLE ortp_cond_t;
typedef HANDLE ortp_mutex_t;
#else
typedef CONDITION_VARIABLE ortp_cond_t;
typedef SRWLOCK ortp_mutex_t;
#endif
typedef HANDLE ortp_thread_t;

#define ortp_thread_create	WIN_thread_create
#define ortp_thread_join	WIN_thread_join
#define ortp_thread_self	WIN_thread_self
#define ortp_thread_exit(arg)
#define ortp_mutex_init		WIN_mutex_init
#define ortp_mutex_lock		WIN_mutex_lock
#define ortp_mutex_unlock	WIN_mutex_unlock
#define ortp_mutex_destroy	WIN_mutex_destroy
#define ortp_cond_init		WIN_cond_init
#define ortp_cond_signal	WIN_cond_signal
#define ortp_cond_broadcast	WIN_cond_broadcast
#define ortp_cond_wait		WIN_cond_wait
#define ortp_cond_destroy	WIN_cond_destroy


#ifdef __cplusplus
extern "C"
{
#endif

ORTP_PUBLIC int WIN_mutex_init(ortp_mutex_t *m, void *attr_unused);
ORTP_PUBLIC int WIN_mutex_lock(ortp_mutex_t *mutex);
ORTP_PUBLIC int WIN_mutex_unlock(ortp_mutex_t *mutex);
ORTP_PUBLIC int WIN_mutex_destroy(ortp_mutex_t *mutex);
ORTP_PUBLIC int WIN_thread_create(ortp_thread_t *t, void *attr_unused, void *(*func)(void*), void *arg);
ORTP_PUBLIC int WIN_thread_join(ortp_thread_t thread, void **unused);
ORTP_PUBLIC unsigned long WIN_thread_self(void);
ORTP_PUBLIC int WIN_cond_init(ortp_cond_t *cond, void *attr_unused);
ORTP_PUBLIC int WIN_cond_wait(ortp_cond_t * cond, ortp_mutex_t * mutex);
ORTP_PUBLIC int WIN_cond_signal(ortp_cond_t * cond);
ORTP_PUBLIC int WIN_cond_broadcast(ortp_cond_t * cond);
ORTP_PUBLIC int WIN_cond_destroy(ortp_cond_t * cond);

#ifdef __cplusplus
}
#endif

#define SOCKET_OPTION_VALUE	char *
#define ORTP_INLINE			__inline

#if defined(_WIN32_WCE)

#define ortp_log10f(x)		(float)log10 ((double)x)

#ifdef assert
	#undef assert
#endif /*assert*/
#define assert(exp)	((void)0)

#ifdef errno
	#undef errno
#endif /*errno*/
#define  errno GetLastError()
#ifdef strerror
		#undef strerror
#endif /*strerror*/
const char * ortp_strerror(DWORD value);
#define strerror ortp_strerror


#else /*_WIN32_WCE*/

#define ortp_log10f(x)	log10f(x)

#endif

#ifdef __cplusplus
extern "C" {
#endif

	ORTP_PUBLIC const char *getWinSocketError(int error);
#ifndef getSocketErrorCode
#define getSocketErrorCode() WSAGetLastError()
#endif
#ifndef getSocketError
#define getSocketError() getWinSocketError(WSAGetLastError())
#endif

#ifndef F_OK
#define F_OK 00 /* Visual Studio does not define F_OK */
#endif


ORTP_PUBLIC int ortp_gettimeofday (struct timeval *tv, void* tz);
#ifdef _WORKAROUND_MINGW32_BUGS
char * WSAAPI gai_strerror(int errnum);
#endif

#ifdef __cplusplus
}
#endif

#endif

#ifndef _BOOL_T_
#define _BOOL_T_
typedef unsigned char bool_t;
#endif /* _BOOL_T_ */
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0


typedef struct ortpTimeSpec{
	int64_t tv_sec;
	int64_t tv_nsec;
}ortpTimeSpec;

#ifdef __cplusplus
extern "C"{
#endif

ORTP_PUBLIC void* ortp_malloc(size_t sz);
ORTP_PUBLIC void ortp_free(void *ptr);
ORTP_PUBLIC void* ortp_realloc(void *ptr, size_t sz);
ORTP_PUBLIC void* ortp_malloc0(size_t sz);
ORTP_PUBLIC char * ortp_strdup(const char *tmp);

/*override the allocator with this method, to be called BEFORE ortp_init()*/
typedef struct _OrtpMemoryFunctions{
	void *(*malloc_fun)(size_t sz);
	void *(*realloc_fun)(void *ptr, size_t sz);
	void (*free_fun)(void *ptr);
}OrtpMemoryFunctions;

void ortp_set_memory_functions(OrtpMemoryFunctions *functions);

#define ortp_new(type,count)	(type*)ortp_malloc(sizeof(type)*(count))
#define ortp_new0(type,count)	(type*)ortp_malloc0(sizeof(type)*(count))

ORTP_PUBLIC int close_socket(ortp_socket_t sock);
ORTP_PUBLIC int set_non_blocking_socket(ortp_socket_t sock);
ORTP_PUBLIC int set_blocking_socket(ortp_socket_t sock);

ORTP_PUBLIC char *ortp_strndup(const char *str,int n);
ORTP_PUBLIC char *ortp_strdup_printf(const char *fmt,...);
ORTP_PUBLIC char *ortp_strdup_vprintf(const char *fmt, va_list ap);
ORTP_PUBLIC char *ortp_strcat_printf(char *dst, const char *fmt,...);

ORTP_PUBLIC char *ortp_strcat_vprintf(char *dst, const char *fmt, va_list ap);

#define ortp_file_exist(pathname) ortp_file_exist(pathname)

ORTP_PUBLIC void ortp_get_cur_time(ortpTimeSpec *ret);
void _ortp_get_cur_time(ortpTimeSpec *ret, bool_t realtime);
ORTP_PUBLIC uint64_t ortp_get_cur_time_ms(void);
ORTP_PUBLIC void ortp_sleep_ms(int ms);
ORTP_PUBLIC void ortp_sleep_until(const ortpTimeSpec *ts);
ORTP_PUBLIC int ortp_timespec_compare(const ortpTimeSpec *s1, const ortpTimeSpec *s2);
ORTP_PUBLIC unsigned int ortp_random(void);

int ortp_sockaddr_to_address(const struct ipstack_sockaddr *sa, socklen_t salen, char *ip, size_t ip_size, int *port);
int ortp_sockaddr_to_print_address(struct ipstack_sockaddr *sa, socklen_t salen, char *printable_ip,
								   size_t printable_ip_size);
int ortp_address_to_sockaddr(int sin_family, char *ip, int port, struct ipstack_sockaddr *sa, socklen_t *salen);
bool_t ortp_sockaddr_equals(const struct ipstack_sockaddr *sa, const struct ipstack_sockaddr *sb);

ORTP_PUBLIC int ortp_get_local_ip_for(int type, const char *dest, int port, char *result, size_t result_len);
struct addrinfo * ortp_name_to_addrinfo(int family, int socktype, const char *name, int port);

void ortp_sockaddr_remove_v4_mapping(const struct sockaddr *v6, struct sockaddr *result, socklen_t *result_len);
void ortp_sockaddr_remove_nat64_mapping(const struct sockaddr *v6, struct sockaddr *result, socklen_t *result_len) ;
void ortp_sockaddr_ipv6_to_ipv4(const struct sockaddr *v6, struct sockaddr *result, socklen_t *result_len);
void ortp_sockaddr_ipv4_to_ipv6(const struct sockaddr *v4, struct sockaddr *result, socklen_t *result_len);

void ortp_timespec_add(ortpTimeSpec *ts, const int64_t lap);

/* portable named pipes  and shared memory*/
#if !defined(_WIN32_WCE)
#ifdef _WIN32
typedef HANDLE ortp_pipe_t;
#define ORTP_PIPE_INVALID INVALID_HANDLE_VALUE
#else
typedef int ortp_pipe_t;
#define ORTP_PIPE_INVALID (-1)
#endif


ORTP_PUBLIC bool_t ortp_is_multicast_addr(const struct sockaddr *addr);

#endif

#ifdef __cplusplus
}

#endif


#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(ORTP_STATIC)
#ifdef ORTP_EXPORTS
   #define ORTP_VAR_PUBLIC    extern __declspec(dllexport)
#else
   #define ORTP_VAR_PUBLIC    __declspec(dllimport)
#endif
#else
   #define ORTP_VAR_PUBLIC    extern
#endif

#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(i)	(((uint8_t *) (i))[0] == 0xff)
#endif

/*define __ios when we are compiling for ios.
 The TARGET_OS_IPHONE macro is stupid, it is defined to 0 when compiling on mac os x.
*/
#if TARGET_OS_IPHONE
#define __ios 1
#endif

#ifndef ORTP_NO_BREAK
#if defined(__GNUC__) && __GNUC__ >= 7
#define ORTP_NO_BREAK __attribute__((fallthrough))
#else
#define ORTP_NO_BREAK
#endif // __GNUC__
#endif // ORTP_NO_BREAK

#define ORTP_VFS_OK           0   /* Successful result */

#define ORTP_VFS_ERROR       -255   /* Some kind of disk I/O error occurred */

#define ORTP_VFS_PRINTF_PAGE_SIZE 4096 /* Size of the page hold in memory by fprintf */
#define ORTP_VFS_GETLINE_PAGE_SIZE 17385 /* Size of the page hold in memory by getnextline */
/**
 * Methods associated with the ortp_vfs_t.
 */
typedef struct ortp_io_methods_t ortp_io_methods_t;

/**
 * VFS file handle.
 */
typedef struct ortp_vfs_file_t ortp_vfs_file_t;

struct ortp_io_methods_t {
	int (*pFuncClose)(ortp_vfs_file_t *pFile);
	ssize_t (*pFuncRead)(ortp_vfs_file_t *pFile, void* buf, size_t count, off_t offset);
	ssize_t (*pFuncWrite)(ortp_vfs_file_t *pFile, const void* buf, size_t count, off_t offset);
	int (*pFuncTruncate)(ortp_vfs_file_t *pFile, int64_t size);
	int64_t (*pFuncFileSize)(ortp_vfs_file_t *pFile);
	int (*pFuncSync)(ortp_vfs_file_t *pFile);
	int (*pFuncGetLineFromFd)(ortp_vfs_file_t *pFile, char* s, int count);
	bool_t (*pFuncIsEncrypted)(ortp_vfs_file_t *pFile);
};

struct ortp_vfs_file_t {
	const struct ortp_io_methods_t *pMethods;  /* Methods for an open file: all Developpers must supply this field at open step*/
	/*the fields below are used by the default implementation. Developpers are not required to supply them, but may use them if they find
	 * them useful*/
	void* pUserData; 				/* Developpers can store private data under this pointer */
	off_t offset;					/* File offset used by ortp_file_fprintf and ortp_file_get_nxtline */
	/* fprintf cache */
	char fPage[ORTP_VFS_PRINTF_PAGE_SIZE];		/* Buffer storing the current page cached by fprintf */
	off_t fPageOffset;				/* The original offset of the cached page */
	size_t fSize;					/* number of bytes in cache */
	/* get_nxtline cache */
	char gPage[ORTP_VFS_GETLINE_PAGE_SIZE+1];	/* Buffer storing the current page cachec by get_nxtline +1 to hold the \0 */
	off_t gPageOffset;				/* The offset of the cached page */
	size_t gSize;					/* actual size of the data in cache */
};

/**
 * VFS definition
 */
typedef struct ortp_vfs_t ortp_vfs_t;
struct ortp_vfs_t {
	const char *vfsName;       /* Virtual file system name */
	int (*pFuncOpen)(ortp_vfs_t *pVfs, ortp_vfs_file_t *pFile, const char *fName, int openFlags);
};

/**
 * Attempts to read count bytes from the open file given by pFile, at the position starting at offset
 * in the file and and puts them in the buffer pointed by buf.
 * @param  pFile  ortp_vfs_file_t File handle pointer.
 * @param  buf    Buffer holding the read bytes.
 * @param  count  Number of bytes to read.
 * @param  offset Where to start reading in the file (in bytes).
 * @return        Number of bytes read on success, ORTP_VFS_ERROR otherwise.
 */
ORTP_PUBLIC ssize_t ortp_file_read(ortp_vfs_file_t *pFile, void *buf, size_t count, off_t offset);

/**
 * Attempts to read count bytes from the open file given by pFile, at the position starting at its offset
 * in the file and and puts them in the buffer pointed by buf.
 * The file offset shall be incremented by the number of bytes actually read.
 * @param  pFile  ortp_vfs_file_t File handle pointer.
 * @param  buf    Buffer holding the read bytes.
 * @param  count  Number of bytes to read.
 * @return        Number of bytes read on success, ORTP_VFS_ERROR otherwise.
 */
ORTP_PUBLIC ssize_t ortp_file_read2(ortp_vfs_file_t *pFile, void *buf, size_t count);

/**
 * Close the file from its descriptor pointed by thw ortp_vfs_file_t handle.
 * @param  pFile File handle pointer.
 * @return      	return value from the pFuncClose VFS Close function on success,
 *                  ORTP_VFS_ERROR otherwise.
 */
ORTP_PUBLIC int ortp_file_close(ortp_vfs_file_t *pFile);

/**
 * Allocates a ortp_vfs_file_t file handle pointer. Opens the file fName
 * with the mode specified by the mode argument. Calls ortp_file_open.
 * @param  pVfs  Pointer to the vfs instance in use.
 * @param  fName Absolute file path.
 * @param  mode  File access mode (char*).
 * @return  pointer to  ortp_vfs_file_t on success, NULL otherwise.
 */
ORTP_PUBLIC ortp_vfs_file_t* ortp_file_open(ortp_vfs_t *pVfs, const char *fName, const char *mode);


/**
 * Allocates a ortp_vfs_file_t file handle pointer. Opens the file fName
 * with the mode specified by the mode argument. Calls ortp_file_open.
 * @param  pVfs  		Pointer to the vfs instance in use.
 * @param  fName 		Absolute file path.
 * @param  openFlags  	File access flags(integer).
 * @return  pointer to  ortp_vfs_file_t on success, NULL otherwise.
 */
ORTP_PUBLIC ortp_vfs_file_t* ortp_file_open2(ortp_vfs_t *pVfs, const char *fName, const int openFlags);


/**
 * Returns the file size.
 * @param  pFile  ortp_vfs_file_t File handle pointer.
 * @return       ORTP_VFS_ERROR if an error occured, file size otherwise.
 */
ORTP_PUBLIC int64_t ortp_file_size(ortp_vfs_file_t *pFile);

/**
 * Truncates/ Extends a file.
 * @param  pFile ortp_vfs_file_t File handle pointer.
 * @param  size  New size of the file.
 * @return       ORTP_VFS_ERROR if an error occured, 0 otherwise.
 */
ORTP_PUBLIC int ortp_file_truncate(ortp_vfs_file_t *pFile, int64_t size);

/**
 * Write count bytes contained in buf to a file associated with pFile at the position
 * offset. Calls pFuncWrite (set to bc_Write by default).
 * @param  pFile 	File handle pointer.
 * @param  buf    	Buffer hodling the values to write.
 * @param  count  	Number of bytes to write to the file.
 * @param  offset 	Position in the file where to start writing.
 * @return        	Number of bytes written on success, ORTP_VFS_ERROR if an error occurred.
 */
ORTP_PUBLIC ssize_t ortp_file_write(ortp_vfs_file_t *pFile, const void *buf, size_t count, off_t offset);

/**
 * Write count bytes contained in buf to a file associated with pFile at the position starting at its
 * offset. Calls pFuncWrite (set to bc_Write by default).
 * The file offset shall be incremented by the number of bytes actually written.
 * @param  pFile 	File handle pointer.
 * @param  buf    	Buffer hodling the values to write.
 * @param  count  	Number of bytes to write to the file.
 * @return        	Number of bytes written on success, ORTP_VFS_ERROR if an error occurred.
 */
ORTP_PUBLIC ssize_t ortp_file_write2(ortp_vfs_file_t *pFile, const void *buf, size_t count);

/**
 * Writes to file.
 * @param  pFile  File handle pointer.
 * @param  offset where to write in the file
 * @param  fmt    format argument, similar to that of printf
 * @return        Number of bytes written if success, ORTP_VFS_ERROR otherwise.
 */
ORTP_PUBLIC ssize_t ortp_file_fprintf(ortp_vfs_file_t *pFile, off_t offset, const char *fmt, ...);

/**
 * Wrapper to pFuncGetNxtLine. Returns a line with at most maxlen characters
 * from the file associated to pFile and  writes it into s.
 * @param  pFile  File handle pointer.
 * @param  s      Buffer where to store the read line.
 * @param  maxlen Number of characters to read to find a line in the file.
 * @return        ORTP_VFS_ERROR if an error occurred, size of line read otherwise.
 */
ORTP_PUBLIC int ortp_file_get_nxtline(ortp_vfs_file_t *pFile, char *s, int maxlen);

/**
 * Simply sync the file contents given through the file handle
 * to the persistent media.
 * @param  pFile  File handle pointer.
 * @return   ORTP_VFS_OK on success, ORTP_VFS_ERROR otherwise
 */
ORTP_PUBLIC int ortp_file_sync(ortp_vfs_file_t *pFile);

/**
 * Set the position to offset in the file, this position is used only by the function
 * ortp_file_get_nxtline. Read and write give their own offset as param and won't modify this one
 * @param  pFile  File handle pointer.
 * @param  offset File offset where to set the position to.
 * @param  whence Either SEEK_SET, SEEK_CUR,SEEK_END
 * @return        ORTP_VFS_ERROR on error, offset otherwise.
 */
ORTP_PUBLIC off_t ortp_file_seek(ortp_vfs_file_t *pFile, off_t offset, int whence);

/**
 * Get the file encryption status
 * @param  pFile  File handle pointer.
 * @return true if the file is encrypted
 */
ORTP_PUBLIC bool_t ortp_file_is_encrypted(ortp_vfs_file_t *pFile);

/**
 * Set default VFS pointer pDefault to my_vfs.
 * By default, the global pointer is set to use VFS implemnted in vfs.c
 * @param my_vfs Pointer to a ortp_vfs_t structure.
 */
ORTP_PUBLIC void ortp_vfs_set_default(ortp_vfs_t *my_vfs);


/**
 * Returns the value of the global variable pDefault,
 * pointing to the default vfs used.
 * @return Pointer to ortp_vfs_t set to operate as default VFS.
 */
ORTP_PUBLIC ortp_vfs_t* ortp_vfs_get_default(void);

/**
 * Return pointer to standard VFS impletentation.
 * @return  pointer to bcVfs
 */
ORTP_PUBLIC ortp_vfs_t* ortp_vfs_get_standard(void);

#endif
