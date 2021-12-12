/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** vpxDecoder.h
** 
**
** -------------------------------------------------------------------------*/
#ifndef __VIDEO_DECODER_HPP__
#define __VIDEO_DECODER_HPP__
#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"
#ifdef __cplusplus
}
#endif
class videoDecoder
{
public:
    videoDecoder()
    {
        this->m_width = 0;
        this->m_height = 0;
        this->m_fmt = 0;
        this->m_fps = 0;
        this->m_out_size = 0;
        this->m_out_buf = NULL;
    }
    virtual ~videoDecoder() = default;
    virtual int videoDecoderSetup(const int width, const int height, const int fmt, const int fps) = 0;
    /*{
	      this->m_width;	
          this->m_height;
          this->m_fmt;
          this->m_fps;
          return 0;
    }
    */
    virtual int videoDecoderInput(const zpl_uint8 *frame, const int len) = 0;
    virtual int videoDecoderOutput(zpl_uint8 *frame, const int len) = 0;
    virtual zpl_uint8 *videoDecoderOutput() = 0;
    virtual int videoDecoderOutputSize(const bool clear = true) = 0;
    virtual int videoDecoderDestroy() = 0;
public:
    int m_width = 0;
    int m_height = 0;
    int m_fmt = 0;
    int m_fps = 0;
    int m_out_size = 0;
    char *m_out_buf = NULL;
};

#endif /* __VIDEO_DECODER_HPP__ */