/*
 * x5_b_io.h
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
 */

#ifndef __X5_B_IO_H__
#define __X5_B_IO_H__


/*
 * 业务相关
 */


extern int x5b_app_open_option(x5b_app_mgt_t *app, void *info, int to);

extern int x5b_app_add_card(x5b_app_mgt_t *app, void *info, int to);
extern int x5b_app_delete_card(x5b_app_mgt_t *app, void *info, int to);

extern int x5b_app_wiggins_setting(x5b_app_mgt_t *app, int wiggins, int to);


#endif /* __X5_B_IO_H__ */
