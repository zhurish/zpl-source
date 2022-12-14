#ifndef _SDK_CMD_DEF_H_
#define _SDK_CMD_DEF_H_

/************************************************************************************
	SDK命令号定义
	参考 《EAIS设备网络协议文档规范.docx》
************************************************************************************/

// 协议CmdID
enum SDK_CMD_DEF
{
	// 通用应答
	SDK_COM_RES					= 1000,			// 通用应答

	// 【基础类】 : 1001~1999
	// 登录
	SDK_LOGIN_REQ				= 1001,			// 登录请求
	SDK_LOGIN_RES				= 1002,			// 登录应答
	// 登出
	SDK_LOGOUT_REQ				= 1003,        	// 注销请求
	SDK_LOGOUT_RES				= 1004,			// 注销响应	
	// 心跳
	SDK_HEART_REQ				= 1005,			// 心跳请求
	SDK_HEART_RES				= 1006,			// 心跳响应
	// 设备搜索
	SDK_SEARCH_DEVICE_REQ		= 1007,			// 设备搜索请求
	SDK_SEARCH_DEVICE_RES		= 1008,			// 设备搜索响应
	// 设备升级
	SDK_UPDATE_REQ				= 1009,			// 设备升级请求
	SDK_UPDATE_RES				= 1010,			// 设备升级响应
	SDK_UPDATE_DATA				= 1010,			// 设备升级数据
	SDK_UPDATE_NOTIFY			= 1011,			// 设备升级状态上报，无需响应
	// 出厂测试
	SDK_FACTORYTEST_REQ			= 1012,			// 出厂测试项请求
	SDK_FACTORYTEST_RES			= 1013,			// 出厂测试项响应


	// 【设备操作类】 : 2001~2999	
	// 获取设备信息
	SDK_GET_DEVICEINFO_REQ		= 2001,			// 获取设备信息请求
	SDK_GET_DEVICEINFO_RES		= 2002,			// 获取设备信息响应
	// RTSP配置
	SDK_GET_RTSPCONFIG_REQ		= 2003,			// 获取RTSP配置请求
	SDK_GET_RTSPCONFIG_RES		= 2004,			// 获取RTSP配置响应
	SDK_SET_RTSPCONFIG_REQ		= 2005,			// 设置RTSP配置请求
	SDK_SET_RTSPCONFIG_RES		= 2006,			// 设置RTSP配置响应

	SDK_GET_IPCONFIG_REQ		= 2007,			// 获取IP配置请求
	SDK_GET_IPCONFIG_RES		= 2008,			// 获取IP配置响应
	SDK_SET_IPCONFIG_REQ		= 2009,			// 设置IP配置请求
	SDK_SET_IPCONFIG_RES		= 2010,			// 设置IP配置响应

	SDK_GET_DEVICETIME_REQ		= 2011,			// 获取设备时间请求
	SDK_GET_DEVICETIME_RES		= 2012,			// 获取设备时间响应
	SDK_SET_DEVICETIME_REQ		= 2013,			// 设置设备时间请求
	SDK_SET_DEVICETIME_RES		= 2014,			// 设置设备时间响应

	SDK_DEVICE_REBOOT_REQ		= 2015,			// 设备重启请求
	SDK_DEVICE_REBOOT_RES		= 2016,			// 设备重启响应

	SDK_GET_SNAPSERVERADDR_REQ  = 2017,			// 获取抓拍服务器地址请求
	SDK_GET_SNAPSERVERADDR_RES	= 2018,			// 获取抓拍服务器地址响应
	SDK_SET_SNAPSERVERADDR_REQ	= 2019,			// 设置抓拍服务器地址请求
	SDK_SET_SNAPSERVERADDR_RES	= 2020,			// 设置抓拍服务器地址响应

	// 设备名称
	SDK_GET_DEVICENAME_REQ		= 2021,			// 获取设备名称请求
	SDK_GET_DEVICENAME_RES		= 2022,			// 获取设备名称响应
	SDK_SET_DEVICENAME_REQ		= 2023,			// 设置设备名称请求
	SDK_SET_DEVICENAME_RES		= 2024,			// 设置设备名称响应

