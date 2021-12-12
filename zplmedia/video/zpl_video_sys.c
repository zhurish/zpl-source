/*
 * zpl_video_sys.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_video_sys.h"



zpl_media_frame_t * zpl_video_frame_clone(zpl_media_frame_t *inframe)
{
    zpl_media_frame_t *frame = malloc(sizeof(zpl_media_frame_t));
    if(frame)
    {
        zpl_uint32 blocksize = 0;
        memset(frame, 0, sizeof(zpl_media_frame_t));
        blocksize =  zpl_video_hal_frame_blksize(inframe->framehdr);
        zpl_video_hal_memblock_init(&frame->m_memblk, blocksize);
        frame->framehdr = zpl_video_hal_frame_clone(inframe->framehdr, &frame->m_memblk);
        return frame;
    }
    return NULL;
}

int zpl_video_frame_destroy(zpl_media_frame_t *inframe)
{
    if(inframe)
    {
        zpl_video_hal_memblock_deinit(&inframe->m_memblk);
        free(inframe);
    }
    return OK;
}
