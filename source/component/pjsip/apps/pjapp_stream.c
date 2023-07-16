/* $Id$ */
/*
 * Copyright (C) 2011 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \page page_pjmedia_samples_vid_streamutil_c Samples: Video Streaming
 *
 * This example mainly demonstrates how to stream video to remote
 * peer using RTP.
 *
 * This file is pjsip-apps/src/samples/vid_streamutil.c
 *
 * \includelineno vid_streamutil.c
 */

#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>
#include <pjapp_stream.h>
#include <pjapp_media_file.h>

#define THIS_FILE "pjapp_media_stream.c"

static void print_stream_stat(pjmedia_stream *stream,
                              const pjmedia_codec_param *codec_param);

/* Util to display the error message for the specified error code  */
int pjapp_app_perror(const char *sender, const char *title,
                      pj_status_t status)
{
    char errmsg[PJ_ERR_MSG_SIZE];

    pj_strerror(status, errmsg, sizeof(errmsg));

    PJ_LOG(3, (sender, "%s: %s [code=%d]", title, errmsg, status));
    return 1;
}


static pj_status_t pjapp_ms_transport_create(pj_pool_t *pool, pjmedia_endpt *med_endpt, pjapp_ms_param_t *cfg, pjmedia_transport **transport)
{
	pj_status_t status = -1;
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	pjmedia_transport *srtp_tp = NULL;
#endif
	if (cfg->mcast)
	{
		pjmedia_sock_info si;
		int reuse = 1;
		unsigned char loop = 0;
		struct pj_ip_mreq imr;
		pj_bzero(&si, sizeof(pjmedia_sock_info));
		si.rtp_sock = si.rtcp_sock = PJ_INVALID_SOCKET;

		/* Create RTP socket */
		status = pj_sock_socket(pj_AF_INET(), pj_SOCK_DGRAM(), 0,
								&si.rtp_sock);
		if (status != PJ_SUCCESS)
			return status;

		status = pj_sock_setsockopt(si.rtp_sock, pj_SOL_SOCKET(),
									pj_SO_REUSEADDR(), &reuse, sizeof(reuse));
		if (status != PJ_SUCCESS)
			return status;

		/* Bind RTP socket */
        pj_sockaddr_cp(&si.rtp_addr_name, &cfg->local_addr);
		status = pj_sock_bind(si.rtp_sock, &si.rtp_addr_name,
							  pj_sockaddr_get_len(&si.rtp_addr_name));
		if (status != PJ_SUCCESS)
			return status;

		/* Create RTCP socket */
		status = pj_sock_socket(pj_AF_INET(), pj_SOCK_DGRAM(), 0,
								&si.rtcp_sock);
		if (status != PJ_SUCCESS)
			return status;

		status = pj_sock_setsockopt(si.rtcp_sock, pj_SOL_SOCKET(),
									pj_SO_REUSEADDR(), &reuse, sizeof(reuse));
		if (status != PJ_SUCCESS)
			return status;

		/* Bind RTCP socket */

		pj_sockaddr_cp(&si.rtcp_addr_name, &cfg->local_addr);
        pj_sockaddr_set_port(&si.rtcp_addr_name,
                            pj_sockaddr_get_port(&si.rtcp_addr_name)+1);

		status = pj_sock_bind(si.rtcp_sock, &si.rtcp_addr_name,
							  pj_sockaddr_get_len(&si.rtcp_addr_name));
		if (status != PJ_SUCCESS)
			return status;

		pj_memset(&imr, 0, sizeof(struct pj_ip_mreq));
		imr.imr_multiaddr.s_addr = cfg->mcast_addr.sin_addr.s_addr;
		imr.imr_interface.s_addr = pj_htonl(PJ_INADDR_ANY);
		status = pj_sock_setsockopt(si.rtp_sock, pj_SOL_IP(),
									pj_IP_ADD_MEMBERSHIP(),
									&imr, sizeof(struct pj_ip_mreq));
		if (status != PJ_SUCCESS)
			return status;

		status = pj_sock_setsockopt(si.rtcp_sock, pj_SOL_IP(),
									pj_IP_ADD_MEMBERSHIP(),
									&imr, sizeof(struct pj_ip_mreq));
		if (status != PJ_SUCCESS)
			return status;

		/* Disable local reception of local sent packets */
		loop = 0;
		pj_sock_setsockopt(si.rtp_sock, pj_SOL_IP(),
						   pj_IP_MULTICAST_LOOP(), &loop, sizeof(loop));
		pj_sock_setsockopt(si.rtcp_sock, pj_SOL_IP(),
						   pj_IP_MULTICAST_LOOP(), &loop, sizeof(loop));

		/* Create media transport from existing sockets */
		status = pjmedia_transport_udp_attach(med_endpt, NULL, &si,
											  PJMEDIA_UDP_NO_SRC_ADDR_CHECKING, transport);
		if (status != PJ_SUCCESS)
			return status;
	}
	else
	{
		/* Create media transport */
        pj_str_t local_addr = pj_str(pj_inet_ntoa(cfg->local_addr.sin_addr));
        status = pjmedia_transport_udp_create2(med_endpt, NULL, &local_addr, ntohs(cfg->local_addr.sin_port), 0, transport);
		//status = pjmedia_transport_udp_create(med_endpt, NULL, ntohs(cfg->local_addr.sin_port), 0, transport);
		if (status != PJ_SUCCESS)
			return status;
	}

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	/* Check if SRTP enabled */
	if (cfg->use_srtp)
	{
		status = pjmedia_transport_srtp_create(med_endpt, *transport, NULL, &srtp_tp);
		if (status != PJ_SUCCESS)
			return status;

		if (cfg->is_dtls_client || cfg->is_dtls_server)
		{
			char fp[128];
			pj_size_t fp_len = sizeof(fp);
			pjmedia_srtp_dtls_nego_param dtls_param;

			pjmedia_transport_srtp_dtls_get_fingerprint(srtp_tp, "SHA-256", fp, &fp_len);
			PJ_LOG(3, (THIS_FILE, "Local cert fingerprint: %s", fp));

			pj_bzero(&dtls_param, sizeof(dtls_param));
			pj_sockaddr_cp(&dtls_param.rem_addr, &cfg->rem_addr);
			pj_sockaddr_cp(&dtls_param.rem_rtcp, &cfg->rem_addr);
			dtls_param.is_role_active = cfg->is_dtls_client;

			status = pjmedia_transport_srtp_dtls_start_nego(srtp_tp, &dtls_param);
			if (status != PJ_SUCCESS)
				return status;
		}
		else
		{
			pjmedia_srtp_crypto tx_plc, rx_plc;

			pj_bzero(&tx_plc, sizeof(pjmedia_srtp_crypto));
			pj_bzero(&rx_plc, sizeof(pjmedia_srtp_crypto));

			tx_plc.key = *cfg->srtp_tx_key;
			tx_plc.name = *cfg->crypto_suite;
			rx_plc.key = *cfg->srtp_rx_key;
			rx_plc.name = *cfg->crypto_suite;

			status = pjmedia_transport_srtp_start(srtp_tp, &tx_plc, &rx_plc);
			if (status != PJ_SUCCESS)
				return status;
		}
		*transport = srtp_tp;
	}
#endif
	return PJ_SUCCESS;
}

static pj_status_t pjapp_ms_create_stream_internel(pj_pool_t *pool, pjapp_ms_cfg_t *cfg, pjapp_ms_param_t *param)
{
#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
	pjmedia_vid_stream_info vinfo;
#endif
	pjmedia_stream_info info;
	pj_status_t status = -1;

#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
	if (cfg->type == PJMEDIA_TYPE_VIDEO)
	{
		/* Reset stream info. */
		pj_bzero(&vinfo, sizeof(vinfo));
		/* Initialize stream info formats */
		vinfo.type = PJMEDIA_TYPE_VIDEO;
		vinfo.dir = cfg->dir;//PJMEDIA_DIR_ENCODING;
		vinfo.codec_info = *cfg->video_codec_info;
		vinfo.tx_pt = (cfg->rtp_pt == -1) ? cfg->video_codec_info->pt : cfg->rtp_pt;
		vinfo.rx_pt = (cfg->rtp_pt == -1) ? cfg->video_codec_info->pt : cfg->rtp_pt;
		vinfo.ssrc = pj_rand();
		if (cfg->video_codec_param)
			vinfo.codec_param = cfg->video_codec_param;
    
		/* Copy remote address */
		pj_memcpy(&vinfo.rem_addr, &param->rem_addr, sizeof(pj_sockaddr_in));

		/* If remote address is not set, set to an arbitrary address
		* (otherwise stream will assert).
		*/
		if (vinfo.rem_addr.addr.sa_family == 0) {
			const pj_str_t addr = pj_str("127.0.0.1");
			pj_sockaddr_in_init(&vinfo.rem_addr.ipv4, &addr, 0);
		}
        pj_sockaddr_cp(&vinfo.rem_rtcp, &vinfo.rem_addr);
        pj_sockaddr_set_port(&vinfo.rem_rtcp,
                            pj_sockaddr_get_port(&vinfo.rem_rtcp)+1);
	}
#endif
	if (cfg->type == PJMEDIA_TYPE_AUDIO)
	{
		/* Reset stream info. */
		pj_bzero(&info, sizeof(info));

		/* Initialize stream info formats */
		info.type = PJMEDIA_TYPE_AUDIO;
		info.dir = cfg->dir;//PJMEDIA_DIR_ENCODING;
		pj_memcpy(&info.fmt, cfg->audio_codec_info, sizeof(pjmedia_codec_info));
		info.tx_pt = cfg->audio_codec_info->pt;
		info.rx_pt = cfg->audio_codec_info->pt;
		info.ssrc = pj_rand();

#if PJMEDIA_HAS_RTCP_XR && PJMEDIA_STREAM_ENABLE_XR
		/* Set default RTCP XR enabled/disabled */
		info.rtcp_xr_enabled = PJ_TRUE;
#endif
		/* Copy remote address */
		pj_memcpy(&info.rem_addr, &param->rem_addr, sizeof(pj_sockaddr_in));

		/* If remote address is not set, set to an arbitrary address
		* (otherwise stream will assert).
		*/
		if (info.rem_addr.addr.sa_family == 0)
		{
			const pj_str_t addr = pj_str("127.0.0.1");
			pj_sockaddr_in_init(&info.rem_addr.ipv4, &addr, 0);
		}

		pj_sockaddr_cp(&info.rem_rtcp, &info.rem_addr);
		pj_sockaddr_set_port(&info.rem_rtcp,
							pj_sockaddr_get_port(&info.rem_rtcp) + 1);
	}
	/* Now that the stream info is initialized, we can create the
	 * stream.
	 */
	if (cfg->type == PJMEDIA_TYPE_AUDIO)
		status = pjmedia_stream_create(cfg->med_endpt, pool, &info,
									   cfg->transport,
									   NULL, &cfg->audio_stream);
#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
	if (cfg->type == PJMEDIA_TYPE_VIDEO)
		status = pjmedia_vid_stream_create(cfg->med_endpt, pool, &vinfo,
										   cfg->transport,
										   NULL, &cfg->video_stream);
#endif

	if (status != PJ_SUCCESS)
	{
		pjapp_app_perror(THIS_FILE, "Error creating stream", status);
		return status;
	}

	/* Start media transport */
	pjmedia_transport_media_start(cfg->transport, 0, 0, 0, 0);

	return PJ_SUCCESS;
}

