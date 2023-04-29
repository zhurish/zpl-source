/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/mscommon.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/mswebcam.h"
#include "../voip/hwcam.h"


#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#define FF_INPUT_BUFFER_PADDING_SIZE 32

#if LIBAVCODEC_VERSION_MAJOR >= 57

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#endif



#include <sys/stat.h>


typedef struct hw_capture_s {
	MSVideoSize vsize;
	char *nowebcamimage;
	uint64_t lasttime;
	float fps;
	mblk_t *pic;
	int mchannel;
	int mlevel;

}hw_capture_t;


void hw_capture_init(MSFilter *f) {
	hw_capture_t *d = (hw_capture_t*)ms_new0(hw_capture_t,1);
	d->vsize.width = MS_VIDEO_SIZE_CIF_W;
	d->vsize.height = MS_VIDEO_SIZE_CIF_H;


	d->lasttime = 0;
	d->pic = NULL;
	d->fps = 1;
	f->data = d;
}

void hw_capture_uninit(MSFilter *f) {
	hw_capture_t *d = (hw_capture_t*)f->data;
	if (d->nowebcamimage) ms_free(d->nowebcamimage);
	ms_free(d);
}

#if 0
static void *msv4l2_thread(void *ptr){
	V4l2State *s=(V4l2State*)ptr;
	uint64_t start;

	ms_message("msv4l2_thread starting");
	if (s->fd==-1){
		if( msv4l2_open(s)!=0){
			ms_warning("msv4l2 could not be openned");
			goto close;
		}
	}

	if (!s->configured && msv4l2_configure(s)!=0){
		ms_warning("msv4l2 could not be configured");
		goto close;
	}

	if (msv4l2_do_mmap(s)!=0)
	{
		ms_warning("msv4l2 do mmap");
		goto close;
	}

	ms_message("V4L2 video capture started.");
	while(s->thread_run)
	{
		if (s->fd!=-1){
			mblk_t *m;
			m=v4lv2_grab_image(s,50);
			if (m){
				mblk_t *om=dupmsg(m);
				mblk_set_marker_info(om,(s->pix_fmt==MS_MJPEG));
				ms_mutex_lock(&s->mutex);
				putq(&s->rq,om);
				ms_mutex_unlock(&s->mutex);
			}
		}
	}
	/*dequeue pending buffers so that we can properly unref them (avoids memleak ), and even worse crashes (vmware)*/
	start=ortp_get_cur_time_ms();
	while(s->queued){
		v4l2_dequeue_ready_buffer(s,50);
		if (ortp_get_cur_time_ms()-start > 5000){
			ms_warning("msv4l2: still [%i] buffers not dequeued at exit !", s->queued);
			break;
		}
	}
	msv4l2_do_munmap(s);
close:
	msv4l2_close(s);
	ms_message("msv4l2_thread exited.");
	ms_thread_exit(NULL);
	return NULL;
}
#endif

/* start */
void hw_capture_preprocess(MSFilter *f) {
	hw_capture_t *d = (hw_capture_t*)f->data;
	if (d->pic == NULL) {
		;//d->pic = ms_load_jpeg_as_yuv(d->nowebcamimage, &d->vsize);
	}
}

void hw_capture_process(MSFilter *f) {
	hw_capture_t *s = (hw_capture_t*)f->data;
	#if 1
	uint64_t frame_interval = (uint64_t)(1000/s->fps);
	/*output a frame whenever needed, i.e. respect the FPS parameter */
	if ((f->ticker->time - s->lasttime>frame_interval) || s->lasttime == 0) {
		ms_mutex_lock(&f->lock);
		if (s->pic) {
			mblk_t *o = dupmsg(s->pic);
			/*prevent mirroring at the output*/
			mblk_set_precious_flag(o,1);
			ms_queue_put(f->outputs[0],o);
		}
		ms_filter_unlock(f);
		s->lasttime = f->ticker->time;
	}
	#endif
#if 0
	uint32_t timestamp;
	int cur_frame;
	uint32_t curtime=f->ticker->time;
	float elapsed;

	if (s->th_frame_count==-1){
		s->start_time=curtime;
		s->th_frame_count=0;
	}
	elapsed=((float)(curtime-s->start_time))/1000.0;
	cur_frame=elapsed*s->fps;

	if (cur_frame>=s->th_frame_count){
		mblk_t *om=NULL;
		ms_mutex_lock(&s->mutex);
		/*keep the most recent frame if several frames have been captured */
		if (s->fd!=-1){
			mblk_t *tmp=NULL;
			while((tmp=getq(&s->rq))!=NULL){
				if (om!=NULL) freemsg(om);
				om=tmp;
			}
		}
		ms_mutex_unlock(&s->mutex);
		if (om!=NULL){
			timestamp=f->ticker->time*90;/* rtp uses a 90000 Hz clockrate for video*/
			mblk_set_timestamp_info(om,timestamp);
			mblk_set_marker_info(om,TRUE);
			ms_queue_put(f->outputs[0],om);
			//ms_average_fps_update(&s->avgfps,f->ticker->time);
		}
		s->th_frame_count++;
	}	
	#endif
}
/* stop */
void hw_capture_postprocess(MSFilter *f) {
	hw_capture_t *d = (hw_capture_t*)f->data;
	if (d->pic) {
		freemsg(d->pic);
		d->pic = NULL;
	}
}

