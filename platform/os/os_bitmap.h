/* os_bitmap.h - bitmap operation library */


#ifndef __OS_BITMAP_H__
#define __OS_BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_type.h"


/*
#define bitListSet ((n), (b)) bitListSet ((n), (b))
#define BIT_LST_CLR (d, b) bitListClr (d, b)
#define BIT_LST_TST (d, b) bitListTst (d, b)
#define BIT_LST_AND (d, s1,s2, n) bitListAnd (d, s1,s2, n)
#define BIT_LST_XOR (d, s1,s2, n) bitListXor (d, s1,s2, n)
#define BIT_LST_OR (d, s1,s2, n) bitListOr (d, s1,s2, n)
#define BIT_LST_EMPTY (d, n) bitListOr (d, n)
#define BIT_LST_CMP (s1, s2, n) bitListOr (s1, s2, n)
#define BIT_LST_COPY (s1, s2, n) bitListOr (s1, s2, n)
#define BIT_LST_MAX (s1, n) bitListOr (s1, n)
#define BIT_LST_CNT (s1, n) bitListOr (s1, n)
*/

void  bitListSet(zpl_bitmap_t bitList,zpl_uint32  bit);
void  bitListClr(zpl_bitmap_t bitList,zpl_uint32  bit);
zpl_uint32 bitListTst(zpl_bitmap_t bitList,zpl_uint32  bit);
void  bitListXor(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2, zpl_uint32 nbit);
void  bitListAnd(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2, zpl_uint32 nbit);
void  bitListOr(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2, zpl_uint32 nbit);
zpl_bool bitListEmpty(zpl_bitmap_t src, zpl_uint32 nbit);
int bitListCmp(const zpl_bitmap_t src1, const zpl_bitmap_t src2, zpl_uint32 nbit);
void  bitListCopy(const zpl_bitmap_t src, zpl_bitmap_t dst, zpl_uint32 nbit);
zpl_uint32 bitListMax(zpl_bitmap_t src, zpl_uint32 nbit);
zpl_uint32 bitListCnt(zpl_bitmap_t src, zpl_uint32 nbit);

 
#ifdef __cplusplus
}
#endif
 
#endif /* __OS_BITMAP_H__ */
