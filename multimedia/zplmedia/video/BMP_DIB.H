#ifndef __DIB_BMP_H__
#define __DIB_BMP_H__

#ifdef __cplusplus
extern "C" {
#endif



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


#define  BMP_BISIZE           ( sizeof(BITMAPINFOHEADER) )//0X28
#define  BIT_MAP_SIZE         ( sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) )//0X36
#define  BIT_RGB_SIZE         ( sizeof(RGBQUAD) * 256 )

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






#pragma pack(1)
//#pragma align 1

typedef struct tagBITMAPFILEHEADER//14
{ 
      unsigned short  bfType;       //
      unsigned int    bfSize;       //
      unsigned short  bfReserved1;  //
      unsigned short  bfReserved2;
      unsigned int    bfOffBits;    //
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER//40
{
      unsigned int    biSize;              //
      unsigned int    biWidth;             //
      unsigned int    biHeight;            //
      unsigned short  biPlanes;            //
      unsigned short  biBitCount;          //
      unsigned int    biCompression;       //
      unsigned int    biSizeImage;         //
      unsigned int    biXPelsPerMeter;     //
      unsigned int    biYPelsPerMeter;     //
      unsigned int    biClrUsed;           //
      unsigned int    biClrImportant;      //
}BITMAPINFOHEADER;

typedef struct tagRGBQUAD //4
{
       unsigned char rgbBlue;         //
       unsigned char rgbGreen;        //
       unsigned char rgbRed;          //
       unsigned char rgbReserved;     //
}RGBQUAD;

typedef struct tagBITMAPINFO
{
	BITMAPFILEHEADER  bmfHeader;
        BITMAPINFOHEADER  bmiHeader;
	RGBQUAD           bmiColors;
}BITMAPINFO;


typedef struct tagRGB_DIB 
{
       FILE           *bmpIp;             //
       unsigned int   bmpWidth;           //
       unsigned int   bmpHeight;          //
       unsigned int   bmpOffBits;         //
       unsigned int   bmpSize;            //
       unsigned short bmpBitCount;        //
       unsigned short bmpByteCount;        //
       unsigned char  *bmpData;           //
}BMP_DIB;

//#pragma align 0
#pragma pack()
/*typedef struct tagRGB_TYPE 
{
       unsigned int   bmpWidth;           //位图图像宽度
       unsigned int   bmpHeight;          //位图图像高度
       unsigned int   bmpSize;            //位图图像数据大小
       unsigned short bmpBitCount;        //位图每像素颜色的位数
}BMP_TYPE;
*/
//#pragma pack()
/***************************************************************/
/***************************************************************/


typedef  BITMAPFILEHEADER *   BITMAPFILEHEADER_ID;
typedef  BITMAPINFOHEADER *   BITMAPINFOHEADER_ID;
typedef  RGBQUAD *            RGBQUAD_ID;
typedef  BITMAPINFO *         BITMAPINFO_ID;
typedef  BMP_DIB *            BMP_DIB_ID;


typedef  int	STATUS;

#define OK	(0)
#define ERROR	(-1)
//typedef  BMP_TYPE *           BMP_TYPE_ID;
//typedef  int                  STATUS;


/***************************************************************/
/***************************************************************/
extern BMP_DIB     uglBitMapOpen(const char *filename);//


extern BMP_DIB     uglBitMapCreate(const char *filename,BMP_DIB_ID bitMap);

extern STATUS      uglTransBitMap(BMP_DIB_ID bitMapH,char *sbuffer,char *dbuffer,int type);
/***************************************************************/
/***************************************************************/
extern STATUS      uglBitMapReadD(BMP_DIB_ID bitMapH,char *buffer);
extern STATUS      uglBitMapWriteD(BMP_DIB_ID bitMapH,char *buffer);
/***************************************************************/
/***************************************************************/
/***************************************************************/
extern STATUS      uglBitMapRead(BMP_DIB_ID bitMapH,char *buffer,int size);//
extern STATUS      uglBitMapWrite(BMP_DIB_ID bitMapH,char *buffer,int size);//
extern STATUS      uglBitMapClose(BMP_DIB_ID BitMapH);//
extern STATUS      uglBitMapSeek(BMP_DIB_ID BitMapH,int bmpOffBits,int type);//
/***************************************************************/
/***************************************************************/
/***************************************************************/
extern STATUS      uglShowBitMap(BMP_DIB_ID bitMapH,int x,int y,char *bmp);
/***************************************************************/
/***************************************************************/
/***************************************************************/

//extern void ShowBmp(int x0,int y0,int h,int l,unsigned char *bmp);
//extern void ShowBmp(int x0,int y0,int h,int l,unsigned char *bmp);


/***************************************************************/

#ifdef __cplusplus
}
#endif
#endif  /* YS2440_EK_H */

