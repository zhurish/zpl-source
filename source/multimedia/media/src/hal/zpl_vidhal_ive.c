#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"


#ifdef ZPL_HISIMPP_MODULE



//free mmz
#define IVE_MMZ_FREE(phy,vir)\
    do{\
        if ((0 != (phy)) && (0 != (vir)))\
        {\
            HI_MPI_SYS_MmzFree((phy),(zpl_void *)(HI_UL)(vir));\
            (phy) = 0;\
            (vir) = 0;\
        }\
    }while(0)

#define IVE_CLOSE_FILE(fp)\
    do{\
        if (NULL != (fp))\
        {\
            fclose((fp));\
            (fp) = NULL;\
        }\
    }while(0)


//#define ZPL_VIDHAL_IVE_CONVERT_64BIT_ADDR(Type,Addr) (Type*)(HI_UL)(Addr)



zpl_uint16 zpl_vidhal_ive_CalcStride(zpl_uint32 u32Width, zpl_uint8 u8Align)
{
    return (u32Width + (u8Align - u32Width % u8Align) % u8Align);
}

int zpl_vidhal_vgs_fillrect(VIDEO_FRAME_INFO_S* pstFrmInfo, ZPL_VIDHAL_RECT_ARRAY_S* pstRect, zpl_uint32 u32Color)
{
    VGS_HANDLE VgsHandle = -1;
    zpl_int32 s32Ret = HI_SUCCESS;
    zpl_uint16 i;
    VGS_TASK_ATTR_S stVgsTask;
    VGS_ADD_COVER_S stVgsAddCover;

    if (0 == pstRect->u16Num)
    {
        return s32Ret;
    }
    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("Vgs begin job fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    memcpy(&stVgsTask.stImgIn, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&stVgsTask.stImgOut, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));

    stVgsAddCover.enCoverType = COVER_QUAD_RANGLE;
    stVgsAddCover.u32Color = u32Color;
    for (i = 0; i < pstRect->u16Num; i++)
    {
        stVgsAddCover.stQuadRangle.bSolid = HI_FALSE;
        stVgsAddCover.stQuadRangle.u32Thick = 2;
        memcpy(stVgsAddCover.stQuadRangle.stPoint, pstRect->astRect[i].astPoint, sizeof(pstRect->astRect[i].astPoint));
        s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, &stVgsAddCover);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n", s32Ret);
            HI_MPI_VGS_CancelJob(VgsHandle);
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("HI_MPI_VGS_EndJob fail,Error(%#x)\n", s32Ret);
        HI_MPI_VGS_CancelJob(VgsHandle);
        return s32Ret;
    }

    return s32Ret;

}

int zpl_vidhal_ive_readfile(IVE_IMAGE_S* pstImg, FILE* pFp)
{
    zpl_uint16 y;
    zpl_uint8* pU8;
    zpl_uint16 height;
    zpl_uint16 width;
    zpl_uint16 loop;
    zpl_int32 s32Ret;

    (zpl_void)fgetc(pFp);
    if (feof(pFp))
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("end of file!\n");
        s32Ret = fseek(pFp, 0 , SEEK_SET );
        if (0 != s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("fseek failed!\n");
            return s32Ret;
        }

    }
    else
    {
        s32Ret = fseek(pFp, -1 , SEEK_CUR );
        if (0 != s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("fseek failed!\n");
            return s32Ret;
        }
    }

    //if (feof(pFp))
    //{
    //    zm_msg_err("end of file!\n");
    //    fseek(pFp, 0 , SEEK_SET);
    //}

    height = pstImg->u32Height;
    width = pstImg->u32Width;

    switch (pstImg->enType)
    {
        case  IVE_IMAGE_TYPE_U8C1:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(pU8, width, 1, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Read file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0];
            }
        }
        break;
        case  IVE_IMAGE_TYPE_YUV420SP:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(pU8, width, 1, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Read file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0];
            }

            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[1];
            for (y = 0; y < height / 2; y++)
            {
                if ( 1 != fread(pU8, width, 1, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Read file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[1];
            }
        }
        break;
        case IVE_IMAGE_TYPE_YUV422SP:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(pU8, width, 1, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Read file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0];
            }

            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[1];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(pU8, width, 1, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Read file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[1];
            }
        }
        break;
        case IVE_IMAGE_TYPE_U8C3_PACKAGE:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(pU8, width * 3, 1, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Read file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0] * 3;
            }

        }
        break;
        case IVE_IMAGE_TYPE_U8C3_PLANAR:
        {
            for (loop = 0; loop < 3; loop++)
            {
                pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[loop];
                for (y = 0; y < height; y++)
                {
                    if ( 1 != fread(pU8, width, 1, pFp))
                    {
                        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                            zm_msg_err("Read file fail\n");
                        return HI_FAILURE;
                    }

                    pU8 += pstImg->au32Stride[loop];
                }
            }

        }
        break;
        case IVE_IMAGE_TYPE_S16C1:
        case IVE_IMAGE_TYPE_U16C1:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for ( y = 0; y < height; y++ )
            {
                if ( sizeof(zpl_uint16) != fread(pU8, width, sizeof(zpl_uint16), pFp) )
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Read file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0] * 2;
            }
        }
        break;
        default:
            break;
    }

    return HI_SUCCESS;
}

