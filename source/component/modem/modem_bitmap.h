/*
 * modem_bitmap.h
 *
 *  Created on: Aug 16, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_BITMAP_H__
#define __MODEM_BITMAP_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct modem_bitmap_s
{
	zpl_uint8 bit[16];
}modem_bitmap_t;




/*
#define MODEM_BITMAP_SET(v, n)		((v) |= (1<<(n)))
#define MODEM_BITMAP_CHK(v, n)		((v) & (1<<(n)))
#define MODEM_BITMAP_CLR(v, n)		((v) &= ~(1<<(n)))
*/

#define MODEM_BITMAP_SET(v, n)		modem_bitmap_set((v), (n))
#define MODEM_BITMAP_CHK(v, n)		modem_bitmap_chk((v), (n))
#define MODEM_BITMAP_CLR(v, n)		modem_bitmap_clr((v), (n))

extern int modem_bitmap_set(modem_bitmap_t *bitmap, zpl_uint32 bit);
extern int modem_bitmap_clr(modem_bitmap_t *bitmap, zpl_uint32 bit);
extern int modem_bitmap_chk(modem_bitmap_t *bitmap, zpl_uint32 bit);
extern int modem_bitmap_bzero(modem_bitmap_t *bitmap);


#ifdef __cplusplus
}
#endif

#endif /* __MODEM_BITMAP_H__ */
