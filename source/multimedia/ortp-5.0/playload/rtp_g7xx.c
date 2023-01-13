#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include <ortp/ortp.h>
#include "rtp_g7xx.h"



/*
 * 发送一帧数据
 */
int rtp_payload_send_g7xx(RtpSession *session, const uint8_t *buffer, uint32_t len, int user_ts)
{
    int ret = rtp_session_send_with_ts(session, buffer, len, user_ts);
    return ret;
}