static int pjapp_media_stream_codecparam_set(pjapp_ms_cfg_t *cfg, char *codec_id, pjmedia_codec_fmtp *fmtp, pjmedia_rect_size vidsize)
{
	pj_status_t status;
	if (cfg->type == PJMEDIA_TYPE_AUDIO)
	{
        cfg->rtp_pt = -1;
        if (codec_id) {
            unsigned count = 1;
            pj_str_t str_codec_id = pj_str(codec_id);
            pjmedia_codec_mgr *codec_mgr = pjmedia_endpt_get_codec_mgr(cfg->med_endpt);
            /*
            pjmedia_codec_mgr_get_codec_info( pjmedia_codec_mgr *mgr,
                                  unsigned pt,
                                  const pjmedia_codec_info **p_info);
            PJ_DEF(char*) pjmedia_codec_info_to_id( const pjmedia_codec_info *info,
                                        char *id, unsigned max_len );
                                        */
            status = pjmedia_codec_mgr_find_codecs_by_id( codec_mgr,
                                                        &str_codec_id, &count,
                                                        &cfg->audio_codec_info, NULL);
            if (status != PJ_SUCCESS) {
                printf("Error: unable to find codec %s\n", codec_id);
                return 1;
            }
        } else {
            /* Default to pcmu */
            pjmedia_codec_mgr_get_codec_info(pjmedia_endpt_get_codec_mgr(cfg->med_endpt),
                                            0, &cfg->audio_codec_info);
        }
        cfg->rtp_pt = cfg->video_codec_info->pt;
        /* Get codec default param for info */
        status = pjmedia_codec_mgr_get_default_param(pjmedia_endpt_get_codec_mgr(cfg->med_endpt), 
                                    cfg->audio_codec_info, 
                                    &cfg->audio_codec_param);
        if (status != PJ_SUCCESS) {
            printf("Error: unable to find codec %s\n", codec_id);
            return 1;
        }                           
	}   
#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
	else if (cfg->type == PJMEDIA_TYPE_VIDEO)
	{
        cfg->rtp_pt = -1;
		if (codec_id)
		{
			unsigned count = 1;
			pj_str_t str_codec_id = pj_str(codec_id);
            /*
            pjmedia_vid_codec_info_to_id(
                                        const pjmedia_vid_codec_info *info,
                                        char *id, unsigned max_len );
            pjmedia_vid_codec_mgr_get_codec_info2(
                                    pjmedia_vid_codec_mgr *mgr,
                                    pjmedia_format_id fmt_id,
                                    const pjmedia_vid_codec_info **p_info);*/
			status = pjmedia_vid_codec_mgr_find_codecs_by_id(NULL,
															 &str_codec_id, &count,
															 &cfg->video_codec_info, NULL);
			if (status != PJ_SUCCESS)
			{
				printf("Error: unable to find codec %s\n", codec_id);
				return 1;
			}
		}
		else
		{
			static pjmedia_vid_codec_info info[1];
			unsigned count = PJ_ARRAY_SIZE(info);

			/* Default to first codec */
			pjmedia_vid_codec_mgr_enum_codecs(NULL, &count, info, NULL);
			cfg->video_codec_info = &info[0];
		}
		cfg->video_codec_param = &cfg->vidcodec_param;
        cfg->rtp_pt = cfg->video_codec_info->pt;
		/* Get codec default param for info */
		status = pjmedia_vid_codec_mgr_get_default_param(NULL, cfg->video_codec_info,
														 cfg->video_codec_param);
		pj_assert(status == PJ_SUCCESS);

		/* Set outgoing video size */
		if (vidsize.w && vidsize.h)
			cfg->video_codec_param->enc_fmt.det.vid.size = vidsize;

        if(fmtp && fmtp->cnt)
        {
            cfg->video_codec_param->enc_fmtp.cnt = fmtp->cnt;
            int i = 0;
            for(i = 0; i < fmtp->cnt; i++)
            {
                cfg->video_codec_param->enc_fmtp.param[i].name = pj_str((char*)fmtp->param[i].name.ptr);
                cfg->video_codec_param->enc_fmtp.param[i].val = pj_str((char*)fmtp->param[i].val.ptr);
            }
            /*
            cfg->video_codec_param->enc_fmtp.param[0].name = pj_str((char*)"profile-level-id");
            cfg->video_codec_param->enc_fmtp.param[0].val = pj_str((char*)"42e01e");
            cfg->video_codec_param->enc_fmtp.param[1].name = pj_str((char*)" packetization-mode");
            cfg->video_codec_param->enc_fmtp.param[1].val = pj_str((char*)"1");
            cfg->video_codec_param->enc_fmtp.param[2].name = pj_str((char*)" sprop-parameter-sets");
            cfg->video_codec_param->enc_fmtp.param[2].val = pj_str((char*)"1");
            */
        }
	}
#endif 
	return status;
}

pj_status_t pjapp_media_stream_set_address(pj_pool_t *pool, char *local_addr, pj_uint16_t local_port, char *remote_addr, pj_uint16_t remote_port, 
    pjapp_ms_t *cfg)
{
    pj_str_t addrstr = pj_str(local_addr);
    pj_sockaddr_in_init(&cfg->param.local_addr, &addrstr, local_port);
    addrstr = pj_str(remote_addr);
    pj_sockaddr_in_init(&cfg->param.rem_addr, &addrstr, remote_port);
    return 0;
}

pj_status_t pjapp_media_stream_set_srtp(pj_pool_t *pool, char *crypto_suite, char *srtp_tx_key, char *srtp_rx_key, 
    pj_bool_t dtls_c, pj_bool_t dtls_s, pjapp_ms_t *cfg)
{
    return 0;
}

/*
 * Create stream based on the codec, dir, remote address, etc.
 */
pjapp_ms_t * pjapp_media_stream_new(int type, char *local_addr, pj_uint16_t local_port, char *remote_addr, pj_uint16_t remote_port)
{
    pjapp_ms_t *ms = NULL;
    pj_status_t status = 0;
    pj_pool_t *pool = pjsua_pool_create("pjmyapp", 1000, 1000);
    if(pool)
    {
        ms = PJ_POOL_ALLOC_T(pool, pjapp_ms_t);
        if(ms)
        {
            pj_memset(ms, 0, sizeof(pjapp_ms_t));
            ms->cfg.type = type;
            ms->cfg.dir = PJMEDIA_DIR_ENCODING;
            if(pjsua_get_pjmedia_endpt())
                ms->cfg.med_endpt = pjsua_get_pjmedia_endpt();
            else
            {
                status = pjmedia_endpt_create(pool->factory, NULL, 1, &ms->cfg.med_endpt);
                if(status != PJ_SUCCESS)
                {
                    pj_pool_safe_release(&pool);
                    return NULL;
                }
            }
            if(ms->cfg.type == PJMEDIA_TYPE_AUDIO)
            {
                status = pjmedia_codec_register_audio_codecs(ms->cfg.med_endpt, NULL);
                if(status != PJ_SUCCESS)
                {
                    pj_pool_safe_release(&pool);
                    return NULL;
                }
            }
            ms->cfg.pool = pool;
            status = pjapp_media_stream_set_address(ms->cfg.pool, local_addr, local_port, remote_addr, remote_port, ms);
            if(status != PJ_SUCCESS)
            {
                if(!pjsua_get_pjmedia_endpt())
                    pjmedia_endpt_destroy(ms->cfg.med_endpt);
                pj_pool_safe_release(&pool);
                return NULL;
            }
        }
    }
    return ms;    
}

/*
  [PCMU/8000/1]    Audio, prio: 132
  [iLBC/8000/1]    Audio, prio: 131
  [PCMA/8000/1]    Audio, prio: 130
  [speex/8000/1]   Audio, prio: 0
  [GSM/8000/1]     Audio, prio: 0
  [speex/16000/1]  Audio, prio: 0
  [speex/32000/1]  Audio, prio: 0
  [G722/16000/1]   Audio, prio: 0
  [G7221/16000/1]  Audio, prio: 0
  [G7221/16000/1]  Audio, prio: 0
  [G7221/32000/1]  Audio, prio: 0
  [G7221/32000/1]  Audio, prio: 0
  [G7221/32000/1]  Audio, prio: 0
  [L16/44100/1]    Audio, prio: 0
  [L16/44100/2]    Audio, prio: 0

H264/97         0   30.00  2560/2560  1920x1080
VP8/102         0   30.00  2560/2560  1920x1080
VP9/105         0   30.00  2560/2560  1920x1080
H263-1998/96    0   15.00   256/ 256  352x288
*/
pj_status_t pjapp_media_stream_codecparam(pjapp_ms_t *ms, char *codec, int w, int h, pjmedia_codec_fmtp *fmtp)
{
    pjmedia_rect_size vidsize;
    vidsize.h = h;
    vidsize.w = w;
    return pjapp_media_stream_codecparam_set(&ms->cfg, codec, fmtp, vidsize);
}

pj_status_t pjapp_media_stream_transport(pjapp_ms_t *ms)
{
    pj_status_t status;
    //status = pjapp_media_stream_set_srtp(ms->cfg.pool, crypto_suite, srtp_tx_key, srtp_rx_key, dtls_c,  dtls_s, ms); 
    status = pjapp_ms_transport_create(ms->cfg.pool, ms->cfg.med_endpt, &ms->param, &ms->cfg.transport);
    return status;
}

pj_status_t pjapp_media_stream_create(pjapp_ms_t *ms)
{
    pj_status_t status;
    status = pjapp_ms_create_stream_internel(ms->cfg.pool, &ms->cfg, &ms->param);
    return status;
}

static void pjapp_media_file_clock_cb(const pj_timestamp *ts, void *user_data)
{
    pjapp_media_file_data *play_file = (pjapp_media_file_data*)user_data;
    pjmedia_frame read_frame, write_frame;
    pj_status_t status;

    /* Read frame from file */
    read_frame.buf = play_file->read_buf;
    read_frame.size = play_file->read_buf_size;
    pjmedia_port_get_frame(play_file->play_port, &read_frame);
    if (status != PJ_SUCCESS)
        read_frame.type = PJMEDIA_FRAME_TYPE_NONE;
    write_frame = read_frame;

    /* Send frame */
    pjmedia_port_put_frame(play_file->stream_port, &write_frame);
}

static pj_status_t pjapp_media_file_done(pjmedia_port *port, void *usr_data)
{
    pjapp_media_file_data *fport = usr_data;
    pjmedia_clock_stop(fport->play_clock);
    //pjmedia_port_stop(fport->stream_port);
    return PJ_SUCCESS;
}


