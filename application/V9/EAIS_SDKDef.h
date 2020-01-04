/*****************************************************************************

		版权所有 (C),2018-2025  开放智能机器（上海）有限公司
							OPEN AI LAB

*****************************************************************************
 文 件 名   : EAIS_SDKDef.h
 作    者   : 陈润城
 生成日期   : 2019年7月6日
 功能描述   : EAIS SDK对外数据类型，定义了对外所需的错误号、枚举、结构体等
******************************************************************************/

#ifndef __EAIS_SDKDEF_H__
#define __EAIS_SDKDEF_H__

#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
	#define LIB_SDK_DLL_EXPORT extern "C" __declspec(dllexport)
#else
	#define LIB_SDK_DLL_EXPORT //extern "C"
	#define __stdcall 
#endif

#define	EAIS_SDK_MAX_COMMON_LEN					64									// 最大长度限制，通用于IPAddr、设备名称等
#define	EAIS_SDK_MAX_RTSP_URL_LEN				128									// 最大URL长度限制
#define	EAIS_SDK_MAX_PATH_LEN					256									// 最大路径长度限制
#define	EAIS_SDK_MAX_CHANNEL_SUPPORT			32									// 最大支持通道数
#define EAIS_SDK_MAX_DEVICE_SUPPORT				1000								// 最大支持设备数
#define EAIS_SDK_MAX_UPDATEPACKET_SIZE			(64 * 1024 * 1024)					// 升级包最大长度
#define	EAIS_SDK_MAX_DEL_NUM					100									// 单次最大删除用户个数
#define	EAIS_SDK_MAX_GROUP_NUM					10									// 最大用户组个数
#define	EAIS_SDK_USER_FACE_COMMENT_LEN			128									// 用户人脸自定义备注长度

#define EAIS_SDK_SUCCESS						0									// 成功
#define EAIS_SDK_GENERAL_ERROR_BEGIN			0									// 通用错误：范围 -1至-999
#define EAIS_SDK_ERROR_BAD_PARAMETER			(EAIS_SDK_GENERAL_ERROR_BEGIN - 1)	// 参数错误
#define EAIS_SDK_ERROR_BAD_LENGHT				(EAIS_SDK_GENERAL_ERROR_BEGIN - 2)	// 长度错误
#define EAIS_SDK_ERROR_NOT_FIND					(EAIS_SDK_GENERAL_ERROR_BEGIN - 3)	// 未找到
#define EAIS_SDK_ERROR_NULL_POITER				(EAIS_SDK_GENERAL_ERROR_BEGIN - 4)	// 空指针
#define EAIS_SDK_ERROR_NOT_ENOUGH_MEMORY		(EAIS_SDK_GENERAL_ERROR_BEGIN - 5)	// 内存不足
#define EAIS_SDK_ERROR_QUEUE_MAX				(EAIS_SDK_GENERAL_ERROR_BEGIN - 6)	// 队列满
#define EAIS_SDK_ERROR_QUEUE_EMPTY				(EAIS_SDK_GENERAL_ERROR_BEGIN - 7)	// 队列为空
#define EAIS_SDK_ERROR_THREAD_START_FAILED		(EAIS_SDK_GENERAL_ERROR_BEGIN - 8)	// 线程启动失败
#define EAIS_SDK_ERROR_THREAD_STOP_FAILED		(EAIS_SDK_GENERAL_ERROR_BEGIN - 9)	// 线程停止失败
#define EAIS_SDK_ERROR_BUSY						(EAIS_SDK_GENERAL_ERROR_BEGIN - 10)	// 设备或数据处于忙状态
#define EAIS_SDK_ERROR_PARAMETER_EXISTED		(EAIS_SDK_GENERAL_ERROR_BEGIN - 11)	// 参数已存在
#define EAIS_SDK_ERROR_BAD_ENVIRONMENT			(EAIS_SDK_GENERAL_ERROR_BEGIN - 12)	// 环境错误
#define EAIS_SDK_ERROR_NOT_SUPPORT				(EAIS_SDK_GENERAL_ERROR_BEGIN - 13)	// 不支持
#define EAIS_SDK_ERROR_FILE_PATH_NOT_EXIST		(EAIS_SDK_GENERAL_ERROR_BEGIN - 14)	// 文件或路径不存在
#define EAIS_SDK_ERROR_ABILITY_NOT_ENOUGH       (EAIS_SDK_GENERAL_ERROR_BEGIN - 15)	// 能力不足
#define EAIS_SDK_ERROR_ALREADY_EXISTED			(EAIS_SDK_GENERAL_ERROR_BEGIN - 16)	// 已初始化，不能重复
#define EAIS_SDK_ERROR_READ_FILE				(EAIS_SDK_GENERAL_ERROR_BEGIN - 17)	// 读文件失败
#define EAIS_SDK_ERROR_BAD_FILE				    (EAIS_SDK_GENERAL_ERROR_BEGIN - 18)	// 无效文件


