/*
 * x5_b_test.h
 *
 *  Created on: 2019年7月23日
 *      Author: DELL
 */

#ifndef __X5_B_MGT_X5_B_TEST_H__
#define __X5_B_MGT_X5_B_TEST_H__


int x5b_app_test_start();
int x5b_app_test_stop();
int x5b_app_test_isstart();

int x5_b_app_test_call_phone(char *num);
int x5_b_app_test_call_phonenum(char *num);
int x5_b_app_test_call_list();

#endif /* __X5_B_MGT_X5_B_TEST_H__ */
