#ifndef __LIVERTSP_SERVER_H__
#define __LIVERTSP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_LIBMEDIA_MODULE
#include "zpl_media.h"
#endif

#ifdef ZPL_MEDIA_BASE_PATH
#define BASEUSAGEENV_BASE_DIR   ZPL_MEDIA_BASE_PATH
#else
#define BASEUSAGEENV_BASE_DIR  "/nfsroot"
#endif

int livertsp_server_loop(void *p);

int livertsp_server_init(int port, char *base, int (*logcb)(const char *fmt,...));

int livertsp_server_exit(void);


#ifdef __cplusplus
}
#endif


#endif /* __LIVERTSP_SERVER_H__ */
