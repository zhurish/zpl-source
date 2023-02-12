/*
 * zpl_media_internal.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_INTERNAL_H__
#define __ZPL_MEDIA_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include <zpl_type.h>
#include <zpl_media.h>
#include <zpl_media_format.h>
#include <zpl_media_codec.h>
#include <zpl_media_buffer.h>
#include <zpl_media_text.h>
#include <zpl_media_area.h>
#include <zpl_media_channel.h>
#include "zpl_media_file.h"
#include "zpl_media_extradata.h"
#include <zpl_media_client.h>
#include <zpl_media_hardadap.h>
#include <zpl_media_hal.h>
#include <zpl_media_resources.h>
#include <zpl_media_config.h>
#include <zpl_media_record.h>
#include <zpl_media_capture.h>
#include "zpl_media_task.h"
#include "zpl_media_log.h"
#include "zpl_media_proxy.h"
#include "zpl_media_area.h"
#include "zpl_media_frame_adap.h"

#include <zpl_media_video_input.h>
#include <zpl_media_video_vpss.h>
#include <zpl_media_video_encode.h>
#include "zpl_media_video_region.h"
#include "zpl_media_video_sys.h"


#include "zpl_media_sensor_type.h"


#include "zpl_vidhal_internal.h"

typedef enum 
{
    VO_MODE_1MUX  ,
    VO_MODE_2MUX  ,
    VO_MODE_4MUX  ,
    VO_MODE_8MUX  ,
    VO_MODE_9MUX  ,
    VO_MODE_16MUX ,
    VO_MODE_25MUX ,
    VO_MODE_36MUX ,
    VO_MODE_49MUX ,
    VO_MODE_64MUX ,
    VO_MODE_2X4   ,
    VO_MODE_BUTT
} ZPL_HDMI_VO_MODE_E;







#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_INTERNAL_H__ */
