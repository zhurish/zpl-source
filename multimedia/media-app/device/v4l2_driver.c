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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdlib.h>

#include "v4l2_driver.h"

/****************************************************************************
	Auxiliary routines
 ****************************************************************************/
static int xioctl(int fd, ospl_uint64 request, void *arg)
{
	int r;

	do
		r = ioctl(fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}
/*
static void free_list(struct drv_list **list_ptr)
{
	struct drv_list *prev,*cur;

	if (list_ptr==NULL)
		return;

	prev=*list_ptr;
	if (prev==NULL)
		return;

	do {
		cur=prev->next;
		if (prev->curr)
			free (prev->curr);	// Free data
		free (prev);			// Free list
		prev=cur;
	} while (prev);

	*list_ptr=NULL;
}
*/
/****************************************************************************
	Auxiliary Arrays to aid debug messages
 ****************************************************************************/
char *v4l2_field_names[] = {
	[V4L2_FIELD_ANY] = "any",
	[V4L2_FIELD_NONE] = "none",
	[V4L2_FIELD_TOP] = "top",
	[V4L2_FIELD_BOTTOM] = "bottom",
	[V4L2_FIELD_INTERLACED] = "interlaced",
	[V4L2_FIELD_SEQ_TB] = "seq-tb",
	[V4L2_FIELD_SEQ_BT] = "seq-bt",
	[V4L2_FIELD_ALTERNATE] = "alternate",
};

char *v4l2_type_names[] = {
	[V4L2_BUF_TYPE_VIDEO_CAPTURE] = "video-cap",
	[V4L2_BUF_TYPE_VIDEO_OVERLAY] = "video-over",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT] = "video-out",
	[V4L2_BUF_TYPE_VBI_CAPTURE] = "vbi-cap",
	[V4L2_BUF_TYPE_VBI_OUTPUT] = "vbi-out",
	[V4L2_BUF_TYPE_SLICED_VBI_CAPTURE] = "sliced-vbi-cap",
	[V4L2_BUF_TYPE_SLICED_VBI_OUTPUT] = "slicec-vbi-out",
};

static char *v4l2_memory_names[] = {
	[V4L2_MEMORY_MMAP] = "mmap",
	[V4L2_MEMORY_USERPTR] = "userptr",
	[V4L2_MEMORY_OVERLAY] = "overlay",
};

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))
#define prt_names(a, arr) (((a) < ARRAY_SIZE(arr)) ? arr[a] : "unknown")

static char *prt_caps(ospl_uint32  caps)
{
	static char s[4096] = "";

	if (V4L2_CAP_VIDEO_CAPTURE & caps)
		strcat(s, "CAPTURE ");
	if (V4L2_CAP_VIDEO_CAPTURE_MPLANE & caps)
		strcat(s, "CAPTURE_MPLANE ");
	if (V4L2_CAP_VIDEO_OUTPUT & caps)
		strcat(s, "OUTPUT ");
	if (V4L2_CAP_VIDEO_OUTPUT_MPLANE & caps)
		strcat(s, "OUTPUT_MPLANE ");
	if (V4L2_CAP_VIDEO_M2M & caps)
		strcat(s, "M2M ");
	if (V4L2_CAP_VIDEO_M2M_MPLANE & caps)
		strcat(s, "M2M_MPLANE ");
	if (V4L2_CAP_VIDEO_OVERLAY & caps)
		strcat(s, "OVERLAY ");
	if (V4L2_CAP_VBI_CAPTURE & caps)
		strcat(s, "VBI_CAPTURE ");
	if (V4L2_CAP_VBI_OUTPUT & caps)
		strcat(s, "VBI_OUTPUT ");
	if (V4L2_CAP_SLICED_VBI_CAPTURE & caps)
		strcat(s, "SLICED_VBI_CAPTURE ");
	if (V4L2_CAP_SLICED_VBI_OUTPUT & caps)
		strcat(s, "SLICED_VBI_OUTPUT ");
	if (V4L2_CAP_RDS_CAPTURE & caps)
		strcat(s, "RDS_CAPTURE ");
	if (V4L2_CAP_RDS_OUTPUT & caps)
		strcat(s, "RDS_OUTPUT ");
	if (V4L2_CAP_SDR_CAPTURE & caps)
		strcat(s, "SDR_CAPTURE ");
	if (V4L2_CAP_TUNER & caps)
		strcat(s, "TUNER ");
	if (V4L2_CAP_HW_FREQ_SEEK & caps)
		strcat(s, "HW_FREQ_SEEK ");
	if (V4L2_CAP_MODULATOR & caps)
		strcat(s, "MODULATOR ");
	if (V4L2_CAP_AUDIO & caps)
		strcat(s, "AUDIO ");
	if (V4L2_CAP_RADIO & caps)
		strcat(s, "RADIO ");
	if (V4L2_CAP_READWRITE & caps)
		strcat(s, "READWRITE ");
	if (V4L2_CAP_ASYNCIO & caps)
		strcat(s, "ASYNCIO ");
	if (V4L2_CAP_STREAMING & caps)
		strcat(s, "STREAMING ");
	if (V4L2_CAP_EXT_PIX_FORMAT & caps)
		strcat(s, "EXT_PIX_FORMAT ");
	if (V4L2_CAP_DEVICE_CAPS & caps)
		strcat(s, "DEVICE_CAPS ");

	return s;
}

