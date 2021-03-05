
#include "zebra.h"
#include "bitmap.h"

static const ospl_uchar bitMask[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};


/*****************************************************************************
*
* bitListSet - Set a bit in the given byte array.
*/
 INLINE void BIT_LST_SET
    (
    ospl_uchar *bitList,
    ospl_ulong  bit
    )
{
    bitList[bit/8] |= bitMask[bit%8];
}

/*****************************************************************************
*
* bitListClr - Remove a bit in the given byte array.
*/
 INLINE void BIT_LST_CLR
    (
    ospl_uchar *bitList,
    ospl_ulong  bit
    )
{
    bitList[bit/8] &= ~bitMask[bit%8];
}

/*****************************************************************************
*
* bitListTst - Return True if the bit is set, else False.
*/
 INLINE ospl_ulong BIT_LST_TST
    (
    ospl_uchar *bitList,
    ospl_ulong  bit
    )
{
    return (bitList[bit/8] & bitMask[bit%8])? 1 : 0;
}


/****************************************************************************
*
*����nbit������ֵ�����ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_XOR(ospl_uchar *dst, const ospl_uchar *src1,
			const ospl_uchar *src2, ospl_ulong nbit)
{
	int k;
	ospl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int nr = nbit/8;
	
	for (k = 0; k < nr; k++)
		dst[k] = src1[k] ^ src2[k];

	k = (nbit % 8);
	if(k)
	{
		dst[nr] = src1[nr] ^ src2[nr];
		dst[nr] &= mask[k-1];
	}
}

/****************************************************************************
*
*����nbit������ֵ����ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_AND(ospl_uchar *dst, const ospl_uchar *src1,
			const ospl_uchar *src2, ospl_ulong nbit)
{
	int k;
	ospl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int nr = nbit/8;
	
	for (k = 0; k < nr; k++)
		dst[k] = src1[k] & src2[k];

	k = (nbit % 8);
	if(k)
	{
		dst[nr] = src1[nr] & src2[nr];
		dst[nr] &= mask[k-1];
	}
}

/****************************************************************************
*
*����nbit������ֵ�Ļ�ֵ
*
��nbit��1��ʼ
*/

 INLINE void BIT_LST_OR(ospl_uchar *dst, const ospl_uchar *src1,
			const ospl_uchar *src2, ospl_ulong nbit)
{
	int k;
	ospl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int nr = nbit/8;

	for (k = 0; k < nr; k++)
		dst[k] = src1[k] | src2[k];

	k = (nbit % 8);
	if(k)
	{
		dst[nr] = src1[nr] | src2[nr];
		dst[nr] &= mask[k-1];
	}
}

/****************************************************************************
*
*���nbit�����ص�ֵ�Ƿ�Ϊ0
*
��nbit����1��ʼ
*/
 INLINE ospl_bool BIT_LST_EMPTY(ospl_uchar *src, ospl_ulong nbit)
{
	ospl_uint32 k;
	ospl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	ospl_uint32 nr = nbit/8;
	
	for (k = 0; k < nr; k++)
		if(src[k] != 0)
			return 0;

	k = (nbit % 8);
	if(k)
		return ((src[nr] & mask[k-1]) ==0);
	else
		return 1;
}

/****************************************************************************
*
*�Ƚ�nbit�����ص������ַ�����ֵ�Ƿ����
*
��nbit����1��ʼ
*/
 INLINE int BIT_LST_CMP(const ospl_uchar *src1, const ospl_uchar *src2, ospl_ulong nbit)
{
	ospl_uint32 k;
	ospl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	ospl_uint32 nr = nbit/8;
	
	for (k = 0; k < nr; k++)
	{
		if(src1[k] > src2[k])
			return 1;
		else if(src1[k] < src2[k])
			return -1;
	}

	k = (nbit % 8);
	if(k)
	{
		if((src1[nr] & mask[k-1]) > (src2[nr] & mask[k-1]))
			return 1;
		else if((src1[nr] & mask[k-1]) < (src2[nr] & mask[k-1]))
			return -1;
		else
			return 0;
	}
	else
		return 0;
}

/****************************************************************************
*
*�Ƚ�nbit�����ص������ַ�����ֵ�Ƿ����
*
��nbit����1��ʼ
*/
 INLINE void BIT_LST_COPY(const ospl_uchar *src, ospl_uchar *dst, ospl_ulong nbit)
{
	int k;
	ospl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int nr = nbit/8;
	
	bcopy(src, dst, nr);

	k = (nbit % 8);
	if(k)
	{
		dst[nr] = src[nr] & mask[k-1];
	}
	return;
}

/****************************************************************************
*
*�������ı���λΪ1��ֵ
*
��nbit����1��ʼ
*/
 INLINE ospl_uint32 BIT_LST_MAX(ospl_uchar *src, ospl_ulong nbit)
{
	ospl_uint32 i,k;
	ospl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	ospl_uchar tmp=0;
	ospl_uint32 nr = nbit/8;

	k = (nbit % 8);
	if(k)
	{
		tmp = (src[nr] & mask[k-1]);
		for(i=0; i < 8; i++)
			if(tmp & (1<<i))
				return (nbit - k + (8-i));
	}

	for (k = nr-1; k >= 0; k--)
	{
		if(src[k] == 0)
			continue;
		for(i=0; i < 8; i++)
			if(src[k] & (1<<i))
				return ((k*8) + (8-i));
	}
	return 0;
}

/*
����bitΪ1�ĸ���
*/

 INLINE ospl_uint32 BIT_LST_CNT(ospl_uchar *src, ospl_ulong nbit)
{
	ospl_uint32 nr = nbit/8;
	ospl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	ospl_uint32 n,k,count=0;

/*
	�������㷨�ĺ��ģ�ֻ�����2��������
	1> ��һ��������1ʱ�������ұߵ��Ǹ�ֵΪ1��Bit����Ϊ0��ͬʱ���ұߵ����е�Bit������1��
	2>��&=����λ�벢��ֵ������ȥ���Ѿ�����������1��������ֵ�������ø�n.

	����㷨ѭ���Ĵ�����bitλΪһ�ĸ�����Ҳ��˵�м���BitΪ1��ѭ�����Ρ�
	��BitΪ1�Ƚ�ϡ�������˵�����ܺܺá��磺0x1000 0000, ѭ��һ�ξͿ��ԡ�
*/

	for (k = 0; k < nr; k++)
	{
		n = src[k];
		while(n)
		{
			count++ ;
			n &= (n - 1) ;
		}
	}
	k = (nbit % 8);
	if(k)
	{
		n= (src[nr] & mask[k-1]);
		while(n)
		{
			count++ ;
			n &= (n - 1) ;
		}
	}
	
	return count ;
}
