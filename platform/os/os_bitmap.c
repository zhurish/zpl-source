
#include "auto_include.h"

#include "os_bitmap.h"

static const zpl_uchar bitMask[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};


/*****************************************************************************
*
* os_bitmap_Set - Set a bit in the given byte array.
*/
void  os_bitmap_set
    (
    zpl_bitmap_t bitmap,
    zpl_uint32  bit,
	zpl_uint32  wigth
    )
{
	int k = 0;
	for (k = 0; k < wigth; k++)
	{
    	bitmap.bitmap[(bit+k)/8] |= bitMask[(bit+k)%8];
	}
}

/*****************************************************************************
*
* os_bitmap_clr - Remove a bit in the given byte array.
*/
 void  os_bitmap_clr
    (
    zpl_bitmap_t bitmap,
    zpl_uint32  bit,
	zpl_uint32  wigth
    )
{
	int k = 0;
	for (k = 0; k < wigth; k++)
	{
    	bitmap.bitmap[(bit+k)/8] &= ~bitMask[(bit+k)%8];
	}	
}

/*****************************************************************************
*
* os_bitmap_tst - Return True if the bit is set, else False.
*/
int os_bitmap_tst
    (
    zpl_bitmap_t bitmap,
    zpl_uint32  bit,
	zpl_uint32  wigth
    )
{
	int k = 0, bittst = 0;
	for (k = 0; k < wigth; k++)
	{
		bittst = (bitmap.bitmap[(bit+k)/8] & bitMask[(bit+k)%8])? 1 : 0;
		if(bittst == 0)
			return 0;
	}	
    return bittst;
}
/*****************************************************************************/
 void os_bitmap_and(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2)
{
	int k = 0;
	int nr = sizeof(src1.bitmap);
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] & src2.bitmap[k];
}
/*****************************************************************************/
 void os_bitmap_or(zpl_bitmap_t dst, const zpl_bitmap_t src1,
			const zpl_bitmap_t src2)
{
	int k = 0;
	int nr = sizeof(src1.bitmap);
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] | src2.bitmap[k];
}
/*****************************************************************************/
int os_bitmap_cmp(const zpl_bitmap_t src1, const zpl_bitmap_t src2)
{
	return memcmp(&src1.bitmap, &src2.bitmap, sizeof(src1.bitmap));
}
/*****************************************************************************/
 void  os_bitmap_copy(const zpl_bitmap_t src, zpl_bitmap_t dst)
{
	memcpy(&dst.bitmap, &src.bitmap, sizeof(dst.bitmap));
	return;
}

/*****************************************************************************/
/*****************************************************************************/
void os_bit64_set(zpl_uint64 *bitmap, zpl_uint32  bit, zpl_uint32  wigth)
{
	zpl_uchar *bittbl = (zpl_uchar *)bitmap;
	int k = 0;
	for (k = 0; k < wigth; k++)
	{
    	bittbl[(bit+k)/8] |= bitMask[(bit+k)%8];
	}
}

void os_bit64_clr(zpl_uint64 *bitmap, zpl_uint32  bit, zpl_uint32  wigth)
{
	zpl_uchar *bittbl = (zpl_uchar *)bitmap;
	int k = 0;
	for (k = 0; k < wigth; k++)
	{
    	bittbl[(bit+k)/8] &= ~bitMask[(bit+k)%8];
	}
}

int os_bit64_tst(zpl_uint64 *bitmap, zpl_uint32  bit, zpl_uint32  wigth)
{
	zpl_uchar *bittbl = (zpl_uchar *)bitmap;
	int k = 0, bittst = 0;
	for (k = 0; k < wigth; k++)
	{
		bittst = (bittbl[(bit+k)/8] & bitMask[(bit+k)%8])? 1 : 0;
		if(bittst == 0)
			return 0;
	}	
    return bittst;
}