static void prt_buf_info(char *name, struct v4l2_buffer *p)
{
	struct v4l2_timecode *tc = &p->timecode;

	printf("%s: %02ld:%02d:%02d.%08ld index=%d, type=%s, "
		   "bytesused=%d, flags=0x%08x, "
		   "field=%s, sequence=%d, memory=%s, offset=0x%08x, length=%d\n",
		   name, (p->timestamp.tv_sec / 3600),
		   (int)(p->timestamp.tv_sec / 60) % 60,
		   (int)(p->timestamp.tv_sec % 60),
		   p->timestamp.tv_usec,
		   p->index,
		   prt_names(p->type, v4l2_type_names),
		   p->bytesused, p->flags,
		   prt_names(p->field, v4l2_field_names),
		   p->sequence,
		   prt_names(p->memory, v4l2_memory_names),
		   p->m.offset,
		   p->length);
	tc = &p->timecode;
	printf("\tTIMECODE: %02d:%02d:%02d type=%d, "
		   "flags=0x%08x, frames=%d, userbits=0x%02x%02x%02x%02x\n",
		   tc->hours, tc->minutes, tc->seconds,
		   tc->type, tc->flags, tc->frames,
		   tc->userbits[0],
		   tc->userbits[1],
		   tc->userbits[2],
		   tc->userbits[3]);
}

/****************************************************************************
	Open V4L2 devices
 ****************************************************************************/
int v4l2_open(char *device, int debug, struct v4l2_driver *drv)
{
	int ret = 0;
	memset(drv, 0, sizeof(*drv));
	drv->debug = debug;

	if ((drv->fd = open(device, O_RDWR)) < 0)
	{
		return (-errno);
	}
	return ret;
}

int v4l2_get_capabilities(struct v4l2_driver *drv)
{
	int ret = 0;
	ret = xioctl(drv->fd, VIDIOC_QUERYCAP, (void *)&drv->cap);
	if (ret < 0)
		return -1;
	if (drv->debug)
	{
		printf("driver=%s, card=%s, bus=%s, version=%d.%d.%d, "
			   "capabilities=%s, device_caps=%s\n",
			   drv->cap.driver, drv->cap.card, drv->cap.bus_info,
			   (drv->cap.version >> 16) & 0xff,
			   (drv->cap.version >> 8) & 0xff,
			   drv->cap.version & 0xff,
			   prt_caps(drv->cap.capabilities),
			   (drv->cap.capabilities & V4L2_CAP_DEVICE_CAPS) ? prt_caps(drv->cap.device_caps) : "N/A");
	}
	return 0;
}
/****************************************************************************
	V4L2 Eumberations
 ****************************************************************************/
