#ifndef __LIVERTSP_SERVER_H__
#define __LIVERTSP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BASEUSAGEENV_BASE_DIR  "/home/zhurish/workspace/working/zpl-source/source"

int livertsp_server_loop(void *p);

int livertsp_server_init(int port, char *base, int (*logcb)(const char *fmt,...));

int livertsp_server_exit(void);


#ifdef __cplusplus
}
#endif


#endif /* __LIVERTSP_SERVER_H__ */
