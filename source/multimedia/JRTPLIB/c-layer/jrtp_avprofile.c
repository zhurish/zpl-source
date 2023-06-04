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


#include "jrtp_payloadtype.h"
#include "jrtp_rtpprofile.h"


char jttp_offset127=127;
char jttp_offset0xD5=(char)0xD5;
char jttp_offset0[4] = {0x00, 0x00, 0x00, 0x00};

/*
 * IMPORTANT : some compiler don't support the tagged-field syntax. Those
 * macros are there to trap the problem This means that if you want to keep
 * portability, payload types must be defined with their fields in the right
 * order.
 */
#if defined(_ISOC99_SOURCE) || defined(__clang__)
// ISO C99's tagged syntax
#define TYPE(val)		.type=(val)
#define CLOCK_RATE(val)		.clock_rate=(val)
#define BITS_PER_SAMPLE(val)	.bits_per_sample=(val)
#define ZERO_PATTERN(val)	.zero_pattern=(val)
#define PATTERN_LENGTH(val)	.pattern_length=(val)
#define NORMAL_BITRATE(val)	.normal_bitrate=(val)
#define MIME_TYPE(val)		.mime_type=(val)
#define CHANNELS(val)		.channels=(val)
#define RECV_FMTP(val)		.recv_fmtp=(val)
#define SEND_FMTP(val)		.send_fmtp=(val)
#define NO_AVPF		.avpf={.features=JRTP_PAYLOAD_TYPE_AVPF_NONE, .trr_interval=0}
#define AVPF(feat, intv)		.avpf={.features=(feat), .trr_interval=(intv)}
#define FLAGS(val)		.flags=(val)
#elif defined(__GNUC__)
// GCC's legacy tagged syntax (even old versions have it)
#define TYPE(val)		type: (val)
#define CLOCK_RATE(val)		clock_rate: (val)
#define BITS_PER_SAMPLE(val)	bits_per_sample: (val)
#define ZERO_PATTERN(val)	zero_pattern: (val)
#define PATTERN_LENGTH(val)	pattern_length: (val)
#define NORMAL_BITRATE(val)	normal_bitrate: (val)
#define MIME_TYPE(val)		mime_type: (val)
#define CHANNELS(val)		channels: (val)
#define RECV_FMTP(val)		recv_fmtp: (val)
#define SEND_FMTP(val)		send_fmtp: (val)
#define NO_AVPF		avpf: {features: JRTP_PAYLOAD_TYPE_AVPF_NONE, trr_interval: 0}
#define AVPF(feat, intv)		avpf: {features: (feat), trr_interval: (intv)}
#define FLAGS(val)		flags: (val)
#else
// No tagged syntax supported
#define TYPE(val)		(val)
#define CLOCK_RATE(val)		(val)
#define BITS_PER_SAMPLE(val)	(val)
#define ZERO_PATTERN(val)	(val)
#define PATTERN_LENGTH(val)	(val)
#define NORMAL_BITRATE(val)	(val)
#define MIME_TYPE(val)		(val)
#define CHANNELS(val)		(val)
#define RECV_FMTP(val)		(val)
#define SEND_FMTP(val)		(val)
#define NO_AVPF		{JRTP_PAYLOAD_TYPE_AVPF_NONE, 0}
#define AVPF(feat, intv)		{(feat), 0, (intv)}
#define FLAGS(val)		(val)

#endif

