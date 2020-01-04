/*
 * os_backtrace.c
 *
 *  Created on: 2019年8月18日
 *      Author: zhurish
 */




static unsigned int * __getpc(void) __attribute__((noinline))
{
unsigned int *rtaddr;
__asm__ volatile ("move %0, $31" : "=r"(rtaddr));
return rtaddr;
}
