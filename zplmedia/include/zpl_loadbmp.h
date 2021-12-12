#ifndef     __LOAD_BMP_H__
#define     __LOAD_BMP_H__

#include "zpl_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/* the color format OSD supported */
typedef enum hiOSD_COLOR_FMT_E
{
    OSD_COLOR_FMT_RGB444    = 0,
    OSD_COLOR_FMT_RGB4444   = 1,
    OSD_COLOR_FMT_RGB555    = 2,
    OSD_COLOR_FMT_RGB565    = 3,
    OSD_COLOR_FMT_RGB1555   = 4,
    OSD_COLOR_FMT_RGB888    = 6,
    OSD_COLOR_FMT_RGB8888   = 7,
    OSD_COLOR_FMT_BUTT
} OSD_COLOR_FMT_E;

typedef struct hiOSD_RGB_S
{
    zpl_uint8   u8B;
    zpl_uint8   u8G;
    zpl_uint8   u8R;
    zpl_uint8   u8Reserved;
} OSD_RGB_S;

typedef struct hiOSD_SURFACE_S
{
    OSD_COLOR_FMT_E enColorFmt;         /* color format */
    zpl_uint8*  pu8PhyAddr;               /* physical address */
    zpl_uint16  u16Height;                /* operation height */
    zpl_uint16  u16Width;                 /* operation width */
    zpl_uint16  u16Stride;                /* surface stride */
    zpl_uint16  u16Reserved;
} OSD_SURFACE_S;

typedef struct tag_OSD_Logo
{
    zpl_uint32    width;        /* out */
    zpl_uint32    height;       /* out */
    zpl_uint32    stride;       /* in */
    zpl_uint8*    pRGBBuffer;   /* in/out */
} OSD_LOGO_T;

typedef struct tag_OSD_BITMAPINFOHEADER
{
    zpl_uint16      biSize;
    zpl_uint32       biWidth;
    zpl_int32       biHeight;
    zpl_uint16       biPlanes;
    zpl_uint16       biBitCount;
    zpl_uint32      biCompression;
    zpl_uint32      biSizeImage;
    zpl_uint32       biXPelsPerMeter;
    zpl_uint32       biYPelsPerMeter;
    zpl_uint32      biClrUsed;
    zpl_uint32      biClrImportant;
} OSD_BITMAPINFOHEADER;

typedef struct tag_OSD_BITMAPFILEHEADER
{
    zpl_uint32   bfSize;
    zpl_uint16    bfReserved1;
    zpl_uint16    bfReserved2;
    zpl_uint32   bfOffBits;
} OSD_BITMAPFILEHEADER;

typedef struct tag_OSD_RGBQUAD
{
    zpl_uint8    rgbBlue;
    zpl_uint8    rgbGreen;
    zpl_uint8    rgbRed;
    zpl_uint8    rgbReserved;
} OSD_RGBQUAD;

typedef struct tag_OSD_BITMAPINFO
{
    OSD_BITMAPINFOHEADER    bmiHeader;
    OSD_RGBQUAD                 bmiColors[1];
} OSD_BITMAPINFO;

typedef struct hiOSD_COMPONENT_INFO_S
{
    int alen;
    int rlen;
    int glen;
    int blen;
} OSD_COMP_INFO;

zpl_int32 LoadImage(const zpl_char* filename, OSD_LOGO_T* pVideoLogo);
zpl_int32 LoadBitMap2Surface(const zpl_char* pszFileName, const OSD_SURFACE_S* pstSurface, zpl_uint8* pu8Virt);
zpl_int32 CreateSurfaceByBitMap(const zpl_char* pszFileName, OSD_SURFACE_S* pstSurface, zpl_uint8* pu8Virt);
zpl_int32 CreateSurfaceByCanvas(const zpl_char* pszFileName, OSD_SURFACE_S* pstSurface, zpl_uint8* pu8Virt, zpl_uint32 u32Width, zpl_uint32 u32Height, zpl_uint32 u32Stride);
zpl_int32 GetBmpInfo(const zpl_char* filename, OSD_BITMAPFILEHEADER*  pBmpFileHeader, OSD_BITMAPINFO* pBmpInfo);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __LOAD_BMP_H__*/

