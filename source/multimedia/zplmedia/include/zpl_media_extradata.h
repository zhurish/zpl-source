/*
 * zpl_media_extradata.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_EXTRADATA_H__
#define __ZPL_MEDIA_EXTRADATA_H__


#ifdef __cplusplus
extern "C" {
#endif
#include <zpl_type.h>
#include <zpl_media.h>


typedef enum {
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12,
} H264_NaluType;

typedef enum {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIORITY_LOW        = 1,
    NALU_PRIORITY_HIGH       = 2,
    NALU_PRIORITY_HIGHEST    = 3
} H264_NaluPriority;

typedef struct
{
    zpl_uint8 hdr_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    zpl_uint8 forbidden_bit;            //! should be always FALSE
    zpl_uint8 nal_idc;        //! NALU_PRIORITY_xxxx
    zpl_uint8 nal_unit_type;            //! NALU_TYPE_xxxx                  //! contains the first byte followed by the EBSP
    zpl_uint32 len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    zpl_uint8 *buf;
}__attribute__ ((packed)) H264_NALU_T ;


typedef struct mpeg_rational_s {
    int num;
    int den;
} zpl_mpeg_rational_t;
 

typedef struct {
    zpl_video_size_t vidsize;
    zpl_mpeg_rational_t pixel_aspect;
    zpl_uint8   profile;
    zpl_uint8   level;
} __attribute__ ((packed)) zpl_h264_sps_data_t;

#define ZPL_VIDEO_EXTRADATA_MAXSIZE 1024
typedef struct 
{
#ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
    zpl_uint8 			fSEI[ZPL_VIDEO_EXTRADATA_MAXSIZE];
    zpl_uint32 			fSEISize;
    zpl_uint8 			fVPS[ZPL_VIDEO_EXTRADATA_MAXSIZE];
    zpl_uint32 			fVPSSize;
    zpl_uint8 			fSPS[ZPL_VIDEO_EXTRADATA_MAXSIZE];
    zpl_uint32 			fSPSSize;
    zpl_uint8 			fPPS[ZPL_VIDEO_EXTRADATA_MAXSIZE];
    zpl_uint32 			fPPSSize;
#else    
    zpl_uint8* 			fSEI;
    zpl_uint32 			fSEISize;
    zpl_uint8* 			fVPS;
    zpl_uint32 			fVPSSize;
    zpl_uint8* 			fSPS;
    zpl_uint32 			fSPSSize;
    zpl_uint8* 			fPPS;
    zpl_uint32 			fPPSSize;
 #endif   
    zpl_uint32 			profileLevelId;

    zpl_h264_sps_data_t h264spsdata;

}zpl_video_extradata_t ;

extern bool is_nalu3_start(zpl_uint8 *buffer);
extern bool is_nalu4_start(zpl_uint8 *buffer);
extern bool zpl_media_channel_isnaluhdr(zpl_uint8 *bufdata, H264_NALU_T *nalu);
extern int zpl_media_channel_nalu_show(H264_NALU_T *nalu);
extern int zpl_media_channel_get_nextnalu(zpl_uint8 *bufdata, zpl_uint32 len);


extern zpl_uint32 zpl_media_channel_get_profileLevelId(zpl_uint8 const* from, zpl_uint32 fromSize, zpl_uint8* to, zpl_uint32 toMaxSize);

extern int zpl_media_channel_nalu2extradata(H264_NALU_T *nalu, zpl_video_extradata_t *extradata);

extern int zpl_media_channel_extradata_import(void *chn, zpl_uint8 *Buf, int len);

extern int zpl_media_channel_extradata_get(void *chn, zpl_video_extradata_t *extradata);
extern int zpl_media_channel_extradata_delete(zpl_video_extradata_t *extradata);

extern int zpl_media_channel_decode_spspps(zpl_uint8 *bufdata, zpl_uint32 nLen,int *width,int *height,int *fps);

extern int zpl_media_channel_decode_sps(const zpl_uint8 *buf, int len, int *width,int *height,int *fps);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_EXTRADATA_H__ */