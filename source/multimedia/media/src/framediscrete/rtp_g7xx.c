#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef ZPL_LIBORTP_MODULE
#include <ortp/ortp.h>
#endif
#include "rtp_g7xx.h"



/*
 * 发送一帧数据
 */
int rtp_payload_send_g7xx(void *session, const uint8_t *buffer, uint32_t len, int user_ts)
{
#ifdef ZPL_LIBORTP_MODULE    
    int ret = rtp_session_send_with_ts(session, buffer, len, user_ts);
#else
    int ret = 0;
#endif    
    return ret;
}
