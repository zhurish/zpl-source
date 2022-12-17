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
    uint8_t hdr_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    uint8_t forbidden_bit;            //! should be always FALSE
    uint8_t nal_idc;        //! NALU_PRIORITY_xxxx
    uint8_t nal_unit_type;            //! NALU_TYPE_xxxx                  //! contains the first byte followed by the EBSP
    uint32_t len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    uint8_t *buf;
} H264_NALU_T;


typedef struct mpeg_rational_s {
    int num;
    int den;
} zpl_mpeg_rational_t;
 

typedef struct {
    zpl_video_size_t vidsize;
    zpl_mpeg_rational_t pixel_aspect;
    uint8_t   profile;
    uint8_t   level;
} zpl_h264_sps_data_t;


typedef struct 
{
    uint8_t* 			fSEI;
    unsigned 			fSEISize;
    uint8_t* 			fVPS;
    unsigned 			fVPSSize;
    uint8_t* 			fSPS;
    unsigned 			fSPSSize;
    uint8_t* 			fPPS;
    unsigned 			fPPSSize;
    uint32_t 			profileLevelId;

    zpl_h264_sps_data_t h264spsdata;

}zpl_video_extradata_t __attribute__ ((aligned (4)));

extern bool is_nalu3_start(zpl_uint8 *buffer);
extern bool is_nalu4_start(zpl_uint8 *buffer);
extern bool zpl_media_channel_isnaluhdr(uint8_t *bufdata, H264_NALU_T *nalu);
extern int zpl_media_channel_nalu_show(H264_NALU_T *nalu);
extern int zpl_media_channel_get_nextnalu(uint8_t *bufdata, uint32_t len);


extern unsigned zpl_media_channel_get_profileLevelId(uint8_t const* from, unsigned fromSize, uint8_t* to, unsigned toMaxSize);

extern int zpl_media_channel_nalu2extradata(H264_NALU_T *nalu, zpl_video_extradata_t *extradata);

extern int zpl_media_channel_extradata_import(void *chn, zpl_uint8 *Buf, int len);

extern int zpl_media_channel_extradata_get(void *chn, zpl_video_extradata_t *extradata);
extern int zpl_media_channel_extradata_delete(zpl_video_extradata_t *extradata);

extern int zpl_media_channel_decode_spspps(uint8_t *bufdata, uint32_t nLen,int *width,int *height,int *fps);

extern int zpl_media_channel_decode_sps(const uint8_t *buf, int len, zpl_h264_sps_data_t *sps);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_EXTRADATA_H__ */