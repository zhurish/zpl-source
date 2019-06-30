README.txt
 Created on: 2019年1月12日
     Author: zhurish

voip_api				提供给CLI，MGT-A单元使用的配置类API

voip_app				拨号业务流程api，主要调用sip-ctl，stream，event单元
voip_dbtest				测试数据
voip_error				错误
voip_event				voip业务处理event事件线程
voip_medistream			mediastream库的api
voip_ring				铃声
voip_sip				sip模块参数管理和接口
voip_socket				部分和mediastream通信的接口
voip_state				voip相关状态
voip_statistic			voip相关统计
voip_stream				voip语音媒体流操作API（对mediastream封装）
voip_task				voip语音媒体流处理线程
voip_util				voip模块工具
voip_volume				voip声卡音量管理




OSIP长时间注册不上出现 CPU占用率高的bug

在OSIP的log出现 cb_nict_kill_transaction


RTP Payload:

support:
	rtp_profile_set_payload(profile,0,&payload_type_pcmu8000);		OK
	rtp_profile_set_payload(profile,1,&payload_type_lpc1016);
	rtp_profile_set_payload(profile,3,&payload_type_gsm);
	rtp_profile_set_payload(profile,7,&payload_type_lpc);			OK
	rtp_profile_set_payload(profile,4,&payload_type_g7231);
	rtp_profile_set_payload(profile,8,&payload_type_pcma8000);		OK
	rtp_profile_set_payload(profile,9,&payload_type_g722);			OK
	rtp_profile_set_payload(profile,10,&payload_type_l16_stereo);	OK
	rtp_profile_set_payload(profile,11,&payload_type_l16_mono);		OK
	rtp_profile_set_payload(profile,13,&payload_type_cn);
	rtp_profile_set_payload(profile,18,&payload_type_g729);			OK 726
	rtp_profile_set_payload(profile,31,&payload_type_h261);
	rtp_profile_set_payload(profile,32,&payload_type_mpv);
	rtp_profile_set_payload(profile,34,&payload_type_h263);
	payload_type_opus												OK
payload_type_g726_40
payload_type_g726_32
payload_type_g726_24
payload_type_g726_16

init: 
	rtp_profile_set_payload(&av_profile,110,&payload_type_speex_nb);
	rtp_profile_set_payload(&av_profile,111,&payload_type_speex_wb);
	rtp_profile_set_payload(&av_profile,112,&payload_type_ilbc);
	rtp_profile_set_payload(&av_profile,113,&payload_type_amr);
	rtp_profile_set_payload(&av_profile,114,args->custom_pt);
	rtp_profile_set_payload(&av_profile,115,&payload_type_lpc1015);
#ifdef VIDEO_ENABLED
	cam=ms_web_cam_new(ms_mire_webcam_desc_get());
	if (cam) ms_web_cam_manager_add_cam(ms_factory_get_web_cam_manager(factory), cam);
	cam=NULL;

	rtp_profile_set_payload(&av_profile,26,&payload_type_jpeg);
	rtp_profile_set_payload(&av_profile,98,&payload_type_h263_1998);
	rtp_profile_set_payload(&av_profile,97,&payload_type_theora);
	rtp_profile_set_payload(&av_profile,99,&payload_type_mp4v);
	rtp_profile_set_payload(&av_profile,100,&payload_type_x_snow);
	rtp_profile_set_payload(&av_profile,102,&payload_type_h264);
	rtp_profile_set_payload(&av_profile,103,&payload_type_vp8);

	args->video=NULL;
#endif