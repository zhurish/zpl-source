/*
 * x5_b_ubus.h
 *
 *  Created on: 2019年4月4日
 *      Author: DELL
 */

#ifndef __X5_B_UBUS_H__
#define __X5_B_UBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "x5b_dbase.h"

extern int x5b_app_global_config_action(void *info, zpl_bool save);

extern int x5b_app_open_option_action(void *p, zpl_bool save, zpl_bool face);

extern int x5b_app_face_config_action(void *info, zpl_bool save);
/*extern int x5b_app_A_unit_test_set_api(zpl_bool enable);
extern int x5b_app_A_update_test(char *filename);*/

#ifdef __cplusplus
}
#endif

#endif /* __X5_B_UBUS_H__ */
