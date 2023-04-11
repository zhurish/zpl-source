#ifndef __ZPL_MEDIA_IMAGE_H__
#define __ZPL_MEDIA_IMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif



typedef enum {
    ZPL_COLOR_UNKNOWN,		/* error/unspecified */
    ZPL_COLOR_GRAYSCALE,		/* monochrome */
    ZPL_COLOR_RGB,		/* red/green/blue, standard RGB (sRGB) */
    ZPL_COLOR_YCbCr,		/* Y/Cb/Cr (also known as YUV), standard YCC */
    ZPL_COLOR_CMYK,		/* C/M/Y/K */
    ZPL_COLOR_YCCK,		/* Y/Cb/Cr/K */
    ZPL_COLOR_BG_RGB,		/* big gamut red/green/blue, bg-sRGB */
    ZPL_COLOR_BG_YCC,		/* big gamut Y/Cb/Cr, bg-sYCC */
    ZPL_COLOR_JPEG,
    ZPL_COLOR_JPG,
} ZPL_COLOR_SPACE;

typedef struct zpl_media_image_s
{
    NODE        node;
    uint32_t    width;
    uint32_t    height;
    ZPL_COLOR_SPACE     color;
    uint8_t     quality;
    uint8_t     *image_data;
    uint32_t    image_len;
    uint32_t    image_maxsize;
}zpl_media_image_t ;


typedef struct zpl_media_imagelst_s
{
    char    *name;
    os_mutex_t	*mutex;
    zpl_uint32	maxsize;
    LIST        list;			//add queue
    LIST        ulist;			//unuse queue
}zpl_media_imglst_t;





extern zpl_media_imglst_t *zpl_media_imglst_create(char *name, zpl_uint32 maxsize);
extern int zpl_media_imglst_destroy(zpl_media_imglst_t *queue);

extern zpl_media_image_t * zpl_media_image_create(zpl_media_imglst_t *queue, zpl_uint32 width, zpl_uint32 height,
                                                  ZPL_COLOR_SPACE color, zpl_uint8 quality, uint8_t *img, zpl_uint32 len);
extern zpl_media_image_t * zpl_media_image_malloc(zpl_media_imglst_t *queue, zpl_uint32 len);
extern zpl_media_image_t * zpl_media_image_clone(zpl_media_imglst_t *queue, zpl_media_image_t *data);
extern int zpl_media_image_copy(zpl_media_image_t *dst, zpl_media_image_t *src);
extern int zpl_media_image_free(zpl_media_image_t *data);

extern zpl_media_image_t * zpl_media_image_dequeue(zpl_media_imglst_t *queue);
extern int zpl_media_image_enqueue(zpl_media_imglst_t *queue, zpl_media_image_t *data);
extern int zpl_media_image_add(zpl_media_imglst_t *queue, zpl_media_image_t *data);
extern zpl_media_image_t * zpl_media_image_get(zpl_media_imglst_t *queue);
extern int zpl_media_image_finsh(zpl_media_imglst_t *queue, zpl_media_image_t *data);

#ifdef __cplusplus
}
#endif




#endif // __ZPL_MEDIA_IMAGE_H__
