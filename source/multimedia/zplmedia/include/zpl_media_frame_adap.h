#ifndef __ZPL_MEDIA_LOADFRAME_H__
#define __ZPL_MEDIA_LOADFRAME_H__

#ifdef __cplusplus
extern "C" {
#endif


struct zpl_media_frame_adap 
{
	char *name;
	int id;
    media_get_frame_hander get_frame;            // 读取一帧数据回调函数
    media_put_frame_hander put_frame;
};

media_get_frame_hander * zpl_media_adap_get_frame_get(uint32_t id);
media_put_frame_hander * zpl_media_adap_put_frame_get(uint32_t id);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_LOADFRAME_H__ */