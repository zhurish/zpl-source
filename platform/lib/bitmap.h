/* bitmap.h - bitmap operation library */


#ifndef __BITMAP_H
#define __BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef INLINE
#define INLINE
#endif

#define BITS_PER_LONG	(sizeof(long)<<3)

#define BITS_TO_LONGS(bits) \
	(((bits)+BITS_PER_LONG-1)/BITS_PER_LONG)

#define BITMAP_LAST_WORD_MASK(nbits)					\
(									\
	((nbits) % BITS_PER_LONG) ?					\
		(1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL		\
)


/*
#define BIT_LST_SET ((n), (b)) bitListSet ((n), (b))
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


/*
*nbits:from 1
*/

/*****************************************************************************
*
* bitListSet - Set a bit in the given byte array.
*/
INLINE void BIT_LST_SET
    (
    ospl_uchar *bitList,
    ospl_ulong  bit
    );
/*****************************************************************************
*
* bitListClr - Remove a bit in the given byte array.
*/
 INLINE void BIT_LST_CLR
    (
    ospl_uchar *bitList,
    ospl_ulong  bit
    );
/*****************************************************************************
*
* bitListTst - Return True if the bit is set, else False.
*/
 INLINE ospl_ulong BIT_LST_TST
    (
    ospl_uchar *bitList,
    ospl_ulong  bit
    );

/****************************************************************************
*
*����nbit������ֵ�����ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_XOR(ospl_uchar *dst, const ospl_uchar *src1,const ospl_uchar *src2, ospl_ulong nbit);

/****************************************************************************
*
*����nbit������ֵ����ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_AND(ospl_uchar *dst, const ospl_uchar *src1,const ospl_uchar *src2, ospl_ulong nbit);

/****************************************************************************
*
*����nbit������ֵ�Ļ�ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_OR(ospl_uchar *dst, const ospl_uchar *src1,const ospl_uchar *src2, ospl_ulong nbit);
/****************************************************************************
*
*���nbit�����ص�ֵ�Ƿ�Ϊ0
*
��nbit����1��ʼ
*/
 INLINE ospl_bool BIT_LST_EMPTY(ospl_uchar *src, ospl_ulong nbit);
/****************************************************************************
*
*�Ƚ�nbit�����ص������ַ�����ֵ�Ƿ����
*
��nbit����1��ʼ
*/
 INLINE int BIT_LST_CMP(const ospl_uchar *src1, const ospl_uchar *src2, ospl_ulong nbit);

/****************************************************************************
*
*�Ƚ�nbit�����ص������ַ�����ֵ�Ƿ����
*
��nbit����1��ʼ
*/
 INLINE void BIT_LST_COPY(const ospl_uchar *src, ospl_uchar *dst, ospl_ulong nbit);
/****************************************************************************
*
*�������ı���λΪ1��ֵ
*
��nbit����1��ʼ
*/
 INLINE ospl_uint32 BIT_LST_MAX(ospl_uchar *src, ospl_ulong nbit);

/*
����bitΪ1�ĸ���
*/

 INLINE ospl_uint32 BIT_LST_CNT(ospl_uchar *src, ospl_ulong nbit);



 
#ifdef __cplusplus
}
#endif
 
#endif /* __BITMAP_H */
