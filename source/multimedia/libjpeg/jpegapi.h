/*
 * cdjpeg.h
 *
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * Modified 2019 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains common declarations for the sample applications
 * cjpeg and djpeg.  It is NOT used by the core JPEG library.
 */

#ifndef __JPEG_API_H__
#define __JPEG_API_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define JPEG_CJPEG_DJPEG	/* define proper options in jconfig.h */
//#define JPEG_INTERNAL_OPTIONS	/* cjpeg.c,djpeg.c need to see xxx_SUPPORTED */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"		/* get library error codes too */
#include "cderror.h"		/* get application-specific error codes */


typedef enum {
    JPEG_IMAGE_COMPRESS = 0,
    JPEG_IMAGE_DECOMPRESS,
}jpeg_imagetype_t;

typedef struct jpeg_image_s
{
    char        *filename;
    jpeg_imagetype_t    type;
    union
    {
        struct jpeg_decompress_struct decompress_info;
        struct jpeg_compress_struct compress_info;

    }jpeg_info;
    struct jpeg_error_mgr jerr;
    FILE    *fp;
}jpeg_image_t;


EXTERN(void) jpeg_image_init(void);
EXTERN(jpeg_image_t *) jpeg_image_create (const char *filename, uint32_t width, uint32_t height, uint32_t color);
EXTERN(jpeg_image_t *) jpeg_image_open (const char *filename, uint32_t color);
EXTERN(int) jpeg_image_set_output (jpeg_image_t *image, unsigned char ** outbuffer, size_t * outsize);
EXTERN(int) jpeg_image_set_input (jpeg_image_t *image, unsigned char *outbuffer, size_t outsize);
EXTERN(int) jpeg_image_write (jpeg_image_t *image, int quality, uint8_t *image_buffer);
EXTERN(int) jpeg_image_read (jpeg_image_t *image, uint8_t **image_buffer);
EXTERN(int) jpeg_image_destroy (jpeg_image_t *image);


#ifdef __cplusplus
}
#endif
#endif /* __JPEG_API_H__ */
