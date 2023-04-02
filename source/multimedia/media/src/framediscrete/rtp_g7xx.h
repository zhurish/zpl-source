#ifndef __RTP_G7XX_H__
#define __RTP_G7XX_H__
#ifdef __cplusplus
extern "C" {
#endif


/*
 * 发送一帧数据
 */
int rtp_payload_send_g7xx(void *session, const uint8_t *buffer, uint32_t len, int user_ts);

#ifdef __cplusplus
}
#endif


#endif // ZPL_RTP_G7XX_H
