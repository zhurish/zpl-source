#ifndef __RTSP_API_H__
#define __RTSP_API_H__

#include "zpl_rtsp_def.h"
#include "zpl_rtsp_session.h"
#ifdef ORTP_VERSION
#include "ortp/ortp.h"
#endif



RTSP_BEGIN_DECLS


RTSP_API int rtsp_module_init();
RTSP_API int rtsp_module_exit();
RTSP_API int rtsp_module_task_init();
RTSP_API int rtsp_module_task_exit();

RTSP_END_DECLS

#endif /* __RTSP_API_H__ */
