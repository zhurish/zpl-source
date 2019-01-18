/*
 * cmd_voip.c
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */



#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"

#include "voip_def.h"
#include "voip_api.h"
#include "voip_sip.h"
#include "voip_ring.h"
#include "voip_statistics.h"
#include "voip_volume.h"
#include "voip_stream.h"




static int voip_show_config(struct vty *vty, int detail);



DEFUN (ip_voip_service,
		ip_voip_service_cmd,
		"service voip",
		"Service configure\n"
		"VOIP Protocol\n")
{
	int ret = ERROR;
	if(!voip_stream->enable)
		ret = voip_enable_set_api(TRUE);
	else
		ret = OK;
	if(ret == OK)
		vty->node = VOIP_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_service,
		no_ip_voip_service_cmd,
		"no service voip",
		NO_STR
		"Service configure\n"
		"VOIP Protocol\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
		ret = voip_enable_set_api(FALSE);
	else
		ret = OK;
	if(ret == OK)
		vty->node = VOIP_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_voip_payload,
		ip_voip_payload_cmd,
		"ip voip payload <0-256>",
		IP_STR
		"VOIP Protocol\n"
		"Payload configure\n"
		"payload index\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
		ret = voip_stream_payload_type_api(voip_stream, NULL, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_payload,
		no_ip_voip_payload_cmd,
		"no ip voip payload",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Payload configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
		ret = voip_stream_payload_type_api(voip_stream, NULL, VOIP_RTP_PAYLOAD_DEFAULT);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_echo_canceller,
		ip_voip_echo_canceller_cmd,
		"ip voip echo-canceller",
		IP_STR
		"VOIP Protocol\n"
		"Echo Canceller configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		if(argc == 0)
			ret = voip_stream_echo_canceller_api(voip_stream, TRUE, voip_stream->ec_tail,
					voip_stream->ec_delay, voip_stream->ec_framesize);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN(ip_voip_echo_canceller_delay,
		ip_voip_echo_canceller_delay_cmd,
		"ip voip echo-canceller delay <0-1000>",
		IP_STR
		"VOIP Protocol\n"
		"Echo Canceller configure\n"
		"Echo canceller delay configure\n"
		"Echo canceller delay value in ms\n")
{
	int ret = ERROR;
	int ec_delay = 0;
	if(voip_stream->enable)
	{
		ec_delay = atoi(argv[0]);
		ret = voip_stream_echo_canceller_api(voip_stream, TRUE, voip_stream->ec_tail,
				ec_delay, voip_stream->ec_framesize);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN(ip_voip_echo_canceller_tail,
		ip_voip_echo_canceller_tail_cmd,
		"ip voip echo-canceller tail <0-1000>",
		IP_STR
		"VOIP Protocol\n"
		"Echo Canceller configure\n"
		"Echo canceller tail length configure\n"
		"Echo canceller tail length value in ms\n")
{
	int ret = ERROR;
	int ec_tail = 0;

	if(voip_stream->enable)
	{
		ec_tail = atoi(argv[0]);
		ret = voip_stream_echo_canceller_api(voip_stream, TRUE, ec_tail,
					voip_stream->ec_delay, voip_stream->ec_framesize);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN(ip_voip_echo_canceller_framesize,
		ip_voip_echo_canceller_framesize_cmd,
		"ip voip echo-canceller framesize <0-1000>",
		IP_STR
		"VOIP Protocol\n"
		"Echo Canceller configure\n"
		"Echo canceller framesize configure\n"
		"Echo canceller framesize value in samples\n")
{
	int ret = ERROR;
	int ec_framesize = 0;
	if(voip_stream->enable)
	{
		ec_framesize = atoi(argv[0]);
		ret = voip_stream_echo_canceller_api(voip_stream, TRUE, voip_stream->ec_tail,
					voip_stream->ec_delay, ec_framesize);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (no_ip_voip_echo_canceller,
		no_ip_voip_echo_canceller_cmd,
		"no ip voip echo-canceller",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Canceller configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
		ret = voip_stream_echo_canceller_api(voip_stream, FALSE, voip_stream->ec_tail,
				voip_stream->ec_delay, voip_stream->ec_framesize);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN(no_ip_voip_echo_canceller_delay,
		no_ip_voip_echo_canceller_delay_cmd,
		"no ip voip echo-canceller delay",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Canceller configure\n"
		"Echo canceller delay configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_canceller_api(voip_stream, TRUE, voip_stream->ec_tail,
				0, voip_stream->ec_framesize);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN(no_ip_voip_echo_canceller_tail,
		no_ip_voip_echo_canceller_tail_cmd,
		"no ip voip echo-canceller tail",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Canceller configure\n"
		"Echo canceller tail length configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_canceller_api(voip_stream, TRUE, 0,
					voip_stream->ec_delay, voip_stream->ec_framesize);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN(no_ip_voip_echo_canceller_framesize,
		no_ip_voip_echo_canceller_framesize_cmd,
		"no ip voip echo-canceller framesize",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Canceller configure\n"
		"Echo canceller framesize configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_canceller_api(voip_stream, TRUE, voip_stream->ec_tail,
					voip_stream->ec_delay, 0);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_voip_echo_limiter,
		ip_voip_echo_limiter_cmd,
		"ip voip echo-limiter",
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				voip_stream->el_speed,
				voip_stream->el_sustain, voip_stream->el_thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_echo_limiter_force,
		ip_voip_echo_limiter_force_cmd,
		"ip voip echo-limiter force VALUE",
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"The proportional coefficient controlling the mic attenuation\n"
		"Value<0-1>\n")
{
	int ret = ERROR;
	float force = 0.0;
	if(voip_stream->enable)
	{
		force = atof(argv[0]);
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, force, voip_stream->el_speed,
				voip_stream->el_sustain, voip_stream->el_thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_echo_limiter_thres,
		ip_voip_echo_limiter_thres_cmd,
		"ip voip echo-limiter thres VALUE",
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"Threshold above which the system becomes active\n"
		"Value<0-1>\n")
{
	int ret = ERROR;
	float thres = 0.0;
	if(voip_stream->enable)
	{
		thres = atof(argv[0]);
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				voip_stream->el_speed,
				voip_stream->el_sustain, thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_echo_limiter_speed,
		ip_voip_echo_limiter_speed_cmd,
		"ip voip echo-limiter speed VALUE",
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"gain changes are smoothed with a coefficent\n"
		"Value<0-1>\n")
{
	int ret = ERROR;
	float speed = 0.0;
	if(voip_stream->enable)
	{
		speed = atof(argv[0]);
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				speed,
				voip_stream->el_sustain, voip_stream->el_thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_echo_limiter_transmit,
		ip_voip_echo_limiter_transmit_cmd,
		"ip voip echo-limiter transmit-thres VALUE",
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"TO BE DOCUMENTED\n"
		"Value<0-1>\n")
{
	int ret = ERROR;
	float el_transmit = 0.0;
	if(voip_stream->enable)
	{
		el_transmit = atof(argv[0]);
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				voip_stream->el_speed,
				voip_stream->el_sustain, voip_stream->el_thres, el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_echo_limiter_sustain,
		ip_voip_echo_limiter_sustain_cmd,
		"ip voip echo-limiter sustain <0-65535>",
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"TO BE DOCUMENTED\n"
		"Value\n")
{
	int ret = ERROR;
	int sustain = 0;
	if(voip_stream->enable)
	{
		sustain = atoi(argv[0]);
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				voip_stream->el_speed,
				sustain, voip_stream->el_thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (no_ip_voip_echo_limiter,
		no_ip_voip_echo_limiter_cmd,
		"no ip voip echo-limiter",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
		ret = voip_stream_echo_limiter_api(voip_stream, FALSE, voip_stream->el_force,
				voip_stream->el_speed,
				voip_stream->el_sustain, voip_stream->el_thres, voip_stream->el_transmit);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_echo_limiter_force,
		no_ip_voip_echo_limiter_force_cmd,
		"no ip voip echo-limiter force",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"The proportional coefficient controlling the mic attenuation\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, 0.0, voip_stream->el_speed,
				voip_stream->el_sustain, voip_stream->el_thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_echo_limiter_thres,
		no_ip_voip_echo_limiter_thres_cmd,
		"no ip voip echo-limiter thres",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"Threshold above which the system becomes active\n")
{
	int ret = ERROR;
	float thres = 0.0;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				voip_stream->el_speed,
				voip_stream->el_sustain, thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_echo_limiter_speed,
		no_ip_voip_echo_limiter_speed_cmd,
		"no ip voip echo-limiter speed",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"gain changes are smoothed with a coefficent\n")
{
	int ret = ERROR;
	float speed = 0.0;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				speed,
				voip_stream->el_sustain, voip_stream->el_thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_echo_limiter_transmit,
		no_ip_voip_echo_limiter_transmit_cmd,
		"no ip voip echo-limiter transmit-thres",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"TO BE DOCUMENTED\n")
{
	int ret = ERROR;
	float el_transmit = 0.0;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				voip_stream->el_speed,
				voip_stream->el_sustain, voip_stream->el_thres, el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_echo_limiter_sustain,
		no_ip_voip_echo_limiter_sustain_cmd,
		"no ip voip echo-limiter sustain",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Echo Limiter configure\n"
		"TO BE DOCUMENTED\n")
{
	int ret = ERROR;
	int sustain = 0;
	if(voip_stream->enable)
	{
		ret = voip_stream_echo_limiter_api(voip_stream, TRUE, voip_stream->el_force,
				voip_stream->el_speed,
				sustain, voip_stream->el_thres, voip_stream->el_transmit);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_bitrate,
		ip_voip_bitrate_cmd,
		"ip voip bitrate <16-10000000>",
		IP_STR
		"VOIP Protocol\n"
		"Bitrate configure\n"
		"Bitrate Value\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		ret = voip_stream_bitrate_api(voip_stream, atoi(argv[0]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_bitrate,
		no_ip_voip_bitrate_cmd,
		"no ip voip bitrate",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Bitrate configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
		ret = voip_stream_bitrate_api(voip_stream, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_jitter,
		ip_voip_jitter_cmd,
		"ip voip jitter <0-1000>",
		IP_STR
		"VOIP Protocol\n"
		"Jitter configure\n"
		"Jitter value\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		ret = voip_stream_jitter_api(voip_stream, atoi(argv[0]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_jitter,
		no_ip_voip_jitter_cmd,
		"no ip voip jitter",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Jitter configure\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
		ret = voip_stream_jitter_api(voip_stream, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_noise_gate,
		ip_voip_noise_gate_cmd,
		"ip voip noise-gate",
		IP_STR
		"VOIP Protocol\n"
		"Noise configure\n"
		"Noise Gate\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
	{
		ret = voip_stream_noise_gate_api(voip_stream, TRUE, voip_stream->ng_floorgain, voip_stream->ng_threshold);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_noise_gate_floorgain,
		ip_voip_noise_gate_floorgain_cmd,
		"ip voip noise-gate floorgain VALUE",
		IP_STR
		"VOIP Protocol\n"
		"Noise Gate configure\n"
		"gain applied to the signal when its energy is below the threshold\n"
		"Value<0-1>\n")
{
	int ret = ERROR;
	float ng_floorgain = 0.0;
	if(voip_stream->enable)
	{
		ng_floorgain = atof(argv[0]);
		ret = voip_stream_noise_gate_api(voip_stream, TRUE, ng_floorgain, voip_stream->ng_threshold);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_voip_noise_gate_threshold,
		ip_voip_noise_gate_threshold_cmd,
		"ip voip noise-gate threshold VALUE",
		IP_STR
		"VOIP Protocol\n"
		"Noise Gate configure\n"
		"Noise Gate threshold\n"
		"Value<0-1>\n")
{
	int ret = ERROR;
	float ng_threshold = 0.0;
	if(voip_stream->enable)
	{
		ng_threshold = atof(argv[0]);
		ret = voip_stream_noise_gate_api(voip_stream, TRUE, voip_stream->ng_floorgain, ng_threshold);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_noise_gate,
		no_ip_voip_noise_gate_cmd,
		"no ip voip noise-gate",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Noise configure\n"
		"Noise Gate\n")
{
	int ret = ERROR;
	if(voip_stream->enable)
		ret = voip_stream_noise_gate_api(voip_stream, FALSE, voip_stream->ng_floorgain, voip_stream->ng_threshold);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_noise_gate_floorgain,
		no_ip_voip_noise_gate_floorgain_cmd,
		"ip voip noise-gate floorgain",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Noise Gate configure\n"
		"gain applied to the signal when its energy is below the threshold\n")
{
	int ret = ERROR;
	float ng_floorgain = 0.0;
	if(voip_stream->enable)
	{
		ret = voip_stream_noise_gate_api(voip_stream, TRUE, ng_floorgain, voip_stream->ng_threshold);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_noise_gate_threshold,
		no_ip_voip_noise_gate_threshold_cmd,
		"ip voip noise-gate threshold",
		NO_STR
		IP_STR
		"VOIP Protocol\n"
		"Noise Gate configure\n"
		"Noise Gate threshold\n")
{
	int ret = ERROR;
	float ng_threshold = 0.0;
	if(voip_stream->enable)
	{
		ret = voip_stream_noise_gate_api(voip_stream, TRUE, voip_stream->ng_floorgain, ng_threshold);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * RTP Module
 */

DEFUN (ip_voip_port,
		ip_voip_port_cmd,
		"ip voip port <512-65535>",
		IP_STR
		"VOIP RTP port\n"
		"port number\n")
{
	int ret = ERROR;
	ret = voip_local_rtp_set_api(0, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_voip_port,
		no_ip_voip_port_cmd,
		"no ip void port",
		NO_STR
		IP_STR
		"RTP port\n")
{
	int ret = ERROR;
	ret = voip_local_rtp_set_api(0, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}




/*
 * Ring Module
 */

DEFUN (voip_ring_set,
		voip_ring_set_cmd,
		"voip call-ring <1-16>",
		"Voip Configure\n"
		"Call Ring Configure\n"
		"Ring ID (default:1)\n")
{
	int ret = ERROR;
	if(voip_call_ring_lookup_api(atoi(argv[0])) != OK)
	{
		vty_out(vty, " This Ring ID is not Exist.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = voip_call_ring_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_voip_ring_set,
		no_voip_ring_set_cmd,
		"no voip call-ring",
		NO_STR
		"Voip Configure\n"
		"Call Ring Configure\n"
		"Ring ID\n")
{
	int ret = ERROR;
	ret = voip_call_ring_set_api(1);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (voip_ring_test,
		voip_ring_test_cmd,
		"voip call-ring test <1-16>",
		"Voip Configure\n"
		"Call Ring Configure\n"
		"Test Call Ring Sound\n"
		"Ring ID (default:1)\n")
{
	int ret = ERROR;
	if(voip_call_ring_lookup_api(atoi(argv[0])) != OK)
	{
		vty_out(vty, " This Ring ID is not Exist.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	//ret = voip_call_ring_stop(atoi(argv[0]));
	//ret = voip_call_ring_start(atoi(argv[0]));
	//ring_test();
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


/*
 * statistics Module
 */
DEFUN (show_voip_statistics,
		show_voip_statistics_cmd,
		"show voip (rtp-stats|bandwidth|quality)",
		SHOW_STR
		"Voip Configure\n"
		"RTP statistics information\n"
		"Bandwidth information\n"
		"Quality information\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "rtp-stats"))
		ret = voip_rtp_stats_display(vty);
	else if(strstr(argv[0], "bandwidth"))
		ret = voip_bandwidth_display(vty);
	else if(strstr(argv[0], "quality"))
		ret = voip_quality_display(vty);
	return  CMD_SUCCESS;
}

/*
 * Dbtest Module
 */
DEFUN (show_voip_dbtest,
		show_voip_dbtest_cmd,
		"show voip dbtest",
		SHOW_STR
		"Voip Configure\n"
		"Data Base information\n")
{
	int ret = ERROR;
	ret = voip_dbtest_show(vty, 0);
	return  CMD_SUCCESS;
}
/*
 * Sound Module
 */

/*
 * Playback
 */
DEFUN (voip_playback_volume,
		voip_playback_volume_cmd,
		"voip playback volume <0-100>",
		"VOIP Configure\n"
		"Playback configure\n"
		"Volume Configure\n"
		"volume value in percent\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		ret = voip_playback_volume_out_set_api(atoi(argv[0]));
	}
	else
	{
		if(strstr(argv[0], "stereo"))
			ret = voip_playback_volume_dac_set_api(atoi(argv[1]));
		else
			ret = voip_playback_volume_mono_set_api(atoi(argv[1]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(voip_playback_volume,
		voip_playback_mono_volume_cmd,
		"voip playback volume (stereo|mono) <0-100>",
		"VOIP Configure\n"
		"Playback configure\n"
		"Volume Configure\n"
		"Stereo Configure\n"
		"Mono Configure\n"
		"volume value in percent\n");

DEFUN (no_voip_playback_volume,
		no_voip_playback_volume_cmd,
		"no voip playback volume",
		NO_STR
		"VOIP Configure\n"
		"Playback configure\n"
		"Volume Configure\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		ret = voip_playback_volume_out_set_api(0);
	}
	else
	{
		if(strstr(argv[0], "stereo"))
			ret = voip_playback_volume_dac_set_api(0);
		else
			ret = voip_playback_volume_mono_set_api(0);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_voip_playback_volume,
		no_voip_playback_mono_volume_cmd,
		"no voip playback volume (stereo|mono)",
		NO_STR
		"VOIP Configure\n"
		"Playback configure\n"
		"Volume Configure\n"
		"Stereo Configure\n"
		"Mono Configure\n");

/*
 * Capture
 */
DEFUN (voip_capture_volume,
		voip_capture_volume_cmd,
		"voip capture volume <0-100>",
		"VOIP Configure\n"
		"Capture configure\n"
		"Volume Configure\n"
		"volume value in percent\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		ret = voip_capture_volume_in_set_api(atoi(argv[0]));
	}
	else
	{
		if(strstr(argv[0], "stereo"))
			ret = voip_capture_volume_adc_set_api(atoi(argv[1]));
		else
			ret = voip_capture_volume_mono_set_api(atoi(argv[1]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(voip_capture_volume,
		voip_capture_mono_volume_cmd,
		"voip capture volume (stereo|mono) <0-100>",
		"VOIP Configure\n"
		"Capture configure\n"
		"Volume Configure\n"
		"Stereo Configure\n"
		"Mono Configure\n"
		"volume value in percent\n");

DEFUN (no_voip_capture_volume,
		no_voip_capture_volume_cmd,
		"no voip capture volume",
		NO_STR
		"VOIP Configure\n"
		"Capture configure\n"
		"Volume Configure\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		ret = voip_capture_volume_in_set_api(0);
	}
	else
	{
		if(strstr(argv[0], "stereo"))
			ret = voip_capture_volume_adc_set_api(0);
		else
			ret = voip_capture_volume_mono_set_api(0);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_voip_capture_volume,
		no_voip_capture_mono_volume_cmd,
		"no voip capture volume (stereo|mono)",
		NO_STR
		"VOIP Configure\n"
		"Capture configure\n"
		"Volume Configure\n"
		"Stereo Configure\n"
		"Mono Configure\n");


DEFUN (voip_capture_boost,
		voip_capture_boost_cmd,
		"voip capture boost <0-8>",
		"VOIP Configure\n"
		"Capture configure\n"
		"boost configure\n"
		"boost value\n")
{
	int ret = ERROR;
	ret = voip_volume_boost_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_voip_capture_boost,
		no_voip_capture_boost_cmd,
		"no voip capture boost",
		"VOIP Configure\n"
		"Capture configure\n"
		"boost configure\n")
{
	int ret = ERROR;
	ret = voip_volume_boost_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (voip_capture_boost_gain,
		voip_capture_boost_gain_cmd,
		"voip capture boost-gain <0-3>",
		"VOIP Configure\n"
		"Capture configure\n"
		"boost gain configure\n"
		"boost value\n")
{
	int ret = ERROR;
	ret = voip_volume_boost_gain_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_voip_capture_boost_gain,
		no_voip_capture_boost_gain_cmd,
		"no voip capture boost-cain",
		"VOIP Configure\n"
		"Capture configure\n"
		"boost gain configure\n")
{
	int ret = ERROR;
	ret = voip_volume_boost_gain_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



/*
 * debug Module
 */

DEFUN (debug_voip_stream,
		debug_voip_stream_cmd,
		"debug voip (all|trace|message|warning|error|fatal)",
		DEBUG_STR
		"VOIP Configure\n"
		"All Information\n"
		"Trace Information\n"
		"Message Information\n"
		"Warning Information\n"
		"Error Information\n"
		"Fatal Information\n")
{
	voip_stream_debug_set_api(TRUE, argv[0]);
	return  CMD_SUCCESS;
}


/*
 * dbtest Module
 */

DEFUN (voip_dbtest_enable_c,
		voip_dbtest_enable_c_cmd,
		"voip dbtest(enable|disable)",
		"VOIP Configure\n"
		"DBtest Information\n"
		"Enable Information\n"
		"Disable Information\n")
{
	if(strstr(argv[0], "enable"))
	{
		voip_dbtest_enable(TRUE);
	}
	else
		voip_dbtest_enable(FALSE);
	return  CMD_SUCCESS;
}

DEFUN (show_voip_dbtest_information,
		show_voip_dbtest_information_cmd,
		"show voip dbtest",
		SHOW_STR
		"VOIP information\n"
		"DBtest Information\n")
{
	voip_dbtest_show(vty, 0);
	return  CMD_SUCCESS;
}



/*
 * show
 */
DEFUN (show_voip_information,
		show_voip_information_cmd,
		"show voip information",
		SHOW_STR
		"VOIP information\n"
		"Information\n")
{
	voip_show_config(vty, 0);
	return  CMD_SUCCESS;
}


static int voip_show_config(struct vty *vty, int detail)
{
	//u_int8 value = 0;
	if(voip_stream_is_enable())
	{
		vty_out(vty, " Service VOIP:%s", VTY_NEWLINE);
		voip_stream_show_config(vty);
		//voip_volume_show_config(vty, 0);
	}
	return 1;
}





static int voip_write_config(struct vty *vty)
{
	//u_int8 value = 0;
	if(voip_stream_is_enable())
	{
		vty_out(vty, "service voip%s", VTY_NEWLINE);

		voip_stream_write_config(vty);
		voip_volume_write_config(vty);
	}
	return 1;
}



static void cmd_voip_base_init(int node)
{
	/*
	 * rtp
	 */
	install_element(node, &ip_voip_port_cmd);
	install_element(node, &no_ip_voip_port_cmd);
	/*
	 * voip
	 */
	install_element(node, &ip_voip_payload_cmd);
	install_element(node, &no_ip_voip_payload_cmd);

	install_element(node, &ip_voip_echo_canceller_cmd);
	install_element(node, &no_ip_voip_echo_canceller_cmd);

	install_element(node, &ip_voip_echo_canceller_delay_cmd);
	install_element(node, &no_ip_voip_echo_canceller_delay_cmd);

	install_element(node, &ip_voip_echo_canceller_tail_cmd);
	install_element(node, &no_ip_voip_echo_canceller_tail_cmd);

	install_element(node, &ip_voip_echo_canceller_framesize_cmd);
	install_element(node, &no_ip_voip_echo_canceller_framesize_cmd);


	install_element(node, &ip_voip_echo_limiter_cmd);
	install_element(node, &no_ip_voip_echo_limiter_cmd);

	install_element(node, &ip_voip_echo_limiter_speed_cmd);
	install_element(node, &no_ip_voip_echo_limiter_speed_cmd);

	install_element(node, &ip_voip_echo_limiter_thres_cmd);
	install_element(node, &no_ip_voip_echo_limiter_thres_cmd);

	install_element(node, &ip_voip_echo_limiter_force_cmd);
	install_element(node, &no_ip_voip_echo_limiter_force_cmd);

	install_element(node, &ip_voip_echo_limiter_transmit_cmd);
	install_element(node, &no_ip_voip_echo_limiter_transmit_cmd);

	install_element(node, &ip_voip_echo_limiter_sustain_cmd);
	install_element(node, &no_ip_voip_echo_limiter_sustain_cmd);

	install_element(node, &ip_voip_bitrate_cmd);
	install_element(node, &no_ip_voip_bitrate_cmd);

	install_element(node, &ip_voip_jitter_cmd);
	install_element(node, &no_ip_voip_jitter_cmd);

	install_element(node, &ip_voip_noise_gate_cmd);
	install_element(node, &no_ip_voip_noise_gate_cmd);

	install_element(node, &ip_voip_noise_gate_floorgain_cmd);
	install_element(node, &no_ip_voip_noise_gate_floorgain_cmd);

	install_element(node, &ip_voip_noise_gate_threshold_cmd);
	install_element(node, &no_ip_voip_noise_gate_threshold_cmd);
	/*
	 * ring
	 */
	install_element(node, &voip_ring_set_cmd);
	install_element(node, &no_voip_ring_set_cmd);


	/*
	 * sound
	 */
	install_element(node, &voip_playback_volume_cmd);
	install_element(node, &voip_playback_mono_volume_cmd);

	install_element(node, &no_voip_playback_volume_cmd);
	install_element(node, &no_voip_playback_mono_volume_cmd);

	install_element(node, &voip_capture_volume_cmd);
	install_element(node, &voip_capture_mono_volume_cmd);

	install_element(node, &no_voip_capture_volume_cmd);
	install_element(node, &no_voip_capture_mono_volume_cmd);

	install_element(node, &voip_capture_boost_cmd);
	install_element(node, &no_voip_capture_boost_cmd);

	install_element(node, &voip_capture_boost_gain_cmd);
	install_element(node, &no_voip_capture_boost_gain_cmd);
}

static void cmd_voip_other_init(int node)
{
/*
#ifdef VOIP_STREAM_DEBUG_TEST
	install_element(node, &voip_start_stream_test_cmd);
	install_element(node, &voip_start_stream_test_port_cmd);
	install_element(node, &voip_start_stream_test_port_local_cmd);
	install_element(node, &voip_stop_stream_test_cmd);
#endif
#ifdef VOIP_STREAM_API_DEBUG_TEST
	install_element(node, &voip_start_test_cmd);
	install_element(node, &voip_start_test_port_cmd);
	install_element(node, &voip_start_test_port_local_cmd);
	install_element(node, &voip_stop_test_cmd);
#endif
*/
	install_element(node, &debug_voip_stream_cmd);

	install_element(node, &voip_ring_test_cmd);
/*
#ifdef VOIP_APP_DEBUG
	install_element(node, &voip_call_test_cmd);
	install_element(node, &voip_call_test_port_cmd);
	install_element(node, &voip_call_test_port_local_cmd);
#endif

#ifdef VOIP_STARTUP_TEST
	install_element(node, &voip_startup_test_cmd);
#endif
*/
}

static void cmd_voip_show_init(int node)
{
	install_element(node, &show_voip_statistics_cmd);
	install_element(node, &show_voip_dbtest_cmd);

	install_element(node, &show_voip_information_cmd);
	install_element(node, &show_voip_dbtest_information_cmd);
}

void cmd_voip_init(void)
{
	cmd_sip_init();

	install_default(VOIP_SERVICE_NODE);
	install_default_basic(VOIP_SERVICE_NODE);

	reinstall_node(VOIP_SERVICE_NODE, voip_write_config);

	install_element(CONFIG_NODE, &ip_voip_service_cmd);
	install_element(CONFIG_NODE, &no_ip_voip_service_cmd);

	cmd_voip_base_init(VOIP_SERVICE_NODE);

	cmd_voip_show_init(VOIP_SERVICE_NODE);
	cmd_voip_show_init(ENABLE_NODE);
	cmd_voip_show_init(CONFIG_NODE);

	cmd_voip_other_init(ENABLE_NODE);

	cmd_voip_test_init(ENABLE_NODE);

	install_element(ENABLE_NODE, &voip_dbtest_enable_c_cmd);
}
