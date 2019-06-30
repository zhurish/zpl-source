/*
 * voip_statistics.c
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
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
#include "vty.h"

#include "voip_def.h"
#include "voip_task.h"
#include "voip_statistics.h"
#include "voip_stream.h"
#include "voip_app.h"


//#define PRId64       "i64d"
#define PRId64       "d"
#ifdef PL_VOIP_MEDIASTREAM
static int _voip_rtp_stats_display(struct vty *vty, const rtp_stats_t *stats, const char *header)
{
	zassert(vty != NULL);
	zassert(stats != NULL);
	//vty_out(vty, "===========================================================%s", VTY_NEWLINE);
	if(header)
		vty_out(vty, " %s:%s", header, VTY_NEWLINE);
	//vty_out(vty, "-----------------------------------------------------------%s", VTY_NEWLINE);
	vty_out(vty, "  sent                          %20"PRId64" packets%s", stats->packet_sent, VTY_NEWLINE);
	vty_out(vty, "                                %20"PRId64" bytes  %s", stats->sent, VTY_NEWLINE);
	vty_out(vty, "  received                      %20"PRId64" packets%s", stats->packet_recv, VTY_NEWLINE);
	vty_out(vty, "                                %20"PRId64" bytes  %s", stats->hw_recv, VTY_NEWLINE);
	vty_out(vty, "  incoming delivered to the app %20"PRId64" bytes  %s", stats->recv, VTY_NEWLINE);
	vty_out(vty, "  lost                          %20"PRId64" packets%s", stats->cum_packet_loss, VTY_NEWLINE);
	vty_out(vty, "  received too late             %20"PRId64" packets%s", stats->outoftime, VTY_NEWLINE);
	vty_out(vty, "  bad formatted                 %20"PRId64" packets%s", stats->bad, VTY_NEWLINE);
	vty_out(vty, "  discarded (queue overflow)    %20"PRId64" packets%s", stats->discarded, VTY_NEWLINE);
	//vty_out(vty, "===========================================================");
	vty_out(vty, "%s", VTY_NEWLINE);
	return OK;
}
#endif

int voip_rtp_stats_display(struct vty *vty)
{
#ifdef PL_VOIP_MEDIASTREAM
	voip_stream_t *args = voip_lookup_stream_api();
	if(args && args->mediastream && args->mediastream->session)
		return _voip_rtp_stats_display(vty, rtp_session_get_stats(args->mediastream->session),"RTP stats");
#endif
	return OK;
}

int voip_quality_display(struct vty *vty)
{
#ifdef PL_VOIP_MEDIASTREAM
	voip_stream_t *args = voip_lookup_stream_api();

	if(args && args->mediastream && args->mediastream->session)
	{
		float audio_load = 0.0;
		float video_load = 0.0;
		float video_1uality = 0.0;
		if (args->mediastream->audio) {
			audio_load = ms_ticker_get_average_load(args->mediastream->audio->ms.sessions.ticker);
			video_1uality = audio_stream_get_quality_rating(args->mediastream->audio);
		}
#if defined(VIDEO_ENABLED)
		if (args->mediastream->video) {
			video_load = ms_ticker_get_average_load(args->mediastream->video->ms.sessions.ticker);
		}
#endif
		vty_out(vty," Voip Thread processing load:%s", VTY_NEWLINE);
		vty_out(vty,"   audio         %f%s"
					"   video         %f%s",
					audio_load, VTY_NEWLINE,
					video_load, VTY_NEWLINE);

		vty_out(vty," Quality indicator : %f%s", video_1uality, VTY_NEWLINE);
		return OK;
		//return _voip_rtp_stats_display(vty, rtp_session_get_stats(args->session),"RTP stats");
	}
#endif
	return OK;
}

int voip_bandwidth_display(struct vty *vty)
{
#ifdef PL_VOIP_MEDIASTREAM
	voip_stream_t *args = voip_lookup_stream_api();

	if(args && args->mediastream && args->mediastream->session)
	{

		vty_out(vty," Voip Bandwidth usage:%s", VTY_NEWLINE);
		vty_out(vty,"   download       : %f kbits/sec%s"
					"   upload         : %f kbits/sec%s",
						rtp_session_get_recv_bandwidth(args->mediastream->session)*1e-3, VTY_NEWLINE,
						rtp_session_get_send_bandwidth(args->mediastream->session)*1e-3, VTY_NEWLINE);
		return OK;
		//return _voip_rtp_stats_display(vty, rtp_session_get_stats(args->session),"RTP stats");
	}
#endif
	return OK;
}
