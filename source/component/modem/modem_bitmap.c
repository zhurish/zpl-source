/*
 * modem_bitmap.c
 *
 *  Created on: Aug 16, 2018
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zpl_type.h"

#include "modem.h"
#include "modem_bitmap.h"


static const zpl_uint8 mdbitMask[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};



int modem_bitmap_set(modem_bitmap_t *bitmap, zpl_uint32 bit)
{
	bitmap->bit[bit/8] |= mdbitMask[bit%8];
	return OK;
}


int modem_bitmap_clr(modem_bitmap_t *bitmap, zpl_uint32 bit)
{
	bitmap->bit[bit/8] &= ~mdbitMask[bit%8];
	return OK;
}

int modem_bitmap_chk(modem_bitmap_t *bitmap, zpl_uint32 bit)
{
	return (bitmap->bit[bit/8] & mdbitMask[bit%8]) ? 1:0;
}


int modem_bitmap_bzero(modem_bitmap_t *bitmap)
{
	os_memset(bitmap, 0, sizeof(modem_bitmap_t));
	return OK;
}
