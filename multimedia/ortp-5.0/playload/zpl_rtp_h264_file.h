/*
 * zpl_rtp_h264_file.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __ZPL_RTP_H264_FILE_H__
#define __ZPL_RTP_H264_FILE_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    FILE    *fp;
    int     file_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    int     file_ofsset;
    int     msec;
	void    *t_master;
    void    *t_read
} zpl_rtp_h264_file_t;


extern zpl_rtp_h264_file_t *zpl_rtp_h264_file_create();
extern int zpl_rtp_h264_file_destroy(zpl_rtp_h264_file_t *filemedia);
extern int zpl_rtp_h264_file_open(zpl_rtp_h264_file_t *filemedia, char *filename);
extern int zpl_rtp_h264_file_get_nalu(char *filename, uint8_t *vps, uint8_t *pps, uint8_t *sps, uint8_t *sei, uint32_t *profileid);
extern int zpl_rtp_h264_file_read_thread(zpl_rtp_h264_file_t *chn);
extern int zpl_rtp_h264_file_read_start(zpl_rtp_h264_file_t *chn, int msec);
extern int zpl_rtp_h264_file_read_stop(zpl_rtp_h264_file_t *chn);



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_RTP_H264_FILE_H__ */