int zpl_vidhal_ive_writefile(IVE_IMAGE_S* pstImg, FILE* pFp)
{
    zpl_uint16 y;
    zpl_uint8* pU8;
    zpl_uint16 height;
    zpl_uint16 width;

    height = pstImg->u32Height;
    width = pstImg->u32Width;

    switch (pstImg->enType)
    {
        case  IVE_IMAGE_TYPE_U8C1:
        case  IVE_IMAGE_TYPE_S8C1:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fwrite(pU8, width, 1, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Write file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0];
            }
        }
        break;
        case  IVE_IMAGE_TYPE_YUV420SP:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for (y = 0; y < height; y++)
            {
                if ( width != fwrite(pU8, 1, width, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Write file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0];
            }

            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[1];
            for (y = 0; y < height / 2; y++)
            {
                if ( width != fwrite(pU8, 1, width, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Write file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[1];
            }
        }
        break;
        case IVE_IMAGE_TYPE_YUV422SP:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for (y = 0; y < height; y++)
            {
                if ( width != fwrite(pU8, 1, width, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Write file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0];
            }

            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[1];
            for (y = 0; y < height; y++)
            {
                if ( width != fwrite(pU8, 1, width, pFp))
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Write file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[1];
            }
        }
        break;
        case IVE_IMAGE_TYPE_S16C1:
        case  IVE_IMAGE_TYPE_U16C1:
        {
            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for ( y = 0; y < height; y++ )
            {
                if ( sizeof(zpl_uint16) != fwrite(pU8, width, sizeof(zpl_uint16), pFp) )
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Write file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0] * 2;
            }
        }
        break;
        case IVE_IMAGE_TYPE_U32C1:
        {

            pU8 = (zpl_uint8 *)(HI_UL)pstImg->au64VirAddr[0];
            for ( y = 0; y < height; y++ )
            {
                if ( width != fwrite(pU8, sizeof(zpl_uint32), width, pFp) )
                {
                    if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("Write file fail\n");
                    return HI_FAILURE;
                }

                pU8 += pstImg->au32Stride[0] * 4;
            }
            break;
        }

        default:
            break;
    }

    return HI_SUCCESS;
}

zpl_void zpl_vidhal_ive_blob2rect(IVE_CCBLOB_S *pstBlob, ZPL_VIDHAL_RECT_ARRAY_S *pstRect,
                                            zpl_uint16 u16RectMaxNum,zpl_uint16 u16AreaThrStep,
                                            zpl_uint32 u32SrcWidth, zpl_uint32 u32SrcHeight,
                                            zpl_uint32 u32DstWidth,zpl_uint32 u32DstHeight)
{
    zpl_uint16 u16Num;
    zpl_uint16 i,j,k;
    zpl_uint16 u16Thr= 0;
    zpl_bool bValid;

    if(pstBlob->u8RegionNum > u16RectMaxNum)
    {
        u16Thr = pstBlob->u16CurAreaThr;
        do
        {
            u16Num = 0;
            u16Thr += u16AreaThrStep;
            for(i = 0;i < 254;i++)
            {
                if(pstBlob->astRegion[i].u32Area > u16Thr)
                {
                    u16Num++;
                }
            }
        }while(u16Num > u16RectMaxNum);
    }

   u16Num = 0;

   for(i = 0;i < 254;i++)
    {
        if(pstBlob->astRegion[i].u32Area > u16Thr)
        {
            pstRect->astRect[u16Num].astPoint[0].s32X = (zpl_uint32)((HI_FLOAT)pstBlob->astRegion[i].u16Left / (HI_FLOAT)u32SrcWidth * (HI_FLOAT)u32DstWidth) & (~1) ;
            pstRect->astRect[u16Num].astPoint[0].s32Y = (zpl_uint32)((HI_FLOAT)pstBlob->astRegion[i].u16Top / (HI_FLOAT)u32SrcHeight * (HI_FLOAT)u32DstHeight) & (~1);

            pstRect->astRect[u16Num].astPoint[1].s32X = (zpl_uint32)((HI_FLOAT)pstBlob->astRegion[i].u16Right/ (HI_FLOAT)u32SrcWidth * (HI_FLOAT)u32DstWidth) & (~1);
            pstRect->astRect[u16Num].astPoint[1].s32Y = (zpl_uint32)((HI_FLOAT)pstBlob->astRegion[i].u16Top / (HI_FLOAT)u32SrcHeight * (HI_FLOAT)u32DstHeight) & (~1);

            pstRect->astRect[u16Num].astPoint[2].s32X = (zpl_uint32)((HI_FLOAT)pstBlob->astRegion[i].u16Right / (HI_FLOAT)u32SrcWidth * (HI_FLOAT)u32DstWidth) & (~1);
            pstRect->astRect[u16Num].astPoint[2].s32Y = (zpl_uint32)((HI_FLOAT)pstBlob->astRegion[i].u16Bottom / (HI_FLOAT)u32SrcHeight * (HI_FLOAT)u32DstHeight) & (~1);

            pstRect->astRect[u16Num].astPoint[3].s32X = (zpl_uint32)((HI_FLOAT)pstBlob->astRegion[i].u16Left / (HI_FLOAT)u32SrcWidth * (HI_FLOAT)u32DstWidth) & (~1);
            pstRect->astRect[u16Num].astPoint[3].s32Y = (zpl_uint32)((HI_FLOAT)pstBlob->astRegion[i].u16Bottom / (HI_FLOAT)u32SrcHeight * (HI_FLOAT)u32DstHeight) & (~1);

            bValid = HI_TRUE;
            for(j = 0; j < 3;j++)
            {
                for (k = j + 1; k < 4;k++)
                {
                    if ((pstRect->astRect[u16Num].astPoint[j].s32X == pstRect->astRect[u16Num].astPoint[k].s32X)
                         &&(pstRect->astRect[u16Num].astPoint[j].s32Y == pstRect->astRect[u16Num].astPoint[k].s32Y))
                    {
                    bValid = HI_FALSE;
                    break;
                    }
                }
            }
            if (HI_TRUE == bValid)
            {
                u16Num++;
            }
        }
    }

    pstRect->u16Num = u16Num;
}

