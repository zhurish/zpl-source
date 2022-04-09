#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include <zpl_vidhal.h>
#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#include "zpl_vidhal_svp.h"

#if 0

static zpl_bool s_bSampleSvpInit = HI_FALSE;

/*
*System init
*/
static int zpl_vidhal_svp_SysInit(zpl_void)
{
    zpl_int32 s32Ret = HI_FAILURE;
    VB_CONFIG_S struVbConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    memset(&struVbConf,0,sizeof(VB_CONFIG_S));

    struVbConf.u32MaxPoolCnt             = 2;
    struVbConf.astCommPool[1].u64BlkSize = 768*576*2;
    struVbConf.astCommPool[1].u32BlkCnt  = 1;

    s32Ret = HI_MPI_VB_SetConfig((const VB_CONFIG_S *)&struVbConf);
    ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_SetConf failed!\n", s32Ret);

    s32Ret = HI_MPI_VB_Init();
    ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_Init failed!\n", s32Ret);

    s32Ret = HI_MPI_SYS_Init();
    ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_Init failed!\n", s32Ret);

    return s32Ret;
}
/*
*System exit
*/
static int zpl_vidhal_svp_SysExit(zpl_void)
{
	zpl_int32 s32Ret = HI_FAILURE;

	s32Ret = HI_MPI_SYS_Exit();
    ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_Exit failed!\n", s32Ret);

	s32Ret = HI_MPI_VB_Exit();
    ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_Exit failed!\n", s32Ret);

	return HI_SUCCESS;
}
/*
*System init
*/
zpl_void zpl_vidhal_svp_CheckSysInit(zpl_void)
{
	if(HI_FALSE == s_bSampleSvpInit)
    {
        if (zpl_vidhal_svp_SysInit())
        {
            ZPL_VIDHAL_SVP_TRACE(ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR,"Svp mpi init failed!\n");
            exit(-1);
        }
        s_bSampleSvpInit = HI_TRUE;
    }

	ZPL_VIDHAL_SVP_TRACE(ZPL_VIDHAL_SVP_ERR_LEVEL_DEBUG,"Svp mpi init ok!\n");
}
/*
*System exit
*/
zpl_void zpl_vidhal_svp_CheckSysExit(zpl_void)
{
    if ( s_bSampleSvpInit)
    {
        zpl_vidhal_svp_SysExit();
        s_bSampleSvpInit = HI_FALSE;
    }

	ZPL_VIDHAL_SVP_TRACE(ZPL_VIDHAL_SVP_ERR_LEVEL_DEBUG,"Svp mpi exit ok!\n");
}
#endif

/*
*Align
*/
zpl_uint32 zpl_vidhal_svp_align(zpl_uint32 u32Size, zpl_uint16 u16Align)
{
	zpl_uint32 u32Stride = u32Size + (u16Align - u32Size%u16Align)%u16Align;
	return u32Stride;
}

