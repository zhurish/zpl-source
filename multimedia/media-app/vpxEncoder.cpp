/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** vpxEncoder.h
** 
**
** -------------------------------------------------------------------------*/
#include <sstream>
#include <iostream>
#include <cstring>

#include <stdint.h>
#include <stdio.h>

#include "vpxEncoder.hpp"

vpxEncoder::vpxEncoder():videoEncoder()
{
}

vpxEncoder::~vpxEncoder()
{
    videoEncoderDestroy();
}

int vpxEncoder::videoEncoderSetup(const int width, const int height, const int fmt, const int fps)
{
    this->m_width = width;
    this->m_height = height;
    this->m_fmt = fmt;
    this->m_fps = fps;
    vpx_codec_enc_cfg_t cfg;
    vpx_codec_err_t res;
    unsigned max_res = MAX_RX_RES;
    int status;
    /*
     * Encoder
     */

    /* Init encoder parameters */
    if (fmt == VIDEO_FORMAT_VP8)
    {
        res = vpx_codec_enc_config_default((vpx_codec_iface_t *)vpx_codec_vp8_cx, &cfg, 0);
    }
    else if (fmt == VIDEO_FORMAT_VP9)
    {
        res = vpx_codec_enc_config_default((vpx_codec_iface_t *)vpx_codec_vp9_cx, &cfg, 0);
    }
    if (res)
    {
        printf("Failed to get encoder default config");
        return -1;
    }
    cfg.g_w = this->m_width;
    cfg.g_h = this->m_height;
    /* timebase is the inverse of fps */
    //cfg.g_timebase.num = 1;
    //cfg.g_timebase.den = 15;
    /*info[i].clock_rate = 90000;
        info[i].dir = PJMEDIA_DIR_ENCODING_DECODING;
        info[i].dec_fmt_id_cnt = 1;
        info[i].dec_fmt_id[0] = PJMEDIA_FORMAT_I420;
        info[i].packings = PJMEDIA_VID_PACKING_PACKETS;
        info[i].fps_cnt = 3;
        info[i].fps[0].num = 15;
        info[i].fps[0].denum = 1;
        info[i].fps[1].num = 25;
        info[i].fps[1].denum = 1;
        info[i].fps[2].num = 30;
        info[i].fps[2].denum = 1;
        */
    /* bitrate in KBps */
    /* Bitrate */
    //DEFAULT_AVG_BITRATE;
    //DEFAULT_MAX_BITRATE;
    cfg.rc_target_bitrate = DEFAULT_AVG_BITRATE / 1000;

    cfg.g_pass = VPX_RC_ONE_PASS;
    cfg.rc_end_usage = VPX_CBR;
    cfg.g_threads = 4;
    cfg.g_lag_in_frames = 0;
    cfg.g_error_resilient = 0;
    cfg.rc_undershoot_pct = 95;
    cfg.rc_min_quantizer = 4;
    cfg.rc_max_quantizer = 56;
    cfg.rc_buf_initial_sz = 400;
    cfg.rc_buf_optimal_sz = 500;
    cfg.rc_buf_sz = 600;
    /* kf max distance is 60s. */
    cfg.kf_max_dist = 60 * 15 /
                      1;
    cfg.rc_resize_allowed = 0;
    cfg.rc_dropframe_thresh = 25;

    enc_input_size = cfg.g_w * cfg.g_h * 3 >> 1;

    /* Initialize encoder */
    if (fmt == VIDEO_FORMAT_VP8)
    {
        res = vpx_codec_enc_init(&enc, (vpx_codec_iface_t *)vpx_codec_vp8_cx, &cfg, 0);
    }
    else if (fmt == VIDEO_FORMAT_VP9)
    {
        res = vpx_codec_enc_init(&enc, (vpx_codec_iface_t *)vpx_codec_vp9_cx, &cfg, 0);
    }
    if (res)
    {
        printf("Failed to initialize encoder");
        return -1;
    }

    /* Values greater than 0 will increase encoder speed at the expense of
     * quality.
     * Valid range for VP8: -16..16
     * Valid range for VP9: -9..9
     */
    vpx_codec_control(&enc, VP8E_SET_CPUUSED, 9);

    return 0;
}