pj_status_t pjapp_media_stream_stop(pjapp_ms_t *ms)
{
    pjmedia_transport *tp;
    /* Stop video devices */
    #if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    if (ms->cfg.vid_port)
        pjmedia_vid_port_stop(ms->cfg.vid_port);
    #endif
    //if (renderer)
    //    pjmedia_vid_port_stop(renderer);

    /* Destroy sound device */
    if (ms->cfg.snd_port) {
        pjmedia_snd_port_destroy( ms->cfg.snd_port );
    }

    /* If there is master port, then we just need to destroy master port
     * (it will recursively destroy upstream and downstream ports, which
     * in this case are file_port and stream_port).
     */
    if (ms->cfg.master_port) {
        pjmedia_master_port_destroy(ms->cfg.master_port, PJ_TRUE);
    }

    /* Stop and destroy file clock */
    if (ms->cfg.play_filedata.play_clock) {
        pjmedia_clock_stop(ms->cfg.play_filedata.play_clock);
        pjmedia_clock_destroy(ms->cfg.play_filedata.play_clock);
    }

    /* Destroy file reader/player */
    if (ms->cfg.play_filedata.play_port)
        pjmedia_port_destroy(ms->cfg.play_filedata.play_port);


    /* Destroy file decoder 
    if (play_decoder) {
        play_decoder->op->close(play_decoder);
        pjmedia_vid_codec_mgr_dealloc_codec(NULL, play_decoder);
    }*/

    /* Destroy video devices */
    #if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    if (ms->cfg.vid_port)
        pjmedia_vid_port_destroy(ms->cfg.vid_port);
    #endif    
    /* Destroy stream */
    if (ms->cfg.type == PJMEDIA_TYPE_AUDIO)
    {
        tp = pjmedia_stream_get_transport(ms->cfg.audio_stream);
        pjmedia_stream_destroy(ms->cfg.audio_stream);
    }
    #if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    else
    {
        tp = pjmedia_vid_stream_get_transport(ms->cfg.video_stream);
        pjmedia_vid_stream_destroy(ms->cfg.video_stream);
    }
    #endif
    if (tp) {
        pjmedia_transport_media_stop(tp);
        pjmedia_transport_close(tp);
    }
    /* Destroy media endpoint. */
    if(!pjsua_get_pjmedia_endpt())
        pjmedia_endpt_destroy( ms->cfg.med_endpt );

    /* Release application pool */
    pj_pool_release( ms->cfg.pool );

    return PJ_SUCCESS;    
}


pj_status_t pjapp_media_stream_startfile(pjapp_ms_t **getms, int type, char *play_file, char *pt, 
    char *localaddr, int localport, char *addr, int port)
{
    pj_status_t status = -1;
    pjapp_ms_t * ms = NULL;
    pjmedia_port *play_file_port = NULL;
    pjmedia_port *stream_port = NULL;
    //char *play_file = "/home/zhurish/Downloads/tftpboot/0-0-1970-01-01-00-48-39-video.H264";
    //char *play_file = "/home/zhurish/workspace/working/zpl-source/source/component/pjsip/hello8000-1s.wav";

    ms = pjapp_media_stream_new(type, localaddr, localport, addr, port);
    if (ms == NULL)
    {
        return -1;
    }
    if (play_file && ms && ms->cfg.type == PJMEDIA_TYPE_AUDIO)
    {
        unsigned wav_ptime;
        status = pjapp_media_stream_codecparam(ms, pt/*"PCMA/8000/1"*/, 0, 0, NULL);
        PJ_ASSERT_GOTO(status == PJ_SUCCESS, fail_end);
        status = pjapp_media_stream_transport(ms);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_create(ms);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        /* Get the port interface of the stream */
        status = pjmedia_stream_get_port(ms->cfg.audio_stream, &stream_port);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

        wav_ptime = PJMEDIA_PIA_PTIME(&stream_port->info);
        status = pjmedia_wav_player_port_create(ms->cfg.pool, play_file, wav_ptime,
                                                0, -1, &play_file_port);
        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Unable to use file", status);
            return -1;
        }
        status = pjmedia_master_port_create(ms->cfg.pool, play_file_port, stream_port,
                                            0, &ms->cfg.master_port);
        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Unable to create master port", status);
            return -1;
        }

        status = pjmedia_master_port_start(ms->cfg.master_port);
        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Error starting master port", status);
            return -1;
        }
        printf("Playing from WAV file %s..\n", play_file);
    }
    else if (ms && ms->cfg.type == PJMEDIA_TYPE_AUDIO)
    {
        status = pjmedia_stream_get_port(ms->cfg.audio_stream, &stream_port);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        /* Create sound device port. */
        //PJ_DECL(pj_status_t) pjmedia_aud_dev_lookup(const char *drv_name,
        //                                    const char *dev_name,
        //                                    pjmedia_aud_dev_index *id);

        if (ms->cfg.dir == PJMEDIA_DIR_ENCODING_DECODING)
            status = pjmedia_snd_port_create(ms->cfg.pool, 0, 1,
                                             PJMEDIA_PIA_SRATE(&stream_port->info),
                                             PJMEDIA_PIA_CCNT(&stream_port->info),
                                             PJMEDIA_PIA_SPF(&stream_port->info),
                                             PJMEDIA_PIA_BITS(&stream_port->info),
                                             0, &ms->cfg.snd_port);
        else if (ms->cfg.dir == PJMEDIA_DIR_ENCODING)
            status = pjmedia_snd_port_create_rec(ms->cfg.pool, 25,
                                                 PJMEDIA_PIA_SRATE(&stream_port->info),
                                                 PJMEDIA_PIA_CCNT(&stream_port->info),
                                                 PJMEDIA_PIA_SPF(&stream_port->info),
                                                 PJMEDIA_PIA_BITS(&stream_port->info),
                                                 0, &ms->cfg.snd_port);
        else
            status = pjmedia_snd_port_create_player(ms->cfg.pool, 0,
                                                    PJMEDIA_PIA_SRATE(&stream_port->info),
                                                    PJMEDIA_PIA_CCNT(&stream_port->info),
                                                    PJMEDIA_PIA_SPF(&stream_port->info),
                                                    PJMEDIA_PIA_BITS(&stream_port->info),
                                                    0, &ms->cfg.snd_port);

        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Unable to create sound port", status);
            return -1;
        }
        status = pjmedia_snd_port_connect(ms->cfg.snd_port, stream_port);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    }
    #if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    if (play_file && ms && ms->cfg.type == PJMEDIA_TYPE_VIDEO)
    {
        //PJ_DECL(pj_status_t) pjmedia_vid_dev_lookup(const char *drv_name,
        //                                    const char *dev_name,
        //                                    pjmedia_vid_dev_index *id);

        pjmedia_vid_codec_param codecparam;
        pjmedia_clock_param clock_param;
        /* Get the port interface of the stream */
		/* Allocate file read buffer */
		ms->cfg.play_filedata.read_buf_size = PJMEDIA_MAX_VIDEO_ENC_FRAME_SIZE;
		ms->cfg.play_filedata.read_buf = pj_pool_zalloc(ms->cfg.pool, ms->cfg.play_filedata.read_buf_size);

        status = pjmedia_file_create_stream(ms->cfg.pool, play_file,
                                  0, &play_file_port);
        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Unable to use file", status);
            return -1;
        }
        status = pjmedia_file_codeparam_get(play_file_port, &codecparam);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_codecparam(ms, pt/*"H264/97"*/, codecparam.enc_fmt.det.vid.size.w, codecparam.enc_fmt.det.vid.size.h, &codecparam.enc_fmtp);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_transport(ms);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_create(ms);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
                     
        /* Create player clock */
        clock_param.usec_interval = PJMEDIA_PTIME(&ms->cfg.video_codec_param->enc_fmt.det.vid.fps);
        clock_param.clock_rate = ms->cfg.video_codec_info->clock_rate;
        status = pjmedia_clock_create2(ms->cfg.pool, &clock_param,
                                       PJMEDIA_CLOCK_NO_HIGHEST_PRIO,
                                       &pjapp_media_file_clock_cb, &ms->cfg.play_filedata, &ms->cfg.play_filedata.play_clock);
        if (status != PJ_SUCCESS)
            return -1;

        status = pjmedia_vid_stream_get_port(ms->cfg.video_stream, PJMEDIA_DIR_ENCODING, &stream_port);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        /* Init play file data */
        ms->cfg.play_filedata.play_port = play_file_port;
        ms->cfg.play_filedata.stream_port = stream_port;
        //play_filedata.vid_stream_port = ms->cfg.video_stream;
        //play_filedata.decoder = play_decoder;
        //if (renderer) {
        //    play_file.renderer = pjmedia_vid_port_get_passive_port(renderer);
        //}
        status = pjmedia_file_set_eof_cb(play_file_port,
                               &ms->cfg.play_filedata,
                               pjapp_media_file_done);

        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);    
        status = pjmedia_clock_start(ms->cfg.play_filedata.play_clock);

        printf("Playing from WAV file %s..\n", play_file);
        ms->cfg.priv = &ms->cfg.play_filedata;
    }
    #endif
    /* Start streaming */
    if (ms->cfg.type == PJMEDIA_TYPE_AUDIO)
    {
        status = pjmedia_stream_start(ms->cfg.audio_stream);
        if (status != PJ_SUCCESS)
            return status;
    }
    #if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    else
    {
        status = pjmedia_vid_stream_start(ms->cfg.video_stream);
        if (status != PJ_SUCCESS)
            return status;
    }
    #endif
    if(getms)
        *getms = ms;
fail_end:   
    return status;
}

static pjapp_ms_t * ms = NULL;

int pjapp_ms_test(void)
{
    char *play_file = "/home/zhurish/Downloads/tftpboot/0-0-1970-01-01-00-48-39-video.H264";
    return pjapp_media_stream_startfile(&ms, PJMEDIA_TYPE_VIDEO, play_file, "H264/97", 
        "192.168.0.103", 5555, "192.168.0.1", 6666);
}
#if 0
static pjapp_ms_t * ms;

