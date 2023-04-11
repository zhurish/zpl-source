/*
 * zpl_media_bmp.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_BMP_H__
#define __ZPL_MEDIA_BMP_H__

#ifdef __cplusplus
extern "C" {
#endif



/***************************************************************/

#define  BMP_FLAG_TYPE        0X4D42

//001Ch biBitCount
#define  BMP_BITCOUNT_1       1

#define  BMP_BITCOUNT_4       4//
#define  BMP_BITCOUNT_8       0x08//
#define  BMP_BITCOUNT_16      16  //
#define  BMP_BITCOUNT_24      0x18//
#define  BMP_BITCOUNT_32      0x20//


#define  BMP_TYPE             (0xFA00)

#define  BMP_INVERT_SAVER     (BMP_TYPE|0X40)
#define  BMP565_DIB555        (BMP_TYPE|0X41)//
#define  BMP555_DIB565        (BMP_TYPE|0X42)//

#define  BMP32_DIB565         (BMP_TYPE|BMP_BITCOUNT_32)//
#define  BMP24_DIB565         (BMP_TYPE|BMP_BITCOUNT_24)//
#define  BMP8_DIB565          (BMP_TYPE|BMP_BITCOUNT_8)//
#define  BMP1_DIB565          (BMP_TYPE|BMP_BITCOUNT_1)//

#define  BMP_SAVER_8          (BMP_TYPE|0X44)//
#define  BMP_SAVER_1          (BMP_TYPE|0X48)//
#define  BMP_SAVER_555        (BMP_TYPE|0X80)//
#define  BMP_SAVER_565        (BMP_TYPE|0X81)//


//001Eh Compression
#define  BI_RGB               0X0//
#define  BI_RLE8              0X1//
#define  BI_RLE4              0X2//
#define  BI_BITFIELDS         0X3//


#define  APP_BMP_BISIZE           ( sizeof(APP_BITMAPINFOHEADER) )//0X28
#define  APP_BIT_MAP_SIZE         ( sizeof(APP_BITMAPFILEHEADER) + sizeof(APP_BITMAPINFOHEADER) )//0X36
#define  APP_BIT_RGB_SIZE         ( sizeof(APP_RGBQUAD) * 256 )

#define  WIDTHBYTES(n)        ( ( (n + 31) >>5 )<<2 )//






#ifndef UGL_RGB
#define UGL_RGB(r,g,b)  ( ( (r>>3)<<11)|((g>>2)<<5)|(b>>3) )
#endif

#ifndef GET_RGB
#define RB_MASK_KEY    (0X1F)
#define G_MASK_KEY     (0X3F)

#define GET_RB_VALUE(r)    ( (r*RB_MASK_KEY)>>8 )
#define GET_G_VALUE(r)     ( (r*G_MASK_KEY)>>8 )

#define GET_RGB(r,g,b)     ( (r<<11)|(g<<5)|b )
#endif







typedef struct
{
    FILE           *bmpIp;             //
    unsigned int   bmpWidth;           //
    unsigned int   bmpHeight;          //
    unsigned int   bmpOffBits;         //
    unsigned int   bmpSize;            //
    unsigned short bmpBitCount;        //
    unsigned short bmpByteCount;        //
    unsigned char  *bmpData;           //
}UGL_BMP_ID;


//#pragma pack()
/***************************************************************/
/***************************************************************/

#ifndef OK
#define OK	(0)
#endif
#ifndef ERROR
#define ERROR	(-1)
#endif


/***************************************************************/
/***************************************************************/
extern UGL_BMP_ID *uglBitMapOpen(const char *filename);//

extern UGL_BMP_ID *uglBitMapCreate(const char *filename, unsigned int bmpWidth, unsigned int bmpHeight, unsigned int bmpBitCount);

extern int      uglTransBitMap(UGL_BMP_ID *bitMapH,char *sbuffer,char *dbuffer,int type);
/***************************************************************/
/***************************************************************/
extern int      uglBitMapReadD(UGL_BMP_ID *bitMapH,char *buffer);
extern int      uglBitMapWriteD(UGL_BMP_ID *bitMapH,char *buffer);
/***************************************************************/
/***************************************************************/
/***************************************************************/
extern int      uglBitMapRead(UGL_BMP_ID *bitMapH,char *buffer,int size);//
extern int      uglBitMapWrite(UGL_BMP_ID *bitMapH,char *buffer,int size);//
extern int      uglBitMapClose(UGL_BMP_ID *BitMapH);//
extern int      uglBitMapSeek(UGL_BMP_ID *BitMapH,int bmpOffBits,int type);//
/***************************************************************/
/***************************************************************/
/***************************************************************/
extern int      uglShowBitMap(UGL_BMP_ID *bitMapH,int x,int y,char *bmp);
/***************************************************************/
/***************************************************************/
/***************************************************************/
//extern void ShowBmp(int x0,int y0,int h,int l,unsigned char *bmp);
//extern void ShowBmp(int x0,int y0,int h,int l,unsigned char *bmp);

extern int test_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_BMP_H__ */
