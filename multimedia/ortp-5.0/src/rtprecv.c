/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of oRTP.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include <ortp/ortp.h>
//#include <bctoolbox/vfs.h>
#include <signal.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#endif


void ssrc_cb(RtpSession *session_test)
{
	printf("hey, the ssrc has changed !\n");
}

#define DefaultTimestampIncrement 90000/30 //(90000/25)

int rtpmain_test()
{
	RtpSession *session_test;
	unsigned char buffer[1500];
	int err;
	uint32_t ts=0;
	int stream_received=0;
	int local_port = 9988;
	int have_more = 0;
	bool_t adapt=TRUE;
	int jittcomp=40;
	ortp_init();
	ortp_scheduler_init();
	ortp_set_log_level_mask(ORTP_DEBUG|ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);

	session_test=rtp_session_new(RTP_SESSION_RECVONLY);	
	rtp_session_set_scheduling_mode(session_test,1);
	rtp_session_set_blocking_mode(session_test,1);
	rtp_session_set_local_addr(session_test,"127.0.0.1",local_port,local_port + 1);
	rtp_session_set_connected_mode(session_test,TRUE);
	rtp_session_set_symmetric_rtp(session_test,TRUE);
	rtp_session_enable_adaptive_jitter_compensation(session_test, adapt);
	rtp_session_set_jitter_compensation(session_test,jittcomp);
	rtp_session_set_payload_type(session_test, 96);
	rtp_session_signal_connect(session_test,"ssrc_changed",(RtpCallback)ssrc_cb,0);
	rtp_session_signal_connect(session_test,"ssrc_changed",(RtpCallback)rtp_session_reset,0);
	
	while(1)
	{
		have_more=1;
		while (have_more){
			err=rtp_session_recv_with_ts(session_test, buffer, 1400, ts, &have_more);
			if (err>0) 
				stream_received=1;
			/* this is to avoid to write to disk some silence before the first RTP packet is returned*/	
			if ((stream_received) && (err>0)) {
				fprintf(stderr,"write to sound (%d)\r\n", err);
				fflush(stderr);
			}
		}
		ts+=DefaultTimestampIncrement;
		//ortp_message("Receiving packet.");
	}
	
	rtp_session_destroy(session_test);
	ortp_exit();
	
	ortp_global_stats_display();
	
	return 0;
}
