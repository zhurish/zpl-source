#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "loadbmp.h"


#define  BMP_DEBUG


#pragma pack(1)

typedef struct __tagBITMAPFILEHEADER//14
{
    unsigned short  bfType;       //
    unsigned int    bfSize;       //
    unsigned short  bfReserved1;  //
    unsigned short  bfReserved2;
    unsigned int    bfOffBits;    //
}APP_BITMAPFILEHEADER;

typedef struct __tagBITMAPINFOHEADER//40
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
}APP_BITMAPINFOHEADER;

typedef struct __tagRGBQUAD //4
{
    unsigned char rgbBlue;         //
    unsigned char rgbGreen;        //
    unsigned char rgbRed;          //
    unsigned char rgbReserved;     //
}APP_RGBQUAD;

typedef struct __tagBITMAPINFO
{
    APP_BITMAPFILEHEADER  bmfHeader;
    APP_BITMAPINFOHEADER  bmiHeader;
    APP_RGBQUAD           bmiColors;
}APP_BITMAPINFO;

typedef  APP_BITMAPFILEHEADER *   BITMAPFILEHEADER_ID;
typedef  APP_BITMAPINFOHEADER *   BITMAPINFOHEADER_ID;
typedef  APP_RGBQUAD *            RGBQUAD_ID;
typedef  APP_BITMAPINFO *         BITMAPINFO_ID;

#pragma pack(0)


#ifdef BMP_DEBUG
static int BMP__Print(UGL_BMP_ID  *bitMapH);
static int BMP_DEBUG_Print(APP_BITMAPINFO *bmpHeader);
#endif

