/*
 * x5_b_ubus.h
 *
 *  Created on: 2019年4月4日
 *      Author: DELL
 */

#ifndef __X5_B_UBUS_H__
#define __X5_B_UBUS_H__

#include "x5b_dbase.h"


typedef struct
{
	int 			faceYawLeft;
	int 			faceYawRight;
	int 			facePitchUp;
	int 			facePitchDown;
	int 			faceRecordWidth;
	int 			faceRecordHeight;
	int 			faceRecognizeWidth;
	int 			faceRecognizeHeight;

	float 			similarRecord;
	float 			similarRecognize;
	float 			similarSecondRecognize;
	u_int8			livenessSwitch;
}make_face_config_t;

#ifdef PL_OPENWRT_UCI
extern int x5_b_ubus_uci_update_cb(char *buf, int len);
#endif
extern int x5b_app_A_unit_test_set_api(BOOL enable);
extern int x5b_app_A_update_test(char *filename);
#endif /* __X5_B_UBUS_H__ */
