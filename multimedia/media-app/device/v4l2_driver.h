/*
   Copyright (C) 2006 Mauro Carvalho Chehab <mchehab@infradead.org>

   The libv4l2util Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The libv4l2util Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
  */
#ifndef __V4L2_DRIVER_H__
#define __V4L2_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <linux/videodev2.h>


#include "zpl_type.h"

/*
struct drv_list {
	void		*curr;
	struct drv_list	*next;
};
*/
#if 0
VIDIOC_QUERYCAP     /* 获取设备支持的操作 */
VIDIOC_G_FMT             /* 获取设置支持的视频格式 */
VIDIOC_S_FMT             /* 设置捕获视频的格式 */
VIDIOC_REQBUFS       /* 向驱动提出申请内存的请求 */
VIDIOC_QUERYBUF    /* 向驱动查询申请到的内存 */
VIDIOC_QBUF              /* 将空闲的内存加入可捕获视频的队列 */
VIDIOC_DQBUF           /* 将已经捕获好视频的内存拉出已捕获视频的队列 */
VIDIOC_STREAMON      /* 打开视频流 */
VIDIOC_STREAMOFF    /* 关闭视频流 */
VIDIOC_QUERYCTRL    /* 查询驱动是否支持该命令 */
VIDIOC_G_CTRL         /* 获取当前命令值 */
VIDIOC_S_CTRL         /* 设置新的命令值 */
VIDIOC_G_TUNER     /* 获取调谐器信息 */
VIDIOC_S_TUNER      /* 设置调谐器信息 */
VIDIOC_G_FREQUENCY  /* 获取调谐器频率 */
VIDIOC_S_FREQUENCY  /* 设置调谐器频率 */
#endif
struct v4l2_t_buf {
	void		*start;
	size_t		length;
};

typedef int v4l2_recebe_buffer (struct v4l2_buffer *v4l2_buf, struct v4l2_t_buf *buf);

struct v4l2_driver {
	int				fd;	/* Driver descriptor */

	int				debug;

	/* V4L2 structs */
	struct v4l2_capability		cap;
	struct v4l2_streamparm		parm;

	/* Several lists to be used to store enumbered values */
	//struct drv_list			*stds,*inputs,*fmt_caps;

	/* Stream control */
	struct v4l2_requestbuffers	reqbuf;
	struct v4l2_buffer		**v4l2_bufs;
	struct v4l2_t_buf 		*bufs;
	zpl_uint32 			sizeimage,n_bufs;

	/* Queue control */
	zpl_uint32 			waitq, currq;
};

enum v4l2_direction {
	V4L2_GET		= 1,	// Bit 1
	V4L2_SET		= 2,	// Bit 2
	V4L2_SET_GET		= 3,	// Bits 1 and 2 - sets then gets and compare
	V4L2_TRY		= 4,	// Bit 3
	V4L2_TRY_SET		= 6,	// Bits 3 and 2 - try then sets
	V4L2_TRY_SET_GET	= 7,	// Bits 3, 2 and 1- try, sets and gets
};

int v4l2_open (char *device, int debug, struct v4l2_driver *drv);
int v4l2_close (struct v4l2_driver *drv);
int v4l2_get_capabilities (struct v4l2_driver *drv);

int v4l2_get_parm (struct v4l2_driver *drv);
int v4l2_set_parm (struct v4l2_driver *drv, int fps);
/*
int v4l2_gettryset_fmt_cap (struct v4l2_driver *drv, enum v4l2_direction dir,
		      struct v4l2_format *fmt,zpl_uint32  width, zpl_uint32  height,
		      zpl_uint32  pixelformat, enum v4l2_field field);
*/			  
int v4l2_enum_fmt (struct v4l2_driver *drv,enum v4l2_buf_type type);
int v4l2_get_fmt_cap (struct v4l2_driver *drv, struct v4l2_format *fmt);
int v4l2_set_fmt_cap (struct v4l2_driver *drv, struct v4l2_format *fmt, zpl_uint32  width, zpl_uint32  height,
		      zpl_uint32  pixelformat, enum v4l2_field field);

int v4l2_mmap_bufs(struct v4l2_driver *drv, zpl_uint32 num_buffers);
int v4l2_free_bufs(struct v4l2_driver *drv);
int v4l2_start_streaming(struct v4l2_driver *drv);
int v4l2_stop_streaming(struct v4l2_driver *drv);
int v4l2_rcvbuf(struct v4l2_driver *drv, v4l2_recebe_buffer *v4l2_rec_buf);
int v4l2_read_outbuf(struct v4l2_driver *drv, struct v4l2_t_buf *obuf);

int v4l2_getset_freq (struct v4l2_driver *drv, enum v4l2_direction dir,
		      double *freq);

//获取手动白平衡
int v4l2_get_balance(struct v4l2_driver *drv);
//设置白平衡色温
int v4l2_set_balance_temperature(struct v4l2_driver *drv, int temp);
//设置亮度
int v4l2_set_brightness(struct v4l2_driver *drv, int temp);
//设置对比度
int v4l2_set_contrast(struct v4l2_driver *drv, int temp);
//设置饱和度
int v4l2_set_saturation(struct v4l2_driver *drv, int temp);
//设置色度
int v4l2_set_hue(struct v4l2_driver *drv, int temp);
//设置锐度
int v4l2_set_sharpness(struct v4l2_driver *drv, int temp);
//设置背光补偿
int v4l2_set_backlight_compensation(struct v4l2_driver *drv, int temp);
/*
设置曝光值：
1. 首先将曝光模式修改为手动曝光。
2. 设置曝光档次或者具体的曝光值。
例1：得到曝光模式，设置为手动曝光模式
*/
int v4l2_set_exposure(struct v4l2_driver *drv, int temp);

/*
int v4l2_enum_stds (struct v4l2_driver *drv);
int v4l2_enum_input (struct v4l2_driver *drv);
int v4l2_setget_std (struct v4l2_driver *drv, enum v4l2_direction dir, v4l2_std_id *id);
int v4l2_setget_input (struct v4l2_driver *drv, enum v4l2_direction dir, struct v4l2_input *input);
*/
 
#ifdef __cplusplus
}
#endif
 
#endif /* __V4L2_DRIVER_H__ */