/*
*Create Image memory
*/
int zpl_vidhal_svp_CreateImage(SVP_IMAGE_S *pstImg,SVP_IMAGE_TYPE_E enType,zpl_uint32 u32Width,
zpl_uint32 u32Height,zpl_uint32 u32AddrOffset)
{
	zpl_uint32 u32Size = 0;
	zpl_int32 s32Ret;

	pstImg->enType = enType;
	pstImg->u32Width = u32Width;
	pstImg->u32Height = u32Height;
	pstImg->au32Stride[0] = zpl_vidhal_svp_align(pstImg->u32Width,ZPL_VIDHAL_SVP_ALIGN_16);

	switch(enType)
	{
	case SVP_IMAGE_TYPE_U8C1:
	case SVP_IMAGE_TYPE_S8C1:
		{
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);
			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;

		}
		break;
	case SVP_IMAGE_TYPE_YUV420SP:
		{
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 3 / 2 + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);
			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
			pstImg->au32Stride[1] = pstImg->au32Stride[0];
			pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
			pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;

		}
		break;
	case SVP_IMAGE_TYPE_YUV422SP:
		{
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 2 + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);
			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
			pstImg->au32Stride[1] = pstImg->au32Stride[0];
			pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
			pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;

		}
		break;
	case SVP_IMAGE_TYPE_YUV420P:
		{
			pstImg->au32Stride[1] = zpl_vidhal_svp_align(pstImg->u32Width/2,ZPL_VIDHAL_SVP_ALIGN_16);
			pstImg->au32Stride[2] = pstImg->au32Stride[1];

			u32Size = pstImg->au32Stride[0] * pstImg->u32Height +  pstImg->au32Stride[1] * pstImg->u32Height + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);

			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
			pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
			pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
			pstImg->au64PhyAddr[2] = pstImg->au64PhyAddr[1] + pstImg->au32Stride[1] * pstImg->u32Height/2;
			pstImg->au64VirAddr[2] = pstImg->au64VirAddr[1] + pstImg->au32Stride[1] * pstImg->u32Height/2;
		}
		break;
	case SVP_IMAGE_TYPE_YUV422P:
		{

			pstImg->au32Stride[1] = zpl_vidhal_svp_align(pstImg->u32Width/2,ZPL_VIDHAL_SVP_ALIGN_16);
			pstImg->au32Stride[2] = pstImg->au32Stride[1];
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height +  pstImg->au32Stride[1] * pstImg->u32Height  * 2 + u32AddrOffset;

			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);

			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
			pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
			pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
			pstImg->au64PhyAddr[2] = pstImg->au64PhyAddr[1] + pstImg->au32Stride[1] * pstImg->u32Height;
			pstImg->au64VirAddr[2] = pstImg->au64VirAddr[1] + pstImg->au32Stride[1] * pstImg->u32Height;

		}
		break;
	case SVP_IMAGE_TYPE_S8C2_PACKAGE:
		{
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 2 + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);

			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
			pstImg->au32Stride[1] = pstImg->au32Stride[0];
			pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + 1;
			pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + 1;

		}
		break;
	case SVP_IMAGE_TYPE_S8C2_PLANAR:
		{
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 2 + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);

			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
			pstImg->au32Stride[1] = pstImg->au32Stride[0];
			pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;
			pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + pstImg->au32Stride[0] * pstImg->u32Height;

		}
		break;
	case SVP_IMAGE_TYPE_S16C1:
	case SVP_IMAGE_TYPE_U16C1:
		{

			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint16) + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);
			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
		}
		break;
	case SVP_IMAGE_TYPE_U8C3_PACKAGE:
		{
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 3 + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);

			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
			pstImg->au32Stride[1] = pstImg->au32Stride[0];
			pstImg->au32Stride[2] = pstImg->au32Stride[0];
			pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] +1;
			pstImg->au64VirAddr[2] = pstImg->au64VirAddr[1] + 1;
			pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + 1;
			pstImg->au64PhyAddr[2] = pstImg->au64PhyAddr[1] + 1;
		}
		break;
	case SVP_IMAGE_TYPE_U8C3_PLANAR:
		{
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * 3 + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);

			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
			pstImg->au32Stride[1] = pstImg->au32Stride[0];
			pstImg->au32Stride[2] = pstImg->au32Stride[0];
			pstImg->au64VirAddr[1] = pstImg->au64VirAddr[0] + (zpl_uint64)pstImg->au32Stride[0] * (zpl_uint64)pstImg->u32Height;
			pstImg->au64VirAddr[2] = pstImg->au64VirAddr[1] + (zpl_uint64)pstImg->au32Stride[1] * (zpl_uint64)pstImg->u32Height;
			pstImg->au64PhyAddr[1] = pstImg->au64PhyAddr[0] + (zpl_uint64)pstImg->au32Stride[0] * (zpl_uint64)pstImg->u32Height;
			pstImg->au64PhyAddr[2] = pstImg->au64PhyAddr[1] + (zpl_uint64)pstImg->au32Stride[1] * (zpl_uint64)pstImg->u32Height;

		}
		break;
	case SVP_IMAGE_TYPE_S32C1:
	case SVP_IMAGE_TYPE_U32C1:
		{
			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint32) + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);
			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
		}
		break;
	case SVP_IMAGE_TYPE_S64C1:
	case SVP_IMAGE_TYPE_U64C1:
		{

			u32Size = pstImg->au32Stride[0] * pstImg->u32Height * sizeof(zpl_uint64) + u32AddrOffset;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->au64PhyAddr[0], (void**)&pstImg->au64VirAddr[0], NULL, HI_NULL, u32Size);
            ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);

			pstImg->au64PhyAddr[0] += u32AddrOffset;
			pstImg->au64VirAddr[0] += u32AddrOffset;
		}
		break;
	default:
		break;

	}

	return HI_SUCCESS;
}
/*
*Destory image memory
*/
zpl_void zpl_vidhal_svp_DestroyImage(SVP_IMAGE_S *pstImg,zpl_uint32 u32AddrOffset)
{
    if (NULL != pstImg)
    {
       if ((0 != pstImg->au64VirAddr[0]) && (0 != pstImg->au64PhyAddr[0]))
       {
         (zpl_void)HI_MPI_SYS_MmzFree(pstImg->au64PhyAddr[0] - u32AddrOffset, (void *) (HI_UL)(pstImg->au64VirAddr[0] - u32AddrOffset));
       }
       memset (pstImg,0,sizeof(*pstImg));
    }
}

