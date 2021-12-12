#ifndef __PTZ_CMD_H__
#define __PTZ_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif




typedef enum _PTZ_CMD_E
{
	PTZ_CMD_STOP		=	0,	//停止
	PTZ_CMD_UP			=	1,	//上
	PTZ_CMD_DOWN		=	2,	//下
	PTZ_CMD_LEFT		=	3,	//左
	PTZ_CMD_RIGHT		=	4,	//右
	PTZ_CMD_LEFT_UP		=	5,	//左上
	PTZ_CMD_LEFT_DOWN	=	6,	//左下
	PTZ_CMD_RIGHT_UP	=	7,	//右上
	PTZ_CMD_RIGHT_DOWN	=	8,	//右下
	PTZ_CMD_AUTOSCAN	=	9,	//自动扫描
	PTZ_CMD_MANUALSCAN	=	10,	//手动扫描
	PTZ_CMD_FOCUS_NEAR	=	11,  //聚焦+
	PTZ_CMD_FOCUS_FAR	=	12,  //聚焦-
	PTZ_CMD_IRIS_CLOSE	=	13,  //光圈+
	PTZ_CMD_IRIS_OPEN	=	14,	//光圈-
	PTZ_CMD_ZOOM_WIDE	=	15,	//变倍+
	PTZ_CMD_ZOOM_TELE	=	16,  //变倍-
	PTZ_CMD_SENSE		=	17,	//

	PTZ_CMD_SET_PRESET	=	18,  //预置位设置
	PTZ_CMD_CLR_PRESET	=	19,	//预置位清楚
	PTZ_CMD_GOTO_PRESET	=	20,  //预置位调用
	PTZ_CMD_FLIP		=	21,  //翻转
	PTZ_CMD_GOTO_ZEROPAN =	22, //零位调用
	PTZ_CMD_SET_AUX		=	23,  //设置辅助开关
	PTZ_CMD_CLR_AUX		=	24,  //清除辅助开关
	PTZ_CMD_REMOTE_RESET =	25, //远程恢复
	PTZ_CMD_ZONE_START	=	26,  //设置花样扫描开始
	PTZ_CMD_ZONE_END	=	27,  //设置花样扫描结束
	PTZ_CMD_WR_CHAR		=	28,  //写字符
	PTZ_CMD_CLR_SCR		=	29,  //清楚字符
	PTZ_CMD_ALM_ACK		=	30,  //报警确认
	PTZ_CMD_ZONE_SCAN_ON =	31, //开启花样扫描
	PTZ_CMD_ZONE_SCAN_OFF =	32, //停止谎言扫描
	PTZ_CMD_PTN_START	=	33,  //
	PTZ_CMD_PTN_STOP		=	34,  //
	PTZ_CMD_RUN_PTN		=	35,  //
	PTZ_CMD_ZOOM_SPEED	=	36, //变倍速度
	PTZ_CMD_FOCUS_SPEED	=	37, //聚焦速度
	PTZ_CMD_RESET_CAMERA =	38, //相机复位
	PTZ_CMD_AUTO_FOCUS	=	39, //自动聚焦
	PTZ_CMD_AUTO_IRIS		=	40,//自动光圈
	PTZ_CMD_AUTO_AGC		=	41, //自动增益
	PTZ_CMD_BACKLIGHT_COMP = 42, //
	PTZ_CMD_AUTO_WB		    =	43,//自动白平衡
	PTZ_CMD_DEV_PHASE_DELAY = 44, //
	PTZ_CMD_SET_SHTTER_SPEED = 45, //设置快门速度
	PTZ_CMD_ADJ_LL_PHASE_DELAY = 46,
	PTZ_CMD_ADJ_WB_RB		= 47, //调整自动白平衡红蓝
	PTZ_CMD_ADJ_WB_MG		= 48, //调整自动白平衡
	PTZ_CMD_ADJ_GAIN			= 49, //调整自动增益
	PTZ_CMD_AUTO_IRIS_LV		= 50, //调整自动光圈level
	PTZ_CMD_AUTO_IRIS_PEAK	= 51, //调整自动光圈peak
	PTZ_CMD_QUERY			= 52, //查询

	PTZ_CMD_INVALID			= 53
} PTZ_CMD_E;



#ifdef __cplusplus
}
#endif


#endif /* __PTZ_CMD_H__ */