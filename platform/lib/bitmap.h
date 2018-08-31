/* bitmap.h - bitmap operation library */


#ifndef __BITMAP_H
#define __BITMAP_H


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
    unsigned char *bitList,
    unsigned long  bit
    );
/*****************************************************************************
*
* bitListClr - Remove a bit in the given byte array.
*/
 INLINE void BIT_LST_CLR
    (
    unsigned char *bitList,
    unsigned long  bit
    );
/*****************************************************************************
*
* bitListTst - Return True if the bit is set, else False.
*/
 INLINE unsigned long BIT_LST_TST
    (
    unsigned char *bitList,
    unsigned long  bit
    );

/****************************************************************************
*
*����nbit������ֵ�����ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_XOR(unsigned char *dst, const unsigned char *src1,const unsigned char *src2, unsigned long nbit);

/****************************************************************************
*
*����nbit������ֵ����ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_AND(unsigned char *dst, const unsigned char *src1,const unsigned char *src2, unsigned long nbit);

/****************************************************************************
*
*����nbit������ֵ�Ļ�ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_OR(unsigned char *dst, const unsigned char *src1,const unsigned char *src2, unsigned long nbit);
/****************************************************************************
*
*���nbit�����ص�ֵ�Ƿ�Ϊ0
*
��nbit����1��ʼ
*/
 INLINE int BIT_LST_EMPTY(unsigned char *src, unsigned long nbit);
/****************************************************************************
*
*�Ƚ�nbit�����ص������ַ�����ֵ�Ƿ����
*
��nbit����1��ʼ
*/
 INLINE int BIT_LST_CMP(const unsigned char *src1, const unsigned char *src2, unsigned long nbit);

/****************************************************************************
*
*�Ƚ�nbit�����ص������ַ�����ֵ�Ƿ����
*
��nbit����1��ʼ
*/
 INLINE void BIT_LST_COPY(const unsigned char *src, unsigned char *dst, unsigned long nbit);
/****************************************************************************
*
*�������ı���λΪ1��ֵ
*
��nbit����1��ʼ
*/
 INLINE int BIT_LST_MAX(unsigned char *src, unsigned long nbit);

/*
����bitΪ1�ĸ���
*/

 INLINE int BIT_LST_CNT(unsigned char *src, unsigned long nbit);




#endif /* __BITMAP_H */
