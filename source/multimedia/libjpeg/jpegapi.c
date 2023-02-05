/*
 * cdjpeg.c
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains common support routines used by the IJG application
 * programs (cjpeg, djpeg, jpegtran).
 */

#include "jpegapi.h"		/* Common decls for cjpeg/djpeg applications */

GLOBAL(void) jpeg_image_init (void)
{
    return;
}

GLOBAL(jpeg_image_t *) jpeg_image_create (const char *filename, uint32_t width, uint32_t height, uint32_t color)
{
    jpeg_image_t *jpeg_img = malloc(sizeof(jpeg_image_t));
    if(jpeg_img)
    {
        memset(jpeg_img, 0, sizeof(jpeg_image_t));
        jpeg_img->type = JPEG_IMAGE_COMPRESS;
        if(filename)
            jpeg_img->filename = strdup(filename);
        if(jpeg_img->type == JPEG_IMAGE_DECOMPRESS)
        {
            jpeg_img->jpeg_info.decompress_info.err = jpeg_std_error(&jpeg_img->jerr);
            jpeg_create_decompress(&jpeg_img->jpeg_info.decompress_info);
            jpeg_img->jerr.first_addon_message = JMSG_FIRSTADDONCODE;
            jpeg_img->jerr.last_addon_message = JMSG_LASTADDONCODE;
            //jpeg_img->jpeg_info.decompress_info.in_color_space = JCS_RGB; /* arbitrary guess */
            //jpeg_set_defaults(&jpeg_img->jpeg_info.decompress_info);
        }
        else if(jpeg_img->type == JPEG_IMAGE_COMPRESS)
        {
            jpeg_img->jpeg_info.compress_info.err = jpeg_std_error(&jpeg_img->jerr);
            jpeg_create_compress(&jpeg_img->jpeg_info.compress_info);
            jpeg_img->jerr.first_addon_message = JMSG_FIRSTADDONCODE;
            jpeg_img->jerr.last_addon_message = JMSG_LASTADDONCODE;
            //jpeg_img->jpeg_info.compress_info.in_color_space = JCS_RGB; /* arbitrary guess */
            jpeg_img->jpeg_info.compress_info.image_width = width; 	/* image width and height, in pixels */
            jpeg_img->jpeg_info.compress_info.image_height = height;
            jpeg_img->jpeg_info.compress_info.input_components = 3;		/* # of color components per pixel */
            jpeg_img->jpeg_info.compress_info.in_color_space = (J_COLOR_SPACE)color; 	/* colorspace of input image */
        }
        return jpeg_img;
    }
    return NULL;
}

GLOBAL(jpeg_image_t *) jpeg_image_open (const char *filename, uint32_t color)
{
    jpeg_image_t *jpeg_img = malloc(sizeof(jpeg_image_t));
    if(jpeg_img)
    {
        memset(jpeg_img, 0, sizeof(jpeg_image_t));
        jpeg_img->type = JPEG_IMAGE_DECOMPRESS;
        if(filename)
            jpeg_img->filename = strdup(filename);
        if(jpeg_img->type == JPEG_IMAGE_DECOMPRESS)
        {
            jpeg_img->jpeg_info.decompress_info.err = jpeg_std_error(&jpeg_img->jerr);
            jpeg_create_decompress(&jpeg_img->jpeg_info.decompress_info);
            jpeg_img->jerr.first_addon_message = JMSG_FIRSTADDONCODE;
            jpeg_img->jerr.last_addon_message = JMSG_LASTADDONCODE;
            //jpeg_img->jpeg_info.decompress_info.in_color_space = JCS_RGB; /* arbitrary guess */
            //jpeg_set_defaults(&jpeg_img->jpeg_info.decompress_info);
        }
        return jpeg_img;
    }
    return NULL;
}