#define EAIS_SDK_ERROR_NETWORK_BEGIN			-1000								// 网络错误号：-1000至-1999
#define EAIS_SDK_ERROR_SOCKET_OPEN_FAILED		(EAIS_SDK_ERROR_NETWORK_BEGIN - 1)	// 套接字open失败
#define EAIS_SDK_ERROR_SOCKET_BIND_FAILED		(EAIS_SDK_ERROR_NETWORK_BEGIN - 2)	// 套接字bind失败
#define EAIS_SDK_ERROR_SOCKET_LISTEN_FAILED		(EAIS_SDK_ERROR_NETWORK_BEGIN - 3)	// 套接字listen失败
#define EAIS_SDK_ERROR_SOCKET_CONNECT_FAILED	(EAIS_SDK_ERROR_NETWORK_BEGIN - 4)	// 套接字connect失败
#define EAIS_SDK_ERROR_SOCKET_CONNECT_TIMEOUT	(EAIS_SDK_ERROR_NETWORK_BEGIN - 5)	// 套接字connect超时
#define EAIS_SDK_ERROR_SOCKET_ACEPT_FAILED		(EAIS_SDK_ERROR_NETWORK_BEGIN - 6)	// 套接字acept失败
#define EAIS_SDK_ERROR_SOCKET_SEND_FAILED		(EAIS_SDK_ERROR_NETWORK_BEGIN - 7)	// 套接字发送失败
#define EAIS_SDK_ERROR_SOCKET_SEND_TIMEOUT		(EAIS_SDK_ERROR_NETWORK_BEGIN - 8)	// 套接字发送超时
#define EAIS_SDK_ERROR_SOCKET_RECV_FAILED		(EAIS_SDK_ERROR_NETWORK_BEGIN - 9)	// 套接字接收失败
#define EAIS_SDK_ERROR_SOCKET_RECV_TIMEOUT		(EAIS_SDK_ERROR_NETWORK_BEGIN - 10)	// 接收超时
#define EAIS_SDK_ERROR_SOCKET_CONNECTION_BROKEN	(EAIS_SDK_ERROR_NETWORK_BEGIN - 11)	// 连接断开
#define EAIS_SDK_ERROR_NETWORKIO_PORT_USED		(EAIS_SDK_ERROR_NETWORK_BEGIN - 12)	// 端口被占用
#define EAIS_SDK_ERROR_DOMAIN_NAME_PARSE_ERROR 	(EAIS_SDK_ERROR_NETWORK_BEGIN - 13)	// 域名解析错误
#define EAIS_SDK_ERROR_XML_PARSE_ERROR 			(EAIS_SDK_ERROR_NETWORK_BEGIN - 14)	// XML解析错误

#define EAIS_SDK_ERROR_BUSINESS_BEGIN			-2000								// 业务错误号：-2000至-2999
#define EAIS_SDK_ERROR_USERNAME_ERROR			(EAIS_SDK_ERROR_BUSINESS_BEGIN - 1)	// 登录用户名错误
#define EAIS_SDK_ERROR_PASSWORD_ERROR			(EAIS_SDK_ERROR_BUSINESS_BEGIN - 2)	// 登录口令错误
#define EAIS_SDK_ERROR_MAX_CONNECTION			(EAIS_SDK_ERROR_BUSINESS_BEGIN - 3)	// 连接数已经达到最大
#define EAIS_SDK_ERROR_MAX_USER					(EAIS_SDK_ERROR_BUSINESS_BEGIN - 4)	// 用户数量已达到最大
#define EAIS_SDK_ERROR_UPDATE_ERROR				(EAIS_SDK_ERROR_BUSINESS_BEGIN - 5)	// 升级失败
#define EAIS_SDK_ERROR_UPDATE_RUNNING			(EAIS_SDK_ERROR_BUSINESS_BEGIN - 6)	// 升级正在运行
#define EAIS_SDK_ERROR_UPDATE_TAR_FAILED		(EAIS_SDK_ERROR_BUSINESS_BEGIN - 7)	// 升级解压文件失败
#define EAIS_SDK_ERROR_UPDATE_CHK_FAILED		(EAIS_SDK_ERROR_BUSINESS_BEGIN - 8)	// 升级检查文件失败
#define EAIS_SDK_ERROR_ILLEGAL_ERROR			(EAIS_SDK_ERROR_BUSINESS_BEGIN - 9) // 非法命令