/******************************************************************************
* function : Create ive image
******************************************************************************/
int zpl_vidhal_ive_create_image(IVE_IMAGE_S* pstImg, IVE_IMAGE_TYPE_E enType, zpl_uint32 u32Width, zpl_uint32 u32Height)
{
    zpl_uint32 u32Size = 0;
    zpl_int32 s32Ret;
    if (NULL == pstImg)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("pstImg is null\n");
        return HI_FAILURE;
    }

    pstImg->enType = enType;
    pstImg->u32Width = u32Width;
    pstImg->u32Height = u32Height;
    pstImg->au32Stride[0] = zpl_vidhal_ive_CalcStride(pstImg->u32Width, IVE_ALIGN);

    switch (enType)
    {
        case IVE_IMAGE_TYPE_U8C1:
        case IVE_IMAGE_TYPE_S8C1:
        {
            u32Size = pstImg->au32Stride[0] * pstImg->u32Height;
            s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
        }
        break;
        case IVE_IMAGE_TYPE_YUV420SP:
        {
            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 3 / 2;
            s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
            pstImg->au32Stride[1] = pstImg->au32Stride[0];
            pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
            pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;

        }
        break;
        case IVE_IMAGE_TYPE_YUV422SP:
        {
            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 2;
            s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
            pstImg->au32Stride[1] = pstImg->au32Stride[0];
            pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
            pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;

        }
        break;
        case IVE_IMAGE_TYPE_YUV420P:
            break;
        case IVE_IMAGE_TYPE_YUV422P:
            break;
        case IVE_IMAGE_TYPE_S8C2_PACKAGE:
            break;
        case IVE_IMAGE_TYPE_S8C2_PLANAR:
            break;
        case IVE_IMAGE_TYPE_S16C1:
        case IVE_IMAGE_TYPE_U16C1:
        {

            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint16);
            s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
        }
        break;
        case IVE_IMAGE_TYPE_U8C3_PACKAGE:
        {
            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 3;
            s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
            pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + 1;
            pstImg->au64VirAddr[2] = pstImg->au64VirAddr[1] + 1;
            pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + 1;
            pstImg->au64PhyAddr[2] = pstImg->au64PhyAddr[1] + 1;
            pstImg->au32Stride[1] = pstImg->au32Stride[0];
            pstImg->au32Stride[2] = pstImg->au32Stride[0];
        }
        break;
        case IVE_IMAGE_TYPE_U8C3_PLANAR:
            break;
        case IVE_IMAGE_TYPE_S32C1:
        case IVE_IMAGE_TYPE_U32C1:
        {
            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint32);
            s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
        }
        break;
        case IVE_IMAGE_TYPE_S64C1:
        case IVE_IMAGE_TYPE_U64C1:
        {

            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint64);
            s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
        }
        break;
        default:
            break;

    }

    return HI_SUCCESS;
}
/******************************************************************************
* function : Create memory info
******************************************************************************/
int zpl_vidhal_ive_create_meminfo(IVE_MEM_INFO_S* pstMemInfo, zpl_uint32 u32Size)
{
    zpl_int32 s32Ret;

    if (NULL == pstMemInfo)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("pstMemInfo is null\n");
        return HI_FAILURE;
    }
    pstMemInfo->u32Size = u32Size;
    s32Ret = HI_MPI_SYS_MmzAlloc(&pstMemInfo->u64PhyAddr, (zpl_void**)&pstMemInfo->u64VirAddr, NULL, HI_NULL, u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
/******************************************************************************
* function : Create ive image by cached
******************************************************************************/
int zpl_vidhal_ive_create_image_cached(IVE_IMAGE_S* pstImg,
        IVE_IMAGE_TYPE_E enType, zpl_uint32 u32Width, zpl_uint32 u32Height)
{
    zpl_uint32 u32Size = 0;
    zpl_int32 s32Ret;
    if (NULL == pstImg)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("pstImg is null\n");
        return HI_FAILURE;
    }

    pstImg->enType = enType;
    pstImg->u32Width = u32Width;
    pstImg->u32Height = u32Height;
    pstImg->au32Stride[0] = zpl_vidhal_ive_CalcStride(pstImg->u32Width, IVE_ALIGN);

    switch (enType)
    {
        case IVE_IMAGE_TYPE_U8C1:
        case IVE_IMAGE_TYPE_S8C1:
        {
            u32Size = pstImg->au32Stride[0] * pstImg->u32Height;
            s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
        }
        break;
        case IVE_IMAGE_TYPE_YUV420SP:
            break;
        case IVE_IMAGE_TYPE_YUV422SP:
            break;
        case IVE_IMAGE_TYPE_YUV420P:
            break;
        case IVE_IMAGE_TYPE_YUV422P:
            break;
        case IVE_IMAGE_TYPE_S8C2_PACKAGE:
            break;
        case IVE_IMAGE_TYPE_S8C2_PLANAR:
            break;
        case IVE_IMAGE_TYPE_S16C1:
        case IVE_IMAGE_TYPE_U16C1:
        {

            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint16);
            s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
        }
        break;
        case IVE_IMAGE_TYPE_U8C3_PACKAGE:
            break;
        case IVE_IMAGE_TYPE_U8C3_PLANAR:
            break;
        case IVE_IMAGE_TYPE_S32C1:
        case IVE_IMAGE_TYPE_U32C1:
        {
            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint32);
            s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
        }
        break;
        case IVE_IMAGE_TYPE_S64C1:
        case IVE_IMAGE_TYPE_U64C1:
        {

            u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint64);
            s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstImg->au64PhyAddr[0], (zpl_void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
                return s32Ret;
            }
        }
        break;
        default:
            break;
    }

    return HI_SUCCESS;
}