int pjapp_ms_test(void)
{

    pj_status_t status;
    pjapp_ms_t * ms = NULL;
    pjmedia_port *play_file_port = NULL;
    pjmedia_port *stream_port = NULL;
    char *play_file = "/home/zhurish/Downloads/tftpboot/0-0-1970-01-01-00-48-39-video.H264";
    //char *play_file = "/home/zhurish/workspace/working/zpl-source/source/component/pjsip/hello8000-1s.wav";
    
    char tmp[10];
    //pjapp_ms_t * ms = pjapp_media_stream_create("192.168.0.103", 5555, "192.168.0.1", 6666, "H264/97", 1920, 1080);
    //pjapp_ms_t *ms = pjapp_media_stream_create("192.168.0.103", 5555, "192.168.0.1", 6666, "PCMA/8000/1", 0, 0);
    //pjapp_ms_t * ms = pjapp_media_stream_create(PJMEDIA_TYPE_AUDIO, "192.168.0.103", 5555, "192.168.0.1", 6666);
    ms = pjapp_media_stream_new(PJMEDIA_TYPE_VIDEO, "192.168.0.103", 5555, "192.168.0.1", 6666);
    if (ms == NULL)
    {
        return -1;
    }
    if (play_file && ms && ms->cfg.type == PJMEDIA_TYPE_AUDIO)
    {
        unsigned wav_ptime;
        status = pjapp_media_stream_codecparam(ms, "PCMA/8000/1", 0, 0, NULL);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_transport(ms);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_create(ms);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        /* Get the port interface of the stream */
        status = pjmedia_stream_get_port(ms->cfg.audio_stream, &stream_port);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

        wav_ptime = PJMEDIA_PIA_PTIME(&stream_port->info);
        status = pjmedia_wav_player_port_create(ms->cfg.pool, play_file, wav_ptime,
                                                0, -1, &play_file_port);
        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Unable to use file", status);
            return -1;
        }
        status = pjmedia_master_port_create(ms->cfg.pool, play_file_port, stream_port,
                                            0, &ms->cfg.master_port);
        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Unable to create master port", status);
            return -1;
        }

        status = pjmedia_master_port_start(ms->cfg.master_port);
        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Error starting master port", status);
            return -1;
        }
        printf("Playing from WAV file %s..\n", play_file);
    }
    else if (ms && ms->cfg.type == PJMEDIA_TYPE_AUDIO)
    {
        status = pjmedia_stream_get_port(ms->cfg.audio_stream, &stream_port);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        /* Create sound device port. */
        //PJ_DECL(pj_status_t) pjmedia_aud_dev_lookup(const char *drv_name,
        //                                    const char *dev_name,
        //                                    pjmedia_aud_dev_index *id);

        if (ms->cfg.dir == PJMEDIA_DIR_ENCODING_DECODING)
            status = pjmedia_snd_port_create(ms->cfg.pool, 0, 1,
                                             PJMEDIA_PIA_SRATE(&stream_port->info),
                                             PJMEDIA_PIA_CCNT(&stream_port->info),
                                             PJMEDIA_PIA_SPF(&stream_port->info),
                                             PJMEDIA_PIA_BITS(&stream_port->info),
                                             0, &ms->cfg.snd_port);
        else if (ms->cfg.dir == PJMEDIA_DIR_ENCODING)
            status = pjmedia_snd_port_create_rec(ms->cfg.pool, 25,
                                                 PJMEDIA_PIA_SRATE(&stream_port->info),
                                                 PJMEDIA_PIA_CCNT(&stream_port->info),
                                                 PJMEDIA_PIA_SPF(&stream_port->info),
                                                 PJMEDIA_PIA_BITS(&stream_port->info),
                                                 0, &ms->cfg.snd_port);
        else
            status = pjmedia_snd_port_create_player(ms->cfg.pool, 0,
                                                    PJMEDIA_PIA_SRATE(&stream_port->info),
                                                    PJMEDIA_PIA_CCNT(&stream_port->info),
                                                    PJMEDIA_PIA_SPF(&stream_port->info),
                                                    PJMEDIA_PIA_BITS(&stream_port->info),
                                                    0, &ms->cfg.snd_port);

        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Unable to create sound port", status);
            return -1;
        }
        status = pjmedia_snd_port_connect(ms->cfg.snd_port, stream_port);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    }
    #if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    if (play_file && ms && ms->cfg.type == PJMEDIA_TYPE_VIDEO)
    {
        //PJ_DECL(pj_status_t) pjmedia_vid_dev_lookup(const char *drv_name,
        //                                    const char *dev_name,
        //                                    pjmedia_vid_dev_index *id);

        pjmedia_vid_codec_param codecparam;
        pjmedia_clock_param clock_param;
        /* Get the port interface of the stream */
		/* Allocate file read buffer */
		ms->cfg.play_filedata.read_buf_size = PJMEDIA_MAX_VIDEO_ENC_FRAME_SIZE;
		ms->cfg.play_filedata.read_buf = pj_pool_zalloc(ms->cfg.pool, ms->cfg.play_filedata.read_buf_size);

        status = pjmedia_file_create_stream(ms->cfg.pool, play_file,
                                  0, &play_file_port);
        if (status != PJ_SUCCESS)
        {
            pjapp_app_perror(THIS_FILE, "Unable to use file", status);
            return -1;
        }
        status = pjmedia_file_codeparam_get(play_file_port, &codecparam);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_codecparam(ms, "H264/97", codecparam.enc_fmt.det.vid.size.w, codecparam.enc_fmt.det.vid.size.h, &codecparam.enc_fmtp);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_transport(ms);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        status = pjapp_media_stream_create(ms);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
                     
        /* Create player clock */
        clock_param.usec_interval = PJMEDIA_PTIME(&ms->cfg.video_codec_param->enc_fmt.det.vid.fps);
        clock_param.clock_rate = ms->cfg.video_codec_info->clock_rate;
        status = pjmedia_clock_create2(ms->cfg.pool, &clock_param,
                                       PJMEDIA_CLOCK_NO_HIGHEST_PRIO,
                                       &pjapp_media_file_clock_cb, &ms->cfg.play_filedata, &ms->cfg.play_filedata.play_clock);
        if (status != PJ_SUCCESS)
            return -1;

        status = pjmedia_vid_stream_get_port(ms->cfg.video_stream, PJMEDIA_DIR_ENCODING, &stream_port);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
        /* Init play file data */
        ms->cfg.play_filedata.play_port = play_file_port;
        ms->cfg.play_filedata.stream_port = stream_port;
        //play_filedata.vid_stream_port = ms->cfg.video_stream;
        //play_filedata.decoder = play_decoder;
        //if (renderer) {
        //    play_file.renderer = pjmedia_vid_port_get_passive_port(renderer);
        //}
        status = pjmedia_file_set_eof_cb(play_file_port,
                               &ms->cfg.play_filedata,
                               pjapp_media_file_done);

        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);    
        status = pjmedia_clock_start(ms->cfg.play_filedata.play_clock);

        printf("Playing from WAV file %s..\n", play_file);
        ms->cfg.priv = &ms->cfg.play_filedata;
    }
    #endif
    /* Start streaming */
    if (ms->cfg.type == PJMEDIA_TYPE_AUDIO)
    {
        status = pjmedia_stream_start(ms->cfg.audio_stream);
        if (status != PJ_SUCCESS)
            return status;
    }
    #if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    else
    {
        status = pjmedia_vid_stream_start(ms->cfg.video_stream);
        if (status != PJ_SUCCESS)
            return status;
    }
    #endif
    return status;
}
#endif

#if 0


pj_status_t pjmedia_stream_create_file_player( pj_pool_t *pool,
                                       const char *file_name,
                                       pjmedia_port **p_play_port)
{
    pjmedia_file_stream *vid_stream = NULL;
    pj_status_t status;

    status = pjmedia_file_create_stream(pool, file_name, 0, &vid_stream);
    if (status != PJ_SUCCESS)
        return status;

    if (!vid_stream)
        return PJ_ENOTFOUND;


    *p_play_port = vid_stream;

    return PJ_SUCCESS;
}


int pjapp_media_stream_io_set(pjapp_ms_t *ms, pjapp_media_data *src, pjapp_media_data *dst)
{
	pj_status_t status = -1;
    
	if (src && src->type == PJAPP_MEDIA_IOTYPE_FILE)
	{
		pjmedia_video_format_detail *file_vfd;
		pjmedia_clock_param clock_param;
		char fmt_name[5];
        pjapp_media_file_data *play_file = PJ_POOL_ALLOC_T(ms->cfg.pool, pjapp_media_file_data);

		/* Collect format info */
		file_vfd = pjmedia_format_get_video_format_detail(&src->_port->info.fmt,
														  PJ_TRUE);
		PJ_LOG(2, (THIS_FILE, "Reading video stream %dx%d %s @%.2ffps",
				   file_vfd->size.w, file_vfd->size.h,
				   pjmedia_fourcc_name(src->_port->info.fmt.id, fmt_name),
				   (1.0 * file_vfd->fps.num / file_vfd->fps.denum)));

		/* Allocate file read buffer */
		play_file->read_buf_size = PJMEDIA_MAX_VIDEO_ENC_FRAME_SIZE;
		play_file->read_buf = pj_pool_zalloc(ms->cfg.pool, play_file->read_buf_size);


		/* Create player clock */
		clock_param.usec_interval = PJMEDIA_PTIME(&file_vfd->fps);
		clock_param.clock_rate = ms->cfg.video_codec_info->clock_rate;
		status = pjmedia_clock_create2(ms->cfg.pool, &clock_param,
									   PJMEDIA_CLOCK_NO_HIGHEST_PRIO,
									   &pjapp_media_file_clock_cb, play_file, &play_file->play_clock);
		if (status != PJ_SUCCESS)
        {
			return status;
        }

		/* Override stream codec param for encoding direction */
		ms->cfg.video_codec_param->enc_fmt.det.vid.size = file_vfd->size;
		ms->cfg.video_codec_param->enc_fmt.det.vid.fps = file_vfd->fps;
        ms->cfg.priv = play_file;
	}
	return status;
}

int pjapp_media_stream_iostart(pj_pool_t *pool, pjapp_ms_cfg_t *cfg, pjapp_ms_param_t *param)
{
	pj_status_t status;
	pjmedia_port *encode_port;
	char addr[64];
	/* Set to ignore fmtp */
	cfg->video_codec_param->ignore_fmtp = PJ_TRUE;

	//status = pjapp_media_stream_create(pool, cfg);

	if (status != PJ_SUCCESS)
		return status;

	/* Get the port interface of the stream */
	status = pjmedia_vid_stream_get_port(cfg->video_stream, PJMEDIA_DIR_ENCODING,
										 &encode_port);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	/* Start streaming */
	status = pjmedia_vid_stream_start(cfg->video_stream);
	if (status != PJ_SUCCESS)
		return status;

	/* Init play file data */
	//play_file->play_port = play_port;
	//play_file->stream_port = encode_port;

	//status = pjmedia_clock_start(play_file->play_clock);
	if (status != PJ_SUCCESS)
		return status;

	/* Done */

	printf("Stream is active, dir is send-only, sending to %s:%d\n",
		   pj_inet_ntop2(pj_AF_INET(), &param->rem_addr.sin_addr, addr,
						 sizeof(addr)),
		   pj_ntohs(param->rem_addr.sin_port));

	PJ_LOG(2, (THIS_FILE, "Sending %dx%d %.*s @%.2ffps",
			   cfg->video_codec_param->enc_fmt.det.vid.size.w,
			   cfg->video_codec_param->enc_fmt.det.vid.size.h,
			   cfg->video_codec_info->encoding_name.slen,
			   cfg->video_codec_info->encoding_name.ptr,
			   (1.0 * cfg->video_codec_param->enc_fmt.det.vid.fps.num /
				cfg->video_codec_param->enc_fmt.det.vid.fps.denum)));
	return status;			
}
#endif