#if 0
int vpxEncoder::vpx_unpacketize(const unsigned char *buf,
                                size_t packet_size,
                                unsigned *p_desc_len)
{
    unsigned desc_len = 1;
    unsigned char *p = (unsigned char *)buf;

#define INC_DESC_LEN()                 \
    {                                  \
        if (++desc_len >= packet_size) \
            return -1;                 \
    }

    if (packet_size <= desc_len)
        return -1;

    if (m_fmt == VIDEO_FORMAT_VP8)
    {
        /*  0 1 2 3 4 5 6 7
         * +-+-+-+-+-+-+-+-+
         * |X|R|N|S|R| PID | (REQUIRED)
         */
        /* X: Extended control bits present. */
        if (p[0] & 0x80)
        {
            INC_DESC_LEN();
            /* |I|L|T|K| RSV   | */
            /* I: PictureID present. */
            if (p[1] & 0x80)
            {
                INC_DESC_LEN();
                /* If M bit is set, the PID field MUST contain 15 bits. */
                if (p[2] & 0x80)
                    INC_DESC_LEN();
            }
            /* L: TL0PICIDX present. */
            if (p[1] & 0x40)
                INC_DESC_LEN();
            /* T: TID present or K: KEYIDX present. */
            if ((p[1] & 0x20) || (p[1] & 0x10))
                INC_DESC_LEN();
        }
    }
    else if (m_fmt == VIDEO_FORMAT_VP9)
    {
        /*  0 1 2 3 4 5 6 7
         * +-+-+-+-+-+-+-+-+
         * |I|P|L|F|B|E|V|-| (REQUIRED)
         */
        /* I: Picture ID (PID) present. */
        if (p[0] & 0x80)
        {
            INC_DESC_LEN();
            /* If M bit is set, the PID field MUST contain 15 bits. */
            if (p[1] & 0x80)
                INC_DESC_LEN();
        }
        /* L: Layer indices present. */
        if (p[0] & 0x20)
        {
            INC_DESC_LEN();
            if (!(p[0] & 0x10))
                INC_DESC_LEN();
        }
        /* F: Flexible mode.
	 * I must also be set to 1, and if P is set, there's up to 3
	 * reference index.
	 */
        if ((p[0] & 0x10) && (p[0] & 0x80) && (p[0] & 0x40))
        {
            unsigned char *q = p + desc_len;

            INC_DESC_LEN();
            if (*q & 0x1)
            {
                q++;
                INC_DESC_LEN();
                if (*q & 0x1)
                {
                    q++;
                    INC_DESC_LEN();
                }
            }
        }
        /* V: Scalability structure (SS) data present. */
        if (p[0] & 0x2)
        {
            unsigned char *q = p + desc_len;
            unsigned N_S = (*q >> 5) + 1;

            INC_DESC_LEN();
            /* Y: Each spatial layer's frame resolution present. */
            if (*q & 0x10)
                desc_len += N_S * 4;

            /* G: PG description present flag. */
            if (*q & 0x8)
            {
                unsigned j;
                unsigned N_G = *(p + desc_len);

                INC_DESC_LEN();
                for (j = 0; j < N_G; j++)
                {
                    unsigned R;

                    q = p + desc_len;
                    INC_DESC_LEN();
                    R = (*q & 0x0F) >> 2;
                    desc_len += R;
                    if (desc_len >= packet_size)
                        return -1;
                }
            }
        }
    }
#undef INC_DESC_LEN

    *p_desc_len = desc_len;
    return 0;
}
#endif

