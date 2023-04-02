/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of oRTP 
 * (see https://gitlab.linphone.org/BC/public/ortp).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file payloadtype.h
 * \brief Definition of payload types
 *
**/

#ifndef __JPAYLOADTYPE_H__
#define __JPAYLOADTYPE_H__
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "rtpconfig.h"
#include "rtptypes.h"
#ifdef __cplusplus
extern "C"{
#endif

#ifndef MIN
#define MIN(a,b)	((a)<(b))?(a):(b)
#endif
/* flags for jrtp_PayloadType::flags */

#define	JRTP_PAYLOAD_TYPE_ALLOCATED (1)
/*payload type represents a VBR codec*/
#define	JRTP_PAYLOAD_TYPE_IS_VBR (1<<1)
#define	JRTP_PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED (1<<2)
/* private flags for future use by ortp */
#define	JRTP_PAYLOAD_TYPE_PRIV1 (1<<3)
/* user flags, can be used by the application on top of oRTP */
#define	JRTP_PAYLOAD_TYPE_USER_FLAG_0 (1<<4)
#define	JRTP_PAYLOAD_TYPE_USER_FLAG_1 (1<<5)
#define	JRTP_PAYLOAD_TYPE_USER_FLAG_2 (1<<6)
#define	JRTP_PAYLOAD_TYPE_USER_FLAG_3 (1<<7)
#define	JRTP_PAYLOAD_TYPE_USER_FLAG_4 (1<<8)
/* ask for more if you need*/

#define JRTP_PAYLOAD_TYPE_FLAG_CAN_RECV JRTP_PAYLOAD_TYPE_USER_FLAG_1
#define JRTP_PAYLOAD_TYPE_FLAG_CAN_SEND JRTP_PAYLOAD_TYPE_USER_FLAG_2

#define JRTP_PAYLOAD_AUDIO_CONTINUOUS 0
#define JRTP_PAYLOAD_AUDIO_PACKETIZED 1
#define JRTP_PAYLOAD_VIDEO 2
#define JRTP_PAYLOAD_TEXT 3
#define JRTP_PAYLOAD_OTHER 4  /* ?? */

#define JRTP_PAYLOAD_TYPE_AVPF_NONE 0
#define JRTP_PAYLOAD_TYPE_AVPF_FIR (1 << 0)
#define JRTP_PAYLOAD_TYPE_AVPF_PLI (1 << 1)
#define JRTP_PAYLOAD_TYPE_AVPF_SLI (1 << 2)
#define JRTP_PAYLOAD_TYPE_AVPF_RPSI (1 << 3)

struct _jPayloadTypeAvpfParams {
	unsigned char features; /**< A bitmask of JRTP_PAYLOAD_TYPE_AVPF_* macros. */
	bool rpsi_compatibility; /*< Linphone uses positive feeback for RPSI. However first versions handling
		AVPF wrongly declared RPSI as negative feedback, so this is kept for compatibility
		with these versions but will probably be removed at some point in time. */
	uint16_t trr_interval; /**< The interval in milliseconds between regular RTCP packets. */
};

struct _jrtpPayloadType
{
	int type; /**< one of JRTP_PAYLOAD_* macros*/
	int clock_rate; /**< rtp clock rate*/
	char bits_per_sample;	/* in case of continuous audio data */
	char *zero_pattern;
	int pattern_length;
	/* other useful information for the application*/
	int normal_bitrate;	/*in bit/s */
	char mime_type[32]; /**<actually the submime, ex: pcm, pcma, gsm*/
	int channels; /**< number of channels of audio */
	char *recv_fmtp; /* various format parameters for the incoming stream */
	char *send_fmtp; /* various format parameters for the outgoing stream */
	struct _jPayloadTypeAvpfParams avpf; /* AVPF parameters */
	int flags;
	void *user_data;
};


typedef struct _jrtpPayloadType jrtp_PayloadType;
typedef struct _jPayloadTypeAvpfParams jrtp_PayloadTypeAvpfParams;


#define jrtp_payload_type_set_flag(pt,flag) (pt)->flags|=((int)flag)
#define jrtp_payload_type_unset_flag(pt,flag) (pt)->flags&=(~(int)flag)
#define jrtp_payload_type_get_flags(pt)	(pt)->flags


extern jrtp_PayloadType *jrtp_payload_type_new(void);
extern jrtp_PayloadType *jrtp_payload_type_clone(const jrtp_PayloadType *payload);
extern char *jrtp_payload_type_get_rtpmap(jrtp_PayloadType *pt);
extern void jrtp_payload_type_destroy(jrtp_PayloadType *pt);
extern void jrtp_payload_type_set_recv_fmtp(jrtp_PayloadType *pt, const char *fmtp);
extern void jrtp_payload_type_set_send_fmtp(jrtp_PayloadType *pt, const char *fmtp);
extern void jrtp_payload_type_append_recv_fmtp(jrtp_PayloadType *pt, const char *fmtp);
extern void jrtp_payload_type_append_send_fmtp(jrtp_PayloadType *pt, const char *fmtp);
#define jrtp_payload_type_get_avpf_params(pt)	((pt)->avpf)
extern void jrtp_payload_type_set_avpf_params(jrtp_PayloadType *pt, jrtp_PayloadTypeAvpfParams params);
extern bool jrtp_payload_type_is_vbr(const jrtp_PayloadType *pt);

#define jrtp_payload_type_get_bitrate(pt)	((pt)->normal_bitrate)
#define jrtp_payload_type_get_rate(pt)		((pt)->clock_rate)
#define jrtp_payload_type_get_mime(pt)		((pt)->mime_type)

extern bool jrtp_payload_fmtp_get_value(const char *fmtp, const char *param_name, char *result, size_t result_len);

#define jrtp_payload_type_set_user_data(pt,p)	(pt)->user_data=(p)
#define jrtp_payload_type_get_user_data(pt)		((pt)->user_data)


/* some payload types */
/* audio */
extern jrtp_PayloadType jrtp_payload_type_pcmu8000;
extern jrtp_PayloadType jrtp_payload_type_pcma8000;
extern jrtp_PayloadType jrtp_payload_type_pcm8000;
extern jrtp_PayloadType jrtp_payload_type_l16_mono;
extern jrtp_PayloadType jrtp_payload_type_l16_stereo;
extern jrtp_PayloadType jrtp_payload_type_lpc1016;
extern jrtp_PayloadType jrtp_payload_type_g729;
extern jrtp_PayloadType jrtp_payload_type_g7231;
extern jrtp_PayloadType jrtp_payload_type_g7221;
extern jrtp_PayloadType jrtp_payload_type_cn;
extern jrtp_PayloadType jrtp_payload_type_g726_40;
extern jrtp_PayloadType jrtp_payload_type_g726_32;
extern jrtp_PayloadType jrtp_payload_type_g726_24;
extern jrtp_PayloadType jrtp_payload_type_g726_16;
extern jrtp_PayloadType jrtp_payload_type_aal2_g726_40;
extern jrtp_PayloadType jrtp_payload_type_aal2_g726_32;
extern jrtp_PayloadType jrtp_payload_type_aal2_g726_24;
extern jrtp_PayloadType jrtp_payload_type_aal2_g726_16;
extern jrtp_PayloadType jrtp_payload_type_gsm;
extern jrtp_PayloadType jrtp_payload_type_lpc;
extern jrtp_PayloadType jrtp_payload_type_lpc1015;
extern jrtp_PayloadType jrtp_payload_type_speex_nb;
extern jrtp_PayloadType jrtp_payload_type_speex_wb;
extern jrtp_PayloadType jrtp_payload_type_speex_uwb;
extern jrtp_PayloadType jrtp_payload_type_ilbc;
extern jrtp_PayloadType jrtp_payload_type_amr;
extern jrtp_PayloadType jrtp_payload_type_amrwb;
extern jrtp_PayloadType jrtp_payload_type_truespeech;
extern jrtp_PayloadType jrtp_payload_type_evrc0;
extern jrtp_PayloadType jrtp_payload_type_evrcb0;
extern jrtp_PayloadType jrtp_payload_type_silk_nb;
extern jrtp_PayloadType jrtp_payload_type_silk_mb;
extern jrtp_PayloadType jrtp_payload_type_silk_wb;
extern jrtp_PayloadType jrtp_payload_type_silk_swb;
extern jrtp_PayloadType jrtp_payload_type_aaceld_16k;
extern jrtp_PayloadType jrtp_payload_type_aaceld_22k;
extern jrtp_PayloadType jrtp_payload_type_aaceld_32k;
extern jrtp_PayloadType jrtp_payload_type_aaceld_44k;
extern jrtp_PayloadType jrtp_payload_type_aaceld_48k;
extern jrtp_PayloadType jrtp_payload_type_opus;
extern jrtp_PayloadType jrtp_payload_type_isac;
extern jrtp_PayloadType jrtp_payload_type_gsm_efr;
extern jrtp_PayloadType jrtp_payload_type_codec2;
extern jrtp_PayloadType jrtp_payload_type_bv16;

/* video */
extern jrtp_PayloadType jrtp_payload_type_mpv;
extern jrtp_PayloadType jrtp_payload_type_h261;
extern jrtp_PayloadType jrtp_payload_type_h263;
extern jrtp_PayloadType jrtp_payload_type_h263_1998;
extern jrtp_PayloadType jrtp_payload_type_h263_2000;
extern jrtp_PayloadType jrtp_payload_type_mp4v;
extern jrtp_PayloadType jrtp_payload_type_theora;
extern jrtp_PayloadType jrtp_payload_type_h264;
extern jrtp_PayloadType jrtp_payload_type_h265;
extern jrtp_PayloadType jrtp_payload_type_x_snow;
extern jrtp_PayloadType jrtp_payload_type_jpeg;
extern jrtp_PayloadType jrtp_payload_type_vp8;

extern jrtp_PayloadType jrtp_payload_type_g722;
extern jrtp_PayloadType jrtp_payload_type_flexfec;
/* text */
extern jrtp_PayloadType jrtp_payload_type_t140;
extern jrtp_PayloadType jrtp_payload_type_t140_red;

/* non standard file transfer over UDP */
extern jrtp_PayloadType jrtp_payload_type_x_udpftp;

/* telephone-event */
extern jrtp_PayloadType jrtp_payload_type_telephone_event;


#ifdef __cplusplus
}
#endif

#endif
