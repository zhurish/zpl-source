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

#include "rtpsession_priv.h"
#include <ortp/payloadtype.h>
#include <ortp/rtpprofile.h>
#include <ortp/ortp.h>
#include <ortp/rtcp.h>

static char offset127=127;
static char offset0xD5=(char)0xD5;
static char offset0[4] = {0x00, 0x00, 0x00, 0x00};

RtpProfile av_profile;
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
#define NO_AVPF		.avpf={.features=PAYLOAD_TYPE_AVPF_NONE, .trr_interval=0}
#define AVPF(feat, intv)		.avpf={.features=(feat), .trr_interval=(intv)}
#define FLAGS(val)		.flags=(val)
#define USERDATA(val)		.user_data=(val)
#elif defined(__GNUC__)
// GCC's legacy tagged syntax (even old versions have it)
#if 1
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
#define NO_AVPF		{PAYLOAD_TYPE_AVPF_NONE, 0, 0}
#define AVPF(feat, intv)		{(feat), FALSE, (intv)}
#define FLAGS(val)		(val)
#define USERDATA(val)		(val)
#else
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
#define NO_AVPF		avpf: {features: PAYLOAD_TYPE_AVPF_NONE, trr_interval: 0}
#define AVPF(feat, intv)		avpf: {features: (feat), trr_interval: (intv)}
#define FLAGS(val)		flags: (val)
#define USERDATA(val)		user_data: (val)
#endif
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
#define NO_AVPF		{PAYLOAD_TYPE_AVPF_NONE, 0}
#define AVPF(feat, intv)		{(feat), FALSE, (intv)}
#define FLAGS(val)		(val)
#define USERDATA(val)		(val)
#endif


