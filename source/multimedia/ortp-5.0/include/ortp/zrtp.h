/*
 The oRTP library is an RTP (Realtime Transport Protocol - rfc3550) stack.
 Copyright (C) 2011 Belledonne Communications

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ortp_zrtp_h
#define ortp_zrtp_h
#ifdef __cplusplus
extern "C" {
#endif
#include <ortp/rtpsession.h>


#ifdef HAVE_zrtp
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#include <srtp/srtp.h>
#include <libzrtpcpp/ZrtpCWrapper.h>

typedef struct _OrtpZrtpContext{
	ortp_mutex_t mutex;
	RtpSession *session;
	uint32_t timerWillTriggerAt;
	uint16_t last_recv_zrtp_seq_number;
	uint16_t last_sent_zrtp_seq_number;
	srtp_t srtpSend;
	srtp_t srtpRecv;
	zrtp_Callbacks zrtp_cb;
	ZrtpContext *zrtpContext; // back link
	RtpTransport rtpt;
	RtpTransport rtcpt;
}OrtpZrtpContext;
#else
typedef void * OrtpZrtpContext ;
#endif

typedef struct OrtpZrtpParams {
	const char *zid_file; // File where to store secrets and other information
} OrtpZrtpParams;



ORTP_PUBLIC bool_t ortp_zrtp_available(void);

ORTP_PUBLIC OrtpZrtpContext* ortp_zrtp_context_new(RtpSession *s, OrtpZrtpParams *params);
ORTP_PUBLIC OrtpZrtpContext* ortp_zrtp_multistream_new(OrtpZrtpContext* activeContext, RtpSession *s, OrtpZrtpParams *params);

ORTP_PUBLIC void ortp_zrtp_sas_verified(OrtpZrtpContext* ctx);
ORTP_PUBLIC void ortp_zrtp_sas_reset_verified(OrtpZrtpContext* ctx);

ORTP_PUBLIC void ortp_zrtp_context_destroy(OrtpZrtpContext *ctx);


#ifdef __cplusplus
}
#endif

#endif
