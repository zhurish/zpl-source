/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** h264-config.h
** 
**
** -------------------------------------------------------------------------*/
#ifndef __H264_CONFIG_HPP__
#define __H264_CONFIG_HPP__

#undef PL_LIBX264_MODULE

#ifdef PL_OPENH264_MODULE
/* OpenH264: */
#include <wels/codec_api.h>
#include <wels/codec_app_def.h>

#define OPENH264_VER_AT_LEAST(maj, min) \
    ((OPENH264_MAJOR  > (maj)) || \
     (OPENH264_MAJOR == (maj) && OPENH264_MINOR >= (min)))

#endif


#ifdef PL_LIBX264_MODULE
/* x264: */
#include <x264.h>
#endif

#if 0
/**
 * Types of media frame.
 */
typedef enum media_frame_type
{
    PJMEDIA_FRAME_TYPE_NONE,	    /**< No frame.		*/
    PJMEDIA_FRAME_TYPE_AUDIO,	    /**< Normal audio frame.	*/
    PJMEDIA_FRAME_TYPE_EXTENDED,    /**< Extended audio frame.	*/
    PJMEDIA_FRAME_TYPE_VIDEO        /**< Video frame.           */

} media_frame_type;


/**
 * This structure describes a media frame.
 */
typedef struct pjmedia_frame
{
    pjmedia_frame_type	 type;	    /**< Frame type.			    */
    void		*buf;	    /**< Pointer to buffer.		    */
    pj_size_t		 size;	    /**< Frame size in bytes.		    */
    pj_timestamp	 timestamp; /**< Frame timestamp.		    */
    pj_uint32_t		 bit_info;  /**< Bit info of the frame, sample case:
					 a frame may not exactly start and end
					 at the octet boundary, so this field
					 may be used for specifying start &
					 end bit offset.		    */
} pjmedia_frame;
#endif

#endif /* __H264_CONFIG_HPP__ */