static PayloadType payload_type_telephone_event={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("telephone-event\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_pcmu8000={
	TYPE(PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(8),
	ZERO_PATTERN( &offset127),
	PATTERN_LENGTH(1),
	NORMAL_BITRATE(64000),
	MIME_TYPE("PCMU\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_pcma8000={
	TYPE(PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(8),
	ZERO_PATTERN(&offset0xD5),
	PATTERN_LENGTH(1),
	NORMAL_BITRATE(64000),
	MIME_TYPE("PCMA\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};
#if 0
static PayloadType payload_type_pcm8000={
	TYPE(PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(16),
	ZERO_PATTERN(offset0),
	PATTERN_LENGTH(1),
	NORMAL_BITRATE(128000),
	MIME_TYPE("PCM\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};
#endif

static PayloadType payload_type_l16_mono={
	TYPE(PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(44100),
	BITS_PER_SAMPLE(16),
	ZERO_PATTERN(offset0),
	PATTERN_LENGTH(2),
	NORMAL_BITRATE(705600),				/* (44100 x 16bits per frame x 1 channel) */
	MIME_TYPE("L16\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_l16_stereo={
	TYPE(PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(44100),
	BITS_PER_SAMPLE(32),				/* 16bits x 2 channels */
	ZERO_PATTERN(offset0),
	PATTERN_LENGTH(4),
	NORMAL_BITRATE(1411200),			/* (44100 x 16bits per frame x 2 channels) */
	MIME_TYPE("L16\n"),
	CHANNELS(2),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_lpc1016={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(2400),
	MIME_TYPE("1016\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_gsm={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(13500),
	MIME_TYPE("GSM\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_lpc={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(5600),		/* 20ms / 14 octets per frame */
	MIME_TYPE("LPC\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_g7231={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(6300),
	MIME_TYPE("G723\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_cn={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(8000),
	MIME_TYPE("CN\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_g729={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(8000),
	MIME_TYPE("G729\n"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};
#if 0
static PayloadType payload_type_g7221={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_g726_40={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_g726_32={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_g726_24={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_g726_16={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_aal2_g726_40={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_aal2_g726_32={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_aal2_g726_24={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_aal2_g726_16={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};
#endif

static PayloadType payload_type_mpv={
	TYPE(PAYLOAD_VIDEO),
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
	FLAGS(0),
	USERDATA(NULL)
};


static PayloadType payload_type_h261={
	TYPE(PAYLOAD_VIDEO),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_h263={
	TYPE(PAYLOAD_VIDEO),
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
	FLAGS(0),
	USERDATA(NULL)
};
#if 0
static PayloadType payload_type_truespeech={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};





/* these are extra payload types that can be used dynamically */
static PayloadType payload_type_lpc1015={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_speex_nb={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_bv16={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_speex_wb={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_speex_uwb={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_ilbc={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_amr={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_amrwb={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_gsm_efr={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(12200),
	MIME_TYPE ("GSM-EFR"),
	CHANNELS(1),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	NO_AVPF,
	FLAGS(0),
	USERDATA(NULL)
};
#endif

static PayloadType payload_type_mp4v={
	TYPE(PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(0),
	MIME_TYPE("MP4V-ES"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	AVPF(PAYLOAD_TYPE_AVPF_FIR | PAYLOAD_TYPE_AVPF_PLI, RTCP_DEFAULT_REPORT_INTERVAL),
	FLAGS(0),
	USERDATA(NULL)
};

#if 0
static PayloadType payload_type_evrc0={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_evrcb0={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_h263_1998={
	TYPE(PAYLOAD_VIDEO),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_h263_2000={
	TYPE(PAYLOAD_VIDEO),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_theora={
	TYPE(PAYLOAD_VIDEO),
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
	FLAGS(0),
	USERDATA(NULL)
};
#endif

PayloadType payload_type_h264={
	TYPE(PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("H264\n"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	AVPF(PAYLOAD_TYPE_AVPF_FIR | PAYLOAD_TYPE_AVPF_PLI, RTCP_DEFAULT_REPORT_INTERVAL),
	FLAGS(PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED),
	USERDATA(NULL)
};

static PayloadType payload_type_h265={
	TYPE(PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("H265\n"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	AVPF(PAYLOAD_TYPE_AVPF_FIR | PAYLOAD_TYPE_AVPF_PLI, RTCP_DEFAULT_REPORT_INTERVAL),
	FLAGS(PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED),
	USERDATA(NULL)
};

#if 0
static PayloadType payload_type_x_snow={
	TYPE(PAYLOAD_VIDEO),
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
	FLAGS(0),
	USERDATA(NULL)
};
#endif
static PayloadType payload_type_jpeg={
	TYPE(PAYLOAD_VIDEO),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_vp8={
	TYPE(PAYLOAD_VIDEO),
	CLOCK_RATE(90000),
	BITS_PER_SAMPLE(0),
	ZERO_PATTERN(NULL),
	PATTERN_LENGTH(0),
	NORMAL_BITRATE(256000),
	MIME_TYPE("VP8"),
	CHANNELS(0),
	RECV_FMTP(NULL),
	SEND_FMTP(NULL),
	AVPF(PAYLOAD_TYPE_AVPF_FIR | PAYLOAD_TYPE_AVPF_PLI | PAYLOAD_TYPE_AVPF_SLI | PAYLOAD_TYPE_AVPF_RPSI, RTCP_DEFAULT_REPORT_INTERVAL),
	FLAGS(PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED),
	USERDATA(NULL)
};
#if 0
static PayloadType	payload_type_t140={
	TYPE(PAYLOAD_TEXT),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType payload_type_t140_red={
	TYPE(PAYLOAD_TEXT),
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
	FLAGS(0),
	USERDATA(NULL)
};

static PayloadType	payload_type_x_udpftp={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};
#endif
static PayloadType payload_type_g722={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};
#if 0
static PayloadType payload_type_silk_nb={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_silk_mb={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_silk_wb={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_silk_swb={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_aaceld_16k={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_aaceld_22k={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_aaceld_32k={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_aaceld_44k={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_aaceld_48k={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_opus = {
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_isac = {
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(PAYLOAD_TYPE_IS_VBR),
	USERDATA(NULL)
};

static PayloadType payload_type_codec2={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
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
	FLAGS(0),
	USERDATA(NULL)
};
#endif



void av_profile_init(RtpProfile *profile)
{
	rtp_profile_clear_all(profile);
	profile->name=ortp_strdup("AV profile");
	
	rtp_profile_set_payload(profile,0,&payload_type_pcmu8000);
	rtp_profile_set_payload(profile,1,&payload_type_lpc1016);
	rtp_profile_set_payload(profile,3,&payload_type_gsm);
	rtp_profile_set_payload(profile,7,&payload_type_lpc);
	rtp_profile_set_payload(profile,4,&payload_type_g7231);
	rtp_profile_set_payload(profile,8,&payload_type_pcma8000);
	rtp_profile_set_payload(profile,9,&payload_type_g722);
	rtp_profile_set_payload(profile,10,&payload_type_l16_stereo);
	rtp_profile_set_payload(profile,11,&payload_type_l16_mono);
	rtp_profile_set_payload(profile,13,&payload_type_cn);
	rtp_profile_set_payload(profile,18,&payload_type_g729);
	rtp_profile_set_payload(profile,31,&payload_type_h261);
	rtp_profile_set_payload(profile,32,&payload_type_mpv);
	rtp_profile_set_payload(profile,34,&payload_type_h263);

    rtp_profile_set_payload(profile,26,&payload_type_jpeg);
    rtp_profile_set_payload(profile,97,&payload_type_mp4v);
    rtp_profile_set_payload(profile,96,&payload_type_h264);
    rtp_profile_set_payload(profile,105,&payload_type_vp8);
    rtp_profile_set_payload(profile,100,&payload_type_h265);

    rtp_profile_set_payload(profile,101,&payload_type_telephone_event);
}

void rtp_profile_payload_update(int pt, const PayloadType *type)
{
    //extern RtpProfile av_profile;
    rtp_profile_set_payload(&av_profile, pt, type);
}