#define EAIS_SDK_ERROR_DECODE_FAILED			(EAIS_SDK_ERROR_BUSINESS_BEGIN - 100) // 图片解码失败
#define EAIS_SDK_ERROR_DETECT_FAILED			(EAIS_SDK_ERROR_BUSINESS_BEGIN - 101) // 人脸检测失败
#define EAIS_SDK_ERROR_ALIGN_FAILED				(EAIS_SDK_ERROR_BUSINESS_BEGIN - 102) // 人脸对齐失败
#define EAIS_SDK_ERROR_GET_FEATURE				(EAIS_SDK_ERROR_BUSINESS_BEGIN - 103) // 获取特征值失败
#define EAIS_SDK_ERROR_QUALITY					(EAIS_SDK_ERROR_BUSINESS_BEGIN - 104) // 质量不达标



#define EAIS_SDK_ERROR_ONVIF_BEGIN				-3000								// ONVIF错误号：-3000至-3999
#define EAIS_ONVIF_ERROR_BAD_REQUEST			(EAIS_SDK_ERROR_ONVIF_BEGIN - 400)	// 客户端语法错误
#define EAIS_ONVIF_ERROR_NOT_AUTH				(EAIS_SDK_ERROR_ONVIF_BEGIN - 401)	// 客户端请求未授权(是否需要ONVIF账户)
#define EAIS_ONVIF_ERROR_FORBIDDEN				(EAIS_SDK_ERROR_ONVIF_BEGIN - 402)	// 请求资源不可用
#define EAIS_ONVIF_ERROR_NOT_FOUND				(EAIS_SDK_ERROR_ONVIF_BEGIN - 403)	// 无法找到指定资源

#define EAIS_ONVIF_ERROR_SERVER_ERROR			(EAIS_SDK_ERROR_ONVIF_BEGIN - 500)	// 服务端异常
#define EAIS_ONVIF_ERROR_NOT_SUPPORT			(EAIS_SDK_ERROR_ONVIF_BEGIN - 501)	// 服务端不支持该功能
#define EAIS_ONVIF_ERROR_BAD_GATEWAY			(EAIS_SDK_ERROR_ONVIF_BEGIN - 502)	// 代理服务器接收到无效响应
#define EAIS_ONVIF_ERROR_SERVER_BUSY			(EAIS_SDK_ERROR_ONVIF_BEGIN - 503)	// 服务器忙碌
#define EAIS_ONVIF_ERROR_SERVER_TIMEOUT			(EAIS_SDK_ERROR_ONVIF_BEGIN - 504)	// 服务器超时
#define EAIS_ONVIF_ERROR_HTTP_VERSION_ERR		(EAIS_SDK_ERROR_ONVIF_BEGIN - 505)	// 服务器不支持的HTTP版本

#define EAIS_DB_ERROR_BEGIN						-4000								// 数据库错误号: -4000至4999
#define EAIS_DB_ERROR_INVALID_PARAM				(EAIS_DB_ERROR_BEGIN -1)			// 数据库无效参数	
#define EAIS_DB_ERROR_OPEN_DB_FAIL				(EAIS_DB_ERROR_BEGIN -2)			// 打开数据库失败
#define EAIS_DB_ERROR_CREATE_TABLE_FAI			(EAIS_DB_ERROR_BEGIN -3)			// 数据库创建表失败
#define EAIS_DB_ERROR_NO_FREE_SPACE				(EAIS_DB_ERROR_BEGIN -4)			// 数据库没有空间
#define EAIS_DB_ERROR_WRITE_FILE_FAILED			(EAIS_DB_ERROR_BEGIN -5)			// 数据库写文件失败
#define EAIS_DB_ERROR_ADD_FACE_FAILED			(EAIS_DB_ERROR_BEGIN -6)			// 数据库添加人脸失败
#define EAIS_DB_ERROR_GET_USER_FAILED			(EAIS_DB_ERROR_BEGIN -7)			// 数据库获取用户失败
#define EAIS_DB_ERROR_EXCUTE_FAILED				(EAIS_DB_ERROR_BEGIN -8)			// 数据库执行过程失败
#define EAIS_DB_ERROR_PREPARE_AFILED			(EAIS_DB_ERROR_BEGIN -9)			// 数据库准备过程失败
#define EAIS_DB_ERROR_MATCH_FAILED				(EAIS_DB_ERROR_BEGIN -10)			// 数据库匹配失败
#define EAIS_DB_ERROR_BLOB_FAILED				(EAIS_DB_ERROR_BEGIN -11)			// 数据库绑定失败
#define EAIS_DB_ERROR_ID_EXIST					(EAIS_DB_ERROR_BEGIN -12)			// 数据库ID已存在
#define EAIS_DB_ERROR_ERROR						(EAIS_DB_ERROR_BEGIN -13)			// 数据库命令释放失败