/*
int v4l2_enum_stds (struct v4l2_driver *drv)
{
	struct v4l2_standard	*p=NULL;
	struct drv_list		*list;
	int			ok=0,i;

	free_list(&drv->stds);

	list=drv->stds=calloc(1,sizeof(*drv->stds));
	assert (list!=NULL);

	for (i=0; ok==0; i++) {
		p=calloc(1,sizeof(*p));
		assert (p);

		p->index=i;
		ok=xioctl(drv->fd,VIDIOC_ENUMSTD,p);
		if (ok<0) {
			ok=-errno;
			free(p);
			break;
		}
		if (drv->debug) {
			printf ("STANDARD: index=%d, id=0x%08x, name=%s, fps=%.3f, "
				"framelines=%d\n", p->index,
				(ospl_uint32)p->id, p->name,
				1.*p->frameperiod.denominator/p->frameperiod.numerator,
				p->framelines);
		}
		if (list->curr) {
			list->next=calloc(1,sizeof(*list->next));
			list=list->next;
			assert (list!=NULL);
		}
		list->curr=p;
	}
	if (i>0 && ok==-EINVAL)
		return 0;

	return ok;
}

int v4l2_enum_input (struct v4l2_driver *drv)
{
	struct v4l2_input	*p=NULL;
	struct drv_list		*list;
	int			ok=0,i;

	free_list(&drv->inputs);

	list=drv->inputs=calloc(1,sizeof(*drv->inputs));
	assert (list!=NULL);

	for (i=0; ok==0; i++) {
		p=calloc(1,sizeof(*p));
		assert (p);
		p->index=i;
		ok=xioctl(drv->fd,VIDIOC_ENUMINPUT,p);
		if (ok<0) {
			ok=errno;
			free(p);
			break;
		}
		if (drv->debug) {
			printf ("INPUT: index=%d, name=%s, type=%d, audioset=%d, "
				"tuner=%d, std=%08x, status=%d\n",
				p->index,p->name,p->type,p->audioset, p->tuner,
				(ospl_uint32)p->std, p->status);
		}
		if (list->curr) {
			list->next=calloc(1,sizeof(*list->next));
			list=list->next;
			assert (list!=NULL);
		}
		list->curr=p;
	}
	if (i>0 && ok==-EINVAL)
		return 0;
	return ok;
}
*/
int v4l2_enum_fmt(struct v4l2_driver *drv, enum v4l2_buf_type type)
{
	struct v4l2_fmtdesc *p = NULL;
	int ok = 0, i;
	{
		p = calloc(1, sizeof(*p));
		assert(p != NULL);

		p->index = 0;
		p->type = type;
		while (1)
		{
			ok = xioctl(drv->fd, VIDIOC_ENUM_FMT, p);
			if (ok < 0)
			{
				ok = errno;
				free(p);
				break;
			}
			p->index++;
			if (drv->debug)
			{
				printf("FORMAT: index=%d, type=%d, flags=%d, description='%s'\n\t"
					   "fourcc=%c%c%c%c\n",
					   p->index, p->type, p->flags, p->description,
					   p->pixelformat & 0xff,
					   (p->pixelformat >> 8) & 0xff,
					   (p->pixelformat >> 16) & 0xff,
					   (p->pixelformat >> 24) & 0xff);
			}
		}
	}
	if (i > 0 && ok == -EINVAL)
		return 0;
	return ok;
}

/****************************************************************************
	Set routines - currently, it also checks results with Get
 ****************************************************************************/
