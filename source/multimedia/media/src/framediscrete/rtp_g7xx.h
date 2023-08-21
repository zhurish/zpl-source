#ifndef __RTP_G7XX_H__
#define __RTP_G7XX_H__
#ifdef __cplusplus
extern "C" {
#endif


/*
 * 发送一帧数据
 */
int rtp_payload_send_g7xx(void *session, const u_int8_t *buffer, u_int32_t len);

#ifdef __cplusplus
}
#endif


#endif // ZPL_RTP_G7XX_H