int zpl_vidhal_ive_create_data(IVE_DATA_S* pstData,zpl_uint32 u32Width, zpl_uint32 u32Height)
{
    zpl_int32 s32Ret;
    zpl_uint32 u32Size;

    if (NULL == pstData)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("pstData is null\n");
        return HI_FAILURE;
    }
    pstData->u32Width = u32Width;
    pstData->u32Height = u32Height;
    pstData->u32Stride = zpl_vidhal_ive_CalcStride(pstData->u32Width, IVE_ALIGN);
    u32Size = pstData->u32Stride * pstData->u32Height;
    s32Ret = HI_MPI_SYS_MmzAlloc(&pstData->u64PhyAddr, (zpl_void**)&pstData->u64VirAddr, NULL, HI_NULL, u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("Mmz Alloc fail,Error(%#x)\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : Dma frame info to  ive image
******************************************************************************/
int zpl_vidhal_ive_dma_image(VIDEO_FRAME_INFO_S *pstFrameInfo,IVE_DST_IMAGE_S *pstDst,zpl_bool bInstant)
{
    zpl_int32 s32Ret;
    IVE_HANDLE hIveHandle;
    IVE_SRC_DATA_S stSrcData;
    IVE_DST_DATA_S stDstData;
    IVE_DMA_CTRL_S stCtrl = {IVE_DMA_MODE_DIRECT_COPY,0};
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;

    //fill src
    //stSrcData.u64VirAddr = pstFrameInfo->stVFrame.u64VirAddr[0];
    stSrcData.u64PhyAddr = pstFrameInfo->stVFrame.u64PhyAddr[0];
    stSrcData.u32Width   = pstFrameInfo->stVFrame.u32Width;
    stSrcData.u32Height  = pstFrameInfo->stVFrame.u32Height;
    stSrcData.u32Stride  = pstFrameInfo->stVFrame.u32Stride[0];

    //fill dst
    //stDstData.u64VirAddr = pstDst->au64VirAddr[0];
    stDstData.u64PhyAddr = pstDst->au64PhyAddr[0];
    stDstData.u32Width   = pstDst->u32Width;
    stDstData.u32Height  = pstDst->u32Height;
    stDstData.u32Stride  = pstDst->au32Stride[0];

    s32Ret = HI_MPI_IVE_DMA(&hIveHandle,&stSrcData,&stDstData,&stCtrl,bInstant);
    if(HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("HI_MPI_IVE_DMA Error(%#x)\n", s32Ret);
    }

    if (HI_TRUE == bInstant)
    {
        s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);
        while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);
        }
        if(HI_SUCCESS != s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("HI_MPI_IVE_Query Error(%#x)\n", s32Ret);
        }
    }
    return HI_SUCCESS;
}

int zpl_vidhal_ive_dma_image2(IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst, zpl_bool bInstant)
{
    zpl_int32 s32Ret;
    IVE_HANDLE hIveHandle;
    IVE_SRC_DATA_S stSrcData;
    IVE_DST_DATA_S stDstData;
    IVE_DMA_CTRL_S stCtrl = {IVE_DMA_MODE_DIRECT_COPY,0};
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;

    //fill dst
    //stDstData.u64VirAddr = pstFrameInfo->stVFrame.u64VirAddr[0];
    stDstData.u64PhyAddr = pstDst->au64PhyAddr[0];
    stDstData.u32Width   = pstDst->u32Width;
    stDstData.u32Height  = pstDst->u32Height;
    stDstData.u32Stride  = pstDst->au32Stride[0];

    //fill src 
    //stSrcData.u64VirAddr = pstDst->au64VirAddr[0];
    stSrcData.u64PhyAddr = pstSrc->au64PhyAddr[0];
    stSrcData.u32Width   = pstSrc->u32Width;
    stSrcData.u32Height  = pstSrc->u32Height;
    stSrcData.u32Stride  = pstSrc->au32Stride[0];

    s32Ret = HI_MPI_IVE_DMA(&hIveHandle,&stSrcData,&stDstData,&stCtrl,bInstant);

    if(HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("HI_MPI_IVE_DMA Error(%#x)\n", s32Ret);
    }
    if (HI_TRUE == bInstant)
    {
        s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);
        while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);
        }
        if(HI_SUCCESS != s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("HI_MPI_IVE_DMA Error(%#x)\n", s32Ret);
        }
    }
    return HI_SUCCESS;
}

