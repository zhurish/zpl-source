/*
 * voip_api.h
 *
 *  Created on: Dec 9, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_API_H__
#define __VOIP_API_H__



//#include "voip_stream.h"

extern int voip_module_init();

extern int voip_module_exit();


extern int voip_module_task_init();
extern int voip_module_task_exit();


extern int voip_enable_set_api(BOOL enable);


extern int voip_local_rtp_set_api(u_int32 ip, u_int16 port);
extern int voip_remote_rtp_set_api(u_int32 ip, u_int16 port);




extern void cmd_voip_init(void);

extern void cmd_voip_test_init(int node);
/*
extern int voip_rtp_write(struct vty *vty);
extern int voip_rtp_write(struct vty *vty);
*/


#endif /* __VOIP_API_H__ */