	// 设置用户密码
	SDK_SET_PASSWORD_REQ		= 2025,			// 设置用户密码请求
	SDK_SET_PASSWORD_RES		= 2026,			// 设置用户密码响应

	// 设备SN
	SDK_GET_DEVICESN_REQ		= 2027,			// 获取设备SN请求
	SDK_GET_DEVICESN_RES		= 2028,			// 获取设备SN响应
	SDK_SET_DEVICESN_REQ		= 2029,			// 设置设备SN请求
	SDK_SET_DEVICESN_RES		= 2030,			// 设置设备SN响应
	
	// 厂商名称
	SDK_GET_MANUFACTURER_REQ	= 2031,			// 获取设备厂商名称请求
	SDK_GET_MANUFACTURER_RES	= 2032,			// 获取设备厂商名称响应
	SDK_SET_MANUFACTURER_REQ	= 2033,			// 设置设备厂商名称请求
	SDK_SET_MANUFACTURER_RES	= 2034,			// 设置设备厂商名称响应

	// 日志相关
	SDK_OPEN_DEVICE_LOG_REQ		= 2035,			// 打开设备日志请求
	SDK_OPEN_DEVICE_LOG_RES		= 2036,			// 打开设备日志响应
	SDK_CLOSE_DEVICE_LOG_REQ	= 2037,			// 关闭设备日志请求
	SDK_CLOSE_DEVICE_LOG_RES	= 2038,			// 关闭设备日志响应
	SDK_GET_DEVICE_LOG_CFG_REQ	= 2039,			// 获取设备日志配置请求
	SDK_GET_DEVICE_LOG_CFG_RES	= 2040,			// 获取设备日志配置响应
	SDK_SET_DEVICE_LOG_CFG_REQ	= 2041,			// 设置设备日志配置请求
	SDK_SET_DEVICE_LOG_CFG_RES	= 2042,			// 设置设备日志配置响应
	SDK_LOAD_LOG_FILE_REQ		= 2043,			// 下载设备日志文件请求
	SDK_LOAD_LOG_FILE_RES		= 2044,			// 下载设备日志文件响应
	SDK_LOAD_LOG_FILE_DATA		= 2045,			// 下载设备日志文件数据
	SDK_DEVICELOG_NOTIFY		= 2050,			// 上传EAIS日志（回调）

	// 检测帧率
	SDK_GET_FRAME_RATE_REQ		= 2051,			// 获取当前检测帧率请求
	SDK_GET_FRAME_RATE_RES		= 2052,			// 获取当前检测帧率响应
	SDK_SET_FRAME_RATE_REQ		= 2053,			// 设置当前检测帧率请求
	SDK_SET_FRAME_RATE_RES		= 2054,			// 设置当前检测帧率响应

	// 恢复出厂设置
	SDK_FACTORY_RESET_REQ		= 2055,			// 恢复出厂设置请求
	SDK_FACTORY_RESET_RES		= 2056,			// 恢复出厂设置响应

	// 设备Mac
	SDK_GET_MAC_REQ				= 2057,			// 获取设备Mac请求
	SDK_GET_MAC_RES				= 2058,			// 获取设备Mac响应
	SDK_SET_MAC_REQ				= 2059,			// 设置设备Mac请求
	SDK_SET_MAC_RES				= 2060,			// 设置设备Mac响应

	// NTP
	SDK_GET_NTP_REQ				= 2061,			// 获取设备NTP请求
	SDK_GET_NTP_RES				= 2062,			// 获取设备NTP响应
	SDK_SET_NTP_REQ				= 2063,			// 设置设备NTP请求
	SDK_SET_NTP_RES				= 2064,			// 设置设备NTP响应

	SDK_GET_VIDEO_NUM_REQ		= 2065,			// 获取视频路数配置请求
	SDK_GET_VIDEO_NUM_RES		= 2066,			// 获取视频路数配置响应
	SDK_SET_VIDEO_NUM_REQ		= 2067,			// 设置视频路数配置请求
	SDK_SET_VIDEO_NUM_RES		= 2068,			// 设置视频路数配置响应
	


	// 【状态/报警主动上报类】 : 3001~3999		
	// 状态
	SDK_DEVICESTATUS_NOTIFY		= 3001,			// 设备状态变更上报