int zpl_vidhal_ive_dma_image2frame(IVE_DST_IMAGE_S *pstSrc, VIDEO_FRAME_INFO_S *pstFrameInfo, zpl_bool bInstant)
{
    zpl_int32 s32Ret;
    IVE_HANDLE hIveHandle;
    IVE_SRC_DATA_S stSrcData;
    IVE_DST_DATA_S stDstData;
    IVE_DMA_CTRL_S stCtrl = {IVE_DMA_MODE_DIRECT_COPY,0};
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;

    //fill dst
    //stDstData.u64VirAddr = pstFrameInfo->stVFrame.u64VirAddr[0];
    stDstData.u64PhyAddr = pstFrameInfo->stVFrame.u64PhyAddr[0];
    stDstData.u32Width   = pstFrameInfo->stVFrame.u32Width;
    stDstData.u32Height  = pstFrameInfo->stVFrame.u32Height;
    stDstData.u32Stride  = pstFrameInfo->stVFrame.u32Stride[0];

    //fill src 
    //stSrcData.u64VirAddr = pstDst->au64VirAddr[0];
    stSrcData.u64PhyAddr = pstSrc->au64PhyAddr[0];
    stSrcData.u32Width   = pstSrc->u32Width;
    stSrcData.u32Height  = pstSrc->u32Height;
    stSrcData.u32Stride  = pstSrc->au32Stride[0];

    s32Ret = HI_MPI_IVE_DMA(&hIveHandle,&stSrcData,&stDstData,&stCtrl,bInstant);

    if(HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("HI_MPI_IVE_DMA Error(%#x)\n", s32Ret);
    }
    if (HI_TRUE == bInstant)
    {
        s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);
        while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);
        }
        if(HI_SUCCESS != s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("HI_MPI_IVE_Query Error(%#x)\n", s32Ret);
        }
    }
    return HI_SUCCESS;
}

int zpl_vidhal_ive_dma_frame(VIDEO_FRAME_INFO_S *SrcFrmInfo, VIDEO_FRAME_INFO_S *DstFrmInfo)
{
    HI_S32 s32Ret       = -1;
    HI_BOOL bInstant = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;
    
    IVE_HANDLE hIveHandle;
    IVE_SRC_DATA_S  stSrcData;
    IVE_DST_DATA_S  stDstData;
    IVE_DMA_CTRL_S  stCtrl = {IVE_DMA_MODE_DIRECT_COPY, 0};
    //src
    stSrcData.u64VirAddr = SrcFrmInfo->stVFrame.u64VirAddr[0];
	stSrcData.u64PhyAddr = SrcFrmInfo->stVFrame.u64PhyAddr[0];
	stSrcData.u32Width   = (HI_U32)SrcFrmInfo->stVFrame.u32Width;
	stSrcData.u32Height  = (HI_U32)SrcFrmInfo->stVFrame.u32Height;
	stSrcData.u32Stride  = (HI_U32)SrcFrmInfo->stVFrame.u32Stride[0];

	//dst
	stDstData.u64VirAddr = DstFrmInfo->stVFrame.u64VirAddr[0];
	stDstData.u64PhyAddr = DstFrmInfo->stVFrame.u64PhyAddr[0];
	stDstData.u32Width   = stSrcData.u32Width;
	stDstData.u32Height  = stSrcData.u32Height;
	stDstData.u32Stride  = (HI_U32)(DstFrmInfo->stVFrame.u32Stride[0]);

	s32Ret = HI_MPI_IVE_DMA(&hIveHandle, &stSrcData, &stDstData, &stCtrl, bInstant);	
	if (HI_SUCCESS != s32Ret)
	{
       if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("HI_MPI_IVE_DMA fail, Error(%#x)\n",s32Ret);
       return s32Ret;
    }
    s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock); 
    while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
    {
        usleep(100);                    
        s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);   
    }
	if (HI_SUCCESS != s32Ret)
	{
       if(ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("HI_MPI_IVE_Query fail, Error(%#x)\n",s32Ret);
       return s32Ret;
    }

    return HI_SUCCESS;
}