#define REGISTER_ERROR_BEGIN					-9000								// 批量注册人脸错误号: -9000至-9999
#define REGISTER_ERROR_OPEN_FILE_ERROR			(REGISTER_ERROR_BEGIN -1)			// 打开配置文件失败	
#define REGISTER_ERROR_READ_STRING_ERROR		(REGISTER_ERROR_BEGIN -2)			// 读取文件字符串失败	
#define REGISTER_ERROR_NOT_FIND_DEST			(REGISTER_ERROR_BEGIN -3)			// 未找到目标字符串	
#define REGISTER_ERROR_PIC_NAMEED_ERROR			(REGISTER_ERROR_BEGIN -4)			// 文件名不符合命名规则	
#define REGISTER_ERROR_NOT_FIND_PIC				(REGISTER_ERROR_BEGIN -5)			// 在指定图片文件夹未找到*.jpg	
#define REGISTER_ERROR_WRITE_LOG_FAIL			(REGISTER_ERROR_BEGIN -6)			// 打开日志文件失败	
#define REGISTER_ERROR_PATH_EMPTY				(REGISTER_ERROR_BEGIN -7)			// 配置文件路径不存在	
#define REGISTER_ERROR_REGISTERING				(REGISTER_ERROR_BEGIN -8)			// 正在注册人脸	

// 设备类型（按功能区分）
enum ENUM_EAIS_DEVICE_TYPE
{
	EAIS_DEVICE_TYPE_UNKNOW  	 = 0,											// 未知类型
	EAIS_DEVICE_TYPE_SNAP 	 	 = 1,											// 抓拍类型EAIS设备
	EAIS_DEVICE_TYPE_RECOGNIZE 	 = 2,											// 识别类型EAIS设备
};
 
// 设备状态
enum ENUM_EAIS_DEVICE_STATUS
{
	EAIS_DEVICE_STATUS_UNKNOW  	 = 0,											// 未知类型
	EAIS_DEVICE_STATUS_ONLINE 	 = 1,											// 在线或RTSP有效
	EAIS_DEVICE_STATUS_OFFLINIE  = 2,											// 离线或RTSP无效
	EAIS_DEVICE_STATUS_AUTH_ERR	 = 3,											// 鉴权失败
	EAIS_DEVICE_STATUS_PARSE_ERR = 4,											// RTSP不兼容
};

// 结构化数据类型
enum ENUM_EAIS_STRUCT_DATA_TYPE
{
	EAIS_DATA_TYPE_UNKNOW		= 0x00,											// 未知类型
	EAIS_DATA_TYPE_P			= 0x01,											// P为Point坐标缩写
	EAIS_DATA_TYPE_D			= 0x02,											// D为Data图片数据缩写
	EAIS_DATA_TYPE_PD			= 0x10,											// P+D类型
	EAIS_DATA_TYPE_PDD			= 0x20,											// P+D+D类型
	EAIS_DATA_TYPE_PDA			= 0x400,										// A为Attr属性缩写，P+D+A类型
	EAIS_DATA_TYPE_PDF			= 0x800,										// F为Feature特征值缩写，P+D+F类型		
	EAIS_DATA_TYPE_PDAF			= 0x4000,										// P+D+A+F类型
	EAIS_DATA_TYPE_CDD			= 0x8000,										// C为人脸比对缩写，C+D+D类型
};

// 方案类型
enum ENUM_EAIS_SOLUTION_TYPE
{
	EAIS_SOLUTION_TYPE_SNAP			= 0,										// 抓拍方案
	EAIS_SOLUTION_TYPE_HELMET		= 1,										// 安全帽方案
	EAIS_SOLUTION_TYPE_RECOGNITION	= 2,										// 识别方案
};

// 用户账户
typedef struct
{
	char						szUserName[EAIS_SDK_MAX_COMMON_LEN];			// 用户名
	char						szPassWord[EAIS_SDK_MAX_COMMON_LEN];			// 密码
	char						szNewPassWord[EAIS_SDK_MAX_COMMON_LEN];			// 新密码
}ST_Account;

// 设备时间
typedef struct
{
	int							nYear;											// 年
	int							nMonth;											// 月
	int							nDay;											// 日
	int							nHour;											// 时
	int							nMinute;										// 分
	int							nSecond;										// 秒;	
}ST_SDKDeviceTime;