GLOBAL(int) jpeg_image_set_output (jpeg_image_t *image, unsigned char ** outbuffer, size_t * outsize)
{
    if(outbuffer == NULL && image->filename)
    {
        if ((image->fp = fopen(image->filename, "wb")) == NULL) {
            fprintf(stderr, "can't open %s\n", image->filename);
            return -1;
        }
        jpeg_stdio_dest(&image->jpeg_info.compress_info, image->fp);
    }
    else
    {
        jpeg_mem_dest (&image->jpeg_info.compress_info, outbuffer, outsize);
    }
    jpeg_set_defaults(&image->jpeg_info.compress_info);
    return 0;
}

GLOBAL(int) jpeg_image_set_input (jpeg_image_t *image, unsigned char *outbuffer, size_t outsize)
{
    if(outbuffer == NULL && image->filename)
    {
        if ((image->fp = fopen(image->filename, "wb")) == NULL) {
            fprintf(stderr, "can't open %s\n", image->filename);
            return -1;
        }
        jpeg_stdio_src(&image->jpeg_info.decompress_info, image->fp);
    }
    else
    {
        jpeg_mem_src (&image->jpeg_info.decompress_info, outbuffer, outsize);
    }
    return jpeg_read_header(&image->jpeg_info.decompress_info, TRUE);
}



