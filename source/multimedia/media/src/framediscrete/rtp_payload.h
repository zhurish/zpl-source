#ifndef __RTP_PAYLOAD_H__
#define __RTP_PAYLOAD_H__
#ifdef __cplusplus
extern "C" {
#endif

#define RTP_IPMAXLEN 20
#define RTP_UDP_MAX_SIZE 1500
#define RTP_FIXED_HEADER_SIZE 12

#ifndef MAX_RTP_PAYLOAD_LENGTH
#  if RTP_HAS_SRTP
#     define MAX_RTP_PAYLOAD_LENGTH     (RTP_UDP_MAX_SIZE - RTP_IPMAXLEN - (128+16) RTP_FIXED_HEADER_SIZE - 8)
#  else
#     define MAX_RTP_PAYLOAD_LENGTH     1400//(UDP_MAX_SIZE - IPMAXLEN - RTP_FIXED_HEADER_SIZE - 8)
#  endif
#endif

#include "rtp_g7xx.h"
#include "rtp_h264.h"

#ifdef __cplusplus
}
#endif

#endif // __RTP_PAYLOAD_H__