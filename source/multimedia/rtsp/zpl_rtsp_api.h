#ifndef __RTSP_API_H__
#define __RTSP_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_LIVE555_MODULE
typedef struct rtsp_srv_s {
    void            *t_master;
    void            *t_accept;
    void            *t_read;
    int             t_sock;
    zpl_taskid_t    t_taskid;
}rtsp_srv_t;
#define RTSP_API extern
#endif
    int ortp_create_init(void);
    int ortp_create_exit(void);
    RTSP_API int rtsp_module_init(void);
    RTSP_API int rtsp_module_exit(void);
    RTSP_API int rtsp_module_task_init(void);
    RTSP_API int rtsp_module_task_exit(void);


#ifdef __cplusplus
}
#endif
#endif /* __RTSP_API_H__ */
