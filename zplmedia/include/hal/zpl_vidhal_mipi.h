/*
 * zpl_vidhal_mipi.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_MIPI_H__
#define __ZPL_VIDHAL_MIPI_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int zpl_vidhal_mipi_start(zpl_int32 snsdev, zpl_int32 mipmdev, ZPL_SENSOR_TYPE_E snstype);
extern int zpl_vidhal_mipi_stop(zpl_int32 snsdev, zpl_int32 mipmdev);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_MIPI_H__ */