/*
int v4l2_setget_std (struct v4l2_driver *drv, enum v4l2_direction dir, v4l2_std_id *id)
{
	v4l2_std_id		s_id=*id;
	int			ret=0;
	char			s[256];

	if (dir & V4L2_SET) {
		ret=xioctl(drv->fd,VIDIOC_S_STD,&s_id);
		if (ret<0) {
			ret=errno;

			sprintf (s,"while trying to set STD to %08x",
								(ospl_uint32) *id);
			perror(s);
		}
	}

	if (dir & V4L2_GET) {
		ret=xioctl(drv->fd,VIDIOC_G_STD,&s_id);
		if (ret<0) {
			ret=errno;
			perror ("while trying to get STD id");
		}
	}

	if (dir == V4L2_SET_GET) {
		if (*id & s_id) {
			if (*id != s_id) {
				printf ("Warning: Received a std subset (%08x"
					" std) while trying to adjust to %08x\n",
					(ospl_uint32) s_id,(ospl_uint32) *id);
			}
		} else {
			fprintf (stderr,"Error: Received %08x std while trying"
				" to adjust to %08x\n",
				(ospl_uint32) s_id,(ospl_uint32) *id);
		}
	}
	return ret;
}

int v4l2_setget_input (struct v4l2_driver *drv, enum v4l2_direction dir, struct v4l2_input *input)
{
	int			ok=0,ret;
	ospl_uint32		inp=input->index;
	char			s[256];

	if (dir & V4L2_SET) {
		ret=xioctl(drv->fd,VIDIOC_S_INPUT,input);
		if (ret<0) {
			ret=errno;
			sprintf (s,"while trying to set INPUT to %d\n", inp);
			perror(s);
		}
	}

	if (dir & V4L2_GET) {
		ret=xioctl(drv->fd,VIDIOC_G_INPUT,input);
		if (ret<0) {
			perror ("while trying to get INPUT id\n");
		}
	}

	if (dir & V4L2_SET_GET) {
		if (input->index != inp) {
			printf ("Input is different than expected (received %i, set %i)\n",
						inp, input->index);
		}
	}

	return ok;
}
*/
int v4l2_get_fmt_cap(struct v4l2_driver *drv, struct v4l2_format *fmt)
{
	int ret = 0;
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = xioctl(drv->fd, VIDIOC_G_FMT, fmt);
	if (ret < 0)
	{
		ret = errno;
		perror("VIDIOC_G_FMT failed\n");
	}
	return ret;
}

int v4l2_set_fmt_cap(struct v4l2_driver *drv, struct v4l2_format *fmt, ospl_uint32  width, ospl_uint32  height,
					 ospl_uint32  pixelformat, enum v4l2_field field)
{
	int ret = 0;
	struct v4l2_pix_format *pix = &(fmt->fmt.pix);
	pix->width = width;
	pix->height = height;
	pix->pixelformat = pixelformat;
	pix->field = field;
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = xioctl(drv->fd, VIDIOC_TRY_FMT, fmt);
	if (ret < 0)
	{
		perror("VIDIOC_TRY_FMT failed\n");
		return -1;
	}

	ret = xioctl(drv->fd, VIDIOC_S_FMT, fmt);
	if (ret < 0)
	{
		perror("VIDIOC_S_FMT failed\n");
	}
	drv->sizeimage = pix->sizeimage;
	if (pix->pixelformat != pixelformat)
	{
		fprintf(stderr, "Error: asked for format %d, received %d", pixelformat,
				pix->pixelformat);
	}
	if (pix->width != width)
	{
		fprintf(stderr, "Error: asked for format %d, received %d\n", width,
				pix->width);
	}
	if (pix->height != height)
	{
		fprintf(stderr, "Error: asked for format %d, received %d\n", height,
				pix->height);
	}
	if (pix->bytesperline == 0)
	{
		fprintf(stderr, "Error: bytesperline = 0\n");
	}
	if (pix->sizeimage == 0)
	{
		fprintf(stderr, "Error: sizeimage = 0\n");
	}
	if (drv->debug)
		printf("FMT SET: %dx%d, fourcc=%c%c%c%c, %d bytes/line,"
			   " %d bytes/frame, colorspace=0x%08x\n",
			   pix->width, pix->height,
			   pix->pixelformat & 0xff,
			   (pix->pixelformat >> 8) & 0xff,
			   (pix->pixelformat >> 16) & 0xff,
			   (pix->pixelformat >> 24) & 0xff,
			   pix->bytesperline,
			   pix->sizeimage,
			   pix->colorspace);

	return ret;
}
/*
int v4l2_gettryset_fmt_cap (struct v4l2_driver *drv, enum v4l2_direction dir,
		      struct v4l2_format *fmt,ospl_uint32  width, ospl_uint32  height,
		      ospl_uint32  pixelformat, enum v4l2_field field)
{
	struct v4l2_pix_format  *pix=&(fmt->fmt.pix);
	int			ret=0;
	fmt->type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (dir == V4L2_GET) {
		ret=xioctl(drv->fd,VIDIOC_G_FMT,fmt);
		if (ret < 0) {
			ret=errno;
			perror("VIDIOC_G_FMT failed\n");
		}
		return ret;
	} else if (dir & (~(V4L2_TRY|V4L2_SET)) ) {
		perror ("Invalid direction\n");
		return EINVAL;
	}
	if (dir & (V4L2_TRY|V4L2_SET)) {
		pix->width       = width;
		pix->height      = height;
		pix->pixelformat = pixelformat;
		pix->field       = field;
		if (dir & V4L2_TRY) {
			ret=xioctl(drv->fd,VIDIOC_TRY_FMT,fmt);
			if (ret < 0) {
				perror("VIDIOC_TRY_FMT failed\n");
			}
		}
		if (dir & V4L2_SET) {
			ret=xioctl(drv->fd,VIDIOC_S_FMT,fmt);
			if (ret < 0) {
				perror("VIDIOC_S_FMT failed\n");
			}
			drv->sizeimage=pix->sizeimage;
		}
		if (pix->pixelformat != pixelformat) {
			fprintf(stderr,"Error: asked for format %d, received %d",pixelformat,
				pix->pixelformat);
		}
		if (pix->width != width) {
			fprintf(stderr,"Error: asked for format %d, received %d\n",width,
				pix->width);
		}
		if (pix->height != height) {
			fprintf(stderr,"Error: asked for format %d, received %d\n",height,
				pix->height);
		}
		if (pix->bytesperline == 0 ) {
			fprintf(stderr,"Error: bytesperline = 0\n");
		}
		if (pix->sizeimage == 0 ) {
			fprintf(stderr,"Error: sizeimage = 0\n");
		}
	}
	if (drv->debug)
		printf( "FMT SET: %dx%d, fourcc=%c%c%c%c, %d bytes/line,"
			" %d bytes/frame, colorspace=0x%08x\n",
			pix->width,pix->height,
			pix->pixelformat & 0xff,
			(pix->pixelformat >>  8) & 0xff,
			(pix->pixelformat >> 16) & 0xff,
			(pix->pixelformat >> 24) & 0xff,
			pix->bytesperline,
			pix->sizeimage,
			pix->colorspace);

	return 0;
}
*/
/****************************************************************************
	Get routines
 ****************************************************************************/