// 设备信息（含能力）
typedef struct
{
	int							nDeviceType;									// 设备类型 ENUM_EAIS_DEVICE_STATUS
	int							nDeviceProto;									// 设备协议，默认OAL协议
	int							nMaxChannelNum;									// 设备最大通道数
	char						szDeviceName[EAIS_SDK_MAX_COMMON_LEN];			// 设备名称
	char						szDeviceId[EAIS_SDK_MAX_COMMON_LEN];			// 设备Id
	int							nNetCardId;										// 网卡Id
	char						szNetCardName[EAIS_SDK_MAX_COMMON_LEN];			// 网卡名，如"eth0"
	char						szDeviceIP[EAIS_SDK_MAX_COMMON_LEN];			// 设备IPAddr
	char						szSubnetMask[EAIS_SDK_MAX_COMMON_LEN];			// 子网掩码
	char						szGatewayAddr[EAIS_SDK_MAX_COMMON_LEN];			// 网关地址
	char						szPreferredDns[EAIS_SDK_MAX_COMMON_LEN];		// 首选DNS
	char						szAlternateDns[EAIS_SDK_MAX_COMMON_LEN];		// 备用DNS
	int							nDeviceSerPort;									// 设备服务端口
	int							nDeviceUpdatePort;								// 设备升级服务端口号
	int							nDeviceHttpPort;								// HTTP服务端口号
	int							nDeviceRTSPPort;								// RTSP服务端口号
	char						szMACAddr[EAIS_SDK_MAX_COMMON_LEN];				// MAC地址
	char						szManufacturerName[EAIS_SDK_MAX_COMMON_LEN];	// 厂商名称
	char						szManufacturerId[EAIS_SDK_MAX_COMMON_LEN];		// 设备型号
	char						szProductModel[EAIS_SDK_MAX_COMMON_LEN];		// 产品模组
	char						szSN[EAIS_SDK_MAX_COMMON_LEN];					// SN
	char						szSoftWareInfo[EAIS_SDK_MAX_COMMON_LEN];		// 软件包信息
	char						szHardWareInfo[EAIS_SDK_MAX_COMMON_LEN];		// 硬件信息
	char                        szCoreSN[EAIS_SDK_MAX_COMMON_LEN];				// 核心板SN号
	unsigned int				nDeviceRunTime;									// 设备运行时间
	int							nWorkMode;										// 0: 正常模式 1：老化模式
	ST_SDKDeviceTime			stDeviceSystemTime;								// 设备当前系统时间
}ST_SDKDeviceInfo;

// Onvif设备搜索信息
typedef struct
{
	char						szServiceAddr[EAIS_SDK_MAX_COMMON_LEN];			// 服务地址
	char						szDeviceIP[EAIS_SDK_MAX_COMMON_LEN];			// 设备IP
	char						szDeviceUUID[EAIS_SDK_MAX_COMMON_LEN];			// 设备uuid
	int							nHttpPort;										// HTTP端口
}ST_OnvifSearchInfo;

// 设备状态
typedef struct
{
	int							nLoginHandle;									// EAIS设备区分
	int							nChannlId;										// 通道区分，如9通道则[1, 9]，0则设备本身
	int							nStatusBit;										// 位操作，目前后3bit有效，0bit置1说明是eESStatus状态，1bit置1说明是eRTSPStatus，如果3个bit都置1，说明3个状态字段都有变更（有效），
	int							nESStatus;										// EAIS设备状态，默认离线。 ENUM_EAIS_DEVICE_STATUS
	int							nRTSPStatus;									// 通道RTSP状态，复用设备状态。默认正常。 ENUM_EAIS_DEVICE_STATUS
	int							nDecodeStatus;									// 解码状态（0：默认正常，1：异常等）
	char						szDeviceIP[EAIS_SDK_MAX_COMMON_LEN];			// 设备IP
}ST_SDKStatusInfo;

// RTSP信息
typedef struct
{
	int							nChannleId;										// 通道区分	
	int							nRTSPStatus;									// 通道RTSP状态 ENUM_EAIS_DEVICE_STATUS
	int							nDecodeStatus;									// 解码状态 
	char						szChanRtspUrl[EAIS_SDK_MAX_RTSP_URL_LEN];		// 通道URL
}ST_SDKRTSPInfo;

// RTSP配置：设置信息时仅RTSP URL有效
typedef struct
{
	int							nRTSPInfoNum;									// 有效RTSP信息个数
	ST_SDKRTSPInfo				stRTSPInfoArr[EAIS_SDK_MAX_CHANNEL_SUPPORT];	// 通道RTSP信息
}ST_SDKRTSPConfig;

