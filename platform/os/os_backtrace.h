/*
 * os_backtrace.h
 *
 *  Created on: 2019年8月18日
 *      Author: zhurish
 */

#ifndef __OS_BACKTRACE_H__
#define __OS_BACKTRACE_H__


inline unsigned int * __getsp(void)
{
unsigned int *sp;
__asm__ volatile ("move %0, $29" : "=r"(sp));
return sp;
}


#endif /* __OS_BACKTRACE_H__ */