int v4l2_get_parm(struct v4l2_driver *drv)
{
	int ret = 0;
	struct v4l2_captureparm *c;

	drv->parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; //获取帧率信息
	if ((ret = xioctl(drv->fd, VIDIOC_G_PARM, &drv->parm)) >= 0)
	{
		c = &drv->parm.parm.capture;
		printf("PARM: capability=%d, capturemode=%d, %.3f fps "
			   "ext=%x, readbuf=%d\n",
			   c->capability,
			   c->capturemode,
			   c->timeperframe.denominator * 1. / c->timeperframe.numerator,
			   c->extendedmode, c->readbuffers);
	}
	else
	{
		ret = errno;

		perror("VIDIOC_G_PARM");
	}

	return ret;
}

int v4l2_set_parm(struct v4l2_driver *drv, int fps)
{
	int ret = 0;
	struct v4l2_captureparm *c;
	struct v4l2_streamparm parm;
	c = &drv->parm.parm.capture;
	drv->parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; //获取帧率信息
	c->timeperframe.denominator = fps;
	c->timeperframe.numerator = 1;

	if ((ret = xioctl(drv->fd, VIDIOC_S_PARM, &drv->parm)) < 0)
	{
		ret = errno;
		perror("VIDIOC_S_PARM");
	}
	if ((ret = xioctl(drv->fd, VIDIOC_G_PARM, &parm)) >= 0)
	{
		c = &parm.parm.capture;
		printf("PARM: capability=%d, capturemode=%d, %.3f fps "
			   "ext=%x, readbuf=%d\n",
			   c->capability,
			   c->capturemode,
			   c->timeperframe.denominator * 1. / c->timeperframe.numerator,
			   c->extendedmode, c->readbuffers);
	}
	else
	{
		ret = errno;
		perror("VIDIOC_G_PARM");
	}
	return ret;
}
/****************************************************************************
	Queue and stream control
 ****************************************************************************/
