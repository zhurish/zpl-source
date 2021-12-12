/*
 * zpl_vidhal_isp.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_ISP_H__
#define __ZPL_VIDHAL_ISP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_sensor_type.h"

#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#else
#define VI_MAX_PIPE_NUM 4
#endif

typedef struct 
{
    zpl_uint32               isp_dev;                    //底层硬件pipe
    ZPL_SENSOR_TYPE_E        sns_type;                   //底层通道号
    zpl_int32                pipe[VI_MAX_PIPE_NUM];
    zpl_int32                vipipe;
    zpl_int32                enWDRMode;
    zpl_int32                bMultiPipe;
    zpl_int32                SnapPipe;
    zpl_int32                bDoublePipe;
    zpl_int32                s32BusId;
}zpl_vidhal_isp_sensor_t;

#ifdef ZPL_HISIMPP_MODULE
extern int zpl_vidhal_sensor_get_ispattr(ZPL_SENSOR_TYPE_E snstype, void *p);
#endif

extern int zpl_vidhal_isp_start_one(zpl_vidhal_isp_sensor_t *sensor);
extern int zpl_vidhal_isp_stop_one(zpl_vidhal_isp_sensor_t *sensor);
extern int zpl_vidhal_isp_stop(zpl_vidhal_isp_sensor_t *sensor);
extern int zpl_vidhal_isp_start(zpl_vidhal_isp_sensor_t *sensor);
extern int zpl_vidhal_isp_setparam(zpl_int32 pipe, zpl_uint32 frameRate);

#ifdef __cplusplus
}
#endif

#endif /* __ZPL_VIDHAL_ISP_H__ */
