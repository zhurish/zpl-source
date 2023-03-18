/*
 * zpl_syshal.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_syshal.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "zpl_media_internal.h"


#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#endif

static int s_s32SampleMemDev = -1;
#ifdef ZPL_HISIMPP_MODULE
static int _sys_mem_init = -1;
#endif

#ifdef ZPL_HISIMPP_MODULE
static const char *zpl_syshal_vi_strerror(zpl_uint32 halerrno)
{
    static char tmpdata[64];
    zpl_uint32 hal_errno = halerrno & 0x1fff;
    switch(hal_errno)
    {
    case ERR_VI_FAILED_NOTENABLE:       /* device or channel not enable */
    return "device or channel not enable";
    case ERR_VI_FAILED_NOTDISABLE:          /* device not disable */
    return "device not disable";
    case ERR_VI_FAILED_CHNOTDISABLE:         /* channel not disable */
    return "channel not disable";
    case ERR_VI_CFG_TIMEOUT:                 /* config timeout */
    return "config timeout";
    case ERR_VI_NORM_UNMATCH:                /* video norm of ADC and VIU is unmatch */
    return "video norm of ADC and VIU is unmatch";
    case ERR_VI_INVALID_WAYID:              /* invlalid way ID */
    return "invlalid way ID";
    case ERR_VI_INVALID_PHYCHNID:           /* invalid phychn id */
    return "invalid phychn id";
    case ERR_VI_FAILED_NOTBIND:              /* device or channel not bind */
    return "device or channel not bind";
    case ERR_VI_FAILED_BINDED:               /* device or channel not unbind */
    return "device or channel not unbind";
    case ERR_VI_DIS_PROCESS_FAIL:             /* dis process failed */
    return "dis process failed";
    case EN_ERR_BUTT:/* maxium code, private error code of all modules
                              ** must be greater than it                      */                  
    default:
        break;                          
    }
    switch(halerrno)
    {
case HI_ERR_VI_INVALID_PARA: return "invlalid param";
case HI_ERR_VI_INVALID_DEVID: return "invlalid Dev ID";
case HI_ERR_VI_INVALID_PIPEID: return "invlalid Pipe ID";
case HI_ERR_VI_INVALID_STITCHGRPID: return "invlalid Sticth";
case HI_ERR_VI_INVALID_CHNID: return "invlalid Chn ID";
case HI_ERR_VI_INVALID_NULL_PTR: return "invlalid point";
case HI_ERR_VI_FAILED_NOTCONFIG: return "Not Config";
case HI_ERR_VI_SYS_NOTREADY: return "Not Ready";
case HI_ERR_VI_BUF_EMPTY: return "Buffer Empty";
case HI_ERR_VI_BUF_FULL: return "Buffer Full";
case HI_ERR_VI_NOMEM: return "no mem";
case HI_ERR_VI_NOT_SUPPORT: return "not support";
case HI_ERR_VI_BUSY: return "busy";
case HI_ERR_VI_NOT_PERM: return "HI_ERR_VI_NOT_PERM";

case HI_ERR_VI_FAILED_NOTENABLE: return "HI_ERR_VI_FAILED_NOTENABLE";
case HI_ERR_VI_FAILED_NOTDISABLE: return "HI_ERR_VI_FAILED_NOTDISABLE";
case HI_ERR_VI_FAILED_CHNOTDISABLE: return "HI_ERR_VI_FAILED_CHNOTDISABLE";
case HI_ERR_VI_CFG_TIMEOUT: return "HI_ERR_VI_CFG_TIMEOUT";
case HI_ERR_VI_NORM_UNMATCH : return "HI_ERR_VI_NORM_UNMATCH";
case HI_ERR_VI_INVALID_WAYID: return "HI_ERR_VI_INVALID_WAYID";
case HI_ERR_VI_INVALID_PHYCHNID: return "HI_ERR_VI_INVALID_PHYCHNID";
case HI_ERR_VI_FAILED_NOTBIND: return "HI_ERR_VI_FAILED_NOTBIND";
case HI_ERR_VI_FAILED_BINDED : return "HI_ERR_VI_FAILED_BINDED";

case HI_ERR_VI_PIPE_EXIST: return "HI_ERR_VI_PIPE_EXIST";
case HI_ERR_VI_PIPE_UNEXIST: return "HI_ERR_VI_PIPE_UNEXIST";
    default:
        break;                          
    }
    memset(tmpdata, 0, sizeof(tmpdata));
    sprintf(tmpdata, "Unknow:%#x",halerrno);
    return tmpdata;
}  
const char *zpl_syshal_strerror(zpl_uint32 halerrno)
{
    static char tmpdata[64];
    zpl_uint32 hal_errno = halerrno & 0x1fff;
    switch(hal_errno)
    {
    case EN_ERR_INVALID_DEVID: /* invlalid device ID                           */
    return "Invlalid Device ID";
    case EN_ERR_INVALID_CHNID: /* invlalid channel ID                          */
    return "Invlalid Channel ID";
    case EN_ERR_ILLEGAL_PARAM: /* at lease one parameter is illagal
                               * eg, an illegal enumeration value             */
    return "Invlalid Parameter";
    case EN_ERR_EXIST: /* resource exists                              */
    return "Resource Exists";
    case EN_ERR_UNEXIST: /* resource unexists                            */
    return "Resource Unexists";

    case EN_ERR_NULL_PTR: /* using a NULL point                           */
        return "NULL Point";
    case EN_ERR_NOT_CONFIG: /* try to enable or initialize system, device
                              ** or channel, before configing attribute       */
        return "Not Config";
    case EN_ERR_NOT_SUPPORT: /* operation or type is not supported by NOW    */
    return "Not Supported";
    case EN_ERR_NOT_PERM: /* operation is not permitted
                              ** eg, try to change static attribute           */
        return "Operation Not Permitted";
    case EN_ERR_INVALID_PIPEID: /* invlalid pipe ID                           */
    return "Invlalid Pipe ID";
    case EN_ERR_INVALID_STITCHGRPID: /* invlalid stitch group ID                           */
    return "Invlalid Stitch Group ID";

    case EN_ERR_NOMEM:/* failure caused by malloc memory              */
        return "failure malloc memory";
    case EN_ERR_NOBUF:/* failure caused by malloc buffer              */
        return "failure malloc buffer";
    case EN_ERR_BUF_EMPTY:/* no data in buffer                            */
    return "no data in buffer";
    case EN_ERR_BUF_FULL:/* no buffer for new data                       */
    return "no buffer for new data";
    case EN_ERR_SYS_NOTREADY:/* System is not ready,maybe not initialed or
                              ** loaded. Returning the error code when opening
                              ** a device file failed.                        */
    return "System is not ready";
    case EN_ERR_BADADDR:/* bad address,
                              ** eg. used for copy_from_user & copy_to_user   */
    return "bad address";
    case EN_ERR_BUSY:/* resource is busy,
                              ** eg. destroy a venc chn without unregister it */
    return "resource is busy";
    case EN_ERR_SIZE_NOT_ENOUGH: /* buffer size is smaller than the actual size required */
    return "buffer size is not enough";
    
    case EN_ERR_BUTT:/* maxium code, private error code of all modules
                              ** must be greater than it                      */                  
    default:
        break;                          
    }
    if(HI_ID_VI == ((halerrno >> 16)&0xff) )
    {
        return zpl_syshal_vi_strerror(halerrno);
    }
    memset(tmpdata, 0, sizeof(tmpdata));
    sprintf(tmpdata, "Unknow:%#x",halerrno);
    return tmpdata;
}   
#else
const char *zpl_syshal_strerror(zpl_uint32 halerrno)
{
    static char buf[32];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "0x%#x", halerrno);
    return buf;
}
#endif

