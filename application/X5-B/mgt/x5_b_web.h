/*
 * x5_b_ubus.h
 *
 *  Created on: 2019年4月4日
 *      Author: DELL
 */

#ifndef __X5_B_UBUS_H__
#define __X5_B_UBUS_H__

//#include "x5b_dbase.h"

extern int x5b_app_global_config_action(void *info, BOOL save);

extern int x5b_app_open_option_action(void *p, BOOL save, BOOL face);

extern int x5b_app_face_config_action(void *info, BOOL save);
/*extern int x5b_app_A_unit_test_set_api(BOOL enable);
extern int x5b_app_A_update_test(char *filename);*/

#endif /* __X5_B_UBUS_H__ */
