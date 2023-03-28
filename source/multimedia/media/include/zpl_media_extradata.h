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

#define NALU_PACKET_SIZE_MAX    1600

typedef enum {
    NALU_TYPE_SLICE = 1,    //非IDR图像中不采用数据划分的片段
    NALU_TYPE_DPA = 2,      //非IDR图像中A类数据划分片段
    NALU_TYPE_DPB = 3,      //非IDR图像中B类数据划分片段
    NALU_TYPE_DPC = 4,      //非IDR图像中C类数据划分片段
    NALU_TYPE_IDR = 5,      //IDR图像的片段
    NALU_TYPE_SEI = 6,      //补充增强信息（SEI）
    NALU_TYPE_SPS = 7,      //序列参数集（SPS）
    NALU_TYPE_PPS = 8,      //图像参数集（PPS）
    NALU_TYPE_AUD = 9,      //分割符
    NALU_TYPE_EOSEQ = 10,   //序列结束符
    NALU_TYPE_EOSTREAM = 11,//流结束符
    NALU_TYPE_FILL = 12,    //填充数据
    //NALU_TYPE_FILL = 13, //序列参数集扩展
    //NALU_TYPE_FILL = 14, //带前缀的NAL单元
    //NALU_TYPE_FILL = 15, //子序列参数集
    //NALU_TYPE_FILL = 16 – 18 保留
    //NALU_TYPE_FILL = 19, //不采用数据划分的辅助编码图像片段
    //NALU_TYPE_FILL = 20, //编码片段扩展
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


typedef struct {
    zpl_uint32 		    profileLevelId;
    zpl_video_size_t    vidsize;
    zpl_uint8           profile;
    zpl_uint8           level;
    zpl_uint8           fps;
} __attribute__ ((packed)) h264_sps_extradata_t;

#define ZPL_VIDEO_EXTRADATA_MAXSIZE 3600

typedef struct 
{
#ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
    zpl_uint8 			fSEI[ZPL_VIDEO_EXTRADATA_MAXSIZE];
    zpl_uint32 			fSEISize;
    zpl_uint32 			fSEIHdrLen;
    zpl_uint8 			fVPS[ZPL_VIDEO_EXTRADATA_MAXSIZE];
    zpl_uint32 			fVPSSize;
    zpl_uint32 			fVPSHdrLen;
    zpl_uint8 			fSPS[ZPL_VIDEO_EXTRADATA_MAXSIZE];
    zpl_uint32 			fSPSSize;
    zpl_uint32 			fSPSHdrLen;
    zpl_uint8 			fPPS[ZPL_VIDEO_EXTRADATA_MAXSIZE];
    zpl_uint32 			fPPSSize;
    zpl_uint32 			fPPSHdrLen;
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

    h264_sps_extradata_t h264spspps;

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

extern int zpl_media_channel_decode_sps(const zpl_uint8 *buf, int len,  h264_sps_extradata_t *);



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_EXTRADATA_H__ */