// 坐标
typedef struct
{
	int							nX;												// 坐标X
	int							nY;												// 坐标Y
}ST_SDKPoint;

// 抓拍策略配置
typedef struct
{
	int							nChannelId;										// 抓拍通道
	int							nConfi;											// 置信度（0, 100）
	int							nQulityScore;									// 质量分数（0, 100）
	int							nAttrEnable;									// 抓拍使能属性输出，0：不使能，1：使能
	int							nFeatureEnable;									// 抓拍使能特征值输出，0：不使能，1：使能
	int							nAlignEnable;									// 抓拍人脸矫正使能，0：不使能，1：使能
	int							nAreaEnable;									// 抓拍区域使能，0：不使能（使用整个画面），1：使能
	int							nTripWireEnable;								// 抓拍绊线使能，0：不使能，1：使能
	int							nSnapMode;										// 抓拍模式(0: 禁用; 1: 实时; 2: 离开; 3: 定时; 4: 进入抓拍)
	int							nIntervalTime;									// 定时/进入上传时间(单位秒)
	int							nMaxSize;										// 抓拍最大脸设置 (0, 1920]
    int							nMinSize;										// 抓拍最小区域设置 (0， 1920)
	ST_SDKPoint					stAreaPointArr[6];								// 抓拍区域，区域为6边形，所以有六个坐标
	int							nAreaConfigWidth;								// 设置抓拍区域时对应的分辨率宽
	int							nAreaConfigHeight;								// 设置抓拍区域时对应的分辨率高
	ST_SDKPoint					stTripWirePointArr[2];							// 绊线有两个坐标，坐标格式(x,y)
	int							nTripConfigWidth;								// 设置抓拍绊线时对应的分辨率宽
	int							nTripConfigHeight;								// 设置抓拍绊线时对应的分辨率高
}ST_SDKSnapInfo;

// 抓拍策略配置
typedef struct
{
	int							nSnapInfoNum;									// 有效数据个数
	int							nOriginalPicEnable;								// 原图输出，0：不使能，1：使能
	ST_SDKSnapInfo				stSnapInfoArr[EAIS_SDK_MAX_CHANNEL_SUPPORT];	// 通道抓拍信息
}ST_SDKSnapConfig;

// 抓拍坐标
typedef struct
{	// 坐标： Point（后续不仅限于人脸）
	int                  		nConfidence;									// 置信度，范围0~100
	int							nQulityScore;									// 质量分数，范围0~100
	int							nSrcW;											// 原图宽度
	int							nSrcH;											// 原图高度
	int                     	nX;                								// 图片框X坐标
	int                     	nY;                								// 图片框Y坐标
	int                     	nW;                								// 图片框宽度
	int                     	nH;												// 图片框高度
	unsigned long long     		lSnapTime;										// 时间戳毫秒 			
}ST_SDKSnapPoint;

// 抓拍数据
typedef struct
{
	int							nDataLen;										// 图片数据长度
	char*						pszSnapData;									// 图片数据
}ST_SDKSnapData;

// 抓拍属性
typedef struct
{
	int 						nTargeType;										// 目标类型（0：人脸，1：人体，2：车牌，3：车体）
	union
	{
		struct
		{
			char				cGender;										// 性别（0：男，1：女）
			char				cMode;											// 心情（0：平静，1：高兴）
			char				cAge;											// 年龄（0：儿童，1：少年，2：青年，3：中年，4：老年）
			char				cComplexion;									// 肤色
			char				cMinority;										// 民族
			char				szReserve[59];									// 预留
		}Face;
	}Type;
}ST_SDKSnapAttr;

// 抓拍特征值
typedef struct
{
	int							nFeatureLen;									// 特征值个数，长度=个数*sizoof(float)
	float*						pfFeatureData;									// 特征值
}ST_SDKSnapFea;

// 人脸比对结果
typedef struct
{
	float 						fSimilarity;									// 相似度
	int							nGroupID;										// 所属组ID
	char						szGroupName[EAIS_SDK_MAX_COMMON_LEN];			// 组名称
	char						szUserID[EAIS_SDK_MAX_COMMON_LEN];				// 证件号
	char						szUserName[EAIS_SDK_MAX_COMMON_LEN];			// 姓名
	int							nGender;										// 人员性别    
	char						szComment[EAIS_SDK_USER_FACE_COMMENT_LEN];		// 用户自定义备注字段
	char						szReserved[64];									// 预留位，便于拓展，默认置空 
}ST_SDKCompareResult;

