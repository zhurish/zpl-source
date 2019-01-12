/*
 * voip_api.h
 *
 *  Created on: Dec 9, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_API_H__
#define __VOIP_API_H__

#define VOIP_STREAM_API_DEBUG_TEST
#define VOIP_STARTUP_TEST


#include "voip_stream.h"

extern int voip_module_init();

extern int voip_module_exit();


extern int voip_module_task_init();
extern int voip_module_task_exit();


extern int voip_enable_set_api(BOOL enable);


extern int voip_local_rtp_set_api(u_int32 ip, u_int16 port);
extern int voip_remote_rtp_set_api(u_int32 ip, u_int16 port);



extern int voip_create_stream_and_start_api(voip_stream_remote_t *config);


#ifdef VOIP_STREAM_API_DEBUG_TEST
int _voip_start_api_shell(char *address, int local, int remote);
#endif
#ifdef VOIP_STARTUP_TEST
int _voip_startup_test(void *p);
#endif

extern void cmd_voip_init(void);

extern void cmd_voip_test_init(int node);
/*
extern int voip_rtp_write(struct vty *vty);
extern int voip_rtp_write(struct vty *vty);
*/


#endif /* __VOIP_API_H__ */
