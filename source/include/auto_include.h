/*
 * auto_include.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __AUTO_INCLUDE_H__
#define __AUTO_INCLUDE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plconfig.h"
#include "version.h"
#include "os_definc.h"
#include "product.h"


/* Define BYTE_ORDER, if not defined. Useful for compiler conditional
 * code, rather than preprocessor conditional.
 * Not all the world has this BSD define.
 */
#ifndef BYTE_ORDER
#define BIG_ENDIAN	4321	/* least-significant byte first (vax, pc) */
#define LITTLE_ENDIAN	1234	/* most-significant byte first (IBM, net) */
#define PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp) */

#if defined(WORDS_BIGENDIAN)
#define BYTE_ORDER	BIG_ENDIAN
#else /* !WORDS_BIGENDIAN */
#define BYTE_ORDER	LITTLE_ENDIAN
#endif /* WORDS_BIGENDIAN */

#endif /* ndef BYTE_ORDER */


/* Some systems do not define UINT32_MAX, etc.. from inttypes.h
 * e.g. this makes life easier for FBSD 4.11 users.
 */
#ifndef INT8_MAX
#define INT8_MAX	(127)
#endif
#ifndef INT16_MAX
#define INT16_MAX	(32767)
#endif
#ifndef INT32_MAX
#define INT32_MAX	(2147483647)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX	(255U)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX	(65535U)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX	(4294967295U)
#endif

/* MAX / MIN are not commonly defined, but useful */
#ifndef MAX
#define MAX(a, b) \
	({ typeof (a) _a = (a); \
	   typeof (b) _b = (b); \
	   _a > _b ? _a : _b; })
#endif
#ifndef MIN
#define MIN(a, b) \
	({ typeof (a) _a = (a); \
	   typeof (b) _b = (b); \
	   _a < _b ? _a : _b; })
#endif


/* Local includes: */
#if !(defined(__GNUC__) || defined(VTYSH_EXTRACT_PL)) 
#define __attribute__(x)
#endif  /* !__GNUC__ || VTYSH_EXTRACT_PL */


#ifndef ZPL_BUILD_LINUX
#define ZPL_BUILD_LINUX
#endif
//#define OS_VXWORKS
//#define OS_OPENBSD
//#define OS_OTHER

//#define OS_PROCESS


#ifndef INT_MAX_MIN_SPACE
#define INT_MAX_MIN_SPACE(v, m, M)	( ((int)(v) > (int)(m)) && ((int)(v) < (int)(M)) )
#endif

#ifndef NOT_INT_MAX_MIN_SPACE
#define NOT_INT_MAX_MIN_SPACE(v, m, M)	( ((int)(v) < (int)(m)) && ((int)(v) > (int)(M)) )
#endif

#ifndef ZPL_SHELL_MODULE
#define VTY_NEWLINE "\r\n"
#endif


#if defined( _OS_SHELL_DEBUG_)||defined( _OS_DEBUG_)
#define OS_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SHELL_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SERVICE_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define _OS_ERROR(format, ...)  printf (format, ##__VA_ARGS__)
#define _OS_WARN(format, ...)   printf (format, ##__VA_ARGS__)
#else
#define _OS_ERROR(format, ...)
#define _OS_WARN(format, ...)
#define OS_DEBUG(format, ...)
#define OS_SHELL_DEBUG(format, ...)
#define OS_SERVICE_DEBUG(format, ...)
#endif


#ifdef __cplusplus
}
#endif

#endif /* __PLAUTO_INCLUDE_H__ */