// 结构化信息
typedef struct
{
	int							nLoginHandle;									// EAIS设备区分
	char						szDeviceIP[EAIS_SDK_MAX_COMMON_LEN];			// 设备IP
	int							nDevicePort;								    // 设备端口
    int							nChannlId;										// 通道区分
	int							nDataType;										// 数据类型：ENUM_EAIS_STRUCT_DATA_TYPE枚举类型
	int							nTargeNum;										// 目标个数，目前仅支持1个目标
	char*						pszStructData;									// 结构化数据，根据nDataType、nTargeNum规则解析。
	char						szDeviceId[EAIS_SDK_MAX_COMMON_LEN];			// 设备Id
	int							nSnapSerialNum;									// 抓拍序号
	int							nSolutionType;									// 方案类型： ENUM_EAIS_SOLUTION_TYPE枚举类型
}ST_SDKStructInfo;

// ip信息
typedef struct
{
    int							nNetCardId;                               		// 网卡Id
	int							nDhcp;											// 默认支持DHCP，0表示dhcp，1表示静态IP
    char						szTargetIPAddr[EAIS_SDK_MAX_COMMON_LEN];		// IP地址
    char						szSubnetMask[EAIS_SDK_MAX_COMMON_LEN];			// 子网掩码
    char						szGatewayAddr[EAIS_SDK_MAX_COMMON_LEN];			// 网关地址
	char						szNetMacAddr[EAIS_SDK_MAX_COMMON_LEN];			// 网卡MAC地址
    char						szPreferredDns[EAIS_SDK_MAX_COMMON_LEN];		// 首选DNS
    char						szAlternateDns[EAIS_SDK_MAX_COMMON_LEN];		// 备用DNS
    char						szNetcardName[EAIS_SDK_MAX_COMMON_LEN];			// 网卡名
}ST_SDKIPInfo;

// ip参数配置
typedef struct
{
	int							nIpInfoNum;										// 有效数据个数
	ST_SDKIPInfo				stIpInfoArr[EAIS_SDK_MAX_CHANNEL_SUPPORT];		// IP信息
	// 以下信息仅适用于UDP修改IP配置，基于登录方式可忽略
	char						szDeviceId[EAIS_SDK_MAX_COMMON_LEN];			// 设备Id
	char						szUserName[32];									// 用户名
	char						szPassWord[32];									// 密码
}ST_SDKIPConfig;

// 抓拍图片上传服务器地址信息，目前支持Ftp、Http协议上传
typedef struct
{
	int							nEnable;										// 0：不使能，1：使能
	char						szServerUrl[EAIS_SDK_MAX_RTSP_URL_LEN];			// Ftp、Http服务URL或IP
	int							nDevicePort;									// 端口 
	char						szUserName[32];									// 用户名
	char						szPassWord[32];									// 密码
	int							nProtocol;										// 当协议为http时有效
	char						szFtpFilePath[EAIS_SDK_MAX_PATH_LEN];			// Ftp文件上传路径
}ST_SDKSnapServerAddr;

// 恢复出厂
typedef struct  
{
	char						szDeviceID[64];									// 设备ID	
	int							nSaveNet;										// 是否保存网络 1：保存 0：不保存
	int						    nSaveAccount;									// 是否保存账户 1：保存 0：不保存
	int						    nSaveRtspUrl;									// 是否保存rtsp url 1 ：保存 0：不保存
}ST_SDKResetInfo;

// NTP配置
typedef struct
{
	int							nEnable;										// NTP使能 0：不使能，1：使能
	char						szDomainName[EAIS_SDK_MAX_COMMON_LEN];			// 域名/ip
	int							nTimingInterval;								// 校时间隔(0, 65535]分钟
}ST_SDKNTPInfo;

// 图片类型枚举
enum ENUM_PICTURE_TYPE
{
	EAIS_PICTURE_JPG			= 0,											// JPG格式
	EAIS_PICTURE_PNG			= 1,											// PNG格式
	EAIS_PICTURE_BASE64			= 2,											// BASE64格式
};

// 图片信息
typedef struct
{
	const char*					szPictureData;									// 图片内容
	int							nDataLen;										// 图片长度（1, 10 * 1024 * 1024]字节
	int							nPictureType;									// 图片类型 类型：ENUM_PICTURE_TYPE
}ST_SDKPictureInfo;

// 坐标集合
typedef struct
{
	int							nNumber;										// 点个数[1, 8]
	ST_SDKPoint					stPointBuf[8];									// 坐标数组
}ST_SDKPointArray;

// 感兴趣区域
typedef struct  
{
	int							nEnable;										// 0: 不使能 1：使能
	ST_SDKPointArray			stPointArray;									// 感兴趣区域坐标
}ST_SDKROIArea;