int v4l2_free_bufs(struct v4l2_driver *drv)
{
	ospl_uint32 i;

	if (!drv->n_bufs)
		return 0;

	/* Requests the driver to free all buffers */
	drv->reqbuf.count = 0;
	drv->reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	drv->reqbuf.memory = V4L2_MEMORY_MMAP;

	/* stop capture */
	if (xioctl(drv->fd, VIDIOC_STREAMOFF, &drv->reqbuf.type) < 0)
		return errno;

	sleep(1); // FIXME: Should check if all buffers are stopped

/* V4L2 API says REQBUFS with count=0 should be used to release buffer.
   However, video-buf.c doesn't implement it.
 */
#if 0
	if (xioctl(drv->fd,VIDIOC_REQBUFS,&drv->reqbuf)<0) {
		perror("reqbufs while freeing buffers");
		return errno;
	}
#endif

	if (drv->reqbuf.count != 0)
	{
		fprintf(stderr, "REQBUFS returned %d buffers while asking for freeing it!\n",
				drv->reqbuf.count);
	}

	for (i = 0; i < drv->n_bufs; i++)
	{
		if (drv->bufs[i].length)
			munmap(drv->bufs[i].start, drv->bufs[i].length);
		if (drv->v4l2_bufs[i])
			free(drv->v4l2_bufs[i]);
	}

	free(drv->v4l2_bufs);
	free(drv->bufs);

	drv->v4l2_bufs = NULL;
	drv->bufs = NULL;
	drv->n_bufs = 0;

	return 0;
}

int v4l2_mmap_bufs(struct v4l2_driver *drv, ospl_uint32 num_buffers)
{
	/* Frees previous allocations, if required */
	v4l2_free_bufs(drv);

	if (drv->sizeimage == 0)
	{
		fprintf(stderr, "Image size is zero! Can't proceed\n");
		return -1;
	}
	/* Requests the specified number of buffers */
	drv->reqbuf.count = num_buffers;
	drv->reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	drv->reqbuf.memory = V4L2_MEMORY_MMAP;

	if (xioctl(drv->fd, VIDIOC_REQBUFS, &drv->reqbuf) < 0)
	{
		perror("reqbufs");
		return errno;
	}

	if (drv->debug)
		printf("REQBUFS: count=%d, type=%s, memory=%s\n",
			   drv->reqbuf.count,
			   prt_names(drv->reqbuf.type, v4l2_type_names),
			   prt_names(drv->reqbuf.memory, v4l2_memory_names));

	/* Allocates the required number of buffers */
	drv->v4l2_bufs = calloc(drv->reqbuf.count, sizeof(*drv->v4l2_bufs));
	assert(drv->v4l2_bufs != NULL);
	drv->bufs = calloc(drv->reqbuf.count, sizeof(*drv->bufs));
	assert(drv->bufs != NULL);

	for (drv->n_bufs = 0; drv->n_bufs < drv->reqbuf.count; drv->n_bufs++)
	{
		struct v4l2_buffer *p;

		/* Requests kernel buffers to be mmapped */
		p = drv->v4l2_bufs[drv->n_bufs] = calloc(1, sizeof(*p));
		assert(p != NULL);
		p->index = drv->n_bufs;
		p->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		p->memory = V4L2_MEMORY_MMAP;
		if (xioctl(drv->fd, VIDIOC_QUERYBUF, p) < 0)
		{
			int ret = errno;
			perror("querybuf");

			free(drv->v4l2_bufs[drv->n_bufs]);

			v4l2_free_bufs(drv);
			return ret;
		}

		if (drv->debug)
			prt_buf_info("QUERYBUF", p);

		if (drv->sizeimage != p->length)
		{
			if (drv->sizeimage < p->length)
			{
				fprintf(stderr, "QUERYBUF: ERROR: VIDIOC_S_FMT said buffer should have %d size, but received %d from QUERYBUF!\n",
						drv->sizeimage, p->length);
			}
			else
			{
				fprintf(stderr, "QUERYBUF: Expecting %d size, received %d buff length\n",
						drv->sizeimage, p->length);
			}
		}

		drv->bufs[drv->n_bufs].length = p->length;
		drv->bufs[drv->n_bufs].start = mmap(NULL, /* start anywhere */
											p->length,
											PROT_READ | PROT_WRITE, /* required */
											MAP_SHARED,				/* recommended */
											drv->fd, p->m.offset);

		if (MAP_FAILED == drv->bufs[drv->n_bufs].start)
		{
			perror("mmap");

			free(drv->v4l2_bufs[drv->n_bufs]);
			v4l2_free_bufs(drv);
			return errno;
		}
	}

	return 0;
}