/*
  VPX_IMG_FMT_NONE,
  VPX_IMG_FMT_YV12 =
      VPX_IMG_FMT_PLANAR | VPX_IMG_FMT_UV_FLIP | 1, < planar YVU 
  VPX_IMG_FMT_I420 = VPX_IMG_FMT_PLANAR | 2,
  VPX_IMG_FMT_I422 = VPX_IMG_FMT_PLANAR | 5,
  VPX_IMG_FMT_I444 = VPX_IMG_FMT_PLANAR | 6,
  VPX_IMG_FMT_I440 = VPX_IMG_FMT_PLANAR | 7,
  VPX_IMG_FMT_NV12 = VPX_IMG_FMT_PLANAR | 9,
  VPX_IMG_FMT_I42016 = VPX_IMG_FMT_I420 | VPX_IMG_FMT_HIGHBITDEPTH,
  VPX_IMG_FMT_I42216 = VPX_IMG_FMT_I422 | VPX_IMG_FMT_HIGHBITDEPTH,
  VPX_IMG_FMT_I44416 = VPX_IMG_FMT_I444 | VPX_IMG_FMT_HIGHBITDEPTH,
  VPX_IMG_FMT_I44016 = VPX_IMG_FMT_I440 | VPX_IMG_FMT_HIGHBITDEPTH
*/
int vpxEncoder::vpx_encode_framed(const unsigned char *packets, const int in_size,
                                  unsigned char *output, unsigned out_size, const bool force_keyframe)
{
    struct vpx_codec_data *vpx_data;
    vpx_image_t img;
    vpx_enc_frame_flags_t flags = 0;
    vpx_codec_err_t res;

    vpx_img_wrap(&img, VPX_IMG_FMT_I420,
                 this->m_width,
                 this->m_height,
                 1, (unsigned char *)packets);

    if (force_keyframe)
    {
        flags |= VPX_EFLAG_FORCE_KF;
    }

    ets = 0;//input->timestamp;
    enc_frame_size = enc_processed = 0;
    enc_iter = NULL;

    res = vpx_codec_encode(&enc, &img, ets, 1,
                           flags, VPX_DL_REALTIME);
    if (res)
    {
        printf("Failed to encode frame %s",
                   vpx_codec_error(&enc));
        return -1;
    }

    do
    {
        const vpx_codec_cx_pkt_t *pkt;

        pkt = vpx_codec_get_cx_data(&enc, &enc_iter);
        if (!pkt)
            break;
        if (pkt->kind == VPX_CODEC_CX_FRAME_PKT)
        {
            /* We have a valid frame packet */
            enc_frame_whole = (unsigned char *)pkt->data.frame.buf;
            enc_frame_size = pkt->data.frame.sz;
            enc_processed = 0;
            if (pkt->data.frame.flags & VPX_FRAME_IS_KEY)
                enc_frame_is_keyframe = true;
            else
                enc_frame_is_keyframe = false;

            break;
        }
    } while (1);
#ifndef PJMEDIA_MAX_MTU			
#  define PJMEDIA_MAX_MTU			1500
#endif    
#ifndef PJMEDIA_MAX_VID_PAYLOAD_SIZE
#  if PJMEDIA_HAS_SRTP
#     define PJMEDIA_MAX_VID_PAYLOAD_SIZE     (PJMEDIA_MAX_MTU - 20 - (128+16))
#  else
#     define PJMEDIA_MAX_VID_PAYLOAD_SIZE     (PJMEDIA_MAX_MTU - 20)
#  endif
#endif
    if (this->enc_processed < this->enc_frame_size)
    {
        unsigned payload_desc_size = 1;
        unsigned max_size = PJMEDIA_MAX_VID_PAYLOAD_SIZE - payload_desc_size;
        unsigned remaining_size = this->enc_frame_size -
                                  this->enc_processed;
        unsigned payload_len = (remaining_size > max_size)? max_size:remaining_size;
        uint8_t *p = (uint8_t *)output;

        if (payload_len + payload_desc_size > out_size)
            return -1;

        //output->timestamp = vpx_data->ets;
        //output->type = PJMEDIA_FRAME_TYPE_VIDEO;
        //output->bit_info = 0;
        if (this->enc_frame_is_keyframe)
        {
            //output->bit_info |= PJMEDIA_VID_FRM_KEYFRAME;
        }

        /* Set payload header */
        p[0] = 0;
        if (this->m_fmt == VIDEO_FORMAT_VP8)
        {
            /* Set N: Non-reference frame */
            if (!this->enc_frame_is_keyframe)
                p[0] |= 0x20;
            /* Set S: Start of VP8 partition. */
            if (this->enc_processed == 0)
                p[0] |= 0x10;
        }
        else if (this->m_fmt == VIDEO_FORMAT_VP9)
        {
            /* Set P: Inter-picture predicted frame */
            if (!this->enc_frame_is_keyframe)
                p[0] |= 0x40;
            /* Set B: Start of a frame */
            if (this->enc_processed == 0)
                p[0] |= 0x8;
            /* Set E: End of a frame */
            if (this->enc_processed + payload_len ==
                this->enc_frame_size)
            {
                p[0] |= 0x4;
            }
        }

        memcpy(p + payload_desc_size,
                  (this->enc_frame_whole + this->enc_processed),
                  payload_len);
        //output->size = payload_len + payload_desc_size;
        m_out_size = payload_len + payload_desc_size;
        this->enc_processed += payload_len;
        //*has_more = (this->enc_processed < this->enc_frame_size);
        return m_out_size;
    }
    return 0;
}

int vpxEncoder::videoEncoderInput(const unsigned char *frame, const int len,const bool keyframe)
{
    return vpx_encode_framed(frame, len,
                             enc_frame_whole, enc_input_size, keyframe);
}

int vpxEncoder::videoEncoderOutput(unsigned char *frame, const int len)
{
    int ret = m_out_size > len ? len : m_out_size;
    memcpy((unsigned char *)frame, enc_frame_whole, ret);
    return ret;
}

unsigned char *vpxEncoder::videoEncoderOutput()
{
    if (enc_frame_whole != nullptr && m_out_size > 0)
        return (unsigned char *)enc_frame_whole;
    return nullptr;
}

int vpxEncoder::videoEncoderOutputSize(const bool clear)
{
    int ret = 0;
    ret = (int)m_out_size;
    if (clear)
        m_out_size = 0;
    return ret;
}

int vpxEncoder::videoEncoderDestroy()
{
    //if(dec_buf != nullptr)
    //    delete [] dec_buf;
    vpx_codec_destroy(&enc);
    return 0;
}
