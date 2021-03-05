/*
 * ospl_def.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __OSPL_DEF_H__
#define __OSPL_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif


#define array_size(ar) (sizeof(ar) / sizeof(ar[0]))



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


#define OS_WAIT_NO	0
#define OS_WAIT_FOREVER	-1


/* Flag manipulation macros. */
#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) |= (F)
#define UNSET_FLAG(V,F)      (V) &= ~(F)
#define RESET_FLAG(V)        (V) = 0

#define FD_IS_STDOUT(f)	((f) < 3)

#ifndef min
#define min(a,b)	( (a) < (b)? (a):(b) )
#endif
#ifndef max
#define max(a,b)	( (a) > (b)? (a):(b) )
#endif
#ifndef MIN
#define MIN(a,b)	( (a) < (b)? (a):(b) )
#endif
#ifndef MAX
#define MAX(a,b)	( (a) > (b)? (a):(b) )
#endif

#ifndef INT_MAX_MIN_SPACE
#define INT_MAX_MIN_SPACE(v, m, M)	( ((v) > (m)) && ((v) < (M)) )
#endif

#ifndef NOT_INT_MAX_MIN_SPACE
#define NOT_INT_MAX_MIN_SPACE(v, m, M)	( ((v) < (m)) && ((v) > (M)) )
#endif



#ifdef __cplusplus
}
#endif

#endif /* __OSPL_DEF_H__ */