GLOBAL(int) jpeg_image_write (jpeg_image_t *image, int quality, uint8_t *image_buffer)
{
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    int row_stride;		/* physical row width in image buffer */
    /* TRUE ensures that we will write a complete interchange-JPEG file.
     * Pass TRUE unless you are very sure of what you're doing.
     */
    jpeg_set_quality(&image->jpeg_info.compress_info, quality, TRUE /* limit to baseline-JPEG values */);

    //////////////////////////////
    if(image->jpeg_info.compress_info.in_color_space == JCS_YCbCr)
    {
        image->jpeg_info.compress_info.raw_data_in = TRUE;
        image->jpeg_info.compress_info.jpeg_color_space = JCS_YCbCr;
        image->jpeg_info.compress_info.comp_info[0].h_samp_factor = 2;
        image->jpeg_info.compress_info.comp_info[0].v_samp_factor = 2;
        image->jpeg_info.compress_info.dct_method = JDCT_ISLOW;
    }
    /////////////////////////
    /// \brief jpeg_start_compress
    ///
    jpeg_start_compress(&image->jpeg_info.compress_info, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */

    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */
    if(image->jpeg_info.compress_info.in_color_space == JCS_RGB)
    {
        row_stride = image->jpeg_info.compress_info.image_width * 3;	/* JSAMPLEs per row in image_buffer */

        while (image->jpeg_info.compress_info.next_scanline < image->jpeg_info.compress_info.image_height) {
            /* jpeg_write_scanlines expects an array of pointers to scanlines.
           * Here the array is only one element long, but you could pass
           * more than one scanline at a time if that's more convenient.
           */
            row_pointer[0] = &image_buffer[image->jpeg_info.compress_info.next_scanline * row_stride];
            (void) jpeg_write_scanlines(&image->jpeg_info.compress_info, row_pointer, 1);
        }
    }
    if(image->jpeg_info.compress_info.in_color_space == JCS_YCbCr)
    {
        unsigned int i = 0, j = 0;
        unsigned char *pY, *pU, *pV;
        unsigned char yuvbuf[image->jpeg_info.compress_info.image_width * 3];
        pY = image_buffer ;
        pU = image_buffer +1 ;
        pV = image_buffer + 3;
        j = 1;
        while (image->jpeg_info.compress_info.next_scanline < image->jpeg_info.compress_info.image_height)
        {
            int index = 0;
            for (i = 0; i < image->jpeg_info.compress_info.image_width; i += 2)
            {
                //输入的YUV图片格式为标准的YUV444格式，所以需要把YUV420转化成YUV444.
                yuvbuf[index++] = *pY;
                yuvbuf[index++] = *pU;
                yuvbuf[index++] = *pV;
                pY += 2;
                yuvbuf[index++] = *pY;
                yuvbuf[index++] = *pU;
                yuvbuf[index++] = *pV;
                pY += 2;
                pU += 4;
                pV += 4;
            }
            row_pointer[0] = yuvbuf;
            (void)jpeg_write_scanlines(&image->jpeg_info.compress_info, row_pointer, 1);//单行图片转换压缩
            j++;
        }
    }
    return 0;
}

GLOBAL(int) jpeg_image_read (jpeg_image_t *image, uint8_t **image_buffer)
{
    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */
    /* Step 5: Start decompressor */

    (void) jpeg_start_decompress(&image->jpeg_info.decompress_info);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */
    row_stride = image->jpeg_info.decompress_info.output_width * image->jpeg_info.decompress_info.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    buffer = (*image->jpeg_info.decompress_info.mem->alloc_sarray)
            ((j_common_ptr) &image->jpeg_info.decompress_info, JPOOL_IMAGE, row_stride, 1);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    while (image->jpeg_info.decompress_info.output_scanline < image->jpeg_info.decompress_info.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
       * Here the array is only one element long, but you could ask for
       * more than one scanline at a time if that's more convenient.
       */
        (void) jpeg_read_scanlines(&image->jpeg_info.decompress_info, buffer, 1);
        /* Assume put_scanline_someplace wants a pointer and sample count. */
        //put_scanline_someplace(buffer[0], row_stride);
    }
    if(image_buffer)
        *image_buffer = (uint8_t *)buffer;
    return 0;
}

GLOBAL(int) jpeg_image_destroy (jpeg_image_t *image)
{
    if(image->type == JPEG_IMAGE_COMPRESS)
    {
        jpeg_finish_compress(&image->jpeg_info.compress_info);
        if(image->fp)
        {
            fclose(image->fp);
        }
    }
    else
        jpeg_finish_decompress(&image->jpeg_info.decompress_info);
    /* After finish_compress, we can close the output file. */
    /* Step 7: release JPEG compression object */

    /* This is an important step since it will release a good deal of memory. */
    if(image->type == JPEG_IMAGE_COMPRESS)
        jpeg_destroy_compress(&image->jpeg_info.compress_info);
    else
    {
        jpeg_destroy_decompress(&image->jpeg_info.decompress_info);
        if(image->fp)
        {
            fclose(image->fp);
        }
    }
    return 0;
}










#if 0
// outJpegFileName：输出的jpeg文件名称
// yuvData: yuv420格式的数据，其数据存储顺序为：y->u->v
// quaulity: 输出的jpeg图像质量，有效范围为0-100
int Yuv420PToJpeg(const char * outJpegFileName, unsigned char* yuvData, int image_width, int image_height, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    FILE * outfile;    // target file
    if ((outfile = fopen(outJpegFileName, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", outJpegFileName);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = 3;    // # of color components per pixel
    cinfo.in_color_space = JCS_YCbCr;  //colorspace of input image
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    //
    //  cinfo.raw_data_in = TRUE;
    cinfo.jpeg_color_space = JCS_YCbCr;
    cinfo.comp_info[0].h_samp_factor = 2;
    cinfo.comp_info[0].v_samp_factor = 2;
    /

    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];

    // 获取y、u、v三个分量各自数据的指针地址
    unsigned char *ybase, *ubase, *vbase;
    ybase = yuvData;
    ubase = yuvData + image_width*image_height;
    vbase = ubase + image_height*image_width / 4;

    unsigned char *yuvLine = new unsigned char[image_width * 3];
    memset(yuvLine, 0, image_width * 3);

    int j = 0;
    while (cinfo.next_scanline < cinfo.image_height)
    {
        int idx = 0;
        for (int i = 0; i<image_width; i++)
        {
            // 分别取y、u、v的数据
            yuvLine[idx++] = ybase[i + j * image_width];
            yuvLine[idx++] = ubase[(j>>1) * image_width/2 + (i>>1) ];
            yuvLine[idx++] = vbase[(j>>1) * image_width/2 + (i>>1) ];
        }
        row_pointer[0] = yuvLine;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        j++;
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);

    delete[]yuvLine;
    return 0;
}
#endif