/*
*Create mem info
*/
int zpl_vidhal_svp_CreateMemInfo(SVP_MEM_INFO_S *pstMemInfo,zpl_uint32 u32Size,zpl_uint32 u32AddrOffset)
{
	zpl_int32 s32Ret;
	zpl_uint32 u32SizeTmp;

	u32SizeTmp = u32Size + u32AddrOffset;
	pstMemInfo->u32Size = u32Size;
	s32Ret = HI_MPI_SYS_MmzAlloc(&pstMemInfo->u64PhyAddr, (void**)&pstMemInfo->u64VirAddr, NULL, HI_NULL, u32SizeTmp);
    ZPL_VIDHAL_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, ZPL_VIDHAL_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_MmzAlloc failed!\n", s32Ret);

	pstMemInfo->u64PhyAddr += u32AddrOffset;
	pstMemInfo->u64VirAddr += u32AddrOffset;

	return s32Ret;
}

/*
Destory mem info
*/
zpl_void zpl_vidhal_svp_DestroyMemInfo(SVP_MEM_INFO_S *pstMemInfo,zpl_uint32 u32AddrOffset)
{
    if (NULL != pstMemInfo)
    {
       if ((0 != pstMemInfo->u64VirAddr) && (0 != pstMemInfo->u64PhyAddr))
       {
         (zpl_void)HI_MPI_SYS_MmzFree(pstMemInfo->u64PhyAddr - u32AddrOffset, (void *) ((HI_UL)pstMemInfo->u64VirAddr - u32AddrOffset));
       }
       memset (pstMemInfo,0,sizeof(*pstMemInfo));
    }
}
/*
*Malloc memory
*/
int zpl_vidhal_svp_MallocMem(zpl_char *pszMmb, zpl_char *pszZone, zpl_uint64 *pu64PhyAddr, zpl_void **ppvVirAddr, zpl_uint32 u32Size)
{
	zpl_int32 s32Ret = HI_SUCCESS;

	s32Ret = HI_MPI_SYS_MmzAlloc(pu64PhyAddr, ppvVirAddr, pszMmb, pszZone, u32Size);

	return s32Ret;
}

/*
*Malloc memory with cached
*/
int zpl_vidhal_svp_MallocCached(zpl_char *pszMmb, zpl_char *pszZone,zpl_uint64 *pu64PhyAddr, zpl_void **ppvVirAddr, zpl_uint32 u32Size)
{
	zpl_int32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_SYS_MmzAlloc_Cached(pu64PhyAddr, ppvVirAddr, pszMmb, pszZone, u32Size);

	return s32Ret;
}

/*
*Fulsh cached
*/
int zpl_vidhal_svp_FlushCache(zpl_uint64 u64PhyAddr, zpl_void *pvVirAddr, zpl_uint32 u32Size)
{
	zpl_int32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_SYS_MmzFlushCache(u64PhyAddr, pvVirAddr,u32Size);
	return s32Ret;
}