	// 【实时/回放视频类】 : 4001~4999		
	SDK_GET_DISPLAYMODE_REQ		= 4001,			// 获取显示模式请求
	SDK_GET_DISPLAYMODE_RES		= 4002,			// 获取显示模式响应
	SDK_SET_DISPLAYMODE_REQ		= 4003,			// 设置显示模式请求
	SDK_SET_DISPLAYMODE_RES		= 4004,			// 设置显示模式响应

	// 【抓拍类】 : 5001~5999	
	// 抓拍配置
	SDK_GET_SNAPCONFIG_REQ		= 5001,			// 获取抓拍配置请求
	SDK_GET_SNAPCONFIG_RES		= 5002,			// 获取抓拍配置响应
	SDK_SET_SNAPCONFIG_REQ		= 5003,			// 设置抓拍配置请求
	SDK_SET_SNAPCONFIG_RES		= 5004,			// 设置抓拍配置响应
	// 打开/关闭抓拍流
	SDK_OPENSNAP_REQ			= 5005,			// 打开抓拍流请求
	SDK_OPENSNAP_RES			= 5006,			// 打开抓拍流响应
	SDK_CLOSESNAP_REQ			= 5007,			// 关闭抓拍流请求
	SDK_CLOSESNAP_RES			= 5008,			// 关闭抓拍流响应
	// 抓拍结构化数据上报
	SDK_STRUCTDATA_NOTIFY		= 5009,			// 抓拍结构化数据上报
	// 获取/设置抓拍编码参数
	SDK_GET_FACEENCODE_REQ		= 5010,			// 获取抓拍编码参数请求
	SDK_GET_FACEENCODE_RES		= 5011,			// 获取抓拍编码参数响应
	SDK_SET_FACEENCODE_REQ		= 5012,			// 设置抓拍编码参数请求
	SDK_SET_FACEENCODE_RES		= 5013,			// 设置抓拍编码参数响应

	// 获取人脸特征值
	SDK_GET_FACEFEA_REQ			= 5014,			// 获取图片特征请求
	SDK_GET_FACEFEA_RES			= 5015,			// 获取图片特征请求响应
	SDK_FACE_DATA				= 5016,			// 图片数据
	SDK_FACEFEA_NOTIFY			= 5017,			// 人脸特征上报

	// 安全帽配置
	SDK_GET_HELMETCONFIG_REQ    = 5018,			// 获取安全帽配置请求
	SDK_GET_HELMETCONFIG_RES    = 5019,			// 获取安全帽配置响应
	SDK_SET_HELMETCONFIG_REQ    = 5020,			// 设置安全帽配置请求
	SDK_SET_HELMETCONFIG_RES    = 5021,			// 设置安全帽配置响应

	// HDMI
	SDK_GET_HDMISTATUS_REQ		= 5022,			// 获取HDMI状态请求
	SDK_GET_HDMISTATUS_RES		= 5023,			// 获取HDMI状态响应
	SDK_SET_HDMISTATUS_REQ		= 5024,			// 设置HDMI状态请求
	SDK_SET_HDMISTATUS_RES		= 5025,			// 设置HDMI状态响应
	
	// 是否使能模型输出质量状态
	SDK_GET_MODELSTATUS_REQ		= 5026,			// 获取模型状态请求
	SDK_GET_MODELSTATUS_RES		= 5027,			// 获取模型状态响应
	SDK_SET_MODELSTATUS_REQ		= 5028,			// 设置模型状态请求
	SDK_SET_MODELSTATUS_RES		= 5029,			// 设置模型状态响应

	// 【识别类】 : 6001~6999	
	SDK_GET_GROUPUSER_REQ       = 6001,			// 获取组内人员信息请求
	SDK_GET_GROUPUSER_RES		= 6002,			// 获取组内人员信息响应
	SDK_GROUPUSER_DATA			= 6003,			// 组内人员信息数据

	// 用户增删改查
	SDK_EDIT_USER_REQ           = 6005,			// 增/改人脸请求
	SDK_EDIT_USER_RES           = 6006,			// 增/改人脸响应

	SDK_DELETE_USER_REQ			= 6007,			// 删除人脸请求
	SDK_DELETE_USER_RES			= 6008,			// 删除人脸响应
	
