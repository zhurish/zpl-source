/*
 * os_backtrace.c
 *
 *  Created on: 2019年8月18日
 *      Author: zhurish
 */




static ospl_uint32 * __getpc(void) __attribute__((noinline))
{
ospl_uint32 *rtaddr;
__asm__ volatile ("move %0, $31" : "=r"(rtaddr));
return rtaddr;
}