/*********************************************************************/
/*********************************************************************/
static int  ReadBitMapHeader(FILE * ip, APP_BITMAPINFO *bmpHeader)//
{
    fseek(ip,0,SEEK_SET);
    fread(&bmpHeader->bmfHeader,sizeof(APP_BITMAPFILEHEADER),1,ip);
    //  fseek(ip,14,SEEK_SET);
    fread(&bmpHeader->bmiHeader,sizeof(APP_BITMAPINFOHEADER),1,ip);
    //  fseek(ip,40,SEEK_SET);
    fread(&bmpHeader->bmiColors,sizeof(APP_RGBQUAD),1,ip);
    if(bmpHeader->bmfHeader.bfType!=BMP_FLAG_TYPE)
    {
        printf("this file is not BMP file!\n");
        return -1;
    }
    return (0);
}
/*********************************************************************/
/*********************************************************************/
static int WriteBitMapHeader(FILE * ip, UGL_BMP_ID *bitMap, APP_BITMAPINFO *bmpHeader)//
{
    bmpHeader->bmfHeader.bfType      = BMP_FLAG_TYPE;
    bmpHeader->bmfHeader.bfSize      = bitMap->bmpSize + bitMap->bmpOffBits;
    bmpHeader->bmfHeader.bfReserved1 = 0;
    bmpHeader->bmfHeader.bfReserved2 = 0;
    bmpHeader->bmfHeader.bfOffBits   = bitMap->bmpOffBits;

    bmpHeader->bmiHeader.biSize          = APP_BMP_BISIZE;
    bmpHeader->bmiHeader.biWidth         = bitMap->bmpWidth;
    bmpHeader->bmiHeader.biHeight        = bitMap->bmpHeight;
    bmpHeader->bmiHeader.biPlanes        = 1;
    bmpHeader->bmiHeader.biBitCount      = bitMap->bmpBitCount;//BMP_BITCOUNT_16;
    bmpHeader->bmiHeader.biCompression   = BI_RGB;
    bmpHeader->bmiHeader.biSizeImage     = bitMap->bmpSize;
    bmpHeader->bmiHeader.biXPelsPerMeter = 0;
    bmpHeader->bmiHeader.biYPelsPerMeter = 0;
    bmpHeader->bmiHeader.biClrUsed       = 0;
    bmpHeader->bmiHeader.biClrImportant  = 0;

    bmpHeader->bmiColors.rgbBlue     = 0;
    bmpHeader->bmiColors.rgbGreen    = 0;
    bmpHeader->bmiColors.rgbRed      = 0;
    bmpHeader->bmiColors.rgbReserved = 0;
    fseek(ip,0,SEEK_SET);
    fwrite((char*)(bmpHeader),sizeof(APP_BITMAPINFO),1,ip);
    return (0);
}
/*********************************************************************/
/*********************************************************************/
static int Tran555BitMap565(unsigned short *sbuffer,unsigned short *dbuffer,int lenth)//
{
    unsigned int i,j=0,r,g,b,rgb;
    for(i=0; i<lenth; i++)
    {
        rgb = sbuffer[i];
        b = rgb&0x1f;
        g = (rgb>>5)&0x1f;
        r = (rgb>>10)&0x1f;
        dbuffer[i]  = (r<<11)|(g<<6)|b;
    }
    return (i);
}
/*********************************************************************/
/*********************************************************************/
static int Tran565BitMap555(unsigned short *sbuffer,unsigned short *dbuffer,int lenth)//
{
    unsigned int i,r,g,b,rgb;
    for(i=0; i<lenth; i++)
    {
        rgb = sbuffer[i];
        b = rgb&0x1f;
        g = (rgb>>5)&0x3f;
        r = (rgb>>11)&0x1f;
        dbuffer[i]  = (r<<10)|((g>>1)<<5)|b;
    }
    return (i);
}
/*********************************************************************/
/*********************************************************************/
static int Tran24To16BitMap(unsigned char *sbuffer,unsigned short *dbuffer,int lenth)//
{
    unsigned int i,j=0,r,g,b,rgb;
#ifdef BMP_DEBUG
    printf("24λλͼ\n");
#endif
    for(i=0; i<lenth; i+=3)
    {
        rgb = 0;
        b = sbuffer[i];
        g = sbuffer[i+1];
        r = sbuffer[i+2];
        b >>= 3;
        g >>= 2;
        r >>= 3;
        rgb |= (r<<11)|(g<<5)|b;
        dbuffer[j]   = rgb;
        j++;
    }
    return (2*j);
}
/*********************************************************************/
/*********************************************************************/
static int Tran32To16BitMap(unsigned char *sbuffer,unsigned short *dbuffer,int lenth)//
{
    unsigned int i,j=0,r,g,b,rgb;
#ifdef BMP_DEBUG
    printf("32λλͼ\n");
#endif
    for(i=0; i<lenth; i+=4)
    {
        rgb = 0;
        b = sbuffer[i];
        g = sbuffer[i+1];
        r = sbuffer[i+2];
        b >>= 3;
        g >>= 2;
        r >>= 3;
        rgb |= (r<<11)|(g<<5)|b;
        dbuffer[j]   = rgb;
        j++;
    }
    return (2*j);
}
/*********************************************************************/
/*********************************************************************/
static int Tran8To565BitMap(unsigned char *sbuffer,unsigned short *dbuffer,int lenth)//
{
    unsigned int i,r,g,b;
#ifdef BMP_DEBUG
    printf("8λ�Ҷ�λͼ\n");
#endif
    for(i=0; i<lenth; i++)
    {
        r = b = GET_RB_VALUE(sbuffer[i]);
        g = GET_G_VALUE(sbuffer[i]);
        dbuffer[i]  = (unsigned short)GET_RGB(r,g,b);// rgb;
        //r = g = b = sbuffer[i];
        //dbuffer[lenth-i]  = (unsigned short)UGL_RGB(r,g,b);// rgb;
        //dbuffer[i]  = (unsigned short)UGL_RGB(r,g,b);// rgb;
    }
    return (2*i);
}
/********************************************************************/
/********************************************************************/
static int TansBwByte(unsigned char buf,unsigned short *dbuf,int value,int n)
{
    int i,mask;
    for(i=0,mask=0x80;i<8;i++)
    {
        if(buf & mask)
            dbuf[i+n] = value;
        else
            dbuf[i+n] = 0;
        mask = mask>>1;
    }
}
/*********************************************************************/
/*********************************************************************/
static int Tran1To565BitMap(unsigned char *buf,unsigned short *dbuf,int lenth)
{
    int i,n;
    for(i=0,n=0;i<lenth;i++,n+=8)
        TansBwByte(buf[i],dbuf,0XFFFF,n);
    return (n*8*2);
}
/*********************************************************************/
/*********************************************************************/
static int Tran565To8BitMap(unsigned short *buf,unsigned char *dbuf,int lenth)
{
    int i,r,g,b;
    for(i=0; i<lenth; i++)
    {
        r = buf[i]&0XF800;
        g = buf[i]&0X0720;
        b = buf[i]&0X001F;
        r = (r<<8)>>5;
        g = (r<<8)>>6;
        b = (r<<8)>>5;
        dbuf[i] = (r + g + b)/3;
    }
    return (i);
}
/*********************************************************************/
/*********************************************************************/
static int Tran565To1BitMap(unsigned short *buf,unsigned char *dbuf,int lenth)
{
    int i,j,mask;
    for(i=0; i<lenth; i+=8)
    {
        for(j=0 ,mask = 0; j<8; j++)
        {
            if(buf[i+j]>0xff)
                mask |= 1<<(8-j);
            else
                mask |= 0<<(8-j);
        }
        dbuf[i] = mask;
    }
    return ((i+1)/8);
}
/*********************************************************************/
/*********************************************************************/
static int ImageInvert(unsigned char *buf,unsigned char *dbuf,int lenth)
{
    unsigned int i;
    for(i=0;i<lenth;i++)
        dbuf[i] = 0xff - buf[i];
    return i;
}
/*********************************************************************/
/*********************************************************************/
/*
static unsigned int GetBitMapBwBit(char *buffer,int lenth,int hit)
{
#define  GET_BIT(n)      (1<<n)
    unsigned int i,m=0;
    for(i=8;i>0;i--)
    {
        if((unsigned int)buffer[8*lenth+i]>=(unsigned int)hit)
            m |= GET_BIT(i);
    }
    return m;
#undef GET_BIT
}
*/
/*********************************************************************/
/*********************************************************************/
/*
static int TransBitMapToBw(UGL_BMP_ID *bitMapH,char *sbuffer,char *dbuffer,int hit)
{
    int i,value;
    int size = (bitMapH->bmpSize)/8;
    for(i=0;i<size;i++)
    {
        value = GetBitMapBwBit(sbuffer,i,hit);
        dbuffer[i] = value & 0XFF;
    }
    return i;
}
*/
/*********************************************************************/
/*********************************************************************/
static int CheckBitMapStruct(UGL_BMP_ID *bitMapH)
{
    int size,bitsize;
    switch(bitMapH->bmpBitCount)
    {                   //WIDTHBYTES(bitMapH->bmpWidth*BMP_BITCOUNT_1)
    case BMP_BITCOUNT_1:   bitsize = ( ( bitMapH->bmpWidth*BMP_BITCOUNT_1+31)>>5)<<2;
        size = bitsize * bitMapH->bmpHeight;    break;

    case BMP_BITCOUNT_4:   bitsize = ( ( bitMapH->bmpWidth*BMP_BITCOUNT_4+31)>>5)<<2;
        size = bitsize * bitMapH->bmpHeight;    break;

    case BMP_BITCOUNT_8:   bitsize = ( ( bitMapH->bmpWidth*BMP_BITCOUNT_8+31)>>5)<<2;
        size = bitsize * bitMapH->bmpHeight;    break;

    case BMP_BITCOUNT_16:  bitsize = ( ( (bitMapH->bmpWidth*BMP_BITCOUNT_16)+31)>>5)<<2;
        size = bitsize * bitMapH->bmpHeight;    break;

    case BMP_BITCOUNT_24:  bitsize = ( ( (bitMapH->bmpWidth*BMP_BITCOUNT_24)+31)>>5)<<2;
        size = bitsize * bitMapH->bmpHeight;    break;

    case BMP_BITCOUNT_32:  bitsize = ( ( (bitMapH->bmpWidth*BMP_BITCOUNT_32)+31)>>5)<<2;
        size = bitsize * bitMapH->bmpHeight;    break;
    default: break;
    }
    if(bitMapH->bmpSize != size)
        bitMapH->bmpSize = size;
    if( (bitsize%4)!=0 )
        return ERROR;
    return OK;
}
/*********************************************************************/
/*********************************************************************/
static int CreateBitMapRGB(UGL_BMP_ID * bitMapH)//
{
    unsigned int i;
    if(bitMapH->bmpBitCount == BMP_BITCOUNT_8)
    {
        APP_RGBQUAD rgbquad[256];
        for(i=0;i<256;i++)
        {
            rgbquad[i].rgbBlue = rgbquad[i].rgbGreen = rgbquad[i].rgbRed=i;
            rgbquad[i].rgbReserved=0;
        }
        fseek(bitMapH->bmpIp,APP_BIT_MAP_SIZE, SEEK_SET);
        fwrite((char *)rgbquad,1,APP_BIT_RGB_SIZE, bitMapH->bmpIp);
    }
    if(bitMapH->bmpBitCount == BMP_BITCOUNT_1)
    {
        APP_RGBQUAD rgbquad[2];
        rgbquad[0].rgbBlue     = rgbquad[0].rgbGreen    = 0;
        rgbquad[0].rgbRed      = rgbquad[0].rgbReserved = 0;

        rgbquad[1].rgbBlue  = rgbquad[1].rgbGreen          = 0xff;
        rgbquad[1].rgbRed   = 0xff; rgbquad[1].rgbReserved = 0;

        fseek(bitMapH->bmpIp,APP_BIT_MAP_SIZE, SEEK_SET);
        fwrite((char *)rgbquad,1,sizeof(APP_RGBQUAD)*2, bitMapH->bmpIp);
    }
    return OK;
}
/*********************************************************************/
/*********************************************************************/
int uglTransBitMap(UGL_BMP_ID *bitMapH,char *sbuffer,char *dbuffer,int type)
{
    unsigned int Type = 0,size = bitMapH->bmpSize;
#ifdef BMP_DEBUG
    BMP__Print(bitMapH);
#endif
    if(type<=BMP_BITCOUNT_32)
        Type = type|BMP_TYPE;
    else Type = type;
    switch(Type)
    {
    case  BMP565_DIB555: return Tran565BitMap555((unsigned short *)sbuffer,
                                                 (unsigned short *)dbuffer,size/2+1);
    case  BMP555_DIB565: return Tran555BitMap565((unsigned short *)sbuffer,
                                                 (unsigned short *)dbuffer,size/2+1);
    case  BMP32_DIB565:  return Tran32To16BitMap((unsigned char *)sbuffer,
                                                 (unsigned short *)dbuffer,size);
    case  BMP24_DIB565:  return Tran24To16BitMap((unsigned char *)sbuffer,
                                                 (unsigned short *)dbuffer,size);
    case  BMP8_DIB565:   return Tran8To565BitMap((unsigned char *)sbuffer,
                                                 (unsigned short *)dbuffer,size);
    case  BMP1_DIB565:   return Tran1To565BitMap((unsigned char *)sbuffer,
                                                 (unsigned short *)dbuffer,size);
    case  BMP_SAVER_8:   return Tran565To8BitMap((unsigned short *)sbuffer,
                                                 (unsigned char *)dbuffer,size/2+1);
    case  BMP_SAVER_1:   return Tran565To1BitMap((unsigned short *)sbuffer,
                                                 (unsigned char *)dbuffer,size/2+1);
    case  BMP_SAVER_555: return Tran565BitMap555((unsigned short *)sbuffer,
                                                 (unsigned short *)dbuffer,size/2+1);
    case  BMP_SAVER_565: return size;

    case  BMP_INVERT_SAVER:return ImageInvert((unsigned char *)sbuffer,
                                              (unsigned char *)dbuffer,size);
    default: return ERROR;
    }
}
/*********************************************************************/
/*********************************************************************/
UGL_BMP_ID  * uglBitMapOpen(const char *filename)//
{
    APP_BITMAPINFO bmpHeader;
    UGL_BMP_ID  *bitMapH = (UGL_BMP_ID*)malloc(sizeof(UGL_BMP_ID));
    if(bitMapH == NULL)
        return NULL;
    bitMapH->bmpIp =NULL;

    bitMapH->bmpIp = fopen(filename,"rb+");
    if(bitMapH->bmpIp==NULL)
    {
        printf("can not open file :%s\n",filename);
        //return NULL;
        free(bitMapH);
        exit(0);
    }
    ReadBitMapHeader(bitMapH->bmpIp, &bmpHeader);

    bitMapH->bmpHeight  = bmpHeader.bmiHeader.biHeight;
    bitMapH->bmpWidth   = bmpHeader.bmiHeader.biWidth;
    bitMapH->bmpOffBits = bmpHeader.bmfHeader.bfOffBits;
    bitMapH->bmpSize    = bmpHeader.bmiHeader.biSizeImage;//bmpHeader.bmfHeader.bfSize - bmpHeader.bmfHeader.bfOffBits;
    bitMapH->bmpBitCount= bmpHeader.bmiHeader.biBitCount;
#ifdef BMP_DEBUG
    BMP_DEBUG_Print(&bmpHeader);
    BMP__Print(bitMapH);
#endif
    return  bitMapH;
}
/*********************************************************************/
/*********************************************************************/
UGL_BMP_ID   *uglBitMapCreate(const char *filename, unsigned int bmpWidth, unsigned int bmpHeight, unsigned int bmpBitCount)
{
    UGL_BMP_ID  *bitMapH = (UGL_BMP_ID*)malloc(sizeof(UGL_BMP_ID));
    if(bitMapH == NULL)
        return NULL;
    APP_BITMAPINFO bmpHeader;
    bitMapH->bmpWidth   = bmpWidth;
    bitMapH->bmpHeight  = bmpHeight;
    bitMapH->bmpOffBits = 0;         //
    bitMapH->bmpSize    = 0;
    bitMapH->bmpBitCount= bmpBitCount;        //

    if(CheckBitMapStruct(bitMapH) == ERROR)
    {
        printf("can not Create file :%s\n",filename);
        printf("file struct is wrong \n");
        free(bitMapH);
        return NULL;
    }
    bitMapH->bmpIp =NULL;

    bitMapH->bmpIp = fopen(filename,"wb+");
    if(bitMapH->bmpIp==NULL)
    {
        printf("can not Create file :%s\n",filename);
        free(bitMapH);
        exit(0);
        //return NULL;
    }
    if(bitMapH->bmpBitCount>=BMP_BITCOUNT_16)
        bitMapH->bmpOffBits = APP_BIT_MAP_SIZE;

    else if(bitMapH->bmpBitCount == BMP_BITCOUNT_8)
        bitMapH->bmpOffBits = APP_BIT_MAP_SIZE + APP_BIT_RGB_SIZE;

    else if(bitMapH->bmpBitCount == BMP_BITCOUNT_1)
        bitMapH->bmpOffBits = APP_BIT_MAP_SIZE + sizeof(APP_RGBQUAD)*2;

    WriteBitMapHeader(bitMapH->bmpIp, bitMapH, &bmpHeader);

    bitMapH->bmpWidth   = bmpHeader.bmiHeader.biWidth;
    bitMapH->bmpHeight  = bmpHeader.bmiHeader.biHeight;
    bitMapH->bmpOffBits = bmpHeader.bmfHeader.bfOffBits;         //
    bitMapH->bmpSize    = bmpHeader.bmiHeader.biSizeImage;
    bitMapH->bmpBitCount= bmpHeader.bmiHeader.biBitCount;        //
    bitMapH->bmpByteCount = bitMapH->bmpBitCount / 8;
    bitMapH->bmpData    = bitMapH->bmpData;           //

    CreateBitMapRGB(bitMapH);

#ifdef BMP_DEBUG
    BMP__Print(bitMapH);
#endif
    return  bitMapH;
}
/*********************************************************************/
/*********************************************************************/
int  uglBitMapRead(UGL_BMP_ID *bitMapH,char *buffer,int size)//
{
    fread(buffer,1,size,bitMapH->bmpIp);
    return (size);
}
/*********************************************************************/
/*********************************************************************/
int  uglBitMapWrite(UGL_BMP_ID *bitMapH,char *buffer,int size)//
{
    fwrite(buffer,1,size,bitMapH->bmpIp);
    return (size);
}
/*********************************************************************/
/*********************************************************************/
int  uglBitMapClose(UGL_BMP_ID *BitMapH)//
{
    if(BitMapH)
    {
        if(BitMapH->bmpIp)
        {
            fclose(BitMapH->bmpIp);
            BitMapH->bmpIp = NULL;
        }
        free(BitMapH);
        BitMapH = NULL;
    }
    return 0;
}
/*********************************************************************/
/*********************************************************************/
int  uglBitMapSeek(UGL_BMP_ID *BitMapH,int bmpOffBits,int type)//
{
    return fseek(BitMapH->bmpIp,bmpOffBits,type);
}
/*********************************************************************/
/*********************************************************************/
int uglBitMapReadD(UGL_BMP_ID *bitMapH,char *buffer)//
{
#ifdef BMP_DEBUG
    BMP__Print(bitMapH);
#endif
    fseek(bitMapH->bmpIp,bitMapH->bmpOffBits,SEEK_SET);
    fread(buffer,1,bitMapH->bmpSize,bitMapH->bmpIp);
    fclose(bitMapH->bmpIp);
    bitMapH->bmpIp = NULL;
    return (bitMapH->bmpSize);

}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
int uglBitMapWriteD(UGL_BMP_ID *bitMapH,char *buffer)//
{

    fseek(bitMapH->bmpIp,bitMapH->bmpOffBits,SEEK_SET);
    fwrite(buffer,1,bitMapH->bmpSize,bitMapH->bmpIp);
    fclose(bitMapH->bmpIp);
    bitMapH->bmpIp = NULL;
#ifdef BMP_DEBUG
    BMP__Print(bitMapH);
#endif
    return (bitMapH->bmpSize);
}
/**************************************************************************/
/**************************************************************************/
int uglShowBitMap(UGL_BMP_ID *bitMapH,int x,int y,char *bmp)
{
    //int size = bitMapH->bmpWidth*bitMapH->bmpHeight;
    //size <<= 1;
    //if(size<=bitMapH->bmpSize)
    //	return ERROR;
    //ShowBmp(x,y,bitMapH->bmpWidth,bitMapH->bmpHeight,(unsigned char *)bmp);
    return  OK;
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*static int Tran8To565(unsigned char *sbuffer,unsigned short *dbuffer,int lenth)
{
    unsigned int i,r,g,b;
#ifdef BMP_DEBUG
    printf("8λ�Ҷ�λͼ\n");
#endif
    for(i=0; i<lenth; i++)
    {
       r = b = GET_RB_VALUE(sbuffer[i]);
       g = GET_G_VALUE(sbuffer[i]);
       dbuffer[lenth-i]  = (unsigned short)GET_RGB(r,g,b);// rgb;
       //r = g = b = sbuffer[i];
       //dbuffer[lenth-i]  = (unsigned short)UGL_RGB(r,g,b);// rgb;
       //dbuffer[i]  = (unsigned short)UGL_RGB(r,g,b);// rgb;
    }
    return (2*i);
}
*/
#include "zpl_rtsp_rtp.h"

int test_init()
{
    UGL_BMP_ID  *bitMapH = NULL;
    bitMapH = uglBitMapOpen("D:/qt-project/live555-test/buildoutput/debug/2.bmp");
    if(bitMapH)//图片数据长度（不含这个结构头大小）
    {
        bitMapH->bmpData = (unsigned char  *)malloc(bitMapH->bmpSize);           //图片数据指针
        if(bitMapH->bmpData)
        {
            uglBitMapReadD(bitMapH,(char *)bitMapH->bmpData);
            ST_Point_T stPoint;
            stPoint.u32X=100;
            stPoint.u32Y=100;
            st_app_osd_DrawText(bitMapH->bmpData, bitMapH->bmpWidth, bitMapH->bmpHeight, stPoint, "AA", 0x0aaa);
            UGL_BMP_ID  *bitMapHaaa = uglBitMapCreate("D:/qt-project/live555-test/buildoutput/debug/3.bmp", bitMapH->bmpWidth, bitMapH->bmpHeight, bitMapH->bmpBitCount);
            if(bitMapHaaa)
            {
                uglBitMapWriteD(bitMapHaaa, bitMapH->bmpData);
                uglBitMapClose(bitMapHaaa);
                //free(bitMapHaaa);
            }
            uglBitMapClose(bitMapH);
        }
    }
    return 0;
}


/*********************************************************************/
/*********************************************************************/

/*********************************************************************/
/*********************************************************************/
#ifdef BMP_DEBUG
static int BMP__Print(UGL_BMP_ID  *bitMapH)
{
    printf("bmpWidth:        %d   \n ",bitMapH->bmpWidth);
    printf("bmpHeight:       %d   \n ",bitMapH->bmpHeight);
    printf("bmpOffBits:      0x%x \n ",bitMapH->bmpOffBits);
    printf("bmpSize:         0x%x \n ",bitMapH->bmpSize);
    printf("bmpBitCount:     %d   \n ",bitMapH->bmpBitCount);
}
static int BMP_DEBUG_Print(APP_BITMAPINFO *bmpHeader)
{
    printf("bfType:         0x%x \n ",bmpHeader->bmfHeader.bfType);
    printf("bfSize:         0x%x \n ",bmpHeader->bmfHeader.bfSize);
    printf("bfReserved1:    0x%x \n ",bmpHeader->bmfHeader.bfReserved1);
    printf("bfReserved2:    0x%x \n ",bmpHeader->bmfHeader.bfReserved2);
    printf("bfOffBits:      0x%x \n ",bmpHeader->bmfHeader.bfOffBits);
    printf("biSize:         0x%x \n ",bmpHeader->bmiHeader.biSize);
    printf("biWidth:        %d   \n ",bmpHeader->bmiHeader.biWidth);
    printf("biHeight:       %d   \n ",bmpHeader->bmiHeader.biHeight);
    printf("biPlanes:       0x%x \n ",bmpHeader->bmiHeader.biPlanes);
    printf("biBitCount:     %d   \n ",bmpHeader->bmiHeader.biBitCount);
    printf("biCompression:  0x%x \n ",bmpHeader->bmiHeader.biCompression);
    printf("biSizeImage:    0x%x \n ",bmpHeader->bmiHeader.biSizeImage);
    printf("biXPelsPerMeter:0x%x \n ",bmpHeader->bmiHeader.biXPelsPerMeter);
    printf("biYPelsPerMeter:0x%x \n ",bmpHeader->bmiHeader.biYPelsPerMeter);
    printf("biClrUsed:      0x%x \n ",bmpHeader->bmiHeader.biClrUsed);
    printf("biClrImportant: 0x%x \n ",bmpHeader->bmiHeader.biClrImportant);
    printf("rgbBlue:        0x%x \n ",bmpHeader->bmiColors.rgbBlue);
    printf("rgbGreen:       0x%x \n ",bmpHeader->bmiColors.rgbGreen);
    printf("rgbRed:         0x%x \n ",bmpHeader->bmiColors.rgbRed);
    printf("rgbReserved:    0x%x \n ",bmpHeader->bmiColors.rgbReserved);
}
#endif