jrtp_PayloadType jrtp_payload_type_pcmu8000={
	TYPE(JRTP_PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(8),
	ZERO_PATTERN( &jttp_offset127),
	PATTERN_LENGTH(1),
	NORMAL_BITRATE(64000),
	MIME_TYPE("PCMU"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_pcma8000={
	TYPE(JRTP_PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(8),
	ZERO_PATTERN(&jttp_offset0xD5),
	PATTERN_LENGTH(1),
	NORMAL_BITRATE(64000),
	MIME_TYPE("PCMA"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_pcm8000={
	TYPE(JRTP_PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(16),
	ZERO_PATTERN(jttp_offset0),
	PATTERN_LENGTH(1),
	NORMAL_BITRATE(128000),
	MIME_TYPE("PCM"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_l16_mono={
	TYPE(JRTP_PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(44100),
	BITS_PER_SAMPLE(16),
	ZERO_PATTERN(jttp_offset0),
	PATTERN_LENGTH(2),
	NORMAL_BITRATE(705600),				/* (44100 x 16bits per frame x 1 channel) */
	MIME_TYPE("L16"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_l16_stereo={
	TYPE(JRTP_PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(44100),
	BITS_PER_SAMPLE(32),				/* 16bits x 2 channels */
	ZERO_PATTERN(jttp_offset0),
	PATTERN_LENGTH(4),
	NORMAL_BITRATE(1411200),			/* (44100 x 16bits per frame x 2 channels) */
	MIME_TYPE("L16"),
	CHANNELS(2),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_lpc1016={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(2400),
	MIME_TYPE("1016"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_gsm={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(13500),
	MIME_TYPE("GSM"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_lpc={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(5600),		/* 20ms / 14 octets per frame */
	MIME_TYPE("LPC"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_g7231={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(6300),
	MIME_TYPE("G723"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_cn={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(8000),
	MIME_TYPE("CN"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_g729={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(8000),
	MIME_TYPE("G729"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_g7221={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(16000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(24000),
	MIME_TYPE("G7221"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_g726_40={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(40000),
	MIME_TYPE("G726-40"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_g726_32={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(32000),
	MIME_TYPE("G726-32"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_g726_24={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(24000),
	MIME_TYPE("G726-24"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_g726_16={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(16000),
	MIME_TYPE("G726-16"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_aal2_g726_40={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(40000),
	MIME_TYPE("AAL2-G726-40"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_aal2_g726_32={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(32000),
	MIME_TYPE("AAL2-G726-32"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_aal2_g726_24={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(24000),
	MIME_TYPE("AAL2-G726-24"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_aal2_g726_16={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(16000),
	MIME_TYPE("AAL2-G726-16"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_mpv={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("MPV"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};


jrtp_PayloadType jrtp_payload_type_h261={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("H261"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_h263={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("H263"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_truespeech={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(8536),
	MIME_TYPE("TSP0"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType	jrtp_payload_type_telephone_event={
	JRTP_PAYLOAD_AUDIO_PACKETIZED, /*type */
	8000,	/*clock rate */
	0,		/* bytes per sample N/A */
	NULL,	/* zero pattern N/A*/
	0,		/*pattern_length N/A */
	0,		/*	normal_bitrate */
	"telephone-event",	/* MIME subtype */
	1,		/* Audio Channels */
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};


#ifdef __cplusplus
extern "C"
{
#endif
jrtp_RtpProfile jrtp_av_profile;
#ifdef __cplusplus
}
#endif


void jrtp_av_profile_init(jrtp_RtpProfile *profile)
{
	jrtp_profile_clear_all(profile);
	profile->name="AV profile";
	
	jrtp_profile_set_payload(profile,0,&jrtp_payload_type_pcmu8000);
	jrtp_profile_set_payload(profile,1,&jrtp_payload_type_lpc1016);
	jrtp_profile_set_payload(profile,3,&jrtp_payload_type_gsm);
	jrtp_profile_set_payload(profile,7,&jrtp_payload_type_lpc);
	jrtp_profile_set_payload(profile,4,&jrtp_payload_type_g7231);
	jrtp_profile_set_payload(profile,8,&jrtp_payload_type_pcma8000);
	jrtp_profile_set_payload(profile,9,&jrtp_payload_type_g722);
	jrtp_profile_set_payload(profile,10,&jrtp_payload_type_l16_stereo);
	jrtp_profile_set_payload(profile,11,&jrtp_payload_type_l16_mono);
	jrtp_profile_set_payload(profile,13,&jrtp_payload_type_cn);
	jrtp_profile_set_payload(profile,18,&jrtp_payload_type_g729);
	jrtp_profile_set_payload(profile,31,&jrtp_payload_type_h261);
	jrtp_profile_set_payload(profile,32,&jrtp_payload_type_mpv);
	jrtp_profile_set_payload(profile,34,&jrtp_payload_type_h263);

    jrtp_profile_set_payload(profile,26,&jrtp_payload_type_jpeg);
    jrtp_profile_set_payload(profile,97,&jrtp_payload_type_mp4v);
    jrtp_profile_set_payload(profile,96,&jrtp_payload_type_h264);
    jrtp_profile_set_payload(profile,105,&jrtp_payload_type_vp8);
    jrtp_profile_set_payload(profile,100,&jrtp_payload_type_h265);

    jrtp_profile_set_payload(profile,101,&jrtp_payload_type_telephone_event);
}

void jrtp_profile_payload_update(int pt, const jrtp_PayloadType *type)
{
    //extern jrtp_RtpProfile jrtp_av_profile;
    jrtp_profile_set_payload(&jrtp_av_profile, pt, type);
}
/* these are extra payload types that can be used dynamically */
jrtp_PayloadType jrtp_payload_type_lpc1015={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(2400),
	MIME_TYPE("1015"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_speex_nb={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(8000),   /*not true: 8000 is the minimum*/
	MIME_TYPE("speex"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_bv16={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(16000),/* 5ms / 80 bits per frame */
	MIME_TYPE("BV16"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_speex_wb={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(16000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(28000),
	MIME_TYPE("speex"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_speex_uwb={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(32000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(28000),
	MIME_TYPE("speex"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_ilbc={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(13300), /* the minimum, with 30ms frames */
	MIME_TYPE("iLBC"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_amr={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(12200),
	MIME_TYPE("AMR"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_amrwb={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(16000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(23850),
	MIME_TYPE("AMR-WB"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_gsm_efr={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(12200),
	MIME_TYPE ("GSM-EFR"),
	CHANNELS(1)
};

jrtp_PayloadType jrtp_payload_type_mp4v={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("MP4V-ES"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	AVPF(JRTP_PAYLOAD_TYPE_AVPF_FIR | JRTP_PAYLOAD_TYPE_AVPF_PLI, 5000),
	FLAGS(0)
};


jrtp_PayloadType jrtp_payload_type_evrc0={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("EVRC0"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_evrcb0={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("EVRCB0"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_h263_1998={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("H263-1998"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_h263_2000={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("H263-2000"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_theora={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("theora"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_h264={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("H264"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	AVPF(JRTP_PAYLOAD_TYPE_AVPF_FIR | JRTP_PAYLOAD_TYPE_AVPF_PLI, 5000),
	FLAGS(JRTP_PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED)
};

jrtp_PayloadType jrtp_payload_type_h265={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("H265"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	AVPF(JRTP_PAYLOAD_TYPE_AVPF_FIR | JRTP_PAYLOAD_TYPE_AVPF_PLI, 5000),
	FLAGS(JRTP_PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED)
};

jrtp_PayloadType jrtp_payload_type_x_snow={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("x-snow"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_jpeg={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("JPEG"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_vp8={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("VP8"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	AVPF(JRTP_PAYLOAD_TYPE_AVPF_FIR | JRTP_PAYLOAD_TYPE_AVPF_PLI | JRTP_PAYLOAD_TYPE_AVPF_SLI | JRTP_PAYLOAD_TYPE_AVPF_RPSI, 5000),
	FLAGS(JRTP_PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED)
};

jrtp_PayloadType	jrtp_payload_type_t140={
	TYPE(JRTP_PAYLOAD_TEXT),
	CLOCK_RATE(1000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("t140"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_t140_red={
	TYPE(JRTP_PAYLOAD_TEXT),
	CLOCK_RATE(1000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("red"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType	jrtp_payload_type_x_udpftp={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(1000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("x-udpftp"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_g722={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(64000),
	MIME_TYPE("G722"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_silk_nb={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(13000),
	MIME_TYPE("SILK"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_silk_mb={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(12000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(15000),
	MIME_TYPE("SILK"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_silk_wb={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(16000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(20000),
	MIME_TYPE("SILK"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_silk_swb={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(24000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(30000),
	MIME_TYPE("SILK"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_aaceld_16k={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(16000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(24000),
	MIME_TYPE("mpeg4-generic"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_aaceld_22k={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(22050),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(32000),
	MIME_TYPE("mpeg4-generic"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_aaceld_32k={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(32000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(48000),
	MIME_TYPE("mpeg4-generic"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_aaceld_44k={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(44100),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(64000),
	MIME_TYPE("mpeg4-generic"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_aaceld_48k={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(48000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(64000),
	MIME_TYPE("mpeg4-generic"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_opus = {
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(48000), /*mandatory according to RFC*/
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(20000),
	MIME_TYPE("opus"),
	CHANNELS(2), /*mandatory according to RFC*/
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_isac = {
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(16000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(32000),
	MIME_TYPE("iSAC"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(JRTP_PAYLOAD_TYPE_IS_VBR)
};

jrtp_PayloadType jrtp_payload_type_codec2={
	TYPE(JRTP_PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(3200),
	MIME_TYPE("CODEC2"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0)
};

jrtp_PayloadType jrtp_payload_type_flexfec={
	TYPE(JRTP_PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(3200),
	MIME_TYPE("flexfec"),
	CHANNELS(0),
	RECV_FMTP("repair-window=200000"),
	SEND_FMTP("repair-window=200000"),
	NO_AVPF,
	FLAGS(0)
};