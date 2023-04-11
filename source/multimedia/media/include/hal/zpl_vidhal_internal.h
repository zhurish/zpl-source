
/*
 * zpl_vidhal_internal.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_INTERNAL_H__
#define __ZPL_VIDHAL_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#endif

#include <zpl_syshal.h>
#include <zpl_vidhal_input.h>
#include <zpl_vidhal_vpss.h>
#include <zpl_vidhal_venc.h>
#include <zpl_vidhal_mipi.h>
#include <zpl_vidhal_crop.h>
#include <zpl_vidhal_region.h>

#include <zpl_vidhal_sensor.h>
#include <zpl_vidhal_vgs.h>
#include <zpl_vidhal_yuv.h>

#include <zpl_vidhal_isp.h>
#include <zpl_vidhal_ive.h>
#include <zpl_vidhal_svp.h>
#include <zpl_vidhal_nnie.h>
#include <zpl_vidhal_hdmi.h>


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_INTERNAL_H__ */
