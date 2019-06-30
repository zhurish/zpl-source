/*
 * voip_api.c
 *
 *  Created on: Dec 9, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "zassert.h"

#include "voip_def.h"
#include "voip_api.h"
#include "voip_app.h"
#include "voip_sip.h"
#include "voip_task.h"
#include "voip_stream.h"
#include "voip_state.h"
#include "voip_log.h"
#include "voip_uci.h"

#include "application.h"

int voip_module_init()
{
	if(!voip_global_enabled())
		return OK;
	voip_app_gl_init();
/*	export MS_AUDIO_PRIO=NORMAL
	export ORTP_SIMULATOR_SCHED_POLICY=SCHED_OTHER*/
	//memset(&voip_call, 0, sizeof(voip_call));
	//voip_call.debug = 0xffff;
	/*
	 * SIP 配置单元初始化
	 */
	voip_sip_module_init();

	/*
	 * 声音音量控制单元初始化
	 */
	voip_volume_module_init();

	/*
	 * 铃声单元初始化
	 */
	voip_call_ring_module_init();

	/*
	 * voip event单元初始化
	 */
	voip_event_module_init();

	voip_socket_module_init();
	/*
	 * stream 单元初始化
	 */
	voip_stream_module_init();


	voip_dbase_load();


	voip_thlog_init();

#ifdef PL_OPENWRT_UCI
	uci_ubus_cb_install(voip_ubus_uci_update_cb);
#endif

	voip_app_media_callback_init(voip_app);

	voip_status_clear_api();
#ifdef APP_X5BA_MODULE
	x5b_app_module_init(NULL, 0);
	voip_app->x5b_app = x5b_app_tmp();
#endif
	return OK;
}

int voip_module_exit()
{
	if(!voip_global_enabled())
		return OK;
	voip_volume_module_exit();
	voip_sip_module_init();
	voip_event_module_exit();
	voip_socket_module_exit();
	voip_stream_module_exit();
	voip_app_gl_exit();
	voip_status_clear_api();
#ifdef APP_X5BA_MODULE
	x5b_app_module_exit();
#endif
	return OK;
}

int voip_module_task_init()
{
	if(!voip_global_enabled())
		return OK;
#ifdef APP_X5BA_MODULE
	x5b_app_module_task_init();
#endif
	/*
	 * mediastream任务启动
	 */
	voip_task_module_init();
	/*
	 * voip event单元
	 */
	voip_event_task_init();
	voip_socket_task_init();
	voip_sip_module_task_init();
	return OK;
}

int voip_module_task_exit()
{
	if(!voip_global_enabled())
		return OK;
	voip_sip_module_task_exit();
	voip_event_task_exit();
	voip_socket_task_exit();
	voip_task_module_exit();
#ifdef APP_X5BA_MODULE
	x5b_app_module_task_exit();
#endif
	return OK;
}


int voip_enable_set_api(BOOL enable)
{
	zassert(voip_stream != NULL);
	voip_stream->enable = enable;
	return OK;
}

int voip_local_rtp_set_api(u_int32 ip, u_int16 port)
{
	//voip_config.l_rtp_port = port;
	zassert(voip_stream != NULL);
	zassert(sip_config != NULL);
	voip_stream_local_port_api(voip_stream, port);
	voip_sip_config_update_api(sip_config);
	return OK;
}

int voip_remote_rtp_set_api(u_int32 ip, u_int16 port)
{
	zassert(voip_stream != NULL);
	voip_stream_remote_address_port_api(voip_stream, inet_address(ip), port);
	return OK;
}



/**************************************************************************************/