/*
*Gen rand data
*/
int zpl_vidhal_svp_GenRandS32(zpl_int32 s32Max,zpl_int32 s32Min)
{
	zpl_int32 s32Ret = 0;

	if (s32Min >= 0)
	{
		s32Ret = s32Min + ((zpl_uint32)rand())%(s32Max - s32Min + 1);
	}
	else
	{
		s32Ret = ((zpl_uint32)rand() )% (s32Max - s32Min + 1);
		s32Ret = s32Ret > s32Max ?   s32Max - s32Ret: s32Ret;
	}

	return s32Ret;
}

/*
*Gen image
*/
zpl_void zpl_vidhal_svp_GenImage(zpl_uint64 au64Buff[3],zpl_uint32 au32Stride[3],SVP_IMAGE_TYPE_E enType,zpl_uint32 u32Width,zpl_uint32 u32Height)
{
	zpl_uint32 i,j,k;
	zpl_uint8 *pu8;
	zpl_int8 *ps8;
	zpl_uint16 *pu16;
	zpl_int16 *ps16;
	zpl_uint32 *pu32;
	zpl_int32 *ps32;
	zpl_uint64 *pu64;
	zpl_int64 *ps64;
    zpl_uint8 *apu8Buff1[3] = {NULL, NULL, NULL};

	switch(enType)
	{
	case SVP_IMAGE_TYPE_U8C1:
		{
			apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];

			pu8 = apu8Buff1[0];
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[0];
			}
		}
		break;
	case SVP_IMAGE_TYPE_S8C1:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];

			ps8 = (zpl_int8*)apu8Buff1[0];
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					ps8[j] =  zpl_vidhal_svp_GenRandS32(127,-128);
				}
				ps8 += au32Stride[0];
			}
		}
		break;
	case SVP_IMAGE_TYPE_YUV420SP:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];

            apu8Buff1[1] = (zpl_uint8 *)(HI_UL)au64Buff[1];

			pu8 = apu8Buff1[0];
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[0];
			}

            pu8 = apu8Buff1[1];
            u32Height  /= 2;
            for(i = 0; i < u32Height;i++)
            {
                for(j = 0; j < u32Width;j++)
                {
                	pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
                }
                pu8 += au32Stride[1];
            }
		}
		break;
	case SVP_IMAGE_TYPE_YUV422SP:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
            apu8Buff1[1] = (zpl_uint8 *)(HI_UL)au64Buff[1];
            pu8 = apu8Buff1[0];
            for(i = 0; i < u32Height;i++)
            {
                for(j = 0; j < u32Width;j++)
                {
                	pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
                }
                pu8 += au32Stride[0];
            }
            pu8 = apu8Buff1[1];
            for(i = 0; i < u32Height;i++)
            {
                for(j = 0; j < u32Width;j++)
                {
                    pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
                }
                pu8 += au32Stride[1];
            }
		}
		break;
	case SVP_IMAGE_TYPE_YUV420P:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
            apu8Buff1[1] = (zpl_uint8 *)(HI_UL)au64Buff[1];
            apu8Buff1[2] = (zpl_uint8 *)(HI_UL)au64Buff[2];

			pu8 = apu8Buff1[0];
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[0];
			}

			pu8 = apu8Buff1[1];
			u32Height /= 2;
			u32Width /= 2;
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[1];
			}

			pu8 = apu8Buff1[2];
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[2];
			}
		}
		break;
	case SVP_IMAGE_TYPE_YUV422P:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
            apu8Buff1[1] = (zpl_uint8 *)(HI_UL)au64Buff[1];
            apu8Buff1[2] = (zpl_uint8 *)(HI_UL)au64Buff[2];

			pu8 = apu8Buff1[0];
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[0];
			}

			pu8 = apu8Buff1[1];
			u32Width /= 2;
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[1];
			}
			pu8 = apu8Buff1[2];

			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[2];
			}
		}
		break;
	case SVP_IMAGE_TYPE_S8C2_PACKAGE:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
			ps8 = (zpl_int8*)apu8Buff1[0];
			u32Width += u32Width;
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					ps8[j] =  zpl_vidhal_svp_GenRandS32(127,-128);
				}
				ps8 += au32Stride[0];
			}
		}
		break;
	case SVP_IMAGE_TYPE_S8C2_PLANAR:
		{
			for (k = 0; k < 2;k++)
			{
                apu8Buff1[k] = (zpl_uint8 *)(HI_UL)au64Buff[k];
				ps8 = (zpl_int8*)apu8Buff1[k];

				for(i = 0; i < u32Height;i++)
				{
					for(j = 0; j < u32Width;j++)
					{
						ps8[j] =  zpl_vidhal_svp_GenRandS32(127,-128);
					}
					ps8 += au32Stride[k];
				}
			}
		}
		break;
	case SVP_IMAGE_TYPE_S16C1:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
			ps16 = (zpl_int16*)apu8Buff1[0];

			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{

					ps16[j] =  (zpl_int16)zpl_vidhal_svp_GenRandS32(32767,-32768);

				}
				ps16 = (zpl_int16*)((zpl_uint8*)ps16 + au32Stride[0]) ;
			}
		}
		break;
	case SVP_IMAGE_TYPE_U16C1:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
			pu16 = (zpl_uint16*)apu8Buff1[0];

			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{

					pu16[j] =  zpl_vidhal_svp_GenRandS32(65535,0);
				}
				pu16 = (zpl_uint16*)((zpl_uint8*)pu16 +  au32Stride[0]) ;
			}
		}
		break;
	case SVP_IMAGE_TYPE_U8C3_PACKAGE:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
			pu8 =  apu8Buff1[0];
			u32Width *= 3;
			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
				}
				pu8 += au32Stride[0];
			}
		}
		break;
	case SVP_IMAGE_TYPE_U8C3_PLANAR:
		{

			for (k = 0; k < 3;k++)
			{
                apu8Buff1[k] = (zpl_uint8 *)(HI_UL)au64Buff[k];

				pu8 = apu8Buff1[k];
				for(i = 0; i < u32Height;i++)
				{
					for(j = 0; j < u32Width;j++)
					{
						pu8[j] =  zpl_vidhal_svp_GenRandS32(255,0);
					}
					pu8 += au32Stride[k];
				}
			}
		}
		break;
	case SVP_IMAGE_TYPE_S32C1:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
			ps32 = (zpl_int32*)apu8Buff1[0];

			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					ps32[j] =  zpl_vidhal_svp_GenRandS32(2147483646,-2147483647);
				}
				ps32 += au32Stride[0];
			}
		}
		break;
	case SVP_IMAGE_TYPE_U32C1:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
			pu32 = (zpl_uint32*)apu8Buff1[0];

			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu32[j] =  zpl_vidhal_svp_GenRandS32(65535,0) * zpl_vidhal_svp_GenRandS32(65535,0);
				}
				pu32 += au32Stride[0];
			}
		}
		break;
	case SVP_IMAGE_TYPE_S64C1:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
			ps64 = (zpl_int64*)apu8Buff1[0];

			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					ps64[j] =  zpl_vidhal_svp_GenRandS32(2147483646,-2147483647) * zpl_vidhal_svp_GenRandS32(65535,0)* zpl_vidhal_svp_GenRandS32(65535,0) ;
				}
				ps64 += au32Stride[0];
			}
		}
		break;
	case SVP_IMAGE_TYPE_U64C1:
		{
            apu8Buff1[0] = (zpl_uint8 *)(HI_UL)au64Buff[0];
			pu64 = (zpl_uint64*)apu8Buff1[0];

			for(i = 0; i < u32Height;i++)
			{
				for(j = 0; j < u32Width;j++)
				{
					pu64[j] = (zpl_uint64)zpl_vidhal_svp_GenRandS32(65535,0) * (zpl_uint64)zpl_vidhal_svp_GenRandS32(65535,0) * (zpl_uint64)zpl_vidhal_svp_GenRandS32(65535,0)* (zpl_uint64)zpl_vidhal_svp_GenRandS32(65535,0) ;
				}
				pu64 += au32Stride[0];
			}
		}
		break;
	default:
		break;

	}
}


#endif