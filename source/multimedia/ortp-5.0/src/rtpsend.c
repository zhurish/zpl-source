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
#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "vty_include.h"
#include <ortp/ortp.h>
#include <signal.h>
#include <stdlib.h>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#endif

int runcond = 1;
static RtpSession *session = NULL;
void stophandler(int signum)
{
	runcond = 0;
}

static const char *help = "usage: rtpsend	filename dest_ip4addr dest_port [ --with-clockslide <value> ] [ --with-jitter <milliseconds>]\n";

int rtpsend_main(int argc, char *argv[])
{
	//RtpSession *session;
	unsigned char buffer[160];
	int i;
	FILE *infile;
	char *ssrc;
	uint32_t user_ts = 0;
	int clockslide = 0;
	int jitter = 0;
	if (argc < 4)
	{
		printf("%s", help);
		return -1;
	}
	for (i = 4; i < argc; i++)
	{
		if (strcmp(argv[i], "--with-clockslide") == 0)
		{
			i++;
			if (i >= argc)
			{
				printf("%s", help);
				return -1;
			}
			clockslide = atoi(argv[i]);
			ortp_message("Using clockslide of %i milisecond every 50 packets.", clockslide);
		}
		else if (strcmp(argv[i], "--with-jitter") == 0)
		{
			ortp_message("Jitter will be added to outgoing stream.");
			i++;
			if (i >= argc)
			{
				printf("%s", help);
				return -1;
			}
			jitter = atoi(argv[i]);
		}
	}

	ortp_init();
	ortp_scheduler_init();
	ortp_set_log_level_mask(ORTP_MESSAGE | ORTP_WARNING | ORTP_ERROR);
	session = rtp_session_new(RTP_SESSION_SENDONLY);

	rtp_session_set_scheduling_mode(session, 1);
	rtp_session_set_blocking_mode(session, 1);
	rtp_session_set_connected_mode(session, TRUE);
	rtp_session_set_remote_addr(session, argv[2], atoi(argv[3]));
	rtp_session_set_payload_type(session, 96);

	ssrc = getenv("SSRC");
	if (ssrc != NULL)
	{
		printf("using SSRC=%i.\n", atoi(ssrc));
		rtp_session_set_ssrc(session, atoi(ssrc));
	}

#ifndef _WIN32
	infile = fopen(argv[1], "r");
#else
	infile = fopen(argv[1], "rb");
#endif

	if (infile == NULL)
	{
		perror("Cannot open file");
		return -1;
	}

	signal(SIGINT, stophandler);
	while (((i = fread(buffer, 1, 160, infile)) > 0) && (runcond))
	{
		rtp_session_send_with_ts(session, buffer, i, user_ts, 0);
		user_ts += 160;
		if (clockslide != 0 && user_ts % (160 * 50) == 0)
		{
			ortp_message("Clock sliding of %i miliseconds now", clockslide);
			rtp_session_make_time_distorsion(session, clockslide);
		}
		/*this will simulate a burst of late packets */
		if (jitter && (user_ts % (8000) == 0))
		{
			ortp_message("Simulating late packets now (%i milliseconds)", jitter);
			ortp_sleep_ms(jitter);
		}
	}

	fclose(infile);
	rtp_session_destroy(session);
	ortp_exit();
	ortp_global_stats_display();

	return 0;
}

#define MAX_RTP_PKT_LENGTH 1400
//#define DefaultTimestampIncrement 3600 //(90000/25)
#define DefaultTimestampIncrement 90000/30 //(90000/25)

uint32_t g_userts = 0;