zpl_void * zpl_sys_iommap(zpl_uint64 u64PhyAddr, zpl_uint32 u32Size)
{
    zpl_uint32 u32Diff;
    zpl_uint64 u64PagePhy;
    zpl_int32 * pPageAddr;
    zpl_uint32    ulPageSize;

    if (s_s32SampleMemDev <= 0)
	{
		s_s32SampleMemDev = open("/dev/mem", O_RDWR|O_SYNC);
		if (s_s32SampleMemDev < 0)
		{
			perror("Open dev/mem error");
			return NULL;
		}
	};

    /**********************************************************
    PageSize will be 0 when u32size is 0 and u32Diff is 0,
    and then mmap will be error (error: Invalid argument)
    ***********************************************************/
    if (!u32Size)
    {
        printf("Func: %s u32Size can't be 0.\n", __FUNCTION__);
        return NULL;
    }

    /* The mmap address should align with page */
    u64PagePhy = u64PhyAddr & 0xfffffffffffff000ULL;
    u32Diff    = u64PhyAddr - u64PagePhy;

    /* The mmap size shuld be mutliples of 1024 */
    ulPageSize = ((u32Size + u32Diff - 1) & 0xfffff000UL) + 0x1000;

    pPageAddr    = mmap ((void *)0, ulPageSize, PROT_READ|PROT_WRITE,
                                    MAP_SHARED, s_s32SampleMemDev, u64PagePhy);
    if (MAP_FAILED == pPageAddr )
    {
        perror("mmap error");
        return NULL;
    }
    return (zpl_void *) (pPageAddr + u32Diff);
}


