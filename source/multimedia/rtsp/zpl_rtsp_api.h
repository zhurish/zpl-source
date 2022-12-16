#ifndef __RTSP_API_H__
#define __RTSP_API_H__

#include "zpl_rtsp_def.h"
#include "zpl_rtsp_session.h"
#ifdef ORTP_VERSION
#include "ortp/ortp.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif
//RTSP_BEGIN_DECLS


RTSP_API int rtsp_module_init(void);
RTSP_API int rtsp_module_exit(void);
RTSP_API int rtsp_module_task_init(void);
RTSP_API int rtsp_module_task_exit(void);

//RTSP_END_DECLS
#ifdef __cplusplus
}
#endif
#endif /* __RTSP_API_H__ */