// 安全帽信息
typedef struct
{
	int							nChannelId;										// 通道Id
	int							nSentImage;										// 是否发送图片0：不使能 1：使能
	int							nImageRatio;									// 图片压缩比配置（0, 100]
	int							nUseTracking;									// 是否使用跟踪0: 不使能 1：使能
	int							nDrawRectangle;									// 是否画框0: 不使能 1：使能
	int							nSnapInterval;									// 抓拍间隔时间（0, 3600]s
	int							nAlarmInterval;									// 警报间隔时间（0, 3600]s
	int							nSnapRatio;										// 抓拍上传图质量比（0, 100]
	int							nUploadMode;									// 抓拍上传模式(0 一刀切模式)（0, 100]
	int							nThreshold;										// 阈值（0, 100]
	ST_SDKROIArea				stROIArea;										// ROI配置							
}ST_SDKHelmetInfo;

// 安全帽配置
typedef struct
{
	int							nHelmetInfoNum;									// 有效数据个数
	ST_SDKHelmetInfo			stHelmetInfoArr[EAIS_SDK_MAX_CHANNEL_SUPPORT];	// 通道安全帽信息
}ST_SDKHelmetConfig;

// 人脸识别
// 用户信息
typedef struct
{
	int							nGroupID;										// 所属组ID  0： 黑名单 1： 白名单
	char						szUserName[EAIS_SDK_MAX_COMMON_LEN];			// 姓名
	char						szUserID[EAIS_SDK_MAX_COMMON_LEN];				// 证件号
	int							nGender;										// 人员性别  0： 女 1： 男
	int        			        nFaceLen;										// 人脸图片数据长度（1, 1024 * 1024]字节
	unsigned char*				szPictureData;									// 图片内容
	char						szComment[EAIS_SDK_USER_FACE_COMMENT_LEN];		// 用户自定义备注字段
	int							nRegisterTime;									// 注册时间，从1970-01-01 00:00:00 (utc) 开始计时的秒数
	char						szReserved[256];								// 预留位，便于拓展，默认置空 
}ST_SDKUserInfo;

// 用户列表
typedef struct  
{
	int							nGroupID;										// 所属组ID  0： 黑名单 1： 白名单
	int							nUserNum;										// 用户个数
	char						stUserIdentityArr[EAIS_SDK_MAX_DEL_NUM][64];	// 最大支持100个
}ST_SDKUserList;

// 用户组信息
typedef struct  
{
	int							nGroupId;										// 所属组ID  0： 黑名单 1： 白名单
	char						szGroupName[EAIS_SDK_MAX_COMMON_LEN];			// 组名称
}ST_SDKGroupInfo;

// 用户组列表信息
typedef struct
{
	int							nGroupNum;										// 个数
	ST_SDKGroupInfo				stGroupInfoArr[EAIS_SDK_MAX_GROUP_NUM];			// 最大支持组10个
}ST_SDKGroupInfoList;

// 用户结构数据信息
typedef struct
{
	int							nUserNum;										// 用户个数
	char*						pszStructData;      							// 用户
}ST_SDKUserData;

// 识别配置参数
typedef struct
{								
	int							nOutSimilarity;									// 输出相似度阈值（0, 100]
	int							nRegisterQuality;								// 注册质量阈值（0, 100]
	int							nOpenUpload;									// 是否打开比对上传0： 不上传 1： 上传
}ST_SDKRecognizeConfig;

// 查询人数统计请求
typedef struct
{
	int							nChannelId;										// 通道ID  p_nChannelID  1~n：某个通道  -1：查询所有通道
	int							nStartTime;										// 开始时间 从1970-01-01 00:00:00 (utc) 开始计时的秒数，当为-1代表查询所有时间段
	int							nEndTime;										// 结束时间 从1970-01-01 00:00:00 (utc) 开始计时的秒数
}ST_SDKQueryPeopleReq;

// 人数统计信息
typedef struct
{
	int							nCaptureCount;									// 抓拍人数
	int							nUploadCount;									// 实际上传人数
	int							nOverLineCount;									// 越线人数 
}ST_SDKPeopleCount;

// 设备登录信息
typedef struct
{
	char						szDeviceIP[EAIS_SDK_MAX_COMMON_LEN];				// IP地址
	int							nDevicePort;											// 端口
	char						szUserName[EAIS_SDK_MAX_COMMON_LEN];			// 用户名
	char						szPassWord[EAIS_SDK_MAX_COMMON_LEN];			// 密码
	int							nTimeOut;										// 登录超时
}ST_SDKLoginInfo;

#endif