static int zpl_media_rtp_task(void* argv)
{
	char buf[2000];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 1;
	host_config_load_waitting();
    os_sleep(1);
		char *ssrc = NULL;
		rtp_session_set_scheduling_mode(session, 1);
		rtp_session_set_blocking_mode(session, 1);
		rtp_session_set_connected_mode(session, TRUE);
		rtp_session_set_remote_addr_and_port(session, "127.0.0.1", 9988, 9988+1);
		rtp_session_set_payload_type(session, 96);
		ssrc = getenv("SSRC");
		if (ssrc != NULL)
		{
			printf("using SSRC=%i.\n", atoi(ssrc));
			rtp_session_set_ssrc(session, atoi(ssrc));
		}
		else
			rtp_session_set_ssrc(session, rand());
	while(1)
	{
		ortp_create_send(buf, 2000);
	}
	return 0;
}
int ortp_create_init()
{
	//char *ssrc = NULL;
	ortp_init();
	ortp_scheduler_init();
	//av_profile_init(&av_profile);
	ortp_set_log_level_mask(ORTP_MESSAGE | ORTP_WARNING | ORTP_ERROR);
	session = rtp_session_new(RTP_SESSION_SENDONLY);
	if (session)
	{
		/*
		rtp_session_set_scheduling_mode(session, 1);
		rtp_session_set_blocking_mode(session, 1);
		rtp_session_set_connected_mode(session, TRUE);
		rtp_session_set_remote_addr(session, "127.0.0.1", "9988");
		rtp_session_set_payload_type(session, 96);
		ssrc = getenv("SSRC");
		if (ssrc != NULL)
		{
			printf("using SSRC=%i.\n", atoi(ssrc));
			rtp_session_set_ssrc(session, atoi(ssrc));
		}
		else
			rtp_session_set_ssrc(session, rand());
*/


    	os_task_create("rtpsTask", OS_TASK_DEFAULT_PRIORITY,
								 0, zpl_media_rtp_task, NULL, OS_TASK_DEFAULT_STACK);
		return 0;
	}
	return -1;
}

int ortp_create_exit()
{
	if (session)
	{
		rtp_session_destroy(session);
		ortp_exit();
		ortp_global_stats_display();
	}
	return 0;
}

int ortp_create_send(char *buffer, int len)
{
	int sendBytes = 0;
	int status;
	uint32_t valid_len = len - 4;
	unsigned char NALU = buffer[4];

	//printf("send len=%d\n",len);
	if (session)
	{
		//如果数据小于MAX_RTP_PKT_LENGTH字节，直接发送：单一NAL单元模式
		if (valid_len <= MAX_RTP_PKT_LENGTH)
		{
			sendBytes = rtp_session_send_with_ts(session,
												 &buffer[4],
												 valid_len,
												 g_userts, 0);
			return sendBytes;
		}
		else if (valid_len > MAX_RTP_PKT_LENGTH)
		{
			//切分为很多个包发送，每个包前要对头进行处理，如第一个包
			valid_len -= 1;
			int k = 0, l = 0;
			k = valid_len / MAX_RTP_PKT_LENGTH;
			l = valid_len % MAX_RTP_PKT_LENGTH;
			int t = 0;
			int pos = 5;
			if (l != 0)
			{
				k = k + 1;
			}
			while (t < k) //||(t==k&&l>0))
			{
				if (t < (k - 1)) //(t<k&&l!=0)||(t<(k-1))&&(l==0))//(0==t)||(t<k&&0!=l))
				{
					buffer[pos - 2] = (NALU & 0x60) | 28;
					buffer[pos - 1] = (NALU & 0x1f);
					if (0 == t)
					{
						buffer[pos - 1] |= 0x80;
					}
					sendBytes = rtp_session_send_with_ts(session,
														 &buffer[pos - 2],
														 MAX_RTP_PKT_LENGTH + 2,
														 g_userts, 0);
					t++;
					pos += MAX_RTP_PKT_LENGTH;
				}
				else //if((k==t&&l>0)||((t==k-1)&&l==0))
				{
					int iSendLen;
					if (l > 0)
					{
						iSendLen = valid_len - t * MAX_RTP_PKT_LENGTH;
					}
					else
						iSendLen = MAX_RTP_PKT_LENGTH;
					buffer[pos - 2] = (NALU & 0x60) | 28;
					buffer[pos - 1] = (NALU & 0x1f);
					buffer[pos - 1] |= 0x40;
					sendBytes = rtp_session_send_with_ts(session,
														 &buffer[pos - 2],
														 iSendLen + 2,
														 g_userts, 0);
					t++;
				}
			}
		}

		g_userts += DefaultTimestampIncrement; //timestamp increase
		return len;
	}
	return -1;
}