#if 0
static HI_S32 ds_dma_mode_copy(IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst,
                               HI_BOOL bInstant, IVE_DMA_CTRL_S ctrl)
{
    HI_S32 s32Ret;
    IVE_HANDLE hIveHandle;
    IVE_SRC_DATA_S stSrcData;
    IVE_DST_DATA_S stDstData;
    IVE_DMA_CTRL_S stCtrl = {IVE_DMA_MODE_DIRECT_COPY, 0};	// 直接快速拷贝模式
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;
    stCtrl = ctrl;
#if defined HI_MPP3
    //fill src
    //stSrcData.pu8VirAddr = pstSrc->pu8VirAddr[0];
    stSrcData.u32PhyAddr = pstSrc->u32PhyAddr[0];
    stSrcData.u16Width   = pstSrc->u16Width;
    stSrcData.u16Height  = pstSrc->u16Height;
    stSrcData.u16Stride  = pstSrc->u16Stride[0];

    //fill dst
    //stDstData.pu8VirAddr = pstDst->pu8VirAddr[0];
    if( pstDst )
    {
        stDstData.u32PhyAddr = pstDst->u32PhyAddr[0];
        stDstData.u16Width   = pstDst->u16Width;
        stDstData.u16Height  = pstDst->u16Height;
        stDstData.u16Stride  = pstDst->u16Stride[0];
    }

    stSrcData.u16Width += stSrcData.u16Width % 2;
    stDstData.u16Width += stDstData.u16Width % 2;
    stSrcData.u16Height += stSrcData.u16Height % 2;
    stDstData.u16Height += stDstData.u16Height % 2;
#elif defined HI_MPP4
    //fill src
    //stSrcData.u64VirAddr = pstSrc->au64VirAddr[0];


    stSrcData.u64PhyAddr = pstSrc->au64PhyAddr[0];
    stSrcData.u32Width   = pstSrc->u32Width;
    stSrcData.u32Height  = pstSrc->u32Height;
    stSrcData.u32Stride  = pstSrc->au32Stride[0];

    //fill dst
    //stDstData.u64VirAddr = pstDst->au64VirAddr[0];
    if( pstDst )
    {
        stDstData.u64PhyAddr = pstDst->au64PhyAddr[0];
        stDstData.u32Width   = pstDst->u32Width;
        stDstData.u32Height  = pstDst->u32Height;
        stDstData.u32Stride  = pstDst->au32Stride[0];
    }

    stSrcData.u32Width += stSrcData.u32Width % 2;
    stDstData.u32Width += stDstData.u32Width % 2;
    stSrcData.u32Height += stSrcData.u32Height % 2;
    stDstData.u32Height += stDstData.u32Height % 2;
#endif



    if( pstDst )
    {
        s32Ret = HI_MPI_IVE_DMA(&hIveHandle, &stSrcData, &stDstData, &stCtrl, bInstant);
    }
    else
    {
        s32Ret = HI_MPI_IVE_DMA(&hIveHandle, &stSrcData, NULL, &stCtrl, bInstant);
    }

    if (HI_SUCCESS != s32Ret)
    {
        DPRINTK("HI_MPI_IVE_DMA fail,Error(%#x),pstSrc->u32Width:%d,pstSrc->u32Height:%d,stDstData.u32Width:%d,stSrcData.u32Height:%d\n", s32Ret,pstSrc->u32Width,pstSrc->u32Height, \
                stDstData.u32Width,stSrcData.u32Height);
        return s32Ret;
    }

    if (HI_TRUE == bInstant)
    {
        s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);

        while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
        }

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}