/* Returns <0, if error, 0 if nothing to read and <size>, if something
   read
 */
int v4l2_rcvbuf(struct v4l2_driver *drv, v4l2_recebe_buffer *rec_buf)
{
	int ret;

	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(drv->fd, VIDIOC_DQBUF, &buf))
	{
		switch (errno)
		{
		case EAGAIN:
			return 0;

		case EIO:
			/* Could ignore EIO, see spec. */

			/* fall through */

		default:
			perror("dqbuf");
			return -errno;
		}
	}
	prt_buf_info("DQBUF", &buf);

	assert(buf.index < drv->n_bufs);

	ret = rec_buf(&buf, &drv->bufs[buf.index]);

	if (ret < 0)
	{
		v4l2_free_bufs(drv);
		return ret;
	}

	if (-1 == xioctl(drv->fd, VIDIOC_QBUF, &buf))
	{
		perror("qbuf");
		return -errno;
	}
	return ret;
}

int v4l2_read_outbuf(struct v4l2_driver *drv, struct v4l2_t_buf *obuf)
{
	int ret;
	int gsize = 0;
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (-1 == xioctl(drv->fd, VIDIOC_DQBUF, &buf))
	{
		switch (errno)
		{
		case EAGAIN:
			return 0;
		case EIO:
			/* Could ignore EIO, see spec. */
			/* fall through */
		default:
			perror("dqbuf");
			return -errno;
		}
	}
	prt_buf_info("DQBUF", &buf);
	assert(buf.index < drv->n_bufs);
	gsize = (buf.bytesused > obuf->length) ? buf.bytesused : obuf->length;
	memcpy(obuf->start, drv->bufs[buf.index].start, gsize);
	ret = gsize;
	//ret = rec_buf (&buf, &drv->bufs[buf.index]);

	if (ret < 0)
	{
		v4l2_free_bufs(drv);
		return ret;
	}
	if (-1 == xioctl(drv->fd, VIDIOC_QBUF, &buf))
	{
		perror("qbuf");
		return -errno;
	}
	return ret;
}

int v4l2_start_streaming(struct v4l2_driver *drv)
{
	ospl_uint32  i;
	struct v4l2_buffer buf;

	if (drv->debug)
		printf("Activating %d queues\n", drv->n_bufs);
	for (i = 0; i < drv->n_bufs; i++)
	{
		int res;

		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		res = xioctl(drv->fd, VIDIOC_QBUF, &buf);

		if (!res)
			prt_buf_info("QBUF", &buf);
		else
		{
			perror("qbuf");
			return errno;
		}
	}

	/* Activates stream */
	if (drv->debug)
		printf("Enabling streaming\n");

	if (xioctl(drv->fd, VIDIOC_STREAMON, &drv->reqbuf.type) < 0)
		return errno;

	return 0;
}

int v4l2_stop_streaming(struct v4l2_driver *drv)
{
	v4l2_free_bufs(drv);

	return 0;
}

/****************************************************************************
	Close V4L2, disallocating all structs
 ****************************************************************************/
int v4l2_close(struct v4l2_driver *drv)
{
	v4l2_free_bufs(drv);

	//free_list(&drv->stds);
	//free_list(&drv->inputs);
	//free_list(&drv->fmt_caps);

	return (close(drv->fd));
}

/****************************************************************************
	Get/Set frequency
 ****************************************************************************/

