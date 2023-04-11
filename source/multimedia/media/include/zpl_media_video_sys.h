/*
 * zpl_video_sys.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_SYS_H__
#define __ZPL_VIDEO_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MEMBLOCK_INVALID_POOLID              (-1U)
#define MEMBLOCK_INVALID_HANDLE              (-1U)

typedef struct
{
    zpl_uint32 hBlock;
    zpl_uint32 hPool;

    zpl_uint64  uPhyAddr;
    zpl_uint8*  pVirAddr;
    zpl_uint32  uBlkSize;
} zpl_media_memblock_t;

/* Definition of the IVE_IMAGE_S. Added by Tan Bing, 2013-7-22. */
typedef struct  {
    zpl_uint64 au64PhyAddr[3];   /* RW;The physical address of the image */
    zpl_uint64 au64VirAddr[3];   /* RW;The virtual address of the image */
    zpl_uint32 au32Stride[3];    /* RW;The stride of the image */
    zpl_uint32 u32Width;         /* RW;The width of the image */
    zpl_uint32 u32Height;        /* RW;The height of the image */
    zpl_uint32 enType; /* RW;The type of the image */
    zpl_media_memblock_t    m_memblk;
} zpl_media_sys_image_t;

typedef struct  {
    void              *framehdr;
    zpl_media_memblock_t    m_memblk;
} zpl_media_video_frame_t;
 
zpl_media_video_frame_t * zpl_media_video_frame_clone(zpl_media_video_frame_t *inframe);
int zpl_media_video_frame_destroy(zpl_media_video_frame_t *inframe);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_SYS_H__ */
