/* os_bitmap.h - bitmap operation library */


#ifndef __OS_BITMAP_H__
#define __OS_BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_type.h"


void os_bitmap_set(zpl_bitmap_t bitmap, zpl_uint32  bit, zpl_uint32  wigth);
void os_bitmap_clr(zpl_bitmap_t bitmap, zpl_uint32  bit, zpl_uint32  wigth);
int os_bitmap_tst(zpl_bitmap_t bitmap, zpl_uint32  bit, zpl_uint32  wigth);

void os_bitmap_and(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2);

void os_bitmap_or(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2);

int os_bitmap_cmp(const zpl_bitmap_t src1, const zpl_bitmap_t src2);
void os_bitmap_copy(const zpl_bitmap_t src, zpl_bitmap_t dst);

void os_bit64_set(zpl_uint64 *bitmap, zpl_uint32  bit, zpl_uint32  wigth);
void os_bit64_clr(zpl_uint64 *bitmap, zpl_uint32  bit, zpl_uint32  wigth);
int os_bit64_tst(zpl_uint64 *bitmap, zpl_uint32  bit, zpl_uint32  wigth);
 
#ifdef __cplusplus
}
#endif
 
#endif /* __OS_BITMAP_H__ */
