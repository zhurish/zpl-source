/*
 * os_backtrace.h
 *
 *  Created on: 2019年8月18日
 *      Author: zhurish
 */

#ifndef __OS_BACKTRACE_H__
#define __OS_BACKTRACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_type.h"

struct zpl_backtrace_symb
{
  char *taskname;
  char *funcname;
  char *schedfrom;
  zpl_uint32 schedfrom_line;
};

extern int zpl_backtrace_symb_set(char *funcname, char *schedfrom, zpl_uint32 schedfrom_line);
extern const char * zpl_backtrace_symb_info(void);
#if 0
inline zpl_uint32 * __getsp(void)
{
zpl_uint32 *sp;
__asm__ volatile ("move %0, $29" : "=r"(sp));
return sp;
}
#endif

#ifdef __cplusplus
}
#endif


#endif /* __OS_BACKTRACE_H__ */