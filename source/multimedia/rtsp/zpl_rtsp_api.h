#ifndef __RTSP_API_H__
#define __RTSP_API_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rtsp_server_s 
{
    zpl_taskid_t    t_taskid;    
    rtsp_srv_t      *rtsp_srv;
    void            *t_master;
    //rtsp_session_list rtsp_seslst;

}rtsp_server_t;

extern int rtsp_module_init(void);
extern int rtsp_module_exit(void);
extern int rtsp_module_task_init(void);
extern int rtsp_module_task_exit(void);


#ifdef __cplusplus
}
#endif
#endif /* __RTSP_API_H__ */