int v4l2_getset_freq(struct v4l2_driver *drv, enum v4l2_direction dir,
					 double *freq)
{
	struct v4l2_tuner tun;
	struct v4l2_frequency frq;
	double d = 62500;

	memset(&tun, 0, sizeof(tun));

	if (-1 == xioctl(drv->fd, VIDIOC_G_TUNER, &tun))
	{
		perror("g_tuner");
		printf("Assuming 62.5 kHz step\n");
	}
	else
	{
		if (tun.capability & V4L2_TUNER_CAP_LOW)
			d = 62.5;
	}

	if (drv->debug)
	{
		if (tun.capability & V4L2_TUNER_CAP_LOW)
			printf("62.5 Hz step\n");
		else
			printf("62.5 kHz step\n");
	}

	memset(&frq, 0, sizeof(frq));

	frq.type = V4L2_TUNER_ANALOG_TV;

	if (dir & V4L2_GET)
	{
		if (-1 == xioctl(drv->fd, VIDIOC_G_FREQUENCY, &frq))
		{
			perror("s_frequency");
			return errno;
		}
		*freq = frq.frequency * d;
		if (drv->debug)
			printf("board is at freq %4.3f MHz (%d)\n",
				   *freq / 1000000, frq.frequency);
	}
	else
	{
		frq.frequency = (ospl_uint32 )(((*freq) + d / 2) / d);

		if (-1 == xioctl(drv->fd, VIDIOC_S_FREQUENCY, &frq))
		{
			perror("s_frequency");
			return errno;
		}
		if (drv->debug)
			printf("board set to freq %4.3f MHz (%d)\n",
				   *freq / 1000000, frq.frequency);
	}
	return 0;
}

int v4l2_get_balance(struct v4l2_driver *drv)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置手动白平衡：******************************】\n");
	ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
	ctrl.value = V4L2_WHITE_BALANCE_MANUAL;
	if (-1 == xioctl(drv->fd, VIDIOC_G_CTRL, &ctrl))
	{
		perror("balance");
		return errno;
	}
	return ret;
}

int v4l2_set_balance_temperature(struct v4l2_driver *drv, int temp)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置白平衡色温******************************】\n");
	ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;
	ctrl.value = temp;
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("balance_temperature");
		return errno;
	}
	return ret;
}

int v4l2_set_brightness(struct v4l2_driver *drv, int temp)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置亮度******************************】\n");
	ctrl.id = V4L2_CID_BRIGHTNESS;
	ctrl.value = temp;
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("brightness");
		return errno;
	}
	return ret;
}

int v4l2_set_contrast(struct v4l2_driver *drv, int temp)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置对比度******************************】\n");
	ctrl.id = V4L2_CID_CONTRAST;
	ctrl.value = temp;
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("contrast");
		return errno;
	}
	return ret;
}
int v4l2_set_saturation(struct v4l2_driver *drv, int temp)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置饱和度******************************】\n");
	ctrl.id = V4L2_CID_SATURATION;
	ctrl.value = temp;
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("saturation");
		return errno;
	}
	return ret;
}

int v4l2_set_hue(struct v4l2_driver *drv, int temp)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置色度******************************】\n");
	ctrl.id = V4L2_CID_HUE;
	ctrl.value = temp;
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("hue");
		return errno;
	}
	return ret;
}

int v4l2_set_sharpness(struct v4l2_driver *drv, int temp)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置锐度******************************】\n");
	ctrl.id = V4L2_CID_SHARPNESS;
	ctrl.value = temp;
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("sharpness");
		return errno;
	}
	return ret;
}
int v4l2_set_backlight_compensation(struct v4l2_driver *drv, int temp)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置背光补偿******************************】\n");
	ctrl.id = V4L2_CID_BACKLIGHT_COMPENSATION;
	ctrl.value = temp;
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("backlight_compensation");
		return errno;
	}
	return ret;
}
/*
设置曝光值：
1. 首先将曝光模式修改为手动曝光。
2. 设置曝光档次或者具体的曝光值。
例1：得到曝光模式，设置为手动曝光模式
*/

int v4l2_set_exposure(struct v4l2_driver *drv, int temp)
{
	int ret = 0;
	struct v4l2_control ctrl;
	printf("【**********************设置背光补偿******************************】\n");
	ctrl.id = V4L2_CID_EXPOSURE_AUTO;
	ctrl.value = V4L2_EXPOSURE_MANUAL;
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("exposure");
		return errno;
	}
	ctrl.id = V4L2_CID_EXPOSURE;
	ctrl.value = temp; //得到曝光档次，A20接受从 -4到4 共9个档次
	if (-1 == xioctl(drv->fd, VIDIOC_S_CTRL, &ctrl))
	{
		perror("exposure");
		return errno;
	}
	return ret;
}