#if 0
/*
 * main()
 */
static int main_func(int argc, char *argv[])
{
    pj_caching_pool cp;
    ;
    pj_pool_t *pool;
    pjmedia_vid_stream *stream = NULL;
    pjmedia_port *enc_port, *dec_port;
    char addr[PJ_INET_ADDRSTRLEN];
    pj_status_t status; 

    pjmedia_vid_port *capture=NULL, *renderer=NULL;
    pjmedia_vid_port_param vpp;

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
    /* SRTP variables */
    pj_bool_t use_srtp = PJ_FALSE;
    char tmp_tx_key[64];
    char tmp_rx_key[64];
    pj_str_t  srtp_tx_key = {NULL, 0};
    pj_str_t  srtp_rx_key = {NULL, 0};
    pj_str_t  srtp_crypto_suite = {NULL, 0};
    int tmp_key_len;
#endif

    /* Default values */
    const pjmedia_vid_codec_info *codec_info;
    pjmedia_vid_codec_param codec_param;
    pjmedia_dir dir = PJMEDIA_DIR_DECODING;
    pj_sockaddr_in remote_addr;
    pj_uint16_t local_port = 4000;
    char *codec_id = NULL;
    pjmedia_rect_size tx_size = {0};
    pj_int8_t rx_pt = -1, tx_pt = -1;

    play_file_data play_file = { NULL };
    pjmedia_port *play_port = NULL;
    pjmedia_vid_codec *play_decoder = NULL;
    pjmedia_clock *play_clock = NULL;

    enum {
        OPT_CODEC       = 'c',
        OPT_LOCAL_PORT  = 'p',
        OPT_REMOTE      = 'r',
        OPT_PLAY_FILE   = 'f',
        OPT_SEND_RECV   = 'b',
        OPT_SEND_ONLY   = 's',
        OPT_RECV_ONLY   = 'i',
        OPT_SEND_WIDTH  = 'W',
        OPT_SEND_HEIGHT = 'H',
        OPT_RECV_PT     = 't',
        OPT_SEND_PT     = 'T',
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
        OPT_USE_SRTP    = 'S',
#endif
        OPT_SRTP_TX_KEY = 'x',
        OPT_SRTP_RX_KEY = 'y',
        OPT_HELP        = 'h',
    };

    struct pj_getopt_option long_options[] = {
        { "codec",          1, 0, OPT_CODEC },
        { "local-port",     1, 0, OPT_LOCAL_PORT },
        { "remote",         1, 0, OPT_REMOTE },
        { "play-file",      1, 0, OPT_PLAY_FILE },
        { "send-recv",      0, 0, OPT_SEND_RECV },
        { "send-only",      0, 0, OPT_SEND_ONLY },
        { "recv-only",      0, 0, OPT_RECV_ONLY },
        { "send-width",     1, 0, OPT_SEND_WIDTH },
        { "send-height",    1, 0, OPT_SEND_HEIGHT },
        { "recv-pt",        1, 0, OPT_RECV_PT },
        { "send-pt",        1, 0, OPT_SEND_PT },
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
        { "use-srtp",       2, 0, OPT_USE_SRTP },
        { "srtp-tx-key",    1, 0, OPT_SRTP_TX_KEY },
        { "srtp-rx-key",    1, 0, OPT_SRTP_RX_KEY },
#endif
        { "help",           0, 0, OPT_HELP },
        { NULL, 0, 0, 0 },
    };

    int c;
    int option_index;


    pj_bzero(&remote_addr, sizeof(remote_addr));


    /* init PJLIB : */
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Parse arguments */
    pj_optind = 0;
    while((c=pj_getopt_long(argc,argv, "h", long_options, &option_index))!=-1)
    {
        switch (c) {
        case OPT_CODEC:
            codec_id = pj_optarg;
            break;

        case OPT_LOCAL_PORT:
            local_port = (pj_uint16_t) atoi(pj_optarg);
            if (local_port < 1) {
                printf("Error: invalid local port %s\n", pj_optarg);
                return 1;
            }
            break;

        case OPT_REMOTE:
            {
                pj_str_t ip = pj_str(strtok(pj_optarg, ":"));
                pj_uint16_t port = (pj_uint16_t) atoi(strtok(NULL, ":"));

                status = pj_sockaddr_in_init(&remote_addr, &ip, port);
                if (status != PJ_SUCCESS) {
                    pjapp_app_perror(THIS_FILE, "Invalid remote address", status);
                    return 1;
                }
            }
            break;

        case OPT_PLAY_FILE:
            play_file.file_name = pj_optarg;
            break;

        case OPT_SEND_RECV:
            dir = PJMEDIA_DIR_ENCODING_DECODING;
            break;

        case OPT_SEND_ONLY:
            dir = PJMEDIA_DIR_ENCODING;
            break;

        case OPT_RECV_ONLY:
            dir = PJMEDIA_DIR_DECODING;
            break;

        case OPT_SEND_WIDTH:
            tx_size.w = (unsigned)atoi(pj_optarg);
            break;

        case OPT_SEND_HEIGHT:
            tx_size.h = (unsigned)atoi(pj_optarg);
            break;

        case OPT_RECV_PT:
            rx_pt = (pj_int8_t)atoi(pj_optarg);
            break;

        case OPT_SEND_PT:
            tx_pt = (pj_int8_t)atoi(pj_optarg);
            break;

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
        case OPT_USE_SRTP:
            use_srtp = PJ_TRUE;
            if (pj_optarg) {
                pj_strset(&srtp_crypto_suite, pj_optarg, strlen(pj_optarg));
            } else {
                srtp_crypto_suite = pj_str("AES_CM_128_HMAC_SHA1_80");
            }
            break;

        case OPT_SRTP_TX_KEY:
            tmp_key_len = my_hex_string_to_octet_string(tmp_tx_key, pj_optarg, 
                                                        (int)strlen(pj_optarg));
            pj_strset(&srtp_tx_key, tmp_tx_key, tmp_key_len/2);
            break;

        case OPT_SRTP_RX_KEY:
            tmp_key_len = my_hex_string_to_octet_string(tmp_rx_key, pj_optarg,
                                                        (int)strlen(pj_optarg));
            pj_strset(&srtp_rx_key, tmp_rx_key, tmp_key_len/2);
            break;
#endif

        case OPT_HELP:
            usage();
            return 1;

        default:
            printf("Invalid options %s\n", argv[pj_optind]);
            return 1;
        }

    }


    /* Verify arguments. */
    if (dir & PJMEDIA_DIR_ENCODING) {
        if (remote_addr.sin_addr.s_addr == 0) {
            printf("Error: remote address must be set\n");
            return 1;
        }
    }

    if (play_file.file_name != NULL && dir != PJMEDIA_DIR_ENCODING) {
        printf("Direction is set to --send-only because of --play-file\n");
        dir = PJMEDIA_DIR_ENCODING;
    }

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
    /* SRTP validation */
    if (use_srtp) {
        if (!srtp_tx_key.slen || !srtp_rx_key.slen)
        {
            printf("Error: Key for each SRTP stream direction must be set\n");
            return 1;
        }
    }
#endif

    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    /* 
     * Initialize media endpoint.
     * This will implicitly initialize PJMEDIA too.
     */
    status = pjmedia_endpt_create(&cp.factory, NULL, 1, &med_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Create memory pool for application purpose */
    pool = pj_pool_create( &cp.factory,     /* pool factory         */
                           "app",           /* pool name.           */
                           4000,            /* init size            */
                           4000,            /* increment size       */
                           NULL             /* callback on error    */
                           );

    /* Init video format manager */
    pjmedia_video_format_mgr_create(pool, 64, 0, NULL);

    /* Init video converter manager */
    pjmedia_converter_mgr_create(pool, NULL);

    /* Init event manager */
    pjmedia_event_mgr_create(pool, 0, NULL);

    /* Init video codec manager */
    pjmedia_vid_codec_mgr_create(pool, NULL);

    /* Init video subsystem */
    pjmedia_vid_dev_subsys_init(&cp.factory);

    /* Register all supported codecs */
    status = init_codecs(&cp.factory);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Find which codec to use. */
    if (codec_id) {
        unsigned count = 1;
        pj_str_t str_codec_id = pj_str(codec_id);

        status = pjmedia_vid_codec_mgr_find_codecs_by_id(NULL,
                                                         &str_codec_id, &count,
                                                         &codec_info, NULL);
        if (status != PJ_SUCCESS) {
            printf("Error: unable to find codec %s\n", codec_id);
            return 1;
        }
    } else {
        static pjmedia_vid_codec_info info[1];
        unsigned count = PJ_ARRAY_SIZE(info);

        /* Default to first codec */
        pjmedia_vid_codec_mgr_enum_codecs(NULL, &count, info, NULL);
        codec_info = &info[0];
    }

    /* Get codec default param for info */
    status = pjmedia_vid_codec_mgr_get_default_param(NULL, codec_info, 
                                                     &codec_param);
    pj_assert(status == PJ_SUCCESS);
    
    /* Set outgoing video size */
    if (tx_size.w && tx_size.h)
        codec_param.enc_fmt.det.vid.size = tx_size;

#if DEF_RENDERER_WIDTH && DEF_RENDERER_HEIGHT
    /* Set incoming video size */
    if (DEF_RENDERER_WIDTH > codec_param.dec_fmt.det.vid.size.w)
        codec_param.dec_fmt.det.vid.size.w = DEF_RENDERER_WIDTH;
    if (DEF_RENDERER_HEIGHT > codec_param.dec_fmt.det.vid.size.h)
        codec_param.dec_fmt.det.vid.size.h = DEF_RENDERER_HEIGHT;
#endif

    if (play_file.file_name) {
        pjmedia_video_format_detail *file_vfd;
        pjmedia_clock_param clock_param;
        char fmt_name[5];

        /* Create file player */
        status = create_file_player(pool, play_file.file_name, &play_port);
        if (status != PJ_SUCCESS)
            goto on_exit;

        /* Collect format info */
        file_vfd = pjmedia_format_get_video_format_detail(&play_port->info.fmt,
                                                          PJ_TRUE);
        PJ_LOG(2, (THIS_FILE, "Reading video stream %dx%d %s @%.2ffps",
                   file_vfd->size.w, file_vfd->size.h,
                   pjmedia_fourcc_name(play_port->info.fmt.id, fmt_name),
                   (1.0*file_vfd->fps.num/file_vfd->fps.denum)));

        /* Allocate file read buffer */
        play_file.read_buf_size = PJMEDIA_MAX_VIDEO_ENC_FRAME_SIZE;
        play_file.read_buf = pj_pool_zalloc(pool, play_file.read_buf_size);

        /* Create decoder, if the file and the stream uses different codec */
        if (codec_info->fmt_id != (pjmedia_format_id)play_port->info.fmt.id) {
            const pjmedia_video_format_info *dec_vfi;
            pjmedia_video_apply_fmt_param dec_vafp = {0};
            const pjmedia_vid_codec_info *codec_info2;
            pjmedia_vid_codec_param codec_param2;

            /* Find decoder */
            status = pjmedia_vid_codec_mgr_get_codec_info2(NULL,
                                                           play_port->info.fmt.id,
                                                           &codec_info2);
            if (status != PJ_SUCCESS)
                goto on_exit;

            /* Init decoder */
            status = pjmedia_vid_codec_mgr_alloc_codec(NULL, codec_info2,
                                                       &play_decoder);
            if (status != PJ_SUCCESS)
                goto on_exit;

            status = play_decoder->op->init(play_decoder, pool);
            if (status != PJ_SUCCESS)
                goto on_exit;

            /* Open decoder */
            status = pjmedia_vid_codec_mgr_get_default_param(NULL, codec_info2,
                                                             &codec_param2);
            if (status != PJ_SUCCESS)
                goto on_exit;

            codec_param2.dir = PJMEDIA_DIR_DECODING;
            status = play_decoder->op->open(play_decoder, &codec_param2);
            if (status != PJ_SUCCESS)
                goto on_exit;

            /* Get decoder format info and apply param */
            dec_vfi = pjmedia_get_video_format_info(NULL,
                                                    codec_info2->dec_fmt_id[0]);
            if (!dec_vfi || !dec_vfi->apply_fmt) {
                status = PJ_ENOTSUP;
                goto on_exit;
            }
            dec_vafp.size = file_vfd->size;
            (*dec_vfi->apply_fmt)(dec_vfi, &dec_vafp);

            /* Allocate buffer to receive decoder output */
            play_file.dec_buf_size = dec_vafp.framebytes;
            play_file.dec_buf = pj_pool_zalloc(pool, play_file.dec_buf_size);
        }

        /* Create player clock */
        clock_param.usec_interval = PJMEDIA_PTIME(&file_vfd->fps);
        clock_param.clock_rate = codec_info->clock_rate;
        status = pjmedia_clock_create2(pool, &clock_param,
                                       PJMEDIA_CLOCK_NO_HIGHEST_PRIO,
                                       &clock_cb, &play_file, &play_clock);
        if (status != PJ_SUCCESS)
            goto on_exit;

        /* Override stream codec param for encoding direction */
        codec_param.enc_fmt.det.vid.size = file_vfd->size;
        codec_param.enc_fmt.det.vid.fps  = file_vfd->fps;

    } else {
        pjmedia_vid_port_param_default(&vpp);

        /* Set as active for all video devices */
        vpp.active = PJ_TRUE;

        /* Create video device port. */
        if (dir & PJMEDIA_DIR_ENCODING) {
            /* Create capture */
            status = pjmedia_vid_dev_default_param(
                                        pool,
                                        PJMEDIA_VID_DEFAULT_CAPTURE_DEV,
                                        &vpp.vidparam);
            if (status != PJ_SUCCESS)
                goto on_exit;

            pjmedia_format_copy(&vpp.vidparam.fmt, &codec_param.enc_fmt);
            vpp.vidparam.fmt.id = codec_param.dec_fmt.id;
            vpp.vidparam.dir = PJMEDIA_DIR_CAPTURE;
            
            status = pjmedia_vid_port_create(pool, &vpp, &capture);
            if (status != PJ_SUCCESS)
                goto on_exit;
        }
        
        if (dir & PJMEDIA_DIR_DECODING) {
            /* Create renderer */
            status = pjmedia_vid_dev_default_param(
                                        pool,
                                        PJMEDIA_VID_DEFAULT_RENDER_DEV,
                                        &vpp.vidparam);
            if (status != PJ_SUCCESS)
                goto on_exit;

            pjmedia_format_copy(&vpp.vidparam.fmt, &codec_param.dec_fmt);
            vpp.vidparam.dir = PJMEDIA_DIR_RENDER;
            vpp.vidparam.disp_size = vpp.vidparam.fmt.det.vid.size;
            vpp.vidparam.flags |= PJMEDIA_VID_DEV_CAP_OUTPUT_WINDOW_FLAGS;
            vpp.vidparam.window_flags = PJMEDIA_VID_DEV_WND_BORDER |
                                        PJMEDIA_VID_DEV_WND_RESIZABLE;

            status = pjmedia_vid_port_create(pool, &vpp, &renderer);
            if (status != PJ_SUCCESS)
                goto on_exit;
        }
    }

    /* Set to ignore fmtp */
    codec_param.ignore_fmtp = PJ_TRUE;

    /* Create stream based on program arguments */
    status = create_stream(pool, med_endpt, codec_info, &codec_param,
                           dir, rx_pt, tx_pt, local_port, &remote_addr, 
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
                           use_srtp, &srtp_crypto_suite, 
                           &srtp_tx_key, &srtp_rx_key,
#endif
                           &stream);
    if (status != PJ_SUCCESS)
        goto on_exit;

    /* Get the port interface of the stream */
    status = pjmedia_vid_stream_get_port(stream, PJMEDIA_DIR_ENCODING,
                                         &enc_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    status = pjmedia_vid_stream_get_port(stream, PJMEDIA_DIR_DECODING,
                                         &dec_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Start streaming */
    status = pjmedia_vid_stream_start(stream);
    if (status != PJ_SUCCESS)
        goto on_exit;

    /* Start renderer */
    if (renderer) {
        status = pjmedia_vid_port_connect(renderer, dec_port, PJ_FALSE);
        if (status != PJ_SUCCESS)
            goto on_exit;
        status = pjmedia_vid_port_start(renderer);
        if (status != PJ_SUCCESS)
            goto on_exit;
    }

    /* Start capture */
    if (capture) {
        status = pjmedia_vid_port_connect(capture, enc_port, PJ_FALSE);
        if (status != PJ_SUCCESS)
            goto on_exit;
        status = pjmedia_vid_port_start(capture);
        if (status != PJ_SUCCESS)
            goto on_exit;
    }

    /* Start playing file */
    if (play_file.file_name) {

#if HAS_LOCAL_RENDERER_FOR_PLAY_FILE
        /* Create local renderer */
        pjmedia_vid_port_param_default(&vpp);
        vpp.active = PJ_FALSE;
        status = pjmedia_vid_dev_default_param(
                                pool,
                                PJMEDIA_VID_DEFAULT_RENDER_DEV,
                                &vpp.vidparam);
        if (status != PJ_SUCCESS)
            goto on_exit;

        vpp.vidparam.dir = PJMEDIA_DIR_RENDER;
        pjmedia_format_copy(&vpp.vidparam.fmt, &codec_param.dec_fmt);
        vpp.vidparam.fmt.det.vid.size = play_port->info.fmt.det.vid.size;
        vpp.vidparam.fmt.det.vid.fps = play_port->info.fmt.det.vid.fps;
        vpp.vidparam.disp_size = vpp.vidparam.fmt.det.vid.size;
        vpp.vidparam.flags |= PJMEDIA_VID_DEV_CAP_OUTPUT_WINDOW_FLAGS;
        vpp.vidparam.window_flags = PJMEDIA_VID_DEV_WND_BORDER |
                                    PJMEDIA_VID_DEV_WND_RESIZABLE;

        status = pjmedia_vid_port_create(pool, &vpp, &renderer);
        if (status != PJ_SUCCESS)
            goto on_exit;
        status = pjmedia_vid_port_start(renderer);
        if (status != PJ_SUCCESS)
            goto on_exit;
#endif

        /* Init play file data */
        play_file.play_port = play_port;
        play_file.stream_port = enc_port;
        play_file.decoder = play_decoder;
        if (renderer) {
            play_file.renderer = pjmedia_vid_port_get_passive_port(renderer);
        }

        status = pjmedia_clock_start(play_clock);
        if (status != PJ_SUCCESS)
            goto on_exit;
    }

    /* Done */

    if (dir == PJMEDIA_DIR_DECODING)
        printf("Stream is active, dir is recv-only, local port is %d\n",
               local_port);
    else if (dir == PJMEDIA_DIR_ENCODING)
        printf("Stream is active, dir is send-only, sending to %s:%d\n",
               pj_inet_ntop2(pj_AF_INET(), &remote_addr.sin_addr, addr,
                             sizeof(addr)),
               pj_ntohs(remote_addr.sin_port));
    else
        printf("Stream is active, send/recv, local port is %d, "
               "sending to %s:%d\n",
               local_port,
               pj_inet_ntop2(pj_AF_INET(), &remote_addr.sin_addr, addr,
                             sizeof(addr)),
               pj_ntohs(remote_addr.sin_port));

    if (dir & PJMEDIA_DIR_ENCODING)
        PJ_LOG(2, (THIS_FILE, "Sending %dx%d %.*s @%.2ffps",
                   codec_param.enc_fmt.det.vid.size.w,
                   codec_param.enc_fmt.det.vid.size.h,
                   codec_info->encoding_name.slen,
                   codec_info->encoding_name.ptr,
                   (1.0*codec_param.enc_fmt.det.vid.fps.num/
                    codec_param.enc_fmt.det.vid.fps.denum)));

    for (;;) {
        char tmp[10];

        puts("");
        puts("Commands:");
        puts("  q     Quit");
        puts("");

        printf("Command: "); fflush(stdout);

        if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
            puts("EOF while reading stdin, will quit now..");
            break;
        }

        if (tmp[0] == 'q')
            break;

    }



    /* Start deinitialization: */
on_exit:

    /* Stop video devices */
    if (capture)
        pjmedia_vid_port_stop(capture);
    if (renderer)
        pjmedia_vid_port_stop(renderer);

    /* Stop and destroy file clock */
    if (play_clock) {
        pjmedia_clock_stop(play_clock);
        pjmedia_clock_destroy(play_clock);
    }

    /* Destroy file reader/player */
    if (play_port)
        pjmedia_port_destroy(play_port);

    /* Destroy file decoder */
    if (play_decoder) {
        play_decoder->op->close(play_decoder);
        pjmedia_vid_codec_mgr_dealloc_codec(NULL, play_decoder);
    }

    /* Destroy video devices */
    if (capture)
        pjmedia_vid_port_destroy(capture);
    if (renderer)
        pjmedia_vid_port_destroy(renderer);

    /* Destroy stream */
    if (stream) {
        pjmedia_transport *tp;

        tp = pjmedia_vid_stream_get_transport(stream);
        pjmedia_vid_stream_destroy(stream);
        
        pjmedia_transport_media_stop(tp);
        pjmedia_transport_close(tp);
    }

    /* Deinit codecs */
    deinit_codecs();

    /* Shutdown video subsystem */
    pjmedia_vid_dev_subsys_shutdown();

    /* Destroy event manager */
    pjmedia_event_mgr_destroy(NULL);

    /* Release application pool */
    pj_pool_release( pool );

    /* Destroy media endpoint. */
    pjmedia_endpt_destroy( med_endpt );

    /* Destroy pool factory */
    pj_caching_pool_destroy( &cp );

    /* Shutdown PJLIB */
    pj_shutdown();

    return (status == PJ_SUCCESS) ? 0 : 1;
}
#endif


static void print_stream_stat(pjmedia_stream *stream,
                              const pjmedia_codec_param *codec_param)
{
    char duration[80], last_update[80];
    char bps[16], ipbps[16], packets[16], bytes[16], ipbytes[16];
    pjmedia_port *port;
    pjmedia_rtcp_stat stat;
    pj_time_val now;


    pj_gettimeofday(&now);
    pjmedia_stream_get_stat(stream, &stat);
    pjmedia_stream_get_port(stream, &port);

    puts("Stream statistics:");

    /* Print duration */
    PJ_TIME_VAL_SUB(now, stat.start);
    sprintf(duration, " Duration: %02ld:%02ld:%02ld.%03ld",
            now.sec / 3600,
            (now.sec % 3600) / 60,
            (now.sec % 60),
            now.msec);


    printf(" Info: audio %dHz, %dms/frame, %sB/s (%sB/s +IP hdr)\n",
        PJMEDIA_PIA_SRATE(&port->info),
        PJMEDIA_PIA_PTIME(&port->info),
        good_number(bps, (codec_param->info.avg_bps+7)/8),
        good_number(ipbps, ((codec_param->info.avg_bps+7)/8) + 
                           (40 * 1000 /
                            codec_param->setting.frm_per_pkt /
                            codec_param->info.frm_ptime)));

    if (stat.rx.update_cnt == 0)
        strcpy(last_update, "never");
    else {
        pj_gettimeofday(&now);
        PJ_TIME_VAL_SUB(now, stat.rx.update);
        sprintf(last_update, "%02ldh:%02ldm:%02ld.%03lds ago",
                now.sec / 3600,
                (now.sec % 3600) / 60,
                now.sec % 60,
                now.msec);
    }

    printf(" RX stat last update: %s\n"
           "    total %s packets %sB received (%sB +IP hdr)%s\n"
           "    pkt loss=%d (%3.1f%%), dup=%d (%3.1f%%), reorder=%d (%3.1f%%)%s\n"
           "          (msec)    min     avg     max     last    dev\n"
           "    loss period: %7.3f %7.3f %7.3f %7.3f %7.3f%s\n"
           "    jitter     : %7.3f %7.3f %7.3f %7.3f %7.3f%s\n",
           last_update,
           good_number(packets, stat.rx.pkt),
           good_number(bytes, stat.rx.bytes),
           good_number(ipbytes, stat.rx.bytes + stat.rx.pkt * 32),
           "",
           stat.rx.loss,
           stat.rx.loss * 100.0 / (stat.rx.pkt + stat.rx.loss),
           stat.rx.dup, 
           stat.rx.dup * 100.0 / (stat.rx.pkt + stat.rx.loss),
           stat.rx.reorder, 
           stat.rx.reorder * 100.0 / (stat.rx.pkt + stat.rx.loss),
           "",
           stat.rx.loss_period.min / 1000.0, 
           stat.rx.loss_period.mean / 1000.0, 
           stat.rx.loss_period.max / 1000.0,
           stat.rx.loss_period.last / 1000.0,
           pj_math_stat_get_stddev(&stat.rx.loss_period) / 1000.0,
           "",
           stat.rx.jitter.min / 1000.0,
           stat.rx.jitter.mean / 1000.0,
           stat.rx.jitter.max / 1000.0,
           stat.rx.jitter.last / 1000.0,
           pj_math_stat_get_stddev(&stat.rx.jitter) / 1000.0,
           ""
           );


    if (stat.tx.update_cnt == 0)
        strcpy(last_update, "never");
    else {
        pj_gettimeofday(&now);
        PJ_TIME_VAL_SUB(now, stat.tx.update);
        sprintf(last_update, "%02ldh:%02ldm:%02ld.%03lds ago",
                now.sec / 3600,
                (now.sec % 3600) / 60,
                now.sec % 60,
                now.msec);
    }

    printf(" TX stat last update: %s\n"
           "    total %s packets %sB sent (%sB +IP hdr)%s\n"
           "    pkt loss=%d (%3.1f%%), dup=%d (%3.1f%%), reorder=%d (%3.1f%%)%s\n"
           "          (msec)    min     avg     max     last    dev\n"
           "    loss period: %7.3f %7.3f %7.3f %7.3f %7.3f%s\n"
           "    jitter     : %7.3f %7.3f %7.3f %7.3f %7.3f%s\n",
           last_update,
           good_number(packets, stat.tx.pkt),
           good_number(bytes, stat.tx.bytes),
           good_number(ipbytes, stat.tx.bytes + stat.tx.pkt * 32),
           "",
           stat.tx.loss,
           stat.tx.loss * 100.0 / (stat.tx.pkt + stat.tx.loss),
           stat.tx.dup, 
           stat.tx.dup * 100.0 / (stat.tx.pkt + stat.tx.loss),
           stat.tx.reorder, 
           stat.tx.reorder * 100.0 / (stat.tx.pkt + stat.tx.loss),
           "",
           stat.tx.loss_period.min / 1000.0, 
           stat.tx.loss_period.mean / 1000.0, 
           stat.tx.loss_period.max / 1000.0,
           stat.tx.loss_period.last / 1000.0,
           pj_math_stat_get_stddev(&stat.tx.loss_period) / 1000.0,
           "",
           stat.tx.jitter.min / 1000.0,
           stat.tx.jitter.mean / 1000.0,
           stat.tx.jitter.max / 1000.0,
           stat.tx.jitter.last / 1000.0,
           pj_math_stat_get_stddev(&stat.tx.jitter) / 1000.0,
           ""
           );


    printf(" RTT delay     : %7.3f %7.3f %7.3f %7.3f %7.3f%s\n", 
           stat.rtt.min / 1000.0,
           stat.rtt.mean / 1000.0,
           stat.rtt.max / 1000.0,
           stat.rtt.last / 1000.0,
           pj_math_stat_get_stddev(&stat.rtt) / 1000.0,
           ""
           );

#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)
    /* RTCP XR Reports */
    do {
        char loss[16], dup[16];
        char jitter[80];
        char toh[80];
        char plc[16], jba[16], jbr[16];
        char signal_lvl[16], noise_lvl[16], rerl[16];
        char r_factor[16], ext_r_factor[16], mos_lq[16], mos_cq[16];
        pjmedia_rtcp_xr_stat xr_stat;

        if (pjmedia_stream_get_stat_xr(stream, &xr_stat) != PJ_SUCCESS)
            break;

        puts("\nExtended reports:");

        /* Statistics Summary */
        puts(" Statistics Summary");

        if (xr_stat.rx.stat_sum.l)
            sprintf(loss, "%d", xr_stat.rx.stat_sum.lost);
        else
            sprintf(loss, "(na)");

        if (xr_stat.rx.stat_sum.d)
            sprintf(dup, "%d", xr_stat.rx.stat_sum.dup);
        else
            sprintf(dup, "(na)");

        if (xr_stat.rx.stat_sum.j) {
            unsigned jmin, jmax, jmean, jdev;

            SAMPLES_TO_USEC(jmin, xr_stat.rx.stat_sum.jitter.min, 
                            port->info.fmt.det.aud.clock_rate);
            SAMPLES_TO_USEC(jmax, xr_stat.rx.stat_sum.jitter.max, 
                            port->info.fmt.det.aud.clock_rate);
            SAMPLES_TO_USEC(jmean, xr_stat.rx.stat_sum.jitter.mean, 
                            port->info.fmt.det.aud.clock_rate);
            SAMPLES_TO_USEC(jdev, 
                           pj_math_stat_get_stddev(&xr_stat.rx.stat_sum.jitter),
                           port->info.fmt.det.aud.clock_rate);
            sprintf(jitter, "%7.3f %7.3f %7.3f %7.3f", 
                    jmin/1000.0, jmean/1000.0, jmax/1000.0, jdev/1000.0);
        } else
            sprintf(jitter, "(report not available)");

        if (xr_stat.rx.stat_sum.t) {
            sprintf(toh, "%11d %11d %11d %11d", 
                    xr_stat.rx.stat_sum.toh.min,
                    xr_stat.rx.stat_sum.toh.mean,
                    xr_stat.rx.stat_sum.toh.max,
                    pj_math_stat_get_stddev(&xr_stat.rx.stat_sum.toh));
        } else
            sprintf(toh, "(report not available)");

        if (xr_stat.rx.stat_sum.update.sec == 0)
            strcpy(last_update, "never");
        else {
            pj_gettimeofday(&now);
            PJ_TIME_VAL_SUB(now, xr_stat.rx.stat_sum.update);
            sprintf(last_update, "%02ldh:%02ldm:%02ld.%03lds ago",
                    now.sec / 3600,
                    (now.sec % 3600) / 60,
                    now.sec % 60,
                    now.msec);
        }

        printf(" RX last update: %s\n"
               "    begin seq=%d, end seq=%d%s\n"
               "    pkt loss=%s, dup=%s%s\n"
               "          (msec)    min     avg     max     dev\n"
               "    jitter     : %s\n"
               "    toh        : %s\n",
               last_update,
               xr_stat.rx.stat_sum.begin_seq, xr_stat.rx.stat_sum.end_seq,
               "",
               loss, dup,
               "",
               jitter,
               toh
               );

        if (xr_stat.tx.stat_sum.l)
            sprintf(loss, "%d", xr_stat.tx.stat_sum.lost);
        else
            sprintf(loss, "(na)");

        if (xr_stat.tx.stat_sum.d)
            sprintf(dup, "%d", xr_stat.tx.stat_sum.dup);
        else
            sprintf(dup, "(na)");

        if (xr_stat.tx.stat_sum.j) {
            unsigned jmin, jmax, jmean, jdev;

            SAMPLES_TO_USEC(jmin, xr_stat.tx.stat_sum.jitter.min, 
                            port->info.fmt.det.aud.clock_rate);
            SAMPLES_TO_USEC(jmax, xr_stat.tx.stat_sum.jitter.max, 
                            port->info.fmt.det.aud.clock_rate);
            SAMPLES_TO_USEC(jmean, xr_stat.tx.stat_sum.jitter.mean, 
                            port->info.fmt.det.aud.clock_rate);
            SAMPLES_TO_USEC(jdev, 
                           pj_math_stat_get_stddev(&xr_stat.tx.stat_sum.jitter),
                           port->info.fmt.det.aud.clock_rate);
            sprintf(jitter, "%7.3f %7.3f %7.3f %7.3f", 
                    jmin/1000.0, jmean/1000.0, jmax/1000.0, jdev/1000.0);
        } else
            sprintf(jitter, "(report not available)");

        if (xr_stat.tx.stat_sum.t) {
            sprintf(toh, "%11d %11d %11d %11d", 
                    xr_stat.tx.stat_sum.toh.min,
                    xr_stat.tx.stat_sum.toh.mean,
                    xr_stat.tx.stat_sum.toh.max,
                    pj_math_stat_get_stddev(&xr_stat.rx.stat_sum.toh));
        } else
            sprintf(toh,    "(report not available)");

        if (xr_stat.tx.stat_sum.update.sec == 0)
            strcpy(last_update, "never");
        else {
            pj_gettimeofday(&now);
            PJ_TIME_VAL_SUB(now, xr_stat.tx.stat_sum.update);
            sprintf(last_update, "%02ldh:%02ldm:%02ld.%03lds ago",
                    now.sec / 3600,
                    (now.sec % 3600) / 60,
                    now.sec % 60,
                    now.msec);
        }

        printf(" TX last update: %s\n"
               "    begin seq=%d, end seq=%d%s\n"
               "    pkt loss=%s, dup=%s%s\n"
               "          (msec)    min     avg     max     dev\n"
               "    jitter     : %s\n"
               "    toh        : %s\n",
               last_update,
               xr_stat.tx.stat_sum.begin_seq, xr_stat.tx.stat_sum.end_seq,
               "",
               loss, dup,
               "",
               jitter,
               toh
               );

        /* VoIP Metrics */
        puts(" VoIP Metrics");

        PRINT_VOIP_MTC_VAL(signal_lvl, xr_stat.rx.voip_mtc.signal_lvl);
        PRINT_VOIP_MTC_VAL(noise_lvl, xr_stat.rx.voip_mtc.noise_lvl);
        PRINT_VOIP_MTC_VAL(rerl, xr_stat.rx.voip_mtc.rerl);
        PRINT_VOIP_MTC_VAL(r_factor, xr_stat.rx.voip_mtc.r_factor);
        PRINT_VOIP_MTC_VAL(ext_r_factor, xr_stat.rx.voip_mtc.ext_r_factor);
        PRINT_VOIP_MTC_VAL(mos_lq, xr_stat.rx.voip_mtc.mos_lq);
        PRINT_VOIP_MTC_VAL(mos_cq, xr_stat.rx.voip_mtc.mos_cq);

        switch ((xr_stat.rx.voip_mtc.rx_config>>6) & 3) {
            case PJMEDIA_RTCP_XR_PLC_DIS:
                sprintf(plc, "DISABLED");
                break;
            case PJMEDIA_RTCP_XR_PLC_ENH:
                sprintf(plc, "ENHANCED");
                break;
            case PJMEDIA_RTCP_XR_PLC_STD:
                sprintf(plc, "STANDARD");
                break;
            case PJMEDIA_RTCP_XR_PLC_UNK:
            default:
                sprintf(plc, "UNKNOWN");
                break;
        }

        switch ((xr_stat.rx.voip_mtc.rx_config>>4) & 3) {
            case PJMEDIA_RTCP_XR_JB_FIXED:
                sprintf(jba, "FIXED");
                break;
            case PJMEDIA_RTCP_XR_JB_ADAPTIVE:
                sprintf(jba, "ADAPTIVE");
                break;
            default:
                sprintf(jba, "UNKNOWN");
                break;
        }

        sprintf(jbr, "%d", xr_stat.rx.voip_mtc.rx_config & 0x0F);

        if (xr_stat.rx.voip_mtc.update.sec == 0)
            strcpy(last_update, "never");
        else {
            pj_gettimeofday(&now);
            PJ_TIME_VAL_SUB(now, xr_stat.rx.voip_mtc.update);
            sprintf(last_update, "%02ldh:%02ldm:%02ld.%03lds ago",
                    now.sec / 3600,
                    (now.sec % 3600) / 60,
                    now.sec % 60,
                    now.msec);
        }

        printf(" RX last update: %s\n"
               "    packets    : loss rate=%d (%.2f%%), discard rate=%d (%.2f%%)\n"
               "    burst      : density=%d (%.2f%%), duration=%d%s\n"
               "    gap        : density=%d (%.2f%%), duration=%d%s\n"
               "    delay      : round trip=%d%s, end system=%d%s\n"
               "    level      : signal=%s%s, noise=%s%s, RERL=%s%s\n"
               "    quality    : R factor=%s, ext R factor=%s\n"
               "                 MOS LQ=%s, MOS CQ=%s\n"
               "    config     : PLC=%s, JB=%s, JB rate=%s, Gmin=%d\n"
               "    JB delay   : cur=%d%s, max=%d%s, abs max=%d%s\n",
               last_update,
               /* pakcets */
               xr_stat.rx.voip_mtc.loss_rate, xr_stat.rx.voip_mtc.loss_rate*100.0/256,
               xr_stat.rx.voip_mtc.discard_rate, xr_stat.rx.voip_mtc.discard_rate*100.0/256,
               /* burst */
               xr_stat.rx.voip_mtc.burst_den, xr_stat.rx.voip_mtc.burst_den*100.0/256,
               xr_stat.rx.voip_mtc.burst_dur, "ms",
               /* gap */
               xr_stat.rx.voip_mtc.gap_den, xr_stat.rx.voip_mtc.gap_den*100.0/256,
               xr_stat.rx.voip_mtc.gap_dur, "ms",
               /* delay */
               xr_stat.rx.voip_mtc.rnd_trip_delay, "ms",
               xr_stat.rx.voip_mtc.end_sys_delay, "ms",
               /* level */
               signal_lvl, "dB",
               noise_lvl, "dB",
               rerl, "",
               /* quality */
               r_factor, ext_r_factor, mos_lq, mos_cq,
               /* config */
               plc, jba, jbr, xr_stat.rx.voip_mtc.gmin,
               /* JB delay */
               xr_stat.rx.voip_mtc.jb_nom, "ms",
               xr_stat.rx.voip_mtc.jb_max, "ms",
               xr_stat.rx.voip_mtc.jb_abs_max, "ms"
               );

        PRINT_VOIP_MTC_VAL(signal_lvl, xr_stat.tx.voip_mtc.signal_lvl);
        PRINT_VOIP_MTC_VAL(noise_lvl, xr_stat.tx.voip_mtc.noise_lvl);
        PRINT_VOIP_MTC_VAL(rerl, xr_stat.tx.voip_mtc.rerl);
        PRINT_VOIP_MTC_VAL(r_factor, xr_stat.tx.voip_mtc.r_factor);
        PRINT_VOIP_MTC_VAL(ext_r_factor, xr_stat.tx.voip_mtc.ext_r_factor);
        PRINT_VOIP_MTC_VAL(mos_lq, xr_stat.tx.voip_mtc.mos_lq);
        PRINT_VOIP_MTC_VAL(mos_cq, xr_stat.tx.voip_mtc.mos_cq);

        switch ((xr_stat.tx.voip_mtc.rx_config>>6) & 3) {
            case PJMEDIA_RTCP_XR_PLC_DIS:
                sprintf(plc, "DISABLED");
                break;
            case PJMEDIA_RTCP_XR_PLC_ENH:
                sprintf(plc, "ENHANCED");
                break;
            case PJMEDIA_RTCP_XR_PLC_STD:
                sprintf(plc, "STANDARD");
                break;
            case PJMEDIA_RTCP_XR_PLC_UNK:
            default:
                sprintf(plc, "unknown");
                break;
        }

        switch ((xr_stat.tx.voip_mtc.rx_config>>4) & 3) {
            case PJMEDIA_RTCP_XR_JB_FIXED:
                sprintf(jba, "FIXED");
                break;
            case PJMEDIA_RTCP_XR_JB_ADAPTIVE:
                sprintf(jba, "ADAPTIVE");
                break;
            default:
                sprintf(jba, "unknown");
                break;
        }

        sprintf(jbr, "%d", xr_stat.tx.voip_mtc.rx_config & 0x0F);

        if (xr_stat.tx.voip_mtc.update.sec == 0)
            strcpy(last_update, "never");
        else {
            pj_gettimeofday(&now);
            PJ_TIME_VAL_SUB(now, xr_stat.tx.voip_mtc.update);
            sprintf(last_update, "%02ldh:%02ldm:%02ld.%03lds ago",
                    now.sec / 3600,
                    (now.sec % 3600) / 60,
                    now.sec % 60,
                    now.msec);
        }

        printf(" TX last update: %s\n"
               "    packets    : loss rate=%d (%.2f%%), discard rate=%d (%.2f%%)\n"
               "    burst      : density=%d (%.2f%%), duration=%d%s\n"
               "    gap        : density=%d (%.2f%%), duration=%d%s\n"
               "    delay      : round trip=%d%s, end system=%d%s\n"
               "    level      : signal=%s%s, noise=%s%s, RERL=%s%s\n"
               "    quality    : R factor=%s, ext R factor=%s\n"
               "                 MOS LQ=%s, MOS CQ=%s\n"
               "    config     : PLC=%s, JB=%s, JB rate=%s, Gmin=%d\n"
               "    JB delay   : cur=%d%s, max=%d%s, abs max=%d%s\n",
               last_update,
               /* pakcets */
               xr_stat.tx.voip_mtc.loss_rate, xr_stat.tx.voip_mtc.loss_rate*100.0/256,
               xr_stat.tx.voip_mtc.discard_rate, xr_stat.tx.voip_mtc.discard_rate*100.0/256,
               /* burst */
               xr_stat.tx.voip_mtc.burst_den, xr_stat.tx.voip_mtc.burst_den*100.0/256,
               xr_stat.tx.voip_mtc.burst_dur, "ms",
               /* gap */
               xr_stat.tx.voip_mtc.gap_den, xr_stat.tx.voip_mtc.gap_den*100.0/256,
               xr_stat.tx.voip_mtc.gap_dur, "ms",
               /* delay */
               xr_stat.tx.voip_mtc.rnd_trip_delay, "ms",
               xr_stat.tx.voip_mtc.end_sys_delay, "ms",
               /* level */
               signal_lvl, "dB",
               noise_lvl, "dB",
               rerl, "",
               /* quality */
               r_factor, ext_r_factor, mos_lq, mos_cq,
               /* config */
               plc, jba, jbr, xr_stat.tx.voip_mtc.gmin,
               /* JB delay */
               xr_stat.tx.voip_mtc.jb_nom, "ms",
               xr_stat.tx.voip_mtc.jb_max, "ms",
               xr_stat.tx.voip_mtc.jb_abs_max, "ms"
               );


        /* RTT delay (by receiver side) */
        printf("          (msec)    min     avg     max     last    dev\n");
        printf(" RTT delay     : %7.3f %7.3f %7.3f %7.3f %7.3f%s\n", 
               xr_stat.rtt.min / 1000.0,
               xr_stat.rtt.mean / 1000.0,
               xr_stat.rtt.max / 1000.0,
               xr_stat.rtt.last / 1000.0,
               pj_math_stat_get_stddev(&xr_stat.rtt) / 1000.0,
               ""
               );
    } while (0);
#endif /* PJMEDIA_HAS_RTCP_XR */

}