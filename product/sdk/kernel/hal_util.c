#include "bsp_types.h"
#include "hal_util.h"




#ifdef ZPL_SDK_KERNEL
static const zpl_uchar bitMask[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
};

void zpl_vlan_bitmap_init(zpl_vlan_bitmap_t bitmap)
{
	memset(bitmap.bitmap, 0, sizeof(zpl_vlan_bitmap_t));
}
/*****************************************************************************
*
* zpl_vlan_bitmap_Set - Set a bit in the given byte array.
*/
void  zpl_vlan_bitmap_set(zpl_vlan_bitmap_t bitmap,zpl_uint32  bit)
{
	bitmap.bitmap[(bit)/8] |= bitMask[(bit)%8];
}

/*****************************************************************************
*
* zpl_vlan_bitmap_clr - Remove a bit in the given byte array.
*/
void  zpl_vlan_bitmap_clr(zpl_vlan_bitmap_t bitmap,zpl_uint32  bit)
{
	bitmap.bitmap[(bit)/8] &= ~bitMask[(bit)%8];
}

/*****************************************************************************
*
* zpl_vlan_bitmap_tst - Return True if the bit is set, else False.
*/
int zpl_vlan_bitmap_tst(zpl_vlan_bitmap_t bitmap,zpl_uint32  bit)
{
	int bittst = 0;
	bittst = (bitmap.bitmap[(bit)/8] & bitMask[(bit)%8])? 1 : 0;
	if(bittst == 0)
		return 0;	
    return bittst;
}

void zpl_vlan_bitmap_or(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2)
{
	int k = 0;
	int nr = sizeof(src1.bitmap);
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] | src2.bitmap[k];
}

void zpl_vlan_bitmap_xor(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2)
{
	int k = 0;
	int nr = sizeof(src1.bitmap);
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] ^ src2.bitmap[k];
}

void zpl_vlan_bitmap_and(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2)
{
	int k = 0;
	int nr = sizeof(src1.bitmap);
	for (k = 0; k < nr; k++)
		dst.bitmap[k] = src1.bitmap[k] & src2.bitmap[k];
}

int zpl_vlan_bitmap_cmp(const zpl_vlan_bitmap_t src1, const zpl_vlan_bitmap_t src2)
{
	return memcmp(&src1.bitmap, &src2.bitmap, sizeof(src1.bitmap));
}

void  zpl_vlan_bitmap_copy(const zpl_vlan_bitmap_t src, zpl_vlan_bitmap_t dst)
{
	memcpy(&dst.bitmap, &src.bitmap, sizeof(dst.bitmap));
	return;
}








void os_bitmap_init(zpl_bitmap_t bitmap)
{
	memset(bitmap.bitmap, 0, sizeof(zpl_bitmap_t));
}
/*****************************************************************************
*
* os_bitmap_Set - Set a bit in the given byte array.
*/
void  os_bitmap_set
    (
    zpl_bitmap_t bitmap,
    zpl_uint32  bit
    )
{
	bitmap.bitmap[(bit)/8] |= bitMask[(bit)%8];
}

/*****************************************************************************
*
* os_bitmap_clr - Remove a bit in the given byte array.
*/
 void  os_bitmap_clr
    (
    zpl_bitmap_t bitmap,
    zpl_uint32  bit
    )
{
	bitmap.bitmap[(bit)/8] &= ~bitMask[(bit)%8];	
}

/*****************************************************************************
*
* os_bitmap_tst - Return True if the bit is set, else False.
*/
int os_bitmap_tst
    (
    zpl_bitmap_t bitmap,
    zpl_uint32  bit
    )
{
	int bittst = 0;
	bittst = (bitmap.bitmap[(bit)/8] & bitMask[(bit)%8])? 1 : 0;
	if(bittst == 0)
		return 0;
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

#endif


