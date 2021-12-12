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

inline zpl_uint32 * __getsp(void)
{
zpl_uint32 *sp;
__asm__ volatile ("move %0, $29" : "=r"(sp));
return sp;
}


#ifdef __cplusplus
}
#endif


#endif /* __OS_BACKTRACE_H__ */