int ds_ive_dma_copy(DS_YUV_COMBINE_TASK task, VIDEO_FRAME_INFO_S* yuv_combine_frame)
{
    IVE_SRC_IMAGE_S stSrcImage;
    IVE_DST_IMAGE_S stDstImage;
    VIDEO_FRAME_INFO_S* src_frame;
    int s32Ret;
    yuv_combine_frame->stVFrame.u64PTS = task.src_yuv_frame1->stVFrame.u64PTS;

    if( task.combine_mode == YUV_COMBINE_MODE_0)
    {
        IVE_DMA_CTRL_S  stDmaCtrl = {IVE_DMA_MODE_INTERVAL_COPY,
                                     0, 2, 1, 2
                                    };
        //抽点放缩的效果，不大好，后期要改为VGS做线性放缩。
        //将src_yuv_frame1图片贴到左上角区域
        src_frame = task.src_yuv_frame1;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        // Y分量
        //stSrcImage.au64VirAddr[0] = (HI_U8*)pstInFrame->stVFrame.pVirAddr[0];
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0];
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[0];
        stDstImage.u32Width = yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 2;
        stDstImage.au32Stride[0]  = yuv_combine_frame->stVFrame.u32Stride[0];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        // UV分量
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1];
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width ;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2;
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1];
        stDstImage.u32Width	= yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 4;
        stDmaCtrl.u8HorSegSize = 4;
        stDmaCtrl.u8ElemSize = 2;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        //将src_yuv_frame2图片贴到右下角区域
        src_frame = task.src_yuv_frame2;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        unsigned int dest_y_offset = yuv_combine_frame->stVFrame.u32Stride[0] *
                yuv_combine_frame->stVFrame.u32Height / 2 + \
                yuv_combine_frame->stVFrame.u32Width / 2;
        // Y分量
        //stSrcImage.au64VirAddr[0] = (HI_U8*)pstInFrame->stVFrame.pVirAddr[0];
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0];
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[0] + dest_y_offset;
        stDstImage.u32Width = yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 2;
        stDstImage.au32Stride[0]  = yuv_combine_frame->stVFrame.u32Stride[0];
        stDmaCtrl.u8HorSegSize = 2;
        stDmaCtrl.u8ElemSize = 1;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        unsigned int dest_uv_offset = yuv_combine_frame->stVFrame.u32Stride[0] *
                yuv_combine_frame->stVFrame.u32Height / 4 + \
                yuv_combine_frame->stVFrame.u32Width / 2;
        // UV分量
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1] ;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width ;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2;
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1] + dest_uv_offset;
        stDstImage.u32Width	= yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 4;
        stDmaCtrl.u8HorSegSize = 4;
        stDmaCtrl.u8ElemSize = 2;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_TRUE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }
    }
    else if( task.combine_mode == YUV_COMBINE_MODE_1)
    {
        IVE_DMA_CTRL_S  stDmaCtrl = {IVE_DMA_MODE_DIRECT_COPY,
                                     0, 0, 0, 0
                                    };
        //将src_yuv_frame1图片贴到左半边区域
        src_frame = task.src_yuv_frame1;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        // Y分量
        int src_frame_y_offset = src_frame->stVFrame.u32Width / 4;
        //stSrcImage.au64VirAddr[0] = (HI_U8*)pstInFrame->stVFrame.pVirAddr[0];
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0] + src_frame_y_offset;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width / 2;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[0];
        stDstImage.u32Width = yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height;
        stDstImage.au32Stride[0]  = yuv_combine_frame->stVFrame.u32Stride[0];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        // UV分量
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1] + src_frame->stVFrame.u32Width / 4;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width / 2 ;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2;
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1];
        stDstImage.u32Width	= yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 2;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        //将src_yuv_frame2图片贴到右半边区域
        src_frame = task.src_yuv_frame2;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        src_frame_y_offset = src_frame->stVFrame.u32Width / 4;
        unsigned int dest_y_offset = yuv_combine_frame->stVFrame.u32Stride[0] *
                yuv_combine_frame->stVFrame.u32Height / 2 + \
                yuv_combine_frame->stVFrame.u32Width / 2;
        // Y分量
        //stSrcImage.au64VirAddr[0] = (HI_U8*)pstInFrame->stVFrame.pVirAddr[0];
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0] + src_frame_y_offset;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width / 2;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[0] +
                yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Width = yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height ;
        stDstImage.au32Stride[0]  = yuv_combine_frame->stVFrame.u32Stride[0];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        // UV分量
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1] +	src_frame->stVFrame.u32Width / 4;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width / 2;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2 ;
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1] +
                src_frame->stVFrame.u32Width / 2;
        stDstImage.u32Width	= yuv_combine_frame->stVFrame.u32Width / 2;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 2 ;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_TRUE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }
    }
    else if( task.combine_mode == YUV_COMBINE_MODE_2)
    {
        IVE_DMA_CTRL_S  stDmaCtrl = {IVE_DMA_MODE_DIRECT_COPY,
                                     0, 0, 0, 0
                                    };
        //将src_yuv_frame1图片贴到左半边区域
        src_frame = task.src_yuv_frame1;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        // Y分量
        int src_frame_y_offset = src_frame->stVFrame.u32Stride[0] * src_frame->stVFrame.u32Height / 4;
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0] + src_frame_y_offset;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width ;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[0];
        stDstImage.u32Width = yuv_combine_frame->stVFrame.u32Width ;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 2;
        stDstImage.au32Stride[0]  = yuv_combine_frame->stVFrame.u32Stride[0];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        // UV分量
        int src_frame_uv_offset = src_frame->stVFrame.u32Stride[0] * src_frame->stVFrame.u32Height / 8;
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1] + src_frame_uv_offset ;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width	;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 4 ;
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1];
        stDstImage.u32Width	= yuv_combine_frame->stVFrame.u32Width;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 4;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        //将src_yuv_frame1图片贴到左半边区域
        src_frame = task.src_yuv_frame2;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        // Y分量
        src_frame_y_offset = src_frame->stVFrame.u32Stride[0] * src_frame->stVFrame.u32Height / 4;
        int dst_frame_y_offset = src_frame->stVFrame.u32Stride[0] * src_frame->stVFrame.u32Height / 2;
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0] + src_frame_y_offset;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width ;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[0] + dst_frame_y_offset;
        stDstImage.u32Width = yuv_combine_frame->stVFrame.u32Width ;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 2;
        stDstImage.au32Stride[0]  = yuv_combine_frame->stVFrame.u32Stride[0];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        // UV分量
        src_frame_uv_offset = src_frame->stVFrame.u32Stride[0] * src_frame->stVFrame.u32Height / 8;
        int dst_frame_uv_offset = src_frame->stVFrame.u32Stride[0] * src_frame->stVFrame.u32Height / 4;
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1] + src_frame_uv_offset ;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width	;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 4 ;
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1] + dst_frame_uv_offset;
        stDstImage.u32Width	= yuv_combine_frame->stVFrame.u32Width;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 4;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_TRUE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }
    }
    else if( task.combine_mode == YUV_COMBINE_MODE_3)
    {
        IVE_DMA_CTRL_S  stDmaCtrl = {IVE_DMA_MODE_DIRECT_COPY,
                                     0, 0, 0, 0
                                    };
        //将src_yuv_frame1图片贴到左半边区域
        src_frame = task.src_yuv_frame1;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        // Y分量
        int src_frame_y_offset = 0 ;
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0] + src_frame_y_offset;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width ;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[0];
        stDstImage.u32Width = yuv_combine_frame->stVFrame.u32Width ;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 2;
        stDstImage.au32Stride[0]  = yuv_combine_frame->stVFrame.u32Stride[0];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        // UV分量
        int src_frame_uv_offset = 0;
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1] + src_frame_uv_offset ;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width	;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2 ;
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1];
        stDstImage.u32Width	= yuv_combine_frame->stVFrame.u32Width;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 4;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        //将src_yuv_frame1图片贴到左半边区域
        src_frame = task.src_yuv_frame2;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        // Y分量
        src_frame_y_offset = 0;
        int dst_frame_y_offset = src_frame->stVFrame.u32Stride[0] * src_frame->stVFrame.u32Height;
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0] + src_frame_y_offset;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width ;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[0] + dst_frame_y_offset;
        stDstImage.u32Width = yuv_combine_frame->stVFrame.u32Width ;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 2;
        stDstImage.au32Stride[0]  = yuv_combine_frame->stVFrame.u32Stride[0];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

        // UV分量
        src_frame_uv_offset = 0;
        int dst_frame_uv_offset = src_frame->stVFrame.u32Stride[0] * src_frame->stVFrame.u32Height / 2;
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1] + src_frame_uv_offset ;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width	;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2 ;
        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1] + dst_frame_uv_offset;
        stDstImage.u32Width	= yuv_combine_frame->stVFrame.u32Width;
        stDstImage.u32Height  = yuv_combine_frame->stVFrame.u32Height / 4;
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_TRUE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }
    }
    else
    {
        DPRINTK( "dtask.combine_mode = %d is error\n", task.combine_mode);
    }

    return 0;