static int hw_capture_set_fps(MSFilter *f, void *arg) {
	hw_capture_t *d = (hw_capture_t*)f->data;
	d->fps = *((float*)arg);
	d->lasttime = 0;
	return 0;
}

static int hw_capture_get_fps(MSFilter *f, void *arg) {
	hw_capture_t *d = (hw_capture_t*)f->data;
	*((float*)arg) = d->fps;
	return 0;
}

int hw_capture_set_vsize(MSFilter *f, void* data) {
#ifndef NO_FFMPEG
	hw_capture_t *d = (hw_capture_t*)f->data;
	d->vsize = *(MSVideoSize*)data;
#else
    // no rescaling without ffmpeg
#endif
	return 0;
}

int hw_capture_get_vsize(MSFilter *f, void* data) {
	hw_capture_t *d = (hw_capture_t*)f->data;
	hw_capture_preprocess(f);
	*(MSVideoSize*)data = d->vsize;
	return 0;
}

int hw_capture_get_pix_fmt(MSFilter *f, void *data) {
	*(MSPixFmt*)data = MS_YUV420P;
	return 0;
}


static int hw_capture_set_nchannels(MSFilter *f, void *arg) {
	int nchannels = 0;
	hw_capture_t *s = (hw_capture_t *)f->data;
	nchannels = *(int *)arg;
	s->mchannel = ((nchannels>>8)&0xff);
	s->mlevel = ((nchannels)&0xff);
	return 0;
}

static int hw_capture_get_nchannels(MSFilter *f, void *arg) {
	int nchannels = 0;
	hw_capture_t *s = (hw_capture_t *)f->data;
	nchannels = (s->mchannel<<8)|s->mlevel;
	*((int *)arg) = nchannels;
	return 0;
}

MSFilterMethod hw_capture_methods[] = {
	{	MS_FILTER_SET_FPS,	hw_capture_set_fps	},
	{	MS_FILTER_GET_FPS,	hw_capture_get_fps	},
	{	MS_FILTER_SET_VIDEO_SIZE, hw_capture_set_vsize },
	{	MS_FILTER_GET_VIDEO_SIZE, hw_capture_get_vsize },
	{	MS_FILTER_GET_PIX_FMT, hw_capture_get_pix_fmt },
	{   MS_FILTER_SET_NCHANNELS, hw_capture_set_nchannels },
	{   MS_FILTER_GET_NCHANNELS, hw_capture_get_nchannels },
	{	0, NULL }
};

MSFilterDesc ms_hw_capture_desc = {
	.id=MS_HWVIDEO_CAPTURE_ID,/**< the id declared in allfilters.h */
	.name="MSHWCapture",/**< the filter name*/
	.text="A filter that HW Capture.",/**< short text describing the filter's function*/
	.category=MS_FILTER_OTHER,/**< filter's category*/
	.enc_fmt=NULL,/**< sub-mime of the format, must be set if category is MS_FILTER_ENCODER or MS_FILTER_DECODER */
	.ninputs=0,/**< number of inputs */
	.noutputs=1,/**< number of outputs */
	.init=hw_capture_init,/**< Filter's init function*/
	.preprocess=hw_capture_preprocess, /**< Filter's preprocess function, called one time before starting to process*/
	.process=hw_capture_process,/**< Filter's process function, called every tick by the MSTicker to do the filter's job*/
	.postprocess=hw_capture_postprocess,/**< Filter's postprocess function, called once after processing (the filter is no longer called in process() after)*/
	.uninit=hw_capture_uninit,/**< Filter's uninit function, used to deallocate internal structures*/
	.methods=hw_capture_methods,/**<Filter's method table*/
	.flags=0	/**<Filter's special flags, from the MSFilterFlags enum.*/
};

MS_FILTER_DESC_EXPORT(ms_hw_capture_desc)

static void hw_capture_detect(MSWebCamManager *obj){
	MSWebCam *cam=ms_web_cam_new(&hw_capture_desc);
	ms_web_cam_manager_add_cam(obj,cam);
}

static void hw_capture_cam_init(MSWebCam *cam) {
	cam->name=ms_strdup("HW Video Capture");
}



static MSFilter *hw_capture_create_reader(MSWebCam *obj){
		return ms_factory_create_filter_from_desc(ms_web_cam_get_factory(obj), &ms_hw_capture_desc);
}

static bool_t hw_capture_encode2mimetype(struct _MSWebCam *obj, const char *mime_type)
{
	return 1;
}

MSWebCamDesc hw_capture_desc={
	"HWCapture",
	&hw_capture_detect,
	&hw_capture_cam_init,
	&hw_capture_create_reader,
	NULL,
	hw_capture_encode2mimetype
};


#if __clang__
#pragma clang diagnostic pop
#endif
