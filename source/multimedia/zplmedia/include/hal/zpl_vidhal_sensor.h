/*
 * zpl_vidhal_sensor.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_SENSOR_H__
#define __ZPL_VIDHAL_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_sensor_type.h"




extern int zpl_vidhal_sensor_get_comboattr(ZPL_SENSOR_TYPE_E snstype, zpl_uint32 MipiDev, void *p);
extern int zpl_vidhal_sensor_get_pipeattr(ZPL_SENSOR_TYPE_E snstype, void *p);
extern int zpl_vidhal_sensor_get_devattr(ZPL_SENSOR_TYPE_E snstype, void *p);
extern int zpl_vidhal_sensor_get_chnattr(ZPL_SENSOR_TYPE_E snstype, void *p);
extern int zpl_vidhal_sensor_get_extchnattr(ZPL_SENSOR_TYPE_E snstype, void *p);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_SENSOR_H__ */
