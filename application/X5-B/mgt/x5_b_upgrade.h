/*
 * x5_b_upgrade.h
 *
 *  Created on: 2019年5月20日
 *      Author: DELL
 */

#ifndef __X5_B_UPGRADE_H__
#define __X5_B_UPGRADE_H__


#ifdef __cplusplus
extern "C" {
#endif

extern int x5b_app_update_mode(x5b_app_mgt_t *app, void *info, int to);
extern int x5b_app_update_mode_exit(x5b_app_mgt_t *app);
extern int x5b_app_update_data(x5b_app_mgt_t *app, void *info, int len, int to);
extern int ymodem_send(int fd, char *filename, int filesize);

extern int x5b_app_A_update_handle(char *buf);
extern int x5b_app_upgrade_handle(char *pathdir, char *filename);

#ifdef __cplusplus
}
#endif

#endif /* __X5_B_UPGRADE_H__ */