	SDK_DELETE_GROUP_REQ		= 6009,			// 删除人脸库请求
	SDK_DELETE_GROUP_RES		= 6010,			// 删除人脸库响应

	SDK_DELETE_USER_LIST_REQ	= 6011,			// 删除人脸列表请求
	SDK_DELETE_USER_LIST_RES	= 6012,			// 删除人脸列表响应

	SDK_INQURE_USER_REQ			= 6013,			// 查询人脸请求
	SDK_INQURE_USER_RES			= 6014,			// 查询人脸响应

	SDK_GET_RECOGNIZE_REQ		= 6015,			// 获取识别配置请求
	SDK_GET_RECOGNIZE_RES       = 6016,			// 获取识别配置响应
	SDK_SET_RECOGNIZE_REQ		= 6017,			// 设置识别配置请求
	SDK_SET_RECOGNIZE_RES		= 6018,			// 设置识别配置响应

	// 查询人数统计
	SDK_QUERY_PEOPLE_REQ		= 6019,			// 查询人数请求
	SDK_QUERY_PEOPLE_RES		= 6020,			// 查询人数响应

	// 组信息
	SDK_QUERY_GROUP_NAME_REQ	= 6021,			// 查询组名称请求
	SDK_QUERY_GROUP_NAME_RES	= 6022,			// 查询组名称响应

	SDK_SET_GROUP_NAME_REQ		= 6023,			// 设置组名称请求
	SDK_SET_GROUP_NAME_RES		= 6024,			// 设置组名称响应


	// 识别相似度优化阈值
	SDK_GET_OPTIMIZE_REQ		= 6031,			// 获取识别相似度优化阈值请求
	SDK_GET_OPTIMIZE_RES		= 6032,			// 获取识别相似度优化阈值响应
	SDK_SET_OPTIMIZE_REQ		= 6033,			// 设置识别相似度优化阈值请求
	SDK_SET_OPTIMIZE_RES		= 6034,			// 设置识别相似度优化阈值响应

	// 报警配置
	SDK_GET_ALARM_REQ			= 6035,			// 获取报警配置请求
	SDK_GET_ALARM_RES			= 6036,			// 获取报警配置响应
	SDK_SET_ALARM_REQ			= 6037,			// 设置报警配置请求
	SDK_SET_ALARM_RES			= 6038,			// 设置报警配置响应
	
	// 设置ROI
	SDK_GET_ROI_REQ				= 6039,			// 获取ROI请求
	SDK_GET_ROI_RES				= 6040,			// 获取ROI响应
	SDK_SET_ROI_REQ				= 6041,			// 设置ROI请求
	SDK_SET_ROI_RES				= 6042,			// 设置ROI响应


	// 罕森
	SDK_GET_ALGORITHM_REQ		= 6043,			// 获取检测结果请求
	SDK_GET_ALGORITHM_RES       = 6044,			// 获取检测结果响应

    SDK_GET_INFRAREDDETECT_REQ  = 6045,			// 获取红外检测结果请求
	SDK_GET_INFRAREDDETECT_RES  = 6046,			// 获取红外检测结果响应

	// 双路门禁
	SDK_GET_ALLATTENDANCE_REQ	= 6047,			// 获取全员考勤请求
	SDK_GET_ALLATTENDANCE_RES	= 6048,			// 获取全员考勤响应

	SDK_GET_SINGLEATTEN_REQ     = 6049,			// 获取个人考勤请求
	SDK_GET_SINGLEATTEN_RES     = 6050,			// 获取个人考勤响应

	SDK_GET_CLOUDCONFIG_REQ		= 6051,			// 获取云配置信息请求
	SDK_GET_CLOUDCONFIG_RES		= 6052,			// 获取云配置信息响应
	SDK_SET_CLOUDCONFIG_REQ     = 6053,			// 设置云配置信息请求
	SDK_SET_CLOUDCONFIG_RES		= 6054,			// 设置云配置信息响应

	SDK_INIT_DB_REQ				= 6055,			// 初始化数据库请求
	SDK_INIT_DB_RES				= 6056,			// 初始化数据库响应
};

#endif // _SDK_CMD_DEF_H_