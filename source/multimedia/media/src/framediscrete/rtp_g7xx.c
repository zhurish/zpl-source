#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef ZPL_JRTPLIB_MODULE 
#include <jrtplib_api.h>
#endif
#include "rtp_payload.h"



/*
 * 发送一帧数据
 */
int rtp_payload_send_g7xx(void *session, const u_int8_t *buffer, u_int32_t len)
{
    int ret = 0;  
    #ifdef ZPL_JRTPLIB_MODULE 
    ret = zpl_mediartp_session_rtp_sendto(session, buffer, len, 255, 1, 1);
    #endif
    return ret;
}
