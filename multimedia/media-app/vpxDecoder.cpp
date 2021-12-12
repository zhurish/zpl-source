/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** vpxDecoder.h
**
** -------------------------------------------------------------------------*/
#include <sstream>
#include <iostream>
#include <cstring>

#include <stdint.h>
#include <stdio.h>

#include "vpxEncoder.hpp"
#include "vpxDecoder.hpp"


vpxDecoder::vpxDecoder():videoDecoder()
{
}

vpxDecoder::~vpxDecoder()
{
    videoDecoderDestroy();
}

static unsigned isqrt(unsigned i)
{
    unsigned res = 1, prev;
    
    /* Rough guess, calculate half bit of input */
    prev = i >> 2;
    while (prev) {
	prev >>= 2;
	res <<= 1;
    }

    /* Babilonian method */
    do {
	prev = res;
	res = (prev + i/prev) >> 1;
    } while ((prev+res)>>1 != res);

    return res;
}

int vpxDecoder::videoDecoderSetup(const int width, const int height, const int fmt, const int fps)
{
    int res = 0, max_res = 0;
    this->m_width = width;
    this->m_height = height;
    this->m_fmt = fmt;
    this->m_fps = fps;
    if (fmt == VIDEO_FORMAT_VP8)
    {
        res = vpx_codec_dec_init(&dec, (vpx_codec_iface_t *)vpx_codec_vp8_dx, NULL, 0);
    }
    else if (fmt == VIDEO_FORMAT_VP9)
    {
        res = vpx_codec_dec_init(&dec, (vpx_codec_iface_t *)vpx_codec_vp9_dx, NULL, 0);
    }
    /*
     * Decoder  
     */

    max_res = MAX_RX_RES;
    if (res)
    {
        printf("Failed to initialize decoder");
        return -1;
    }
    /*
    attr->dec_fmtp.param[0].name = pj_str((char *)"max-fr");
    attr->dec_fmtp.param[0].val = pj_str((char *)"30");
    attr->dec_fmtp.param[1].name = pj_str((char *)" max-fs");
    attr->dec_fmtp.param[1].val = pj_str((char *)"580");
    //Bitrate 
    attr->enc_fmt.det.vid.avg_bps = DEFAULT_AVG_BITRATE;
    attr->enc_fmt.det.vid.max_bps = DEFAULT_MAX_BITRATE;
    */
    int max_fs = 580;
    if (max_fs > 0) {
    	max_res = ((int)isqrt(max_fs * 8)) * 16;
    }
    this->dec_buf_size = (max_res * max_res * 3 >> 1) + (max_res);
    this->dec_buf = new zpl_uint8 [this->dec_buf_size];
    return 0;
}

#if 0
int vpxDecoder::vpx_unpacketize(const zpl_uint8 *buf,
                                size_t packet_size,
                                unsigned *p_desc_len)
{
    unsigned desc_len = 1;
    zpl_uint8 *p = (zpl_uint8 *)buf;

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
            zpl_uint8 *q = p + desc_len;

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
            zpl_uint8 *q = p + desc_len;
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

int vpxDecoder::vpx_decode_framed(const zpl_uint8 *packets, const  int in_size,
                                  zpl_uint8 *output, unsigned out_size)
{
    bool has_frame = false;
    unsigned i, whole_len = 0;
    vpx_codec_iter_t iter = NULL;
    vpx_image_t *img = NULL;
    vpx_codec_err_t res;
    unsigned pos = 0;
    int plane = 0;

    /* Decode */
    res = vpx_codec_decode(&dec, (const zpl_uint8 *)packets, in_size, 0, VPX_DL_REALTIME);
    if (res)
    {
        printf ("Failed to decode frame %s",
                   vpx_codec_error(&dec));
        return -1;
    }

    img = vpx_codec_get_frame(&dec, &iter);
    if (!img)
    {
        printf ("Failed to get decoded frame %s",
                   vpx_codec_error(&dec));
        return -1;
    }

    has_frame = true;

    if (img->d_w * img->d_h * 3 / 2 > out_size)
        return -1;

    //output->type = PJMEDIA_FRAME_TYPE_VIDEO;
    //output->timestamp = packets[0].timestamp;

    for (plane = 0; plane < 3; ++plane)
    {
        const zpl_uint8 *buf = img->planes[plane];
        const int stride = img->stride[plane];
        const int w = (plane ? img->d_w / 2 : img->d_w);
        const int h = (plane ? img->d_h / 2 : img->d_h);
        int y;

        for (y = 0; y < h; ++y)
        {
            memcpy((char *)output + pos, buf, w);
            pos += w;
            buf += stride;
        }
    }
    //output->size = pos;
    m_out_size = pos;
    return pos;
}

int vpxDecoder::videoDecoderInput(const  zpl_uint8 *frame, const int len)
{
    return vpx_decode_framed((zpl_uint8 *)frame, len,
                                  dec_buf, dec_buf_size);
}

int vpxDecoder::videoDecoderOutput(zpl_uint8 *frame, const int len)
{
    int ret = m_out_size > len ? len:m_out_size;
    memcpy((char *)frame, dec_buf, ret);
    return ret;
}

zpl_uint8 *vpxDecoder::videoDecoderOutput()
{
    if(dec_buf != nullptr && m_out_size > 0)
        return (zpl_uint8 *)dec_buf;
    return nullptr;
}

int vpxDecoder::videoDecoderOutputSize(const bool clear)
{
    int ret = 0;
    ret = (int)m_out_size;
    if (clear)
        m_out_size = 0;
    return ret;
}

int vpxDecoder::videoDecoderDestroy()
{
    if(dec_buf != nullptr)
        delete [] dec_buf;
    vpx_codec_destroy(&dec);
    return 0;
}
