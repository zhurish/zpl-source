/*
 * zpl_hal_hisi.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_HAL_HISI_H__
#define __ZPL_HAL_HISI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#ifdef ZPL_HISIMPP_MODULE

#include "hi_common.h"
#include "hi_buffer.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vpss.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"
#include "hi_comm_hdmi.h"
#include "hi_mipi.h"
#include "hi_comm_vgs.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#include "mpi_region.h"
#include "mpi_audio.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "hi_math.h"
#include "hi_sns_ctrl.h"
#include "mpi_hdmi.h"
#include "mpi_vgs.h"

#include "hi_mipi_tx.h"
#include "hi_debug.h"
#include "hi_comm_ive.h"
#include "mpi_ive.h"
#include "hi_comm_video.h"
#include "hi_comm_svp.h"
#include "hi_nnie.h"
#include "mpi_nnie.h"
#include "hi_ae_comm.h"
#include "hi_awb_comm.h"
#include "acodec.h"
#endif

#define COLOR_RGB_RED      0xFF0000
#define COLOR_RGB_GREEN    0x00FF00
#define COLOR_RGB_BLUE     0x0000FF
#define COLOR_RGB_BLACK    0x000000
#define COLOR_RGB_YELLOW   0xFFFF00
#define COLOR_RGB_CYN      0x00ffff
#define COLOR_RGB_WHITE    0xffffff


#define WDR_MAX_PIPE_NUM        4
//typedef HI_BOOL zpl_bool;
//#define zpl_false HI_FALSE
//#define zpl_true HI_TRUE



#define ZPL_CHECK_CHN_RET(express,Chn,name)\
    do{\
        int Ret;\
        Ret = express;\
        if (0 != Ret)\
        {\
            printf("\033[0;31m%s chn %d failed at %s: LINE: %d with %#x!\033[0;39m\n", name, Chn, __FUNCTION__, __LINE__, Ret);\
            fflush(stdout);\
            return Ret;\
        }\
    }while(0)

#define ZPL_CHECK_RET(express,name)\
    do{\
        int Ret;\
        Ret = express;\
        if (0 != Ret)\
        {\
            printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", name, __FUNCTION__, __LINE__, Ret);\
            return Ret;\
        }\
    }while(0)


#define ZPL_MEDIA_DEBUG_PRINT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)







#ifdef __cplusplus
}
#endif
#endif /* __ZPL_HAL_HISI_H__ */
