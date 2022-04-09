/*
 * zpl_vidhal_test.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"


#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"

/******************************************************************************
* funciton : save stream
******************************************************************************/
static FILE* pTestFd = NULL; 

int vidhal_venc_save_stream_test(VENC_STREAM_S* pstStream, zpl_uint32 len, zpl_video_size_t vidsize)
{
    HI_S32 i;
    if(pTestFd == NULL)
    {
        pTestFd = fopen("./venc-test.h264", "wb");
    }
    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        fwrite(pstStream->pstPack[i].pu8Addr + pstStream->pstPack[i].u32Offset,
               pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset, 1, pTestFd);

        fflush(pTestFd);
    }
    return HI_SUCCESS;
}

int vidhal_venc_stop_test()
{
    if(pTestFd)
    {
        fclose(pTestFd);
        pTestFd = NULL;
    }
    return 0;
}

#endif