err:
    return -1;
}

static int vc_image_superposition_one_copy(VIDEO_FRAME_INFO_S* src_frame, VIDEO_FRAME_INFO_S* yuv_combine_frame,
                     HI_BOOL bInstant)
{
    IVE_SRC_IMAGE_S stSrcImage;
    IVE_DST_IMAGE_S stDstImage;
    //VIDEO_FRAME_INFO_S* src_frame;
    VIDEO_FRAME_INFO_S* dst_frame;
    int s32Ret;
    //printf("=============come in==vc_set_bottom_area_pic= vc_image_superposition_one_copy ======================\r\n");

    yuv_combine_frame->stVFrame.u64PTS = task.src_yuv_frame1->stVFrame.u64PTS;
    {
        //src_frame = task.src_yuv_frame1;
        dst_frame = yuv_combine_frame;
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        // Y分量
        int src_frame_y_offset = 0;
        //stSrcImage.au64VirAddr[0] = (HI_U8*)pstInFrame->stVFrame.pVirAddr[0];
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[0] + src_frame_y_offset;
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[0];
        //右上角
        src_frame_y_offset = task.dst_rect.x + (task.dst_rect.y) * dst_frame->stVFrame.u32Stride[0] + (task.dst_rect.width - src_frame->stVFrame.u32Width);

        stDstImage.au64PhyAddr[0] = dst_frame->stVFrame.u64PhyAddr[0] + src_frame_y_offset;
        stDstImage.u32Width = src_frame->stVFrame.u32Width;
        stDstImage.u32Height  = src_frame->stVFrame.u32Height;
        stDstImage.au32Stride[0]  = dst_frame->stVFrame.u32Stride[0];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, HI_FALSE, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x),src_frame->stVFrame.u32Width:%d,src_frame->stVFrame.u32Height:%d,src_frame->stVFrame.u32Width:%d,src_frame->stVFrame.u32Height\n", s32Ret,src_frame->stVFrame.u32Width,\
                     src_frame->stVFrame.u32Height,src_frame->stVFrame.u32Width,src_frame->stVFrame.u32Height);
            goto err;
        }

        // UV分量
        stSrcImage.au64PhyAddr[0] = src_frame->stVFrame.u64PhyAddr[1];
        stSrcImage.u32Width	= src_frame->stVFrame.u32Width;
        stSrcImage.u32Height  = src_frame->stVFrame.u32Height / 2;
        stSrcImage.au32Stride[0]  = src_frame->stVFrame.u32Stride[1];
        //右上角
        src_frame_y_offset =  (task.dst_rect.x ) + (task.dst_rect.y / 2)  *
                (dst_frame->stVFrame.u32Stride[1]) + (task.dst_rect.width - src_frame->stVFrame.u32Width);

        stDstImage.au64PhyAddr[0] = yuv_combine_frame->stVFrame.u64PhyAddr[1] + src_frame_y_offset;
        stDstImage.u32Width	= stSrcImage.u32Width;
        stDstImage.u32Height  = stSrcImage.u32Height;
        stDstImage.au32Stride[0]  = dst_frame->stVFrame.u32Stride[1];
        s32Ret = ds_dma_mode_copy(&stSrcImage, &stDstImage, bInstant, stDmaCtrl);

        if (HI_SUCCESS != s32Ret)
        {
            DPRINTK( "ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            goto err;
        }

    }
    return 0;
err:
    return -1;
}
#endif

#endif