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


#include "voip_def.h"
#include "voip_sip.h"
#include "voip_task.h"
#include "voip_api.h"
#include "voip_app.h"
#include "voip_stream.h"
#include "voip_dbtest.h"

int voip_module_init()
{
	memset(&voip_call, 0, sizeof(voip_call));
	voip_call.debug = 0xffff;
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


	voip_dbtest_load();

	return OK;
}

int voip_module_exit()
{
	voip_volume_module_exit();
	voip_sip_module_init();
	voip_event_module_exit();
	voip_socket_module_exit();
	voip_stream_module_exit();
	return OK;
}

int voip_module_task_init()
{
	/*
	 * mediastream任务启动
	 */
	voip_task_module_init();
	/*
	 * voip event单元
	 */
	voip_event_task_init();
	voip_socket_task_init();
	return OK;
}

int voip_module_task_exit()
{
	voip_event_task_exit();
	voip_socket_task_exit();
	voip_task_module_exit();
	return OK;
}


int voip_enable_set_api(BOOL enable)
{
	voip_stream->enable = enable;
	return OK;
}

int voip_local_rtp_set_api(u_int32 ip, u_int16 port)
{
	//voip_config.l_rtp_port = port;
	voip_stream_local_port_api(voip_stream, port);
	return OK;
}

int voip_remote_rtp_set_api(u_int32 ip, u_int16 port)
{
	voip_stream_remote_address_port_api(voip_stream, inet_address(ip), port);
	return OK;
}



/**************************************************************************************/

/**************************************************************************************/


int voip_create_stream_and_start_api(voip_stream_remote_t *config)
{

	if(!voip_stream->l_rtp_port)
		voip_stream->l_rtp_port = VOIP_RTP_PORT_DEFAULT;

	zlog_debug(ZLOG_VOIP,"-----------%s 192.168.2.100:%d -> %s:%d",
			__func__, voip_stream->l_rtp_port, config->r_rtp_address, config->r_rtp_port);


	voip_stream_local_port_api(voip_stream, voip_stream->l_rtp_port);

	voip_stream_remote_address_port_api(voip_stream, config->r_rtp_address, config->r_rtp_port);

	voip_stream_payload_type_api(voip_stream, NULL, voip_stream->payload);

	voip_stream_echo_canceller_api(voip_stream, voip_stream->echo_canceller, voip_stream->ec_tail,
			voip_stream->ec_delay, voip_stream->ec_framesize);
	voip_stream_echo_limiter_api(voip_stream, voip_stream->echo_limiter, voip_stream->el_force,
			voip_stream->el_speed, voip_stream->el_sustain, voip_stream->el_thres, voip_stream->el_transmit);

	voip_stream_noise_gate_api(voip_stream, voip_stream->noise_gate,
			voip_stream->ng_floorgain, voip_stream->ng_threshold);

	return voip_stream_start_api(voip_stream);
}


#ifdef VOIP_STREAM_API_DEBUG_TEST
int _voip_start_api_shell(char *address, int local, int remote)
{
	voip_stream_remote_t config;
	memset(&config, 0, sizeof(config));
	config.r_rtp_port = remote ? remote:5555;
	voip_stream->l_rtp_port = local ? local:5555;
	strcpy(config.r_rtp_address, address);
	return voip_create_stream_and_start_api(&config);
}
#endif

#ifdef VOIP_STARTUP_TEST
int _voip_startup_test(void *p)
{
	voip_stream_remote_t config;
	memset(&config, 0, sizeof(config));
	config.r_rtp_port = 5555;
	voip_stream->l_rtp_port = 5555;
	printf("%s:\n",__func__);
	if(os_read_string("/app/etc/remote-test.txt",config.r_rtp_address, sizeof(config.r_rtp_address)) == OK)
	//strcpy(config.r_rtp_address, "192.168.2.11");
	return voip_create_stream_and_start_api(&config);
	return OK;
}
#endif

