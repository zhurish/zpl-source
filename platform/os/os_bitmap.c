
#include "os_include.h"

#include "os_bitmap.h"

static const zpl_uchar bitMask[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};


/*****************************************************************************
*
* bitListSet - Set a bit in the given byte array.
*/
void  bitListSet
    (
    zpl_bitmap_t bitList,
    zpl_uint32  bit
    )
{
    bitList.bitmap[bit/8] |= bitMask[bit%8];
}

/*****************************************************************************
*
* bitListClr - Remove a bit in the given byte array.
*/
 void  bitListClr
    (
    zpl_bitmap_t bitList,
    zpl_uint32  bit
    )
{
    bitList.bitmap[bit/8] &= ~bitMask[bit%8];
}

/*****************************************************************************
*
* bitListTst - Return True if the bit is set, else False.
*/
zpl_uint32 bitListTst
    (
    zpl_bitmap_t bitList,
    zpl_uint32  bit
    )
{
    return (bitList.bitmap[bit/8] & bitMask[bit%8])? 1 : 0;
}


/****************************************************************************
*
*����nbit������ֵ�����ֵ
*
��nbit��1��ʼ
*/

 void  bitListXor(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2, zpl_uint32 nbit)
{
	int k;
	zpl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int nr = nbit/8;
	
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] ^ src2.bitmap[k];

	k = (nbit % 8);
	if(k)
	{
		dst.bitmap[nr] = src1.bitmap[nr] ^ src2.bitmap[nr];
		dst.bitmap[nr] &= mask[k-1];
	}
}

/****************************************************************************
*
*����nbit������ֵ����ֵ
*
��nbit��1��ʼ
*/

 void  bitListAnd(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2, zpl_uint32 nbit)
{
	int k;
	zpl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int nr = nbit/8;
	
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] & src2.bitmap[k];

	k = (nbit % 8);
	if(k)
	{
		dst.bitmap[nr] = src1.bitmap[nr] & src2.bitmap[nr];
		dst.bitmap[nr] &= mask[k-1];
	}
}

/****************************************************************************
*
*����nbit������ֵ�Ļ�ֵ
*
��nbit��1��ʼ
*/

 void  bitListOr(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2, zpl_uint32 nbit)
{
	int k;
	zpl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int nr = nbit/8;

	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] | src2.bitmap[k];

	k = (nbit % 8);
	if(k)
	{
		dst.bitmap[nr] = src1.bitmap[nr] | src2.bitmap[nr];
		dst.bitmap[nr] &= mask[k-1];
	}
}

/****************************************************************************
*
*���nbit�����ص�ֵ�Ƿ�Ϊ0
*
��nbit����1��ʼ
*/
zpl_bool bitListEmpty(zpl_bitmap_t src, zpl_uint32 nbit)
{
	zpl_uint32 k;
	zpl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	zpl_uint32 nr = nbit/8;
	
	for (k = 0; k < nr; k++)
		if(src.bitmap[k] != 0)
			return 0;

	k = (nbit % 8);
	if(k)
		return ((src.bitmap[nr] & mask[k-1]) ==0);
	else
		return 1;
}

/****************************************************************************
*
*�Ƚ�nbit�����ص������ַ�����ֵ�Ƿ����
*
��nbit����1��ʼ
*/
int bitListCmp(const zpl_bitmap_t src1, const zpl_bitmap_t src2, zpl_uint32 nbit)
{
	zpl_uint32 k;
	zpl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	zpl_uint32 nr = nbit/8;
	
	for (k = 0; k < nr; k++)
	{
		if(src1.bitmap[k] > src2.bitmap[k])
			return 1;
		else if(src1.bitmap[k] < src2.bitmap[k])
			return -1;
	}

	k = (nbit % 8);
	if(k)
	{
		if((src1.bitmap[nr] & mask[k-1]) > (src2.bitmap[nr] & mask[k-1]))
			return 1;
		else if((src1.bitmap[nr] & mask[k-1]) < (src2.bitmap[nr] & mask[k-1]))
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
 void  bitListCopy(const zpl_bitmap_t src, zpl_bitmap_t dst, zpl_uint32 nbit)
{
	int k;
	zpl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int nr = nbit/8;
	
	bcopy(&src.bitmap, &dst.bitmap, nr);

	k = (nbit % 8);
	if(k)
	{
		dst.bitmap[nr] = src.bitmap[nr] & mask[k-1];
	}
	return;
}

/****************************************************************************
*
*�������ı���λΪ1��ֵ
*
��nbit����1��ʼ
*/
zpl_uint32 bitListMax(zpl_bitmap_t src, zpl_uint32 nbit)
{
	zpl_uint32 i,k;
	zpl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	zpl_uchar tmp=0;
	zpl_uint32 nr = nbit/8;

	k = (nbit % 8);
	if(k)
	{
		tmp = (src.bitmap[nr] & mask[k-1]);
		for(i=0; i < 8; i++)
			if(tmp & (1<<i))
				return (nbit - k + (8-i));
	}

	for (k = nr-1; k >= 0; k--)
	{
		if(src.bitmap[k] == 0)
			continue;
		for(i=0; i < 8; i++)
			if(src.bitmap[k] & (1<<i))
				return ((k*8) + (8-i));
	}
	return 0;
}

/*
����bitΪ1�ĸ���
*/

zpl_uint32 bitListCnt(zpl_bitmap_t src, zpl_uint32 nbit)
{
	zpl_uint32 nr = nbit/8;
	zpl_uchar mask[8] ={0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	zpl_uint32 n,k,count=0;

/*
	�������㷨�ĺ��ģ�ֻ�����2��������
	1> ��һ��������1ʱ�������ұߵ��Ǹ�ֵΪ1��Bit����Ϊ0��ͬʱ���ұߵ����е�Bit������1��
	2>��&=����λ�벢��ֵ������ȥ���Ѿ�����������1��������ֵ�������ø�n.

	����㷨ѭ���Ĵ�����bitλΪһ�ĸ�����Ҳ��˵�м���BitΪ1��ѭ�����Ρ�
	��BitΪ1�Ƚ�ϡ�������˵�����ܺܺá��磺0x1000 0000, ѭ��һ�ξͿ��ԡ�
*/

	for (k = 0; k < nr; k++)
	{
		n = src.bitmap[k];
		while(n)
		{
			count++ ;
			n &= (n - 1) ;
		}
	}
	k = (nbit % 8);
	if(k)
	{
		n= (src.bitmap[nr] & mask[k-1]);
		while(n)
		{
			count++ ;
			n &= (n - 1) ;
		}
	}
	
	return count ;
}