int zpl_sys_munmap(zpl_void* pVirAddr, zpl_uint32 u32Size)
{
    zpl_uint64 u64PageAddr;
    zpl_uint32 u32PageSize;
    zpl_uint32 u32Diff;

    u64PageAddr = (((zpl_uint32)pVirAddr) & 0xfffffffffffff000ULL);
    u32Diff     = (zpl_uint32)pVirAddr - u64PageAddr;
    u32PageSize = ((u32Size + u32Diff - 1) & 0xfffff000UL) + 0x1000;

    return munmap((zpl_void*)u64PageAddr, u32PageSize);
}

/******************************************************************************
* function : Set system memory location
******************************************************************************/
int zpl_syshal_mem_init(void)
{
#ifdef ZPL_HISIMPP_MODULE	
    zpl_uint32 i, j;
    int s32Ret = HI_SUCCESS;
    zpl_char* pcMmzName = NULL;
    MPP_CHN_S stMppChn;

    /*config memory for vi*/
    for (i = 0; i < VI_MAX_PIPE_NUM; i++)
    {
        for (j = 0; j < VI_MAX_CHN_NUM; j++)
        {
            stMppChn.enModId  = HI_ID_VI;
            stMppChn.s32DevId = i;
            stMppChn.s32ChnId = j;
            s32Ret = HI_MPI_SYS_SetMemConfig(&stMppChn, pcMmzName);

            if (s32Ret)
            {
                if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
                    zm_msg_err("HI_MPI_SYS_SetMemConfig failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
                return HI_FAILURE;
            }
        }
    }

    /*config memory for avs */
    for (i = 0; i < AVS_MAX_GRP_NUM; i++)
    {
        stMppChn.enModId  = HI_ID_AVS;
        stMppChn.s32DevId = i;
        stMppChn.s32ChnId = 0;
        s32Ret = HI_MPI_SYS_SetMemConfig(&stMppChn, pcMmzName);

        if (s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
                zm_msg_err("HI_MPI_SYS_SetMemConfig failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
            return HI_FAILURE;
        }
    }

    /*config memory for vpss */
    for (i = 0; i < VPSS_MAX_GRP_NUM; i++)
    {
        stMppChn.enModId  = HI_ID_VPSS;
        stMppChn.s32DevId = i;
        stMppChn.s32ChnId = 0;
        s32Ret = HI_MPI_SYS_SetMemConfig(&stMppChn, pcMmzName);

        if (s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
                zm_msg_err("HI_MPI_SYS_SetMemConfig failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
            return HI_FAILURE;
        }
    }

    /*config memory for venc */
    for (i = 0; i < VENC_MAX_CHN_NUM; i++)
    {

        stMppChn.enModId  = HI_ID_VENC;
        stMppChn.s32DevId = 0;
        stMppChn.s32ChnId = i;
        s32Ret = HI_MPI_SYS_SetMemConfig(&stMppChn, pcMmzName);

        if (s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
                zm_msg_err("HI_MPI_SYS_SetMemConf failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
            return HI_FAILURE;
        }
    }

    /*config memory for vo*/
    for (i = 0; i < VO_MAX_LAYER_NUM; i++)
    {
        for (j = 0; j < VO_MAX_CHN_NUM; j++)
        {
            stMppChn.enModId    = HI_ID_VO;
            stMppChn.s32DevId = i;
            stMppChn.s32ChnId = j;
            s32Ret = HI_MPI_SYS_SetMemConfig(&stMppChn, pcMmzName);

            if (s32Ret)
            {
                if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
                    zm_msg_err("HI_MPI_SYS_SetMemConfig failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
                return HI_FAILURE;
            }
        }
    }

    /*config memory for vdec */
    for (i = 0; i < VDEC_MAX_CHN_NUM; i++)
    {

        stMppChn.enModId  = HI_ID_VDEC;
        stMppChn.s32DevId = 0;
        stMppChn.s32ChnId = i;
        s32Ret = HI_MPI_SYS_SetMemConfig(&stMppChn, pcMmzName);

        if (s32Ret)
        {
            if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
                zm_msg_err("HI_MPI_SYS_SetMemConf failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
	return OK;
#endif
}


/******************************************************************************
* function : vb exit & MPI system exit
******************************************************************************/
void zpl_syshal_mem_exit(void)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 i=0, j=0;

    for (i=0; i < VO_MAX_LAYER_NUM; i++)
    {
        for (j=0; j<VO_MAX_CHN_NUM; j++)
        {
            HI_MPI_VO_DisableChn(i, j);
        }
    }

    for (i = 0; i < VO_MAX_LAYER_NUM; i++)
    {
        HI_MPI_VO_DisableVideoLayer(i);
    }

    for (i = 0; i < VO_MAX_DEV_NUM; i++)
    {
        HI_MPI_VO_Disable(i);
    }
    HI_MPI_SYS_Exit();
    HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
    HI_MPI_VB_Exit();
#endif
    return;
}

/******************************************************************************
* function : vb init & MPI system init
******************************************************************************/

int zpl_syshal_vbmem_init(void)
{
#ifdef ZPL_HISIMPP_MODULE    
    HI_S32 s32Ret = HI_FAILURE, u32MaxPoolCnt = 0;
    VB_CONFIG_S stVbConfig;
    zpl_uint64 u64BlkSize = 0;
	VB_SUPPLEMENT_CONFIG_S stSupplementConf = {0};
    if(_sys_mem_init != -1)
        return 0;
    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();
    
    memset_s(&stVbConfig, sizeof(VB_CONFIG_S), 0, sizeof(VB_CONFIG_S));

    //u64BlkSize = COMMON_GetPicBufferSize(vdsize.width, vdsize.height, 
    //    PIXEL_FORMAT_YVU_SEMIPLANAR_422, DATA_BITWIDTH_8, COMPRESS_MODE_SEG,DEFAULT_ALIGN);
    u64BlkSize = zpl_syshal_get_buffer_size(ZPL_VIDEO_FORMAT_1080P, PIXEL_FORMAT_YVU_SEMIPLANAR_420);    
    stVbConfig.astCommPool[u32MaxPoolCnt].u64BlkSize   = u64BlkSize;
    stVbConfig.astCommPool[u32MaxPoolCnt].u32BlkCnt    = 30;
    u32MaxPoolCnt++;

    //u64BlkSize = COMMON_GetPicBufferSize(720, 576, PIXEL_FORMAT_YVU_SEMIPLANAR_422, //pal
    //    DATA_BITWIDTH_8, COMPRESS_MODE_SEG,DEFAULT_ALIGN);
    u64BlkSize = zpl_syshal_get_buffer_size(ZPL_VIDEO_FORMAT_D1_PAL, PIXEL_FORMAT_YVU_SEMIPLANAR_420);
    stVbConfig.astCommPool[u32MaxPoolCnt].u64BlkSize   = u64BlkSize;
    stVbConfig.astCommPool[u32MaxPoolCnt].u32BlkCnt    = 30;
    u32MaxPoolCnt++;
/*
    u64BlkSize = zpl_syshal_get_buffer_size(ZPL_VIDEO_FORMAT_D1_NTSC, PIXEL_FORMAT_YVU_SEMIPLANAR_420);
    stVbConfig.astCommPool[u32MaxPoolCnt].u64BlkSize   = u64BlkSize;
    stVbConfig.astCommPool[u32MaxPoolCnt].u32BlkCnt    = 10;
    u32MaxPoolCnt++;
*/
    stVbConfig.u32MaxPoolCnt = u32MaxPoolCnt;

    s32Ret = HI_MPI_VB_SetConfig(&stVbConfig);

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
            zm_msg_err("HI_MPI_VB_SetConf failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
	
    stSupplementConf.u32SupplementConfig = VB_SUPPLEMENT_ISPINFO_MASK;
    s32Ret = HI_MPI_VB_SetSupplementConfig(&stSupplementConf);

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
            zm_msg_err("HI_MPI_VB_SetSupplementConf failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
	
    s32Ret = HI_MPI_VB_Init();

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
            zm_msg_err("HI_MPI_VB_Init failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
            zm_msg_err("HI_MPI_SYS_Init failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        HI_MPI_VB_Exit();
        return HI_FAILURE;
    }
    _sys_mem_init = 1;
#endif
    return OK;
}


zpl_uint32 zpl_syshal_get_buffer_size(ZPL_VIDEO_FORMAT_E format, zpl_uint32 pixelformat)
{
    zpl_video_size_t stSize;
    if(zpl_media_video_format_resolution(format, &stSize) == OK)
    {
        #ifdef ZPL_HISIMPP_MODULE 
        zpl_uint32 u64BlkSize = COMMON_GetPicBufferSize(stSize.width, stSize.height, 
            pixelformat, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
        #else
        zpl_uint64 u64BlkSize = 1;
        #endif    
        return u64BlkSize; 
    }
    return 0;
}

zpl_uint32 zpl_syshal_get_membuf_size(zpl_uint32 u32Width, zpl_uint32 u32Height,
        zpl_uint32 enPixelFormat, zpl_uint32 enBitWidth, zpl_uint32 enCmpMode, zpl_uint32 u32Align)
{
    #ifdef ZPL_HISIMPP_MODULE
    return COMMON_GetPicBufferSize( u32Width,  u32Height,
         enPixelFormat,  enBitWidth,  enCmpMode,  u32Align);
    #else
    return 0;     
    #endif
}

/******************************************************************************
* function : get picture size(w*h), according enPicSize
******************************************************************************/
int zpl_syshal_vivpss_mode_set(zpl_int32 inputpipe, zpl_int32 mode)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    VI_VPSS_MODE_S vivpss_mode;
    if(inputpipe >= VI_MAX_PIPE_NUM || inputpipe < 0)
        return HI_FAILURE;
     s32Ret = HI_MPI_SYS_GetVIVPSSMode(&vivpss_mode);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
            zm_msg_err("HI_MPI_SYS_GetVIVPSSMode failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
    vivpss_mode.aenMode[inputpipe] = mode;
    s32Ret = HI_MPI_SYS_SetVIVPSSMode(&vivpss_mode);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
            zm_msg_err("HI_MPI_SYS_SetVIVPSSMode failed:%s(%x)!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif	
}



int zpl_syshal_input_bind_vpss(zpl_int32 inputpipe, zpl_int32 inputchn, zpl_int32 vpssgrp)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VI;
    stSrcChn.s32DevId  = inputpipe;
    stSrcChn.s32ChnId  = inputchn;

    stDestChn.enModId  = HI_ID_VPSS;
    stDestChn.s32DevId = vpssgrp;
    stDestChn.s32ChnId = 0;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "zpl_syshal_input_bind_vpss(VI-VPSS)");
    return HI_SUCCESS;
#endif	
	return OK;	
}
int zpl_syshal_input_unbind_vpss(zpl_int32 inputpipe, zpl_int32 inputchn, zpl_int32 vpssgrp)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VI;
    stSrcChn.s32DevId  = inputpipe;
    stSrcChn.s32ChnId  = inputchn;

    stDestChn.enModId  = HI_ID_VPSS;
    stDestChn.s32DevId = vpssgrp;
    stDestChn.s32ChnId = 0;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "zpl_syshal_input_unbind_vpss(VI-VPSS)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_vpss_bind_venc(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_int32 vencchn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VPSS;
    stSrcChn.s32DevId  = vpssgrp;
    stSrcChn.s32ChnId  = vpsschn;

    stDestChn.enModId  = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = vencchn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "zpl_syshal_vpss_bind_venc(VPSS-VENC)");
    return HI_SUCCESS;
#endif	
	return OK;
}
int zpl_syshal_vpss_unbind_venc(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_int32 vencchn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VPSS;
    stSrcChn.s32DevId  = vpssgrp;
    stSrcChn.s32ChnId  = vpsschn;

    stDestChn.enModId  = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = vencchn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "zpl_syshal_vpss_unbind_venc(VPSS-VENC)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_input_bind_hdmi(zpl_int32 inputpipe, zpl_int32 inputchn, 
	zpl_int32 hdmilayer, zpl_int32 hdmichn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VI;
    stSrcChn.s32DevId  = inputpipe;
    stSrcChn.s32ChnId  = inputchn;

    stDestChn.enModId  = HI_ID_VO;
    stDestChn.s32DevId = hdmilayer;
    stDestChn.s32ChnId = hdmichn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "zpl_syshal_input_bind_hdmi(VI-VO)");
    return HI_SUCCESS;
#endif	
	return OK;
}
int zpl_syshal_input_unbind_hdmi(zpl_int32 inputpipe, zpl_int32 inputchn, 
	zpl_int32 hdmilayer, zpl_int32 hdmichn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VI;
    stSrcChn.s32DevId  = inputpipe;
    stSrcChn.s32ChnId  = inputchn;

    stDestChn.enModId  = HI_ID_VO;
    stDestChn.s32DevId = hdmilayer;
    stDestChn.s32ChnId = hdmichn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "zpl_syshal_input_unbind_hdmi(VI-VO)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_input_bind_venc(zpl_int32 inputpipe, zpl_int32 inputchn, zpl_int32 vencchn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VI;
    stSrcChn.s32DevId  = inputpipe;
    stSrcChn.s32ChnId  = inputchn;

    stDestChn.enModId  = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = vencchn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "zpl_syshal_input_bind_venc(VI-VENC)");
    return HI_SUCCESS;
#endif	
	return OK;
}
int zpl_syshal_input_unbind_venc(zpl_int32 inputpipe, zpl_int32 inputchn, zpl_int32 vencchn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VI;
    stSrcChn.s32DevId  = inputpipe;
    stSrcChn.s32ChnId  = inputchn;

    stDestChn.enModId  = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = vencchn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "zpl_syshal_input_unbind_venc(VI-VENC)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_vpss_bind_avs(zpl_int32 vpssgrp, zpl_int32 vpsschn, 
	zpl_int32 avsgrp, zpl_int32 avschn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VPSS;
    stSrcChn.s32DevId  = vpssgrp;
    stSrcChn.s32ChnId  = vpsschn;

    stDestChn.enModId  = HI_ID_AVS;
    stDestChn.s32DevId = avsgrp;
    stDestChn.s32ChnId = avschn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "zpl_syshal_vpss_bind_avs(VPSS-AVS)");
    return HI_SUCCESS;
#endif	
	return OK;
}
int zpl_syshal_vpss_unbind_avs(zpl_int32 vpssgrp, zpl_int32 vpsschn, 
	zpl_int32 avsgrp, zpl_int32 avschn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VPSS;
    stSrcChn.s32DevId  = vpssgrp;
    stSrcChn.s32ChnId  = vpsschn;

    stDestChn.enModId  = HI_ID_AVS;
    stDestChn.s32DevId = avsgrp;
    stDestChn.s32ChnId = avschn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "zpl_syshal_vpss_unbind_avs(VPSS-AVS)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_vpss_bind_hdmi(zpl_int32 vpssgrp, zpl_int32 vpsschn, 
	zpl_int32 hdmilayer, zpl_int32 hdmichn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VPSS;
    stSrcChn.s32DevId  = vpssgrp;
    stSrcChn.s32ChnId  = vpsschn;

    stDestChn.enModId  = HI_ID_VO;
    stDestChn.s32DevId = hdmilayer;
    stDestChn.s32ChnId = hdmichn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "zpl_syshal_vpss_bind_hdmi(VPSS-VO)");
    return HI_SUCCESS;
#endif	
	return OK;
}
int zpl_syshal_vpss_unbind_hdmi(zpl_int32 vpssgrp, zpl_int32 vpsschn, 
	zpl_int32 hdmilayer, zpl_int32 hdmichn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VPSS;
    stSrcChn.s32DevId  = vpssgrp;
    stSrcChn.s32ChnId  = vpsschn;

    stDestChn.enModId  = HI_ID_VO;
    stDestChn.s32DevId = hdmilayer;
    stDestChn.s32ChnId = hdmichn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "zpl_syshal_vpss_unbind_hdmi(VPSS-VO)");
    return HI_SUCCESS;
#endif	
	return OK;	
}


int zpl_syshal_avs_bind_avs(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 dstavsgrp, zpl_int32 dstavschn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_AVS;
    stSrcChn.s32DevId  = avsgrp;
    stSrcChn.s32ChnId  = avschn;

    stDestChn.enModId  = HI_ID_AVS;
    stDestChn.s32DevId = dstavsgrp;
    stDestChn.s32ChnId = dstavschn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(AVS-AVS)");
    return HI_SUCCESS;
#endif	
	return OK;
}

int zpl_syshal_avs_unbind_avs(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 dstavsgrp, zpl_int32 dstavschn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_AVS;
    stSrcChn.s32DevId  = avsgrp;
    stSrcChn.s32ChnId  = avschn;

    stDestChn.enModId  = HI_ID_AVS;
    stDestChn.s32DevId = dstavsgrp;
    stDestChn.s32ChnId = dstavschn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_UnBind(AVS-AVS)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_avs_bind_vpss(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 vpssgrp, zpl_int32 vpsschn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_AVS;
    stSrcChn.s32DevId  = avsgrp;
    stSrcChn.s32ChnId  = avschn;

    stDestChn.enModId  = HI_ID_VPSS;
    stDestChn.s32DevId = vpssgrp;
    stDestChn.s32ChnId = vpsschn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(AVS-VPSS)");
    return HI_SUCCESS;
#endif	
	return OK;
}

int zpl_syshal_avs_unbind_vpss(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 vpssgrp, zpl_int32 vpsschn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_AVS;
    stSrcChn.s32DevId  = avsgrp;
    stSrcChn.s32ChnId  = avschn;

    stDestChn.enModId  = HI_ID_VPSS;
    stDestChn.s32DevId = vpssgrp;
    stDestChn.s32ChnId = vpsschn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_UnBind(AVS-VPSS)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_avs_bind_venc(zpl_int32 avsgrp, zpl_int32 avschn, zpl_int32 vencchn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_AVS;
    stSrcChn.s32DevId  = avsgrp;
    stSrcChn.s32ChnId  = avschn;

    stDestChn.enModId  = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = vencchn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(AVS-VENC)");
    return HI_SUCCESS;
#endif	
	return OK;
}

int zpl_syshal_avs_unbind_venc(zpl_int32 avsgrp, zpl_int32 avschn, zpl_int32 vencchn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_AVS;
    stSrcChn.s32DevId  = avsgrp;
    stSrcChn.s32ChnId  = avschn;

    stDestChn.enModId  = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = vencchn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_UnBind(AVS-VENC)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_avs_bind_hdmi(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 hdmilayer, zpl_int32 hdmichn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_AVS;
    stSrcChn.s32DevId  = avsgrp;
    stSrcChn.s32ChnId  = avschn;

    stDestChn.enModId  = HI_ID_VO;
    stDestChn.s32DevId = hdmilayer;
    stDestChn.s32ChnId = hdmichn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(AVS-VO)");
    return HI_SUCCESS;
#endif	
	return OK;
}

int zpl_syshal_avs_unbind_hdmi(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 hdmilayer, zpl_int32 hdmichn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_AVS;
    stSrcChn.s32DevId  = avsgrp;
    stSrcChn.s32ChnId  = avschn;

    stDestChn.enModId  = HI_ID_VO;
    stDestChn.s32DevId = hdmilayer;
    stDestChn.s32ChnId = hdmichn;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_UnBind(AVS-VO)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_vdec_bind_vpss(zpl_int32 vdecchn, zpl_int32 vpssgrp)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = HI_ID_VDEC;
    stSrcChn.s32DevId  = 0;
    stSrcChn.s32ChnId  = vdecchn;

    stDestChn.enModId  = HI_ID_VO;
    stDestChn.s32DevId = vpssgrp;
    stDestChn.s32ChnId = 0;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(VDEC-VPSS)");
    return HI_SUCCESS;
#endif	
	return OK;
}

int zpl_syshal_vdec_unbind_vpss(zpl_int32 vdecchn, zpl_int32 vpssgrp)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId  = vdecchn;

    stDestChn.enModId  = HI_ID_VPSS;
    stDestChn.s32DevId = vpssgrp;
    stDestChn.s32ChnId = 0;
    ZPL_CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_UnBind(VDEC-VPSS)");
    return HI_SUCCESS;
#endif	
	return OK;	
}

int zpl_syshal_hdmi_bind_hdmi(zpl_int32 hdmilayer, zpl_int32 hdmichn, 
	zpl_int32 dsthdmilayer, zpl_int32 dsthdmichn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId    = HI_ID_VO;
    stSrcChn.s32DevId  = hdmilayer;
    stSrcChn.s32ChnId  = hdmichn;

    stDestChn.enModId  = HI_ID_VO;
    stDestChn.s32DevId = dsthdmilayer;
    stDestChn.s32ChnId = dsthdmichn;
    ZPL_CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(VO-VO)");
    return HI_SUCCESS;
#endif	
	return OK;
}

int zpl_syshal_hdmi_unbind_hdmi(zpl_int32 hdmilayer, zpl_int32 hdmichn)
{
#ifdef ZPL_HISIMPP_MODULE
    MPP_CHN_S stDestChn;


    stDestChn.enModId   = HI_ID_VO;
    stDestChn.s32DevId  = hdmilayer;
    stDestChn.s32ChnId  = hdmichn;

    return HI_MPI_SYS_UnBind(NULL, &stDestChn);
#endif	
	return OK;	
}