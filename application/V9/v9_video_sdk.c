/*
 * v9_sdk.c
 *
 *  Created on: 2019年12月1日
 *      Author: zhurish
 */

#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"

//#define APP_V9_MODULE

#include "v9_device.h"
#include "v9_util.h"
#include "v9_video.h"
#include "v9_serial.h"
#include "v9_slipnet.h"
#include "v9_cmd.h"

#include "v9_video_disk.h"
#include "v9_user_db.h"
#include "v9_video_db.h"

#include "v9_board.h"
#include "v9_video_sdk.h"
#include "v9_video_user.h"
#include "v9_video_board.h"
#include "v9_video_api.h"


static BOOL v9_video_sdk_isopen(v9_video_sdk_t *);
static int v9_video_sdk_open(v9_video_sdk_t *);
static int v9_video_sdk_timeout(struct eloop *eloop);

static BOOL __sdk_initialization = FALSE;
int __sdk_debug_flag = V9_SDK_DEBUG_EVENT|V9_SDK_DEBUG_ERROR|V9_SDK_DEBUG_WARN;


char *v9_video_sdk_errnostr(int err)
{
#ifdef V9_VIDEO_SDK_API
	switch(err)
	{
		case EAIS_SDK_ERROR_BAD_PARAMETER:// 参数错误
			return "BAD Parameter";
		case EAIS_SDK_ERROR_BAD_LENGHT:// 长度错误
			return "BAD Lenght";
		case EAIS_SDK_ERROR_NOT_FIND:// 未找到
			return "NOT Find";
		case EAIS_SDK_ERROR_NULL_POITER:// 空指针
			return "NULL Pointer";
		case EAIS_SDK_ERROR_NOT_ENOUGH_MEMORY:// 内存不足
			return "NOT Enough Memory";
		case EAIS_SDK_ERROR_QUEUE_MAX:// 队列满
			return "FULL Queue";
		case EAIS_SDK_ERROR_QUEUE_EMPTY:// 队列为空
			return "Queue Empty";
		case EAIS_SDK_ERROR_THREAD_START_FAILED:// 线程启动失败
			return "Thread Start Failed";
		case EAIS_SDK_ERROR_THREAD_STOP_FAILED:// 线程停止失败
			return "Thread Stop Failed";
		case EAIS_SDK_ERROR_BUSY:// 设备或数据处于忙状态
			return "BUSY";
		case EAIS_SDK_ERROR_PARAMETER_EXISTED:// 参数已存在
			return "Parameter Existed";
		case EAIS_SDK_ERROR_BAD_ENVIRONMENT:// 环境错误
			return "BAD Environment";
		case EAIS_SDK_ERROR_NOT_SUPPORT:// 不支持
			return "NOT Support";
		case EAIS_SDK_ERROR_FILE_PATH_NOT_EXIST:// 文件或路径不存在
			return "File Path Not Exist";
		case EAIS_SDK_ERROR_ABILITY_NOT_ENOUGH:// 能力不足
			return "Ability Not Enough";
		case EAIS_SDK_ERROR_ALREADY_EXISTED:// 已初始化，不能重复
			return "Already Existed";
		case EAIS_SDK_ERROR_READ_FILE:// 读文件失败
			return "Read File";
		case EAIS_SDK_ERROR_BAD_FILE:// 无效文件
			return "BAD File";

		// 网络错误号：-1000至-1999
		case EAIS_SDK_ERROR_SOCKET_OPEN_FAILED:// 套接字open失败
			return "Open Socket Failed";
		case EAIS_SDK_ERROR_SOCKET_BIND_FAILED:// 套接字bind失败
			return "Bind Socket Failed";
		case EAIS_SDK_ERROR_SOCKET_LISTEN_FAILED:// 套接字listen失败
			return "Listen Socket Failed";
		case EAIS_SDK_ERROR_SOCKET_CONNECT_FAILED:// 套接字connect失败
			return "Connect Socket Failed";
		case EAIS_SDK_ERROR_SOCKET_CONNECT_TIMEOUT:// 套接字connect超时
			return "Connect Socket Timeout";
		case EAIS_SDK_ERROR_SOCKET_ACEPT_FAILED:// 套接字acept失败
			return "Acept Socket Failed";
		case EAIS_SDK_ERROR_SOCKET_SEND_FAILED:// 套接字发送失败
			return "Send Socket Failed";
		case EAIS_SDK_ERROR_SOCKET_SEND_TIMEOUT:// 套接字发送超时
			return "Send Socket Timeout";
		case EAIS_SDK_ERROR_SOCKET_RECV_FAILED:// 套接字接收失败
			return "Recv Socket Failed";
		case EAIS_SDK_ERROR_SOCKET_RECV_TIMEOUT:// 接收超时
			return "Recv Socket Timeout";
		case EAIS_SDK_ERROR_SOCKET_CONNECTION_BROKEN:// 连接断开
			return "Connection Socket Broken";
		case EAIS_SDK_ERROR_NETWORKIO_PORT_USED:// 端口被占用
			return "Port Already Use";
		case EAIS_SDK_ERROR_DOMAIN_NAME_PARSE_ERROR:// 域名解析错误
			return "Domain Name Error";
		case EAIS_SDK_ERROR_XML_PARSE_ERROR:// XML解析错误
			return "XML Error";
		// 业务错误号：-2000至-2999
		case EAIS_SDK_ERROR_USERNAME_ERROR:// 登录用户名错误
			return "Username Error";
		case EAIS_SDK_ERROR_PASSWORD_ERROR:// 登录口令错误
			return "Password Error";
		case EAIS_SDK_ERROR_MAX_CONNECTION:// 连接数已经达到最大
			return "Too Much Connection";
		case EAIS_SDK_ERROR_MAX_USER:// 用户数量已达到最大
			return "Too Much User";
		case EAIS_SDK_ERROR_UPDATE_ERROR:// 升级失败
			return "Update Error";
		case EAIS_SDK_ERROR_UPDATE_RUNNING:// 升级正在运行
			return "Update Running";
		case EAIS_SDK_ERROR_UPDATE_TAR_FAILED:// 升级解压文件失败
			return "Update Tar Error";
		case EAIS_SDK_ERROR_UPDATE_CHK_FAILED:// 升级检查文件失败
			return "Update Check Error";
		case EAIS_SDK_ERROR_ILLEGAL_ERROR:// 非法命令
			return "Illegal command";
		case EAIS_SDK_ERROR_DECODE_FAILED:// 图片解码失败
			return "Decode Failed";
		case EAIS_SDK_ERROR_DETECT_FAILED:// 人脸检测失败
			return "Detect Failed";
		case EAIS_SDK_ERROR_ALIGN_FAILED:// 人脸对齐失败
			return "Align Failed";
		case EAIS_SDK_ERROR_GET_FEATURE:// 获取特征值失败
			return "Get Feature Failed";
		case EAIS_SDK_ERROR_QUALITY:// 质量不达标
			return "Quality Failed";

		// ONVIF错误号：-3000至-3999
		case EAIS_ONVIF_ERROR_BAD_REQUEST:// 客户端语法错误
			return "ONVIF Bad Request";
		case EAIS_ONVIF_ERROR_NOT_AUTH:// 客户端请求未授权(是否需要ONVIF账户)
			return "ONVIF Not Auth";
		case EAIS_ONVIF_ERROR_FORBIDDEN:// 请求资源不可用
			return "ONVIF Forbidden";
		case EAIS_ONVIF_ERROR_NOT_FOUND:// 无法找到指定资源
			return "ONVIF Not Found";

		case EAIS_ONVIF_ERROR_SERVER_ERROR:// 服务端异常
			return "ONVIF Server Error";
		case EAIS_ONVIF_ERROR_NOT_SUPPORT:// 服务端不支持该功能
			return "ONVIF Not Support";
		case EAIS_ONVIF_ERROR_BAD_GATEWAY:// 代理服务器接收到无效响应
			return "ONVIF Bad Gateway";
		case EAIS_ONVIF_ERROR_SERVER_BUSY:// 服务器忙碌
			return "ONVIF Server Busy";
		case EAIS_ONVIF_ERROR_SERVER_TIMEOUT:// 服务器超时
			return "ONVIF Server Timeout";
		case EAIS_ONVIF_ERROR_HTTP_VERSION_ERR:// 服务器不支持的HTTP版本
			return "ONVIF Http Version Not Support";
		// 数据库错误号: -4000至4999
		case EAIS_DB_ERROR_INVALID_PARAM:// 数据库无效参数
			return "DB Invalid Param";
		case EAIS_DB_ERROR_OPEN_DB_FAIL:// 打开数据库失败
			return "DB Open Filed";
		case EAIS_DB_ERROR_CREATE_TABLE_FAI:// 数据库创建表失败
			return "DB Create Failed";
		case EAIS_DB_ERROR_NO_FREE_SPACE:// 数据库没有空间
			return "DB No Free Space";
		case EAIS_DB_ERROR_WRITE_FILE_FAILED:// 数据库写文件失败
			return "DB Write File Failed";
		case EAIS_DB_ERROR_ADD_FACE_FAILED:// 数据库添加人脸失败
			return "DB Add Face Failed";
		case EAIS_DB_ERROR_GET_USER_FAILED:// 数据库获取用户失败
			return "DB Get User Failed";
		case EAIS_DB_ERROR_EXCUTE_FAILED:// 数据库执行过程失败
			return "DB Excute Failed";
		case EAIS_DB_ERROR_PREPARE_AFILED:// 数据库准备过程失败
			return "DB Prepare Failed";
		case EAIS_DB_ERROR_MATCH_FAILED:// 数据库匹配失败
			return "DB Match Failed";
		case EAIS_DB_ERROR_BLOB_FAILED:// 数据库绑定失败
			return "DB BLOB Failed";
		case EAIS_DB_ERROR_ID_EXIST:// 数据库ID已存在
			return "DB ID Exist";
		case EAIS_DB_ERROR_ERROR:// 数据库命令释放失败
			return "DB Free Error";

		//case EAIS_HS_ERROR_BEGIN:// 罕森业务错误号: -5000至-5999
		case EAIS_HS_ERROR_LOGIN_FAIL:		// IPC登录失败
			return "HS Login Fail";
		case EAIS_HS_ERROR_ATTACH_RADIO_FAIL:			// 订阅IPC热图服务失败
			return "HS Attach Radio Fail";
		case EAIS_HS_ERROR_FETCH_RADIO_FAIL:			// 获取一次热图数据失败
			return "HS Fetch Radio Fail";
		case EAIS_HS_ERROR_PARSE_RADIO_FAIL:		// 解析热图数据失败
			return "HS Parse Radio Fail";
		case EAIS_HS_ERROR_SDK_INIT_FAIL:			// 大华SDK初始化失败
			return "HS SDK Init Fail";
		case EAIS_HS_ERROR_BUSY_COUNTING:		// 正在统计区域温度
			return "HS Busy Fail";

		// 批量注册人脸错误号: -9000至-9999
		case REGISTER_ERROR_OPEN_FILE_ERROR:// 打开配置文件失败
			return "Reg Open File Error";
		case REGISTER_ERROR_READ_STRING_ERROR:// 读取文件字符串失败
			return "Reg Read String Error";
		case REGISTER_ERROR_NOT_FIND_DEST:// 未找到目标字符串
			return "Reg Not Find Dest";
		case REGISTER_ERROR_PIC_NAMEED_ERROR:// 文件名不符合命名规则
			return "Reg PIC Nameed Error";
		case REGISTER_ERROR_NOT_FIND_PIC:// 在指定图片文件夹未找到*.jpg
			return "Reg Not Find Pic";
		case REGISTER_ERROR_WRITE_LOG_FAIL:// 打开日志文件失败
			return "Reg Write Log Failed";
		case REGISTER_ERROR_PATH_EMPTY:// 配置文件路径不存在
			return "Reg Path Empty";
		case REGISTER_ERROR_REGISTERING:// 正在注册人脸
			return "Reg is running";
		default:
			return "Unknown Error";
	}
#endif
	return "Unknown Error";
}




static BOOL v9_video_sdk_isopen(v9_video_sdk_t *sdk)
{
	zassert(sdk);
	return sdk? sdk->login:FALSE;
}

/*BOOL v9_video_sdk_isopen(v9_video_sdk_t *sdk)
{
	zassert(sdk);
	return sdk? sdk->login:FALSE;
}*/

v9_video_sdk_t * v9_video_sdk_lookup(u_int8 id)
{
	int i =0;
	zassert(v9_video_board);
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board[i].id == id)
		{
			return &v9_video_board[i].sdk;
		}
	}
	return NULL;
}


int v9_video_sdk_init(v9_video_sdk_t *sdk, void *board)
{
	zassert(sdk);
	zassert(board);

	//初始化最大支持4个EAIS设备
#ifdef V9_VIDEO_SDK_API
	if(__sdk_initialization == FALSE)
	{
		if (EAIS_SDK_SUCCESS != EAIS_SDK_Init(V9_APP_BOARD_MAX + 1))
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP, "EAIS_SDK_Init ERROR");
			return ERROR;
		}
		__sdk_initialization = TRUE;
	}
#endif

	if (sdk->initialization == FALSE)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->device == NULL)
			sdk->device = XMALLOC(MTYPE_VIDEO_SDK, sizeof(ST_SDKDeviceInfo));
		if (sdk->device)
			memset(sdk->device, 0, sizeof(ST_SDKDeviceInfo));
#endif
		if (master_eloop[PL_APPLICATION_MODULE_START + 1] == NULL)
			sdk->master = master_eloop[PL_APPLICATION_MODULE_START + 1] =
					eloop_master_module_create(PL_APPLICATION_MODULE_START + 1);

		sdk->interval = 5;
		//sdk->cnt = 0;
		sdk->status = 0;
		sdk->handle = -1;
		//sdk->port = V9_VIDEO_SDK_PORT;
		strcpy(sdk->username, V9_VIDEO_SDK_USERNAME);
		strcpy(sdk->password, V9_VIDEO_SDK_PASSWORD);
		sdk->board = board;
		//sdk->master = m;
		if (sdk->master)
		{
			if (sdk->interval)
			{
				sdk->t_timeout = eloop_add_timer(sdk->master,
						v9_video_sdk_timeout, sdk, 3);

			}
		}
		sdk->initialization = TRUE;
		return OK;
	}
	return ERROR;
}



#ifdef V9_VIDEO_SDK_API
static int v9_video_sdk_device_clk(ST_SDKDeviceInfo* p_pstStatusInfo, void* p_pUserData)
{
	// 如有状态变更， sdk 则会调用该函数
	//v9_video_sdk_t *psdk = (v9_video_sdk_t *)p_pUserData;
	if(NULL == p_pstStatusInfo/* || NULL == p_pUserData*/)
		return EAIS_SDK_ERROR_NULL_POITER;

/*	if(p_pstStatusInfo->nDeviceType == EAIS_DEVICE_TYPE_SNAP)
		psdk->type = EAIS_DEVICE_TYPE_SNAP;
	else if(p_pstStatusInfo->nDeviceType == EAIS_DEVICE_TYPE_RECOGNIZE)
		psdk->type = EAIS_DEVICE_TYPE_RECOGNIZE;*/
	V9_SDK_DBGPRF( "v9_video_sdk_device_clk nDeviceType=%d szDeviceIP=%s", p_pstStatusInfo->nDeviceType, p_pstStatusInfo->szDeviceIP);
	//printf("v9_video_sdk_device_clk nDeviceType=%d szDeviceIP=%s\r\n", p_pstStatusInfo->nDeviceType, p_pstStatusInfo->szDeviceIP);
	//if(psdk->type)
	//{
		//zlog_warn(ZLOG_APP,"v9_video_sdk_device_clk find");
		//psdk->find = TRUE;
	//}
	//zlog_warn(ZLOG_APP," v9_video_sdk_device_clk ");
	if(strlen(p_pstStatusInfo->szDeviceIP))
	{
		int i =0;
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			//V9_SDK_DBGPRF("v9_video_sdk_device_clk [%d]address=%s szDeviceIP=%s", i,
			//				inet_address(v9_video_board[i].address), p_pstStatusInfo->szDeviceIP);
			if(v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].address != 0 &&
					(v9_video_board[i].address == ntohl(inet_addr(p_pstStatusInfo->szDeviceIP))))
			{
				V9_SDK_DBGPRF("v9_video_sdk_device_clk [%d]address=%s szDeviceIP=%s", i,
							  inet_address(v9_video_board[i].address), p_pstStatusInfo->szDeviceIP);
				//printf("v9_video_sdk_device_clk [%d]address=%s szDeviceIP=%s\r\n", i, v9_video_board[i].address, p_pstStatusInfo->szDeviceIP);

				if(!v9_video_board[i].sdk.find)
				{
					if(p_pstStatusInfo->nDeviceType == EAIS_DEVICE_TYPE_SNAP)
						v9_video_board[i].sdk.type = EAIS_DEVICE_TYPE_SNAP;
					else if(p_pstStatusInfo->nDeviceType == EAIS_DEVICE_TYPE_RECOGNIZE)
						v9_video_board[i].sdk.type = EAIS_DEVICE_TYPE_RECOGNIZE;
					v9_video_board[i].sdk.find = TRUE;

					v9_board_set_ready(&v9_video_board[i]);
					if(v9_video_board[i].sdk.master)
					{
						if(v9_video_board[i].sdk.t_timeout)
						{
							eloop_cancel(v9_video_board[i].sdk.t_timeout);
							v9_video_board[i].sdk.t_timeout = NULL;
						}
						v9_video_board[i].sdk.t_timeout = eloop_add_timer(v9_video_board[i].sdk.master,
																	v9_video_sdk_timeout, &v9_video_board[i].sdk, 1);
					}
					V9_SDK_DBGPRF("v9_video_sdk_device_clk find");
					return EAIS_SDK_SUCCESS;
				}
			}
		}
	}
	return EAIS_SDK_SUCCESS;
}

static int v9_video_sdk_status_clk(ST_SDKStatusInfo* p_pstStatusInfo, void* p_pUserData)
{
	v9_video_board_t *board = NULL;
	v9_video_stream_t *stream = NULL;
	v9_video_sdk_t *psdk = (v9_video_sdk_t *)p_pUserData;
	zassert(psdk);
	board = psdk->board;
	// 如有状态变更， sdk 则会调用该函数
	if(NULL == p_pstStatusInfo || NULL == p_pUserData)
		return EAIS_SDK_ERROR_NULL_POITER;
	/** 【业务层处理：状态参数解析】
	1） p_pstInfo->nLoginHandle： 可以得知是哪个 EAIS 设备状态变更.
	2） p_pstInfo->nChannlId： 可以得知哪个通道变更， 0 则表示 EAIS 设备本身
	3） p_pstInfo->nStatusBit： 0bit 为 1 则说明是 ESStatus， 支持多个位设置 1
	4） p_pstInfo->nESStatus： 可以得知 EAIS 设备当前状态.
	5） p_pstInfo->nRTSPStatus： 通道 URL 状态，此时 nChannlId 不为 0.
	6） p_pstInfo->nDecodeStatus： 通道解码状态，此时 nChannlId 不为 0.
	**/
	V9_SDK_DBGPRF("v9_video_sdk_status_clk nChannlId=%d", p_pstStatusInfo->nChannlId);
	if(p_pstStatusInfo->nChannlId > 0)
	{
		stream = v9_video_board_stream_lookup_by_id_and_ch(board->id, p_pstStatusInfo->nChannlId);
		if(stream)
		{
			//zlog_warn(ZLOG_APP, "v9_video_sdk_status_clk nChannlId=%d", p_pstStatusInfo->nChannlId);
			//stream->connect;				//视频流连接状态
			if(p_pstStatusInfo->nStatusBit & 0x01)
				stream->dev_status = p_pstStatusInfo->nESStatus;					// EAIS设备状态，默认离线。 ENUM_EAIS_DEVICE_STATUS
			if(p_pstStatusInfo->nStatusBit & 0x02)
				stream->rtsp_status = p_pstStatusInfo->nRTSPStatus;									// 通道RTSP状态 ENUM_EAIS_DEVICE_STATUS
			if(p_pstStatusInfo->nStatusBit & 0x04)
				stream->decode_status = p_pstStatusInfo->nDecodeStatus;									// 解码状态
			if(stream->rtsp_status == EAIS_DEVICE_STATUS_ONLINE || stream->decode_status)
				stream->connect = TRUE;
			//stream->change;
		}
	}
	V9_SDK_DBGPRF("v9_video_sdk_status_clk");
	return EAIS_SDK_SUCCESS;
}

static int v9_video_sdk_snap_clk(ST_SDKStructInfo* p_pstStatusInfo, void* p_pUserData)
{
	v9_video_sdk_t *psdk = (v9_video_sdk_t *)p_pUserData;
	// 如有状态变更， sdk 则会调用该函数
	zassert(psdk);
	if(NULL == p_pstStatusInfo || NULL == p_pUserData)
		return EAIS_SDK_ERROR_NULL_POITER;

	// 结构化数据类型
	V9_SDK_DBGPRF("v9_video_sdk_snap_clk nChannlId=%d", p_pstStatusInfo->nChannlId);
	psdk->datatype = p_pstStatusInfo->nDataType;

	// 方案类型
	if(p_pstStatusInfo->nSolutionType == EAIS_SOLUTION_TYPE_SNAP)
		psdk->mode = EAIS_DEVICE_TYPE_SNAP;
	else if(p_pstStatusInfo->nSolutionType == EAIS_SOLUTION_TYPE_HELMET)
		psdk->mode = EAIS_DEVICE_TYPE_RECOGNIZE;
	else if(p_pstStatusInfo->nSolutionType == EAIS_SOLUTION_TYPE_RECOGNITION)
		psdk->mode = EAIS_DEVICE_TYPE_RECOGNIZE;

	V9_SDK_DBGPRF("v9_video_sdk_snap_clk");
	return EAIS_SDK_SUCCESS;
}
#endif


static int v9_video_sdk_open(v9_video_sdk_t *sdk)
{
	zassert(sdk);
#ifdef V9_VIDEO_SDK_API
	int ret = 0;
#endif
	v9_video_board_t *board = sdk->board;
	v9_video_board_lock();
	if(!v9_board_ready(board))
	{
		v9_video_board_unlock();
		return ERROR;
	}
	v9_video_board_unlock();
	if(sdk->login != FALSE)
		return OK;
	V9_SDK_DBGPRF(" board ID=%d %s %d %s %s", V9_APP_BOARD_HW_ID(board->id), inet_address(board->address), board->port, sdk->username, sdk->password);

#ifdef V9_VIDEO_SDK_API

	if(!sdk->find)
	{
		V9_SDK_DBGPRF(" EAIS_SDK_StartDiscovery ");
		EAIS_SDK_StartDiscovery(v9_video_sdk_device_clk, NULL);

		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," can not find board ID=%d %s %d %s %s", V9_APP_BOARD_HW_ID(board->id), inet_address(board->address), board->port, sdk->username, sdk->password);
		return ERROR;
	}

	sdk->handle = EAIS_SDK_Login(inet_address(board->address), board->port, sdk->username, sdk->password, V9_VIDEO_SDK_TIMEOUT);
	if(sdk->handle < EAIS_SDK_SUCCESS)
	{
		v9_video_board_lock();
		sdk->handle = -1;
		sdk->login = FALSE;
		sdk->getstate = FALSE;
		sdk->status = FALSE;
		v9_video_board_active(board->id, FALSE);
		v9_video_board_unlock();
		if(V9_SDK_DEBUG(ERROR))
			zlog_err(ZLOG_APP, " EAIS SDK Login failed on Board(%d) %s:%d username=%s password=%s timeout=%d; ERROR(%s)",
					 V9_APP_BOARD_HW_ID(board->id), inet_address(board->address), board->port,
				 sdk->username, sdk->password, V9_VIDEO_SDK_TIMEOUT, v9_video_sdk_errnostr(sdk->handle));
		return ERROR;
	}
	printf("EAIS_SDK_Login OK");

	// 回调注册：设备状态/图片流
	EAIS_SDK_SetStatusCallBack(sdk->handle, v9_video_sdk_status_clk, sdk);
	EAIS_SDK_SetSnapCallBack(sdk->handle, v9_video_sdk_snap_clk, sdk);

	ret = EAIS_SDK_GetDeviceInfo(sdk->handle, sdk->device);
	if(EAIS_SDK_SUCCESS == ret)
	{
		v9_video_board_lock();
		v9_video_board_active(board->id, TRUE);
		sdk->login = TRUE;
		v9_video_board_unlock();
		V9_SDK_DBGPRF("EAIS_SDK_GetDeviceInfo OK");

		v9_video_sdk_open_snap_api(board->id, EAIS_DATA_TYPE_PDAF);
		//v9_video_sdk_ntp_api(board->id, "10.10.10.254", 60);
		//v9_video_sdk_timer_api(board->id);
		return OK;
	}
	if(V9_SDK_DEBUG(ERROR))
		zlog_err(ZLOG_APP,"EAIS_SDK_GetDeviceInfo Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
	EAIS_SDK_Logout(sdk->handle);
	v9_video_board_lock();
	sdk->login = FALSE;
	sdk->handle = -1;
	sdk->getstate = FALSE;
	sdk->status = FALSE;
	v9_video_board_active(board->id, FALSE);
	v9_video_board_unlock();
	return ERROR;
#else
	return OK;
#endif
}


static int v9_video_sdk_devstatus_get(v9_video_sdk_t *sdk)
{
	zassert(sdk);
#ifdef V9_VIDEO_SDK_API
	int ret = 0;
	v9_video_board_lock();
	if(sdk && sdk->login && sdk->handle >= 0 && sdk->device)
	{
		sdk->getstate = FALSE;
		//sdk->status = FALSE;
		ret = EAIS_SDK_GetDeviceInfo(sdk->handle, sdk->device);
		if(EAIS_SDK_SUCCESS == ret)
		{
			sdk->getstate = TRUE;
			sdk->status = TRUE;
			if(sdk->nfs == FALSE)
			{
				/*
				* address:10.10.10.254
				* dir:/mnt/diska1/board1/
				*/
				v9_video_board_t *board = (v9_video_board_t *)sdk->board;

				if(board && board->id != APP_BOARD_MAIN)
				{
					if(v9_video_sdk_nfsdir_api(board->id, "10.10.10.254", NULL, NULL, v9_video_disk_root_dir(board->id)) == OK)
						sdk->nfs = TRUE;
				}
			}
			v9_video_board_unlock();
			return OK;
		}
		sdk->status = FALSE;
		if(V9_SDK_DEBUG(ERROR))
			zlog_err(ZLOG_APP,"EAIS_SDK_GetDeviceInfo Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));

		if(ret == EAIS_SDK_ERROR_SOCKET_CONNECT_TIMEOUT)
		{
			v9_video_board_active(((v9_video_board_t *)sdk->board)->id, FALSE);
			EAIS_SDK_Logout(sdk->handle);
			sdk->login = FALSE;
			sdk->handle = -1;
			sdk->getstate = FALSE;
			sdk->status = FALSE;
		}
		else if(ret == EAIS_SDK_ERROR_SOCKET_CONNECT_FAILED)
		{
			//EAIS_SDK_Logout(sdk->handle);
			v9_video_board_active(((v9_video_board_t *)sdk->board)->id, FALSE);
			sdk->login = FALSE;
			sdk->handle = -1;
			sdk->getstate = FALSE;
			sdk->status = FALSE;
		}
		v9_video_board_unlock();
		return ERROR;
	}
	v9_video_board_unlock();
	if(V9_SDK_DEBUG(WARN))
		zlog_warn(ZLOG_APP," Video Board SDK not connect");
	return ERROR;
#else
	return OK;
#endif
}
/***************************************************************************/
/***************************************************************************/

static int v9_video_sdk_timeout(struct eloop *eloop)
{
	v9_video_sdk_t *sdk = NULL;
#ifdef V9_VIDEO_SDK_API
//	int ret = 0;
#endif
	sdk = ELOOP_ARG(eloop);
	if(!sdk)
	{
		if(V9_SDK_DEBUG(ERROR))
			zlog_err(ZLOG_APP,"v9_video_sdk_timeout sdk = NULL");
		return 0;
	}
	//v9_video_board_t *board = sdk->board;
	sdk->t_timeout = NULL;

	if(sdk->initialization)
	{
		if(!v9_video_sdk_isopen(sdk))
			v9_video_sdk_open(sdk);
		else
		{
			v9_video_sdk_devstatus_get(sdk);

		}
		sdk->t_timeout = eloop_add_timer(sdk->master,
				v9_video_sdk_timeout, sdk, sdk->interval);
	}
	return OK;
}

#ifdef V9_DEBUGING_TEST
static void v9_video_sdk_test()
{
	v9_video_board_t * board = NULL;
	if(!v9_video_board_isactive(APP_BOARD_CALCU_1))
	{
		board = v9_video_board_lookup(APP_BOARD_CALCU_1);
		if(board)
			v9_board_set_ready(board);
	}
	if(!v9_video_board_isactive(APP_BOARD_CALCU_2))
	{
		board = v9_video_board_lookup(APP_BOARD_CALCU_2);
		if(board)
			v9_board_set_ready(board);
	}
/*	if(!v9_video_board_isactive(APP_BOARD_CALCU_3))
	{
		board = v9_video_board_lookup(APP_BOARD_CALCU_3);
		if(board)
			v9_board_set_ready(board);
	}
	if(!v9_video_board_isactive(APP_BOARD_CALCU_4))
	{
		board = v9_video_board_lookup(APP_BOARD_CALCU_4);
		if(board)
			v9_board_set_ready(board);
	}*/
}
#endif

static int v9_video_sdk_task(void *argv)
{
	module_setup_task(PL_APPLICATION_MODULE_START + 1, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}

	v9_video_sdk_restart_all();
#ifdef V9_DEBUGING_TEST
	V9_SDK_DBGPRF("---------%s---call v9_board_set_ready----", __func__);
	v9_video_sdk_test();
#endif
	eloop_start_running(master_eloop[PL_APPLICATION_MODULE_START + 1], PL_APPLICATION_MODULE_START + 1);
	return OK;
}

int v9_video_sdk_task_init ()
{
/*
	zassert(v9_video_board != NULL);
	if(master_eloop[PL_APPLICATION_MODULE_START + 1] == NULL)
		v9_video_board->master = master_eloop[PL_APPLICATION_MODULE_START + 1] = eloop_master_module_create(PL_APPLICATION_MODULE_START + 1);
*/

	//V9_SDK_DBGPRF("---------%s---------", __func__);
	//v9_video_board->enable = TRUE;
	//v9_video_board->task_id =
			os_task_create("appSdk", OS_TASK_DEFAULT_PRIORITY,
	               0, v9_video_sdk_task, NULL, OS_TASK_DEFAULT_STACK * 2);
	//if(v9_video_board->task_id)
	//	return OK;
	return OK;
}




int v9_video_sdk_restart_all()
{
	int i = 0;
	if(!v9_video_board)
		return ERROR;
	if(master_eloop[PL_APPLICATION_MODULE_START + 1] == NULL)
		master_eloop[PL_APPLICATION_MODULE_START + 1] = eloop_master_module_create(PL_APPLICATION_MODULE_START + 1);
	v9_video_board_lock();
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		v9_video_board[i].sdk.master = master_eloop[PL_APPLICATION_MODULE_START + 1];
		if (v9_video_board[i].sdk.master && v9_video_board[i].id != APP_BOARD_MAIN)
		{
			if (v9_video_board[i].sdk.initialization && v9_video_board[i].sdk.interval)
			{
				if(v9_video_board[i].sdk.t_timeout)
				{
					eloop_cancel(v9_video_board[i].sdk.t_timeout);
					v9_video_board[i].sdk.t_timeout = NULL;
				}
				v9_video_board[i].sdk.t_timeout = eloop_add_timer(v9_video_board[i].sdk.master,
															v9_video_sdk_timeout, &v9_video_board[i].sdk, 3);
			}
		}
	}
	v9_video_board_unlock();
	return OK;
}

/*
int v9_video_sdk_start(u_int32 id)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(!v9_video_board)
		return ERROR;
	if(!sdk)
		return ERROR;
	if(!v9_video_sdk_isopen(sdk))
		return v9_video_sdk_open(sdk);
	else
		return OK;
	return ERROR;
}

int v9_video_sdk_stop(u_int32 id)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			EAIS_SDK_Logout(sdk->handle);
			v9_video_board_active(id, FALSE);
			sdk->login = FALSE;
			sdk->handle = -1;
			sdk->getstate = FALSE;
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}
*/


static int _v9_video_sdk_show(struct vty * vty, v9_video_sdk_t *sdk, int debug)
{
	zassert(sdk);
	if(sdk && sdk->device && sdk->getstate && sdk->board)
	{
#ifdef V9_VIDEO_SDK_API
		char datatype[64];
		memset(datatype, 0, sizeof(datatype));
		ST_SDKDeviceInfo *device = (ST_SDKDeviceInfo *)sdk->device;
		v9_video_board_t *board = (v9_video_board_t *)sdk->board;

		if(board->id == APP_BOARD_MAIN)
			vty_out (vty, "Main Board Status(ID:%d):%s", V9_APP_BOARD_HW_ID(board->id), VTY_NEWLINE);
		else
			vty_out (vty, "Board Status(ID:%d):%s", V9_APP_BOARD_HW_ID(board->id), VTY_NEWLINE);
		vty_out (vty, " Device Name         : %s%s", device->szDeviceName, VTY_NEWLINE);
		vty_out (vty, " Device ID           : %s%s", device->szDeviceId, VTY_NEWLINE);

		if(device->nDeviceType == EAIS_DEVICE_TYPE_UNKNOW)
			vty_out (vty, " Type                : UNKNOW%s", VTY_NEWLINE);// 未知类型
		else if(device->nDeviceType == EAIS_DEVICE_TYPE_SNAP)
			vty_out (vty, " Type                : SNAP%s", VTY_NEWLINE);// 抓拍类型EAIS设备
		else if(device->nDeviceType == EAIS_DEVICE_TYPE_RECOGNIZE)
			vty_out (vty, " Type                : RECOGNIZE%s", VTY_NEWLINE);// 识别类型EAIS设备
		else
			vty_out (vty, " Type                : UNKNOW%s", VTY_NEWLINE);

		if(sdk->mode == EAIS_SOLUTION_TYPE_SNAP)
			vty_out (vty, " Mode                : SNAP%s", VTY_NEWLINE);// 未知类型
		else if(sdk->mode == EAIS_SOLUTION_TYPE_HELMET)
			vty_out (vty, " Mode                : HELMET%s", VTY_NEWLINE);// 抓拍类型EAIS设备
		else if(sdk->mode == EAIS_SOLUTION_TYPE_RECOGNITION)
			vty_out (vty, " Mode                : RECOGNIZE%s", VTY_NEWLINE);// 识别类型EAIS设备
		else if(sdk->mode == EAIS_SOLUTION_TYPE_PEDESTRIAN)
			vty_out (vty, " Mode                : PEDESTRIAN%s", VTY_NEWLINE);// 行人方案
		else if(sdk->mode == EAIS_SOLUTION_TYPE_FIRE)
			vty_out (vty, " Mode                : FIRE%s", VTY_NEWLINE);// 火焰方案
		else if(sdk->mode == EAIS_SOLUTION_TYPE_PLATE)
			vty_out (vty, " Mode                : PLATE%s", VTY_NEWLINE);// 车牌方案
		else
			vty_out (vty, " Mode                : UNKNOW%s", VTY_NEWLINE);

		if(sdk->datatype == EAIS_DATA_TYPE_UNKNOW)
			strcat(datatype, "UNKNOW");// 未知类型
		else if(sdk->datatype & EAIS_DATA_TYPE_P)
			strcat(datatype, "P,");
		else if(sdk->datatype & EAIS_DATA_TYPE_D)
			strcat(datatype, "D,");
		else if(sdk->datatype & EAIS_DATA_TYPE_PD)
			strcat(datatype, "P+D,");
		else if(sdk->datatype & EAIS_DATA_TYPE_PDD)
			strcat(datatype, "P+D+D,");
		else if(sdk->datatype & EAIS_DATA_TYPE_PDA)
			strcat(datatype, "P+D+A,");
		else if(sdk->datatype & EAIS_DATA_TYPE_PDF)
			strcat(datatype, "P+D+F,");
		else if(sdk->datatype & EAIS_DATA_TYPE_PDAF)
			strcat(datatype, "P+D+A+F,");
		else if(sdk->datatype & EAIS_DATA_TYPE_CDD)
			strcat(datatype, "C+D+D");

		vty_out (vty, " DataType            : %s%s", datatype, VTY_NEWLINE);// 识别类型EAIS设备

		vty_out (vty, " Proto               : 0x%x%s", device->nDeviceProto, VTY_NEWLINE);// 设备协议，默认OAL协议
		vty_out (vty, " MaxChannelNum       : 0x%x%s", device->nMaxChannelNum, VTY_NEWLINE);// 设备最大通道数
		vty_out (vty, " NetCard             : %s-%d%s", device->szNetCardName, device->nNetCardId, VTY_NEWLINE);//  网卡名，如"eth0" 网卡Id
		vty_out (vty, " MAC                 : %02x:%02x:%02x:%02x:%02x:%02x%s", device->szMACAddr[0],
				device->szMACAddr[1], device->szMACAddr[2], device->szMACAddr[3], device->szMACAddr[4],
				device->szMACAddr[5], VTY_NEWLINE);// RTSP服务端口号

		vty_out (vty, " IP Address          : %s%s", device->szDeviceIP, VTY_NEWLINE);
		vty_out (vty, " Netmask             : %s%s", device->szSubnetMask, VTY_NEWLINE);
		vty_out (vty, " Gateway             : %s%s", device->szGatewayAddr, VTY_NEWLINE);
		vty_out (vty, " DNS                 : %s %s%s", device->szPreferredDns, device->szAlternateDns, VTY_NEWLINE);

		vty_out (vty, " Service Port        : %d%s", device->nDeviceSerPort, VTY_NEWLINE);// 设备服务端口
		vty_out (vty, " Update Port         : %d%s", device->nDeviceUpdatePort, VTY_NEWLINE);// 设备升级服务端口号
		vty_out (vty, " HTTP Port           : %d%s", device->nDeviceHttpPort, VTY_NEWLINE);// HTTP服务端口号
		vty_out (vty, " RTSP Port           : %d%s", device->nDeviceRTSPPort, VTY_NEWLINE);// RTSP服务端口号

		vty_out (vty, " Manufacturer        : %s%s", device->szManufacturerName, VTY_NEWLINE);// 厂商名称
		vty_out (vty, " Manufacturer ID     : %s%s", device->szManufacturerId, VTY_NEWLINE);// 设备型号
		vty_out (vty, " Product Model       : %s%s", device->szProductModel, VTY_NEWLINE);// 产品模组
		vty_out (vty, " SN                  : %s%s", device->szSN, VTY_NEWLINE);// SN

		vty_out (vty, " SoftWareInfo        : %s%s", device->szSoftWareInfo, VTY_NEWLINE);// 软件包信息
		vty_out (vty, " HardWareInfo        : %s%s", device->szHardWareInfo, VTY_NEWLINE);// 硬件信息
		vty_out (vty, " CoreSN              : %s%s", device->szCoreSN, VTY_NEWLINE);// 核心板SN号
		vty_out (vty, " RunTime             : %s%s", os_time_string(device->nDeviceRunTime), VTY_NEWLINE);// 设备运行时间

		vty_out (vty, " WorkMode            : %s%s", (device->nWorkMode)? "Aging Test":"Normal", VTY_NEWLINE);// 0: 正常模式 1：老化模式
		// 设备当前系统时间
		vty_out (vty, " SystemTime          : %d/%d/%d %d:%d:%d%s", device->stDeviceSystemTime.nYear,
				device->stDeviceSystemTime.nMonth, device->stDeviceSystemTime.nDay, device->stDeviceSystemTime.nHour,
				device->stDeviceSystemTime.nMinute, device->stDeviceSystemTime.nSecond, VTY_NEWLINE);// SN
#else
		vty_out (vty, "Board Status: Not SDK%s", VTY_NEWLINE);
#endif
		if(debug == TRUE)
		{
			vty_out (vty, " initialization      : %s%s", (sdk->initialization)? "TRUE":"FALSE", VTY_NEWLINE);
			vty_out (vty, " login               : %s%s", (sdk->login)? "TRUE":"FALSE", VTY_NEWLINE);
			vty_out (vty, " handle              : %d%s", sdk->handle, VTY_NEWLINE);
			vty_out (vty, " status              : %d%s", sdk->status, VTY_NEWLINE);
			vty_out (vty, " interval            : %d%s", sdk->interval, VTY_NEWLINE);
			vty_out (vty, " getstate            : %s%s", (sdk->getstate)? "TRUE":"FALSE", VTY_NEWLINE);
			vty_out (vty, " find                : %s%s", (sdk->find)? "TRUE":"FALSE", VTY_NEWLINE);
			vty_out (vty, " type                : %d%s", sdk->type, VTY_NEWLINE);
			vty_out (vty, " mode                : %d%s", sdk->mode, VTY_NEWLINE);
			vty_out (vty, " datatype            : %d%s", sdk->datatype, VTY_NEWLINE);
			//vty_out (vty, " datatype            : %d%s", sdk->datatype, VTY_NEWLINE);
		}
		return OK;
	}
	vty_out (vty, "Board Status(ID:%d): Not Connected%s", V9_SDK_ID(sdk), VTY_NEWLINE);
	return ERROR;
}

int v9_video_sdk_show(struct vty * vty, int id, int debug)
{
	if(v9_video_board)
	{
		if(id)
		{
			v9_video_board_lock();
			v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
			if(sdk)
				_v9_video_sdk_show(vty, sdk,  debug);
			v9_video_board_unlock();
		}
		else
		{
			int i = 0;
			v9_video_board_lock();
			for(i = 0; i < V9_APP_BOARD_MAX; i++)
			{
				if(v9_video_board[i].id != APP_BOARD_MAIN)
				_v9_video_sdk_show(vty, &v9_video_board[i].sdk,  debug);
			}
			v9_video_board_unlock();
		}
	}
	return OK;
}


/*******************************************************************************/
/*******************************************************************************/
int v9_video_sdk_reboot_api(u_int32 id)
{
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if(id <= APP_BOARD_MAIN)
	{
		int i =0, ret = 0;
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if(v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login && v9_video_board[i].sdk.handle >= 0)
			{
				ret |= EAIS_SDK_DeviceReboot(v9_video_board[i].sdk.handle);

				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_DeviceReboot  Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					v9_video_board_lock();
					v9_video_board_active(id, FALSE);
					v9_video_board[i].sdk.login = FALSE;
					v9_video_board[i].sdk.handle = -1;
					v9_video_board[i].sdk.getstate = FALSE;
					v9_video_board[i].sdk.status = FALSE;
					v9_video_board_unlock();
					return ERROR;
				}
			}
		}
		return OK;
	}
	else
	{
		int ret = 0;
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if(sdk && sdk->login && sdk->handle >= 0)
		{
			ret |= EAIS_SDK_DeviceReboot(sdk->handle);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_DeviceReboot Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				v9_video_board_lock();
				v9_video_board_active(id, FALSE);
				sdk->login = FALSE;
				sdk->handle = -1;
				sdk->getstate = FALSE;
				sdk->status = FALSE;
				v9_video_board_unlock();
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
	}
	return ERROR;
#else
	return OK;
#endif
}


int v9_video_sdk_reset_api(u_int32 id)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = OK;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	ST_SDKResetInfo p_pstResetInfo;
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ST_SDKDeviceInfo *device =
						(ST_SDKDeviceInfo *) v9_video_board[i].sdk.device;
				memset(&p_pstResetInfo, 0, sizeof(ST_SDKResetInfo));
				if (device)
					strcpy(p_pstResetInfo.szDeviceID, device->szDeviceId);

				ret |= EAIS_SDK_FactoryReset(v9_video_board[i].sdk.handle,
						&p_pstResetInfo);

				if (ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP, "EAIS_SDK_FactoryReset  Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					v9_video_board_lock();
					v9_video_board_active(id, FALSE);
					v9_video_board[i].sdk.login = FALSE;
					v9_video_board[i].sdk.handle = -1;
					v9_video_board[i].sdk.getstate = FALSE;
					v9_video_board[i].sdk.status = FALSE;
					v9_video_board_unlock();
					return ERROR;
				}
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKDeviceInfo *device = (ST_SDKDeviceInfo *) sdk->device;
			memset(&p_pstResetInfo, 0, sizeof(ST_SDKResetInfo));
			if (device)
				strcpy(p_pstResetInfo.szDeviceID, device->szDeviceId);
			ret |= EAIS_SDK_FactoryReset(sdk->handle, &p_pstResetInfo);
			if (ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_FactoryReset Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				v9_video_board_lock();
				v9_video_board_active(id, FALSE);
				sdk->login = FALSE;
				sdk->handle = -1;
				sdk->getstate = FALSE;
				sdk->status = FALSE;
				v9_video_board_unlock();
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}


//获取当前正常连接的视频路数
int v9_video_sdk_getvch_api(u_int32 id)
{
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
	if (!v9_video_board || id <= APP_BOARD_MAIN)
		return ERROR;

	v9_video_sdk_t *sdk = v9_video_sdk_lookup (id);
	if (v9_video_board && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		if (sdk->login && sdk->handle >= 0)
		{
			ST_SDKRTSPConfig p_pstRTSPConfig;
			memset (&p_pstRTSPConfig, 0, sizeof(ST_SDKRTSPConfig));
			ret = EAIS_SDK_GetRtspConfig (sdk->handle, &p_pstRTSPConfig);
			if (ret == EAIS_SDK_SUCCESS)
			{
				int i = 0, vch = 0;
				if (p_pstRTSPConfig.nRTSPInfoNum == 0)
					return 0;
				for (i = 0;
						(i < p_pstRTSPConfig.nRTSPInfoNum)
								&& (i < EAIS_SDK_MAX_CHANNEL_SUPPORT); i++)
				{
					if (p_pstRTSPConfig.stRTSPInfoArr[i].nRTSPStatus
							== EAIS_DEVICE_STATUS_ONLINE)
						vch++;
				}
				return vch;
			}
			else
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_GetRtspConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return ERROR;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
#else
		return ERROR;
#endif
	}
	return ERROR;
}

//把上层配置的视频URL下发到SDK
int v9_video_sdk_set_vch_api(u_int32 id, int cnum, void *p)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	v9_video_stream_t *stream = p;
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
	if(!stream)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Stream memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
					ST_SDKRTSPConfig stRTSPConfig;
					memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
					stRTSPConfig.nRTSPInfoNum = cnum;
					for(i = 0; i < cnum; i++)
					{
						if(strlen(stream->video_url)/*stream->video_url*/)
						{
							strcpy(stRTSPConfig.stRTSPInfoArr[i].szChanRtspUrl, stream->video_url);
						}
					}
					ret = EAIS_SDK_SetRtspConfig(v9_video_board[i].sdk.handle, &stRTSPConfig);

					if(ret != EAIS_SDK_SUCCESS)
					{
						if(V9_SDK_DEBUG(ERROR))
							zlog_err(ZLOG_APP,"EAIS_SDK_SetRtspConfig Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
						return ERROR;
					}
					return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKRTSPConfig stRTSPConfig;
			memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
			stRTSPConfig.nRTSPInfoNum = cnum;
			for(i = 0; i < cnum; i++)
			{
				if(strlen(stream->video_url)/*stream->video_url*/)
				{
					strcpy(stRTSPConfig.stRTSPInfoArr[i].szChanRtspUrl, stream->video_url);
				}
			}
			ret = EAIS_SDK_SetRtspConfig(sdk->handle, &stRTSPConfig);

			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetRtspConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_add_vch_api(u_int32 id, int ch, char *url)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ST_SDKRTSPConfig stRTSPConfig;
				memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
				stRTSPConfig.nRTSPInfoNum = 1;
				stRTSPConfig.stRTSPInfoArr[0].nChannleId = ch;
				strcpy(stRTSPConfig.stRTSPInfoArr[0].szChanRtspUrl, url);
				ret = EAIS_SDK_SetRtspConfig(v9_video_board[i].sdk.handle, &stRTSPConfig);

				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_SetRtspConfig  Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKRTSPConfig stRTSPConfig;
			memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
			stRTSPConfig.nRTSPInfoNum = 1;
			stRTSPConfig.stRTSPInfoArr[0].nChannleId = ch;
			strcpy(stRTSPConfig.stRTSPInfoArr[0].szChanRtspUrl, url);
			ret = EAIS_SDK_SetRtspConfig(sdk->handle, &stRTSPConfig);

			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetRtspConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_del_vch_api(u_int32 id, int ch)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ST_SDKRTSPConfig stRTSPConfig;
				memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
				if(ch == -1)
				{
					stRTSPConfig.nRTSPInfoNum = 9;
					stRTSPConfig.stRTSPInfoArr[0].nChannleId = 1;
					stRTSPConfig.stRTSPInfoArr[1].nChannleId = 2;
					stRTSPConfig.stRTSPInfoArr[2].nChannleId = 3;
					stRTSPConfig.stRTSPInfoArr[3].nChannleId = 4;
					stRTSPConfig.stRTSPInfoArr[4].nChannleId = 5;
					stRTSPConfig.stRTSPInfoArr[5].nChannleId = 6;
					stRTSPConfig.stRTSPInfoArr[6].nChannleId = 7;
					stRTSPConfig.stRTSPInfoArr[7].nChannleId = 8;
				}
				else
				{
					stRTSPConfig.nRTSPInfoNum = 1;
					stRTSPConfig.stRTSPInfoArr[0].nChannleId = ch;
				}
				ret = EAIS_SDK_SetRtspConfig(v9_video_board[i].sdk.handle, &stRTSPConfig);

				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_SetRtspConfig Video Board (%d) ERROR(%s)",
									 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKRTSPConfig stRTSPConfig;
			memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
			if(ch == -1)
			{
				stRTSPConfig.nRTSPInfoNum = 9;
				stRTSPConfig.stRTSPInfoArr[0].nChannleId = 1;
				stRTSPConfig.stRTSPInfoArr[1].nChannleId = 2;
				stRTSPConfig.stRTSPInfoArr[2].nChannleId = 3;
				stRTSPConfig.stRTSPInfoArr[3].nChannleId = 4;
				stRTSPConfig.stRTSPInfoArr[4].nChannleId = 5;
				stRTSPConfig.stRTSPInfoArr[5].nChannleId = 6;
				stRTSPConfig.stRTSPInfoArr[6].nChannleId = 7;
				stRTSPConfig.stRTSPInfoArr[7].nChannleId = 8;
			}
			else
			{
				stRTSPConfig.nRTSPInfoNum = 1;
				stRTSPConfig.stRTSPInfoArr[0].nChannleId = ch;
			}
			ret = EAIS_SDK_SetRtspConfig(sdk->handle, &stRTSPConfig);

			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetRtspConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}
#ifdef V9_VIDEO_SDK_API
static int v9_video_sdk_lookup_vch_hw(u_int32 id, v9_video_sdk_t *sdk, int ch, ST_SDKRTSPConfig *pstRTSPConfig)
{
	int ret = 0, i = 0;
	if (sdk && sdk->login && sdk->handle >= 0)
	{
		ST_SDKRTSPConfig stRTSPConfig;
		memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
		ret = EAIS_SDK_GetRtspConfig(sdk->handle, &stRTSPConfig);
		if(ret != EAIS_SDK_SUCCESS)
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP,"EAIS_SDK_GetRtspConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
			return ERROR;
		}
		if(pstRTSPConfig)
		{
			memcpy(pstRTSPConfig, &stRTSPConfig, sizeof(ST_SDKRTSPConfig));
			return OK;
		}
		for(i = 0; i < stRTSPConfig.nRTSPInfoNum; i++)
		{
			if(stRTSPConfig.stRTSPInfoArr[i].nChannleId == ch && strlen(stRTSPConfig.stRTSPInfoArr[i].szChanRtspUrl))										// 通道区分
				return OK;
		}
		return ERROR;
	}
	if(V9_SDK_DEBUG(WARN))
		zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}
#endif

int v9_video_sdk_get_vch_api(u_int32 id, int ch, void *p)
{
#ifdef V9_VIDEO_SDK_API
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}

#ifdef V9_VIDEO_SDK_API
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if(sdk)
		{
			ST_SDKRTSPConfig stRTSPConfig;
			memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
			ret = v9_video_sdk_lookup_vch_hw(id, sdk, ch, &stRTSPConfig);
			return OK;
		}

	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_lookup_vch_api(u_int32 id, int ch)
{
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if(sdk)
			return v9_video_sdk_lookup_vch_hw(id, sdk, ch, NULL);
	}
	return ERROR;
#else
	return OK;
#endif
}


int v9_video_sdk_get_rtsp_status_api(u_int32 id)
{
#ifdef V9_VIDEO_SDK_API
	int ret = 0, i = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}

#ifdef V9_VIDEO_SDK_API
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if(sdk)
		{
			ST_SDKRTSPConfig stRTSPConfig;
			memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
			ret = v9_video_sdk_lookup_vch_hw(id, sdk, 0, &stRTSPConfig);

			for(i = 0; i < stRTSPConfig.nRTSPInfoNum; i++)
			{
				if(strlen(stRTSPConfig.stRTSPInfoArr[i].szChanRtspUrl))										// 通道区分
				{
					if(V9_SDK_DEBUG(EVENT))
						zlog_debug(ZLOG_APP,"Video Board (%d) Get Channel (%d) RTSPStatus=%d decodeStatus=%d", id,
							   stRTSPConfig.stRTSPInfoArr[i].nChannleId,
							   stRTSPConfig.stRTSPInfoArr[i].nRTSPStatus,
							   stRTSPConfig.stRTSPInfoArr[i].nDecodeStatus);

					v9_video_board_stream_status_change( id,
								stRTSPConfig.stRTSPInfoArr[i].nChannleId,
								stRTSPConfig.stRTSPInfoArr[i].nRTSPStatus,
								stRTSPConfig.stRTSPInfoArr[i].nDecodeStatus);
				}
			}
			//zlog_debug(ZLOG_APP," Video Board loading RTSP Status OK");
			return OK;
		}
	}
	return ERROR;
#else
	return OK;
#endif
}


int v9_video_sdk_open_snap_api(u_int32 id, int type)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_OpenSnapStream(v9_video_board[i].sdk.handle, type);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_OpenSnapStream Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ret = EAIS_SDK_OpenSnapStream(sdk->handle, type);

			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_OpenSnapStream Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_close_snap_api(u_int32 id)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_CloseSnapStream(v9_video_board[i].sdk.handle);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_CloseSnapStream Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ret = EAIS_SDK_CloseSnapStream(sdk->handle);

			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_CloseSnapStream Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_set_snap_dir_api(u_int32 id, BOOL http, char *address, int port,
								  char *user, char *pass, char *dir)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		ST_SDKSnapServerAddr ServerAddr;
		memset(&ServerAddr, 0, sizeof(ST_SDKSnapServerAddr));
		ServerAddr.nEnable = address ? 1:0;										// 0：不使能，1：使能
		if(address)
		{
			strcpy(ServerAddr.szServerUrl, address);			// Ftp、Http服务URL或IP
			ServerAddr.nDevicePort = port;									// 端口
			if(user)
				strcpy(ServerAddr.szUserName, user);									// 用户名
			if(pass)
				strcpy(ServerAddr.szPassWord, pass);									// 密码
			if(http)
				ServerAddr.nProtocol = 1;										// 当协议为http时有效
			if(dir)
				strcpy(ServerAddr.szFtpFilePath, dir);			// Ftp文件上传路径
		}
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_SetSnapServerAddr(v9_video_board[i].sdk.handle, http, &ServerAddr);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_SetSnapServerAddr Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKSnapServerAddr ServerAddr;
			memset(&ServerAddr, 0, sizeof(ST_SDKSnapServerAddr));
			ServerAddr.nEnable = address ? 1:0;										// 0：不使能，1：使能
			if(address)
			{
				strcpy(ServerAddr.szServerUrl, address);			// Ftp、Http服务URL或IP
				ServerAddr.nDevicePort = port;									// 端口
				if(user)
					strcpy(ServerAddr.szUserName, user);									// 用户名
				if(pass)
					strcpy(ServerAddr.szPassWord, pass);									// 密码
				if(http)
					ServerAddr.nProtocol = 1;										// 当协议为http时有效
				if(dir)
					strcpy(ServerAddr.szFtpFilePath, dir);			// Ftp文件上传路径
			}
			ret = EAIS_SDK_SetSnapServerAddr(sdk->handle, http, &ServerAddr);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetSnapServerAddr Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_nfsdir_api(u_int32 id, char *address, char *user, char *pass, char *dir)
{
#ifdef V9_VIDEO_SDK_API
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if (sdk && sdk->login && sdk->handle >= 0)
	{
		ST_SDKSnapServerAddr ServerAddr;
		memset(&ServerAddr, 0, sizeof(ST_SDKSnapServerAddr));
		ret = EAIS_SDK_GetSnapServerAddr(sdk->handle, 2, &ServerAddr);
		if(ret != EAIS_SDK_SUCCESS)
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP,"EAIS_SDK_GetSnapServerAddr Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
			return ERROR;
		}
		//已经配置
		if(ServerAddr.nEnable &&
				strlen(ServerAddr.szServerUrl) &&
				strlen(ServerAddr.szFtpFilePath))
		{
			if(address)
			{
				//配置参数和原参数一致
				if(strcmp(ServerAddr.szServerUrl, address) == 0 && strcmp(ServerAddr.szFtpFilePath + 1, dir) == 0)
				{
					zlog_warn(ZLOG_APP, "EAIS_SDK_SetSnapServerAddr Same NFS URL and Path(Do not need setup)");
					return OK;
				}
			}
/*			else
				return OK;*/
		}
		memset(&ServerAddr, 0, sizeof(ST_SDKSnapServerAddr));
		ServerAddr.nEnable = address ? 1:0;										// 0：不使能，1：使能
		if(address)
		{
			strcpy(ServerAddr.szServerUrl, address);			// Ftp、Http服务URL或IP
			if(user)
				strcpy(ServerAddr.szUserName, user);									// 用户名
			if(pass)
				strcpy(ServerAddr.szPassWord, pass);									// 密码
			//if(http)
			//	ServerAddr.nProtocol = 1;										// 当协议为http时有效
			if(dir)
			{
				strcpy(ServerAddr.szFtpFilePath, ":");
				strcat(ServerAddr.szFtpFilePath, dir);			// Ftp文件上传路径(:/home/ccheng)
			}
		}
		ret = EAIS_SDK_SetSnapServerAddr(sdk->handle, 2, &ServerAddr);
		if(ret != EAIS_SDK_SUCCESS)
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP,"EAIS_SDK_SetSnapServerAddr Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
			return ERROR;
		}
		return OK;
	}
	if(V9_SDK_DEBUG(WARN))
		zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
	return ERROR;
#else
	return OK;
#endif
}


// 获取/设置人脸识别配置参数
int v9_video_sdk_recognize_config_set_api(u_int32 id, int nOutSimilarity,
									  int nRegisterQuality, BOOL nOpenUpload)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		ST_SDKRecognizeConfig stRecognizeConfig;
		memset(&stRecognizeConfig, 0, sizeof(ST_SDKRecognizeConfig));

		stRecognizeConfig.nOutSimilarity = nOutSimilarity;
		stRecognizeConfig.nRegisterQuality = nRegisterQuality;
		stRecognizeConfig.nOpenUpload = nOpenUpload;

		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_SetRecognizeCofig(v9_video_board[i].sdk.handle, &stRecognizeConfig);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_SetRecognizeCofig Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKRecognizeConfig stRecognizeConfig;
			memset(&stRecognizeConfig, 0, sizeof(ST_SDKRecognizeConfig));

			stRecognizeConfig.nOutSimilarity = nOutSimilarity;
			stRecognizeConfig.nRegisterQuality = nRegisterQuality;
			stRecognizeConfig.nOpenUpload = nOpenUpload;
			ret = EAIS_SDK_SetRecognizeCofig(sdk->handle, &stRecognizeConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetRecognizeCofig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_recognize_config_get_api(u_int32 id, int *nOutSimilarity,
										  int *nRegisterQuality, BOOL *nOpenUpload)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->login && sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKRecognizeConfig stRecognizeConfig;
			memset(&stRecognizeConfig, 0, sizeof(ST_SDKRecognizeConfig));

			ret = EAIS_SDK_GetRecognizeCofig(sdk->handle, &stRecognizeConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_GetRecognizeCofig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			if(nOutSimilarity)
				*nOutSimilarity = stRecognizeConfig.nOutSimilarity;
			if(nRegisterQuality)
				*nRegisterQuality = stRecognizeConfig.nRegisterQuality;
			if(nOpenUpload)
				*nOpenUpload = stRecognizeConfig.nOpenUpload;
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
#else
		return OK;
#endif
	}
	return ERROR;
}

// 安全帽配置
int v9_video_sdk_helmet_config_set_api(u_int32 id, u_int32 ch, void *data)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		ST_SDKHelmetConfig stHelmetConfig;
		memset(&stHelmetConfig, 0, sizeof(ST_SDKHelmetConfig));

		stHelmetConfig.nHelmetInfoNum = 1;
		memcpy(&stHelmetConfig.stHelmetInfoArr[0], data, sizeof(ST_SDKHelmetInfo));

		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_SetHelmetConfig(v9_video_board[i].sdk.handle, &stHelmetConfig);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_SetHelmetConfig Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKHelmetConfig stHelmetConfig;
			memset(&stHelmetConfig, 0, sizeof(ST_SDKHelmetConfig));

			stHelmetConfig.nHelmetInfoNum = 1;
			memcpy(&stHelmetConfig.stHelmetInfoArr[0], data, sizeof(ST_SDKHelmetInfo));

			ret = EAIS_SDK_SetHelmetConfig(sdk->handle, &stHelmetConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetHelmetConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}


int v9_video_sdk_helmet_config_get_api(u_int32 id, u_int32 ch, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk && data)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->login && sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKHelmetConfig stHelmetConfig;
			memset(&stHelmetConfig, 0, sizeof(ST_SDKHelmetConfig));

			ret = EAIS_SDK_GetHelmetConfig(sdk->handle, &stHelmetConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_GetHelmetConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			for(ret = 0; ret < stHelmetConfig.nHelmetInfoNum; ret++)
			{
				if(stHelmetConfig.stHelmetInfoArr[ret].nChannelId == ch)
				{
					memcpy(data, &stHelmetConfig.stHelmetInfoArr[ret], sizeof(ST_SDKHelmetInfo));
					return OK;
				}
			}
			if(V9_SDK_DEBUG(WARN))
				zlog_err(ZLOG_APP," Video Board (%d) Can not get helmet config of channel %d", V9_APP_BOARD_HW_ID(id),ch);
			return ERROR;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
#else
		return OK;
#endif
	}
	return ERROR;
}

// 抓拍策略配置
int v9_video_sdk_snap_config_set_api(u_int32 id, u_int32 ch, void *data)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		ST_SDKSnapConfig pstSnapConfig;
		memset(&pstSnapConfig, 0, sizeof(ST_SDKSnapConfig));

		pstSnapConfig.nSnapInfoNum = 1;

		memcpy(&pstSnapConfig.stSnapInfoArr[0], data, sizeof(ST_SDKSnapInfo));
		pstSnapConfig.stSnapInfoArr[0].nChannelId = ch;
		pstSnapConfig.stSnapInfoArr[0].nAttrEnable = 1;
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_SetSnapConfig(v9_video_board[i].sdk.handle, &pstSnapConfig);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_SetSnapConfig Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKSnapConfig pstSnapConfig;
			memset(&pstSnapConfig, 0, sizeof(ST_SDKSnapConfig));

			pstSnapConfig.nSnapInfoNum = 1;

			memcpy(&pstSnapConfig.stSnapInfoArr[0], data, sizeof(ST_SDKSnapInfo));
			pstSnapConfig.stSnapInfoArr[0].nChannelId = ch;

			ret = EAIS_SDK_SetSnapConfig(sdk->handle, &pstSnapConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetSnapConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}


int v9_video_sdk_snap_config_get_api(u_int32 id, u_int32 ch, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk && data)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->login && sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKSnapConfig pstSnapConfig;
			memset(&pstSnapConfig, 0, sizeof(ST_SDKSnapConfig));
			ret = EAIS_SDK_GetSnapConfig(sdk->handle, &pstSnapConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_GetSnapConfig ERROR(%s)", v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			for(ret = 0; ret < pstSnapConfig.nSnapInfoNum; ret++)
			{
				if(pstSnapConfig.stSnapInfoArr[ret].nChannelId == ch)
				{
					memcpy(data, &pstSnapConfig.stSnapInfoArr[ret], sizeof(ST_SDKSnapInfo));
					return OK;
				}
			}
			if(V9_SDK_DEBUG(WARN))
				zlog_err(ZLOG_APP," Video Board (%d) Can not get snap config of channel %d", V9_APP_BOARD_HW_ID(id),ch);
			return ERROR;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
#else
		return OK;
#endif
	}
	return ERROR;
}

//原图输出
int v9_video_sdk_original_pic_enable_set_api(u_int32 id, BOOL enable)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		ST_SDKSnapConfig stSnapConfig;
		memset(&stSnapConfig, 0, sizeof(ST_SDKSnapConfig));

		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_GetSnapConfig(v9_video_board[i].sdk.handle, &stSnapConfig);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_GetSnapConfig Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				stSnapConfig.nOriginalPicEnable = enable;
				ret = EAIS_SDK_SetSnapConfig(v9_video_board[i].sdk.handle, &stSnapConfig);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_SetSnapConfig Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKSnapConfig stSnapConfig;
			memset(&stSnapConfig, 0, sizeof(ST_SDKSnapConfig));
			ret = EAIS_SDK_GetSnapConfig(sdk->handle, &stSnapConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_GetSnapConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			stSnapConfig.nOriginalPicEnable = enable;
			ret = EAIS_SDK_SetSnapConfig(sdk->handle, &stSnapConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetSnapConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_original_pic_enable_get_api(u_int32 id, BOOL *enable)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->login && sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKSnapConfig stSnapConfig;
			memset(&stSnapConfig, 0, sizeof(ST_SDKSnapConfig));
			ret = EAIS_SDK_GetSnapConfig(sdk->handle, &stSnapConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_GetSnapConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			if(enable)
				*enable = stSnapConfig.nOriginalPicEnable;
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
#else
		return OK;
#endif
	}
	return ERROR;
}



// 告警信息配置
int v9_video_sdk_alarm_config_set_api(u_int32 id, u_int32 ch, void *data)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		ST_SDKAlarmConfig pstAlarmConfig;
		memset(&pstAlarmConfig, 0, sizeof(ST_SDKAlarmConfig));

		pstAlarmConfig.nAlarmParamNum = 1;

		memcpy(&pstAlarmConfig.stAlarmParamArr[0], data, sizeof(ST_SDKAlarmParam));
		pstAlarmConfig.stAlarmParamArr[0].nChannelId = ch;

		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_SetAlarmConfig(v9_video_board[i].sdk.handle, &pstAlarmConfig);
				if(ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP,"EAIS_SDK_SetAlarmConfig Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
		}
		return OK;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ST_SDKAlarmConfig pstAlarmConfig;
			memset(&pstAlarmConfig, 0, sizeof(ST_SDKAlarmConfig));

			pstAlarmConfig.nAlarmParamNum = 1;

			memcpy(&pstAlarmConfig.stAlarmParamArr[0], data, sizeof(ST_SDKAlarmParam));
			pstAlarmConfig.stAlarmParamArr[0].nChannelId = ch;

			ret = EAIS_SDK_SetAlarmConfig(sdk->handle, &pstAlarmConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_SetAlarmConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_alarm_config_get_api(u_int32 id, s_int32 ch, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk && data)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->login && sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKAlarmConfig pstAlarmConfig;
			memset(&pstAlarmConfig, 0, sizeof(ST_SDKAlarmConfig));
			ret = EAIS_SDK_GetAlarmConfig(sdk->handle, ch, &pstAlarmConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_GetAlarmConfig ERROR(%s)", v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			if(ch == -1)
			{
				if(data)
					memcpy(data, &pstAlarmConfig.stAlarmParamArr[ret], sizeof(ST_SDKAlarmParam));
				return OK;
			}
			if(data)
			{
				for(ret = 0; ret < pstAlarmConfig.nAlarmParamNum; ret++)
				{
					if(pstAlarmConfig.stAlarmParamArr[ret].nChannelId == ch)
					{
						memcpy(data, &pstAlarmConfig.stAlarmParamArr[ret], sizeof(ST_SDKAlarmParam));
						return OK;
					}
				}
			}
			if(V9_SDK_DEBUG(WARN))
				zlog_err(ZLOG_APP," Video Board (%d) Can not get alarm config of channel %d", V9_APP_BOARD_HW_ID(id),ch);
			return ERROR;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
#else
		return OK;
#endif
	}
	return ERROR;
}


int v9_video_sdk_update_api(u_int32 id, char *filename)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk/* && sdk->login && sdk->handle >= 0*/)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		int nDataLen = 0;
		char* pszPacketData = NULL;
		v9_video_board_t *vboard = sdk->board;
		nDataLen = os_file_size(filename);
		if(nDataLen != ERROR && nDataLen > 0 && vboard)
		{
			pszPacketData = XMALLOC(MTYPE_VIDEO_TMP, nDataLen + 1);
			if(!pszPacketData)
				return ERROR;
			memset(pszPacketData, 0, nDataLen + 1);
			ret = os_read_file(filename, pszPacketData, nDataLen);
			if(ret != OK)
			{
				XFREE(MTYPE_VIDEO_TMP, pszPacketData);
				return ERROR;
			}
			ret = EAIS_SDK_Update(inet_address(vboard->address), V9_VIDEO_SDK_UPDATE_PORT,
								  sdk->username, sdk->password, pszPacketData, nDataLen);

			XFREE(MTYPE_VIDEO_TMP, pszPacketData);

			if(ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_Update Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		return ERROR;
#else
		return OK;
#endif
	}
	return ERROR;
}

/*
 * 查询抓拍统计
 */
int v9_video_sdk_query_api(u_int32 id, u_int32 ch, u_int32 nStartTime, u_int32 nEndTime, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk && data && sdk->login && sdk->handle >= 0)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;

		ST_SDKQueryPeopleReq stQueryPeopleReq;
		ST_SDKPeopleCount stPeopleCount;
		stQueryPeopleReq.nChannelId = ch;
		stQueryPeopleReq.nEndTime = nStartTime;
		stQueryPeopleReq.nEndTime = nEndTime;
		memset(&stPeopleCount, 0, sizeof(stPeopleCount));
		ret = EAIS_SDK_QueryPeopleCount(sdk->handle, &stQueryPeopleReq, &stPeopleCount);
		if(ret != EAIS_SDK_SUCCESS)
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP,"EAIS_SDK_QueryPeopleCount Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
			return ERROR;
		}
		memcpy(data, &stPeopleCount, sizeof(ST_SDKPeopleCount));
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}






// 抓拍识别方案
// 用户增删改查
// 修改(新增)用户 p_bNewUser false:修改用户 true:新用户
int v9_video_sdk_add_user_api(u_int32 id, BOOL gender, int group, char *user, char *ID, char *pic, char *text, BOOL add)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		int nDataLen = 0;
		ST_SDKUserInfo pstUserInfo;
		memset(&pstUserInfo, 0, sizeof(ST_SDKUserInfo));
		nDataLen = os_file_size(pic);
		if (nDataLen != ERROR && nDataLen > 0)
		{
			pstUserInfo.szPictureData = XMALLOC(MTYPE_VIDEO_PIC, nDataLen + 1);
			if (!pstUserInfo.szPictureData)
				return ERROR;
			memset(pstUserInfo.szPictureData, 0, nDataLen + 1);
			ret = os_read_file(pic, pstUserInfo.szPictureData, nDataLen);
			if (ret != OK)
			{
				XFREE(MTYPE_VIDEO_PIC, pstUserInfo.szPictureData);
				return ERROR;
			}
			pstUserInfo.nGroupID = group;				// 所属组ID  0： 黑名单 1： 白名单
			strcpy(pstUserInfo.szUserName, user);			// 姓名
			strcpy(pstUserInfo.szUserID, ID);				// 证件号
			pstUserInfo.nGender = gender;					// 人员性别  0： 女 1： 男
			pstUserInfo.nFaceLen = nDataLen;	// 人脸图片数据长度（1, 1024 * 1024]字节
			//pstUserInfo.szPictureData;									// 图片内容
			if(text)
				strcpy(pstUserInfo.szComment, text);		// 用户自定义备注字段
			//pstUserInfo.nRegisterTime;									// 注册时间，从1970-01-01 00:00:00 (utc) 开始计时的秒数
			//pstUserInfo.szReserved[256];								// 预留位，便于拓展，默认置空
			for (i = 0; i < V9_APP_BOARD_MAX; i++)
			{
				if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
						&& v9_video_board[i].sdk.handle >= 0)
				{
					ret = EAIS_SDK_EditUser(v9_video_board[i].sdk.handle,
							&pstUserInfo, add ? TRUE : FALSE);
					if (ret != EAIS_SDK_SUCCESS)
					{
						XFREE(MTYPE_VIDEO_PIC, pstUserInfo.szPictureData);
						if(V9_SDK_DEBUG(ERROR))
							zlog_err(ZLOG_APP, "EAIS_SDK_EditUser Video Board (%d) ERROR(%s)",
									 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
						return ERROR;
					}

				}
			}
			XFREE(MTYPE_VIDEO_PIC, pstUserInfo.szPictureData);
			return OK;
		}
		if(V9_SDK_DEBUG(ERROR))
			zlog_err(ZLOG_APP," Can not open picfile(%s)", pic);
		return ERROR;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			int nDataLen = 0;
			ST_SDKUserInfo pstUserInfo;
			memset(&pstUserInfo, 0, sizeof(ST_SDKUserInfo));
			nDataLen = os_file_size(pic);
			if (nDataLen != ERROR && nDataLen > 0)
			{
				pstUserInfo.szPictureData = XMALLOC(MTYPE_VIDEO_PIC, nDataLen + 1);
				if (!pstUserInfo.szPictureData)
					return ERROR;
				memset(pstUserInfo.szPictureData, 0, nDataLen + 1);
				ret = os_read_file(pic, pstUserInfo.szPictureData, nDataLen);
				if (ret != OK)
				{
					XFREE(MTYPE_VIDEO_PIC, pstUserInfo.szPictureData);
					return ERROR;
				}

				pstUserInfo.nGroupID = group;			// 所属组ID  0： 黑名单 1： 白名单
				strcpy(pstUserInfo.szUserName, user);			// 姓名
				strcpy(pstUserInfo.szUserID, ID);				// 证件号
				pstUserInfo.nGender = gender;				// 人员性别  0： 女 1： 男
				pstUserInfo.nFaceLen = nDataLen;// 人脸图片数据长度（1, 1024 * 1024]字节
				//pstUserInfo.szPictureData;									// 图片内容
				if(text)
					strcpy(pstUserInfo.szComment, text);		// 用户自定义备注字段
				//pstUserInfo.nRegisterTime;									// 注册时间，从1970-01-01 00:00:00 (utc) 开始计时的秒数
				//pstUserInfo.szReserved[256];								// 预留位，便于拓展，默认置空

				ret = EAIS_SDK_EditUser(sdk->handle,
										&pstUserInfo, add ? TRUE : FALSE);

				XFREE(MTYPE_VIDEO_PIC, pstUserInfo.szPictureData);

				if (ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP, "EAIS_SDK_EditUser Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
					return ERROR;
				}
				return OK;
			}
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP," Can not open picfile(%s)", pic);
			return ERROR;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

// 删除用户
// 根据用户ID删除用户
int v9_video_sdk_del_user_api(u_int32 id, int group, char *ID)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if (!v9_video_board || !ID)
		return ERROR;
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_DelUser(v9_video_board[i].sdk.handle, ID);
				if (ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP, "EAIS_SDK_DelUser Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
			}
			return OK;
		}
		return ERROR;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ret = EAIS_SDK_DelUser(sdk->handle, ID);
			if (ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_DelUser Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}


// 批量删除某个库的用户
int v9_video_sdk_del_group_user_api(u_int32 id, void *p_pstUserList)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_DelGroupUserList(v9_video_board[i].sdk.handle, p_pstUserList);
				if (ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP, "EAIS_SDK_DelGroupUserList Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
			}
			return OK;
		}
		return ERROR;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ret = EAIS_SDK_DelGroupUserList(sdk->handle, p_pstUserList);
			if (ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_DelGroupUserList Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}


/*
 * 添加某个分组
 */
int v9_video_sdk_add_group_api(u_int32 id, int group, char *name)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
	ST_SDKGroupInfoList GroupInfo;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	memset(&GroupInfo, 0, sizeof(GroupInfo));
	GroupInfo.nGroupNum = 1;
	GroupInfo.stGroupInfoArr[0].nGroupId = group;
	strcpy(GroupInfo.stGroupInfoArr[0].szGroupName, name);
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_SetGroupNameList(v9_video_board[i].sdk.handle, &GroupInfo);
				if (ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP, "EAIS_SDK_SetGroupNameList Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
			}
			return OK;
		}
		return ERROR;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ret = EAIS_SDK_SetGroupNameList(sdk->handle, &GroupInfo);
			if (ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_SetGroupNameList Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}
/*
 * 删除某个分组
 */
// 删除库 p_nGroupID : -1 删除所有分组人脸库信息， 0 删除黑名单所有人脸信息，1 删除白名单所有人脸信息
int v9_video_sdk_del_group_api(u_int32 id, int group)
{
#ifdef V9_VIDEO_SDK_API
	int i = 0;
	int ret = 0;
#endif
	if(!v9_video_board)
	{
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board memory exceptions");
		return ERROR;
	}
#ifdef V9_VIDEO_SDK_API
	if (id <= APP_BOARD_MAIN)
	{
		for (i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if (v9_video_board[i].id != APP_BOARD_MAIN && v9_video_board[i].sdk.login
					&& v9_video_board[i].sdk.handle >= 0)
			{
				ret = EAIS_SDK_DelGroup(v9_video_board[i].sdk.handle, group);
				if (ret != EAIS_SDK_SUCCESS)
				{
					if(V9_SDK_DEBUG(ERROR))
						zlog_err(ZLOG_APP, "EAIS_SDK_DelGroup Video Board (%d) ERROR(%s)",
							 V9_APP_BOARD_HW_ID(v9_video_board[i].id), v9_video_sdk_errnostr(ret));
					return ERROR;
				}
			}
			return OK;
		}
		return ERROR;
	}
	else
	{
		v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
		if (sdk && sdk->login && sdk->handle >= 0)
		{
			ret = EAIS_SDK_DelGroup(sdk->handle, group);
			if (ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_DelGroup Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			return OK;
		}
		if(V9_SDK_DEBUG(WARN))
			zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
		return ERROR;
	}
	return ERROR;
#else
	return OK;
#endif
}

int v9_video_sdk_get_user_api(u_int32 id, char* ID, void* UserInfo)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk && sdk->login && sdk->handle >= 0)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		ret = EAIS_SDK_GetUser(sdk->handle, ID, UserInfo);
		if(ret != EAIS_SDK_SUCCESS)
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP,"EAIS_SDK_GetUser Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
			return ERROR;
		}
		return OK;
#else
		return OK;
#endif
	}
	if(V9_SDK_DEBUG(WARN))
		zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

/*
 * 获取图片特征值
 */
int v9_video_sdk_get_keyvalue_api(u_int32 id, char * pic, void* UserInfo)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk && sdk->login && sdk->handle >= 0)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		int nDataLen = 0;
		ST_SDKPictureInfo PictureInfo;
		ST_SDKSnapFea SDKSnapFea;
		sql_snapfea_key *key = UserInfo;
		memset(&PictureInfo, 0, sizeof(ST_SDKPictureInfo));
		memset(&SDKSnapFea, 0, sizeof(ST_SDKSnapFea));
		//key->feature_len = 0;
		nDataLen = os_file_size(pic);
		if (nDataLen != ERROR && nDataLen > 0)
		{
			PictureInfo.szPictureData = XMALLOC(MTYPE_VIDEO_PIC, nDataLen + 1);
			if (!PictureInfo.szPictureData)
				return ERROR;
			memset(PictureInfo.szPictureData, 0, nDataLen + 1);
			ret = os_read_file(pic, PictureInfo.szPictureData, nDataLen);
			if (ret != OK)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP," Video Board (%d) can not open file size (%s)", V9_APP_BOARD_HW_ID(id), pic);
				XFREE(MTYPE_VIDEO_PIC, PictureInfo.szPictureData);
				PictureInfo.szPictureData = NULL;
				return ERROR;
			}
			PictureInfo.nPictureType = EAIS_PICTURE_JPG;				// 人员性别  0： 女 1： 男
			PictureInfo.nDataLen = nDataLen;// 人脸图片数据长度（1, 1024 * 1024]字节

			ret = EAIS_SDK_GetPictureFeature(sdk->handle, &PictureInfo, &SDKSnapFea);
			if(ret != EAIS_SDK_SUCCESS)
			{
				XFREE(MTYPE_VIDEO_PIC, PictureInfo.szPictureData);
				PictureInfo.szPictureData = NULL;
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP,"EAIS_SDK_GetPictureFeature Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				return ERROR;
			}
			if(key)
			{
				if(SDKSnapFea.nFeatureLen > APP_FEATURE_MAX)
				{
					key->feature.feature_data = key->feature.ckey_data = XREALLOC(MTYPE_VIDEO_KEY,
						key->feature.ckey_data, SDKSnapFea.nFeatureLen * sizeof(float) + sizeof(float));									// 特征值
				}
				if(key->feature.ckey_data)
				{
					key->feature_len = SDKSnapFea.nFeatureLen;									// 特征值个数，长度=个数*sizoof(float)
					memcpy(key->feature.feature_data, SDKSnapFea.pfFeatureData, key->feature_len * sizeof(float));									// 特征值
					if(PictureInfo.szPictureData)
					{
						XFREE(MTYPE_VIDEO_PIC, PictureInfo.szPictureData);
						PictureInfo.szPictureData = NULL;
					}
					return OK;
				}
			}
			if(PictureInfo.szPictureData)
			{
				XFREE(MTYPE_VIDEO_PIC, PictureInfo.szPictureData);
				PictureInfo.szPictureData = NULL;
			}
			return ERROR;
		}
		else
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP," Video Board (%d) can not get file size (%s)", V9_APP_BOARD_HW_ID(id), pic);
			return ERROR;
		}
#else
		return OK;
#endif
	}
	if(V9_SDK_DEBUG(WARN))
		zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}


int v9_video_sdk_get_sosine_similarity_api(const float* p_fFirstArray, const float* p_fSecondArray, int p_nlength, float* p_fResult)
{
#ifdef V9_VIDEO_SDK_API
	int ret = 0;
	ret = EAIS_SDK_GetSosineSimilarity(p_fFirstArray, p_fSecondArray, p_nlength, p_fResult);
	if(ret != EAIS_SDK_SUCCESS)
	{
		if(V9_SDK_DEBUG(ERROR))
			zlog_err(ZLOG_APP," Video can not get sosine similarity ERROR(%s)", v9_video_sdk_errnostr(ret));
		return ERROR;
	}
	zlog_debug(ZLOG_APP," Video get sosine similarity %f", *p_fResult);
	return OK;
#else
	return ERROR;
#endif
}


int v9_video_sdk_get_config(struct vty *vty, u_int32 id, v9_video_sdk_t *sdk)
{
	if (sdk && sdk->login && sdk->handle >= 0)
	{
		int ret = 0;

		if (sdk->mode == EAIS_SOLUTION_TYPE_SNAP)
		{
			ST_SDKSnapConfig stSnapConfig;
			memset (&stSnapConfig, 0, sizeof(ST_SDKSnapConfig));
			ret = EAIS_SDK_GetSnapConfig (sdk->handle, &stSnapConfig);
			if (ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_GetSnapConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				vty_out (vty, " video sdk %d can not get snap config %s", V9_APP_BOARD_HW_ID(id),
									 VTY_NEWLINE);
				return ERROR;
			}
			//stHelmetConfig.nOriginalPicEnable
			vty_out (vty, " video sdk %d original picture %s%s", V9_APP_BOARD_HW_ID(id),
					 stSnapConfig.nOriginalPicEnable ? "enable" : "disabled",
					 VTY_NEWLINE);

			for (ret = 0; ret < stSnapConfig.nSnapInfoNum; ret++)
			{
				if (stSnapConfig.stSnapInfoArr[ret].nChannelId)
				{
					vty_out (vty, " video sdk %d channel %d snap confi %d%s",
							 V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							 stSnapConfig.stSnapInfoArr[ret].nConfi,
							 VTY_NEWLINE);
					vty_out (vty,
							 " video sdk %d channel %d snap qulityscore %d%s",
							 V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							 stSnapConfig.stSnapInfoArr[ret].nQulityScore,
							 VTY_NEWLINE);
					if(vty->type != VTY_FILE)
					{
					vty_out (
							vty,
							" video sdk %d channel %d snap attr %s%s",
							V9_APP_BOARD_HW_ID(id),
							stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nAttrEnable ?
									"enable" : "disabled",
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d snap feature %s%s",
							V9_APP_BOARD_HW_ID(id),
							stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nFeatureEnable ?
									"enable" : "disabled",
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d snap align %s%s",
							V9_APP_BOARD_HW_ID(id),
							stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nAlignEnable ?
									"enable" : "disabled",
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d snap area %s%s",
							V9_APP_BOARD_HW_ID(id),
							stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nAreaEnable ?
									"enable" : "disabled",
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d snap tripwire %s%s",
							V9_APP_BOARD_HW_ID(id),
							stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nTripWireEnable ?
									"enable" : "disabled",
							VTY_NEWLINE);
					}
					switch (stSnapConfig.stSnapInfoArr[ret].nSnapMode)
					{
						case 0:
							vty_out (
									vty,
									" video sdk %d channel %d snap snapmode %s%s",
									V9_APP_BOARD_HW_ID(id),
									stSnapConfig.stSnapInfoArr[ret].nChannelId,
									"disabled", VTY_NEWLINE);
							break;
						case 1:
							vty_out (
									vty,
									" video sdk %d channel %d snap snapmode %s%s",
									V9_APP_BOARD_HW_ID(id),
									stSnapConfig.stSnapInfoArr[ret].nChannelId,
									"realtime", VTY_NEWLINE);
							break;
						case 2:
							vty_out (
									vty,
									" video sdk %d channel %d snap snapmode %s%s",
									V9_APP_BOARD_HW_ID(id),
									stSnapConfig.stSnapInfoArr[ret].nChannelId,
									"leave", VTY_NEWLINE);
							break;
						case 3:
							vty_out (
									vty,
									" video sdk %d channel %d snap snapmode %s%s",
									V9_APP_BOARD_HW_ID(id),
									stSnapConfig.stSnapInfoArr[ret].nChannelId,
									"timing", VTY_NEWLINE);
							break;
						case 4:
							vty_out (
									vty,
									" video sdk %d channel %d snap snapmode %s%s",
									V9_APP_BOARD_HW_ID(id),
									stSnapConfig.stSnapInfoArr[ret].nChannelId,
									"into", VTY_NEWLINE);
							break;
						default:
							break;
					}
					vty_out (vty,
							 " video sdk %d channel %d snap intervaltime %d%s",
							 V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							 stSnapConfig.stSnapInfoArr[ret].nIntervalTime,
							 VTY_NEWLINE);
					vty_out (vty, " video sdk %d channel %d snap maxsize %d%s",
							 V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							 stSnapConfig.stSnapInfoArr[ret].nMaxSize,
							 VTY_NEWLINE);
					vty_out (vty, " video sdk %d channel %d snap minsize %d%s",
							 V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							 stSnapConfig.stSnapInfoArr[ret].nMinSize,
							 VTY_NEWLINE);
					if(vty->type != VTY_FILE)
					{
					vty_out (
							vty,
							" video sdk %d channel %d snap areaconfigwidth %d%s",
							V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nAreaConfigWidth,
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d snap areaconfigheight %d%s",
							V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nAreaConfigHeight,
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d snap tripconfigwidth %d%s",
							V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nTripConfigWidth,
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d snap tripconfigheight %d%s",
							V9_APP_BOARD_HW_ID(id), stSnapConfig.stSnapInfoArr[ret].nChannelId,
							stSnapConfig.stSnapInfoArr[ret].nTripConfigHeight,
							VTY_NEWLINE);
					}
					/*
					 vty_out(vty, " video sdk %d channel %d confi %d%s", id, stSnapConfig.stSnapInfoArr[ret].nChannelId, stSnapConfig.stSnapInfoArr[ret].nConfi, VTY_NEWLINE);
					 vty_out(vty, " video sdk %d channel %d confi %d%s", id, stSnapConfig.stSnapInfoArr[ret].nChannelId, stSnapConfig.stSnapInfoArr[ret].nConfi, VTY_NEWLINE);
					 vty_out(vty, " video sdk %d channel %d confi %d%s", id, stSnapConfig.stSnapInfoArr[ret].nChannelId, stSnapConfig.stSnapInfoArr[ret].nConfi, VTY_NEWLINE);
					 vty_out(vty, " video sdk %d channel %d confi %d%s", id, stSnapConfig.stSnapInfoArr[ret].nChannelId, stSnapConfig.stSnapInfoArr[ret].nConfi, VTY_NEWLINE);
					 vty_out(vty, " video sdk %d channel %d confi %d%s", id, stSnapConfig.stSnapInfoArr[ret].nChannelId, stSnapConfig.stSnapInfoArr[ret].nConfi, VTY_NEWLINE);*/
				}
			}
			return OK;
		}
#if 0
		else if (sdk->mode == EAIS_SOLUTION_TYPE_HELMET)
		{
			ST_SDKHelmetConfig stHelmetConfig;
			memset (&stHelmetConfig, 0, sizeof(ST_SDKHelmetConfig));

			ret = EAIS_SDK_GetHelmetConfig (sdk->handle, &stHelmetConfig);
			if (ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_GetHelmetConfig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				vty_out (vty, " video sdk %d can not get helmet config%s", V9_APP_BOARD_HW_ID(id),
												 VTY_NEWLINE);
				return ERROR;
			}

			for (ret = 0; ret < stHelmetConfig.nHelmetInfoNum; ret++)
			{
				if (stHelmetConfig.stHelmetInfoArr[ret].nChannelId)
				{

					vty_out (
							vty,
							" video sdk %d channel %d helmet sentimage %s%s",
							V9_APP_BOARD_HW_ID(id),
							stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							stHelmetConfig.stHelmetInfoArr[ret].nSentImage ?
									"enable" : "disabled",
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d helmet tracking %s%s",
							V9_APP_BOARD_HW_ID(id),
							stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							stHelmetConfig.stHelmetInfoArr[ret].nUseTracking ?
									"enable" : "disabled",
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d helmet drawrectangle %s%s",
							V9_APP_BOARD_HW_ID(id),
							stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							stHelmetConfig.stHelmetInfoArr[ret].nDrawRectangle ?
									"enable" : "disabled",
							VTY_NEWLINE);

					vty_out (vty,
							 " video sdk %d channel %d helmet imageratio %d%s",
							 V9_APP_BOARD_HW_ID(id), stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							 stHelmetConfig.stHelmetInfoArr[ret].nImageRatio,
							 VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d helmet snapinterval %d%s",
							V9_APP_BOARD_HW_ID(id), stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							stHelmetConfig.stHelmetInfoArr[ret].nSnapInterval,
							VTY_NEWLINE);
					vty_out (
							vty,
							" video sdk %d channel %d helmet alarminterval %d%s",
							V9_APP_BOARD_HW_ID(id), stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							stHelmetConfig.stHelmetInfoArr[ret].nAlarmInterval,
							VTY_NEWLINE);

					vty_out (vty,
							 " video sdk %d channel %d helmet snapratio %d%s",
							 V9_APP_BOARD_HW_ID(id), stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							 stHelmetConfig.stHelmetInfoArr[ret].nSnapRatio,
							 VTY_NEWLINE);
					if(vty->type != VTY_FILE)
					{
					vty_out (vty,
							 " video sdk %d channel %d helmet uploadmode %d%s",
							 V9_APP_BOARD_HW_ID(id), stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							 stHelmetConfig.stHelmetInfoArr[ret].nUploadMode,
							 VTY_NEWLINE);
					}
					vty_out (vty,
							 " video sdk %d channel %d helmet threshold %d%s",
							 V9_APP_BOARD_HW_ID(id), stHelmetConfig.stHelmetInfoArr[ret].nChannelId,
							 stHelmetConfig.stHelmetInfoArr[ret].nThreshold,
							 VTY_NEWLINE);

					/*
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
					 */
				}
			}
			return OK;
		}
#endif
		else if (sdk->mode == EAIS_SOLUTION_TYPE_RECOGNITION)
		{
			ST_SDKRecognizeConfig stRecognizeConfig;
			memset (&stRecognizeConfig, 0, sizeof(ST_SDKRecognizeConfig));

			ret = EAIS_SDK_GetRecognizeCofig (sdk->handle, &stRecognizeConfig);
			if (ret != EAIS_SDK_SUCCESS)
			{
				if(V9_SDK_DEBUG(ERROR))
					zlog_err(ZLOG_APP, "EAIS_SDK_GetRecognizeCofig Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
				vty_out (vty, " video sdk %d can not get recognize cofig %s", V9_APP_BOARD_HW_ID(id),
												 VTY_NEWLINE);
				return ERROR;
			}

			vty_out (
					vty,
					" video sdk %d recognize similarity %d registerquality %d%s",
					V9_APP_BOARD_HW_ID(id), stRecognizeConfig.nOutSimilarity,
					stRecognizeConfig.nRegisterQuality, VTY_NEWLINE);
			vty_out (vty, " video sdk %d recognize openupload %s%s", V9_APP_BOARD_HW_ID(id),
					 stRecognizeConfig.nOpenUpload ? "enable" : "disabled",
					 VTY_NEWLINE);
			return OK;
		}
	}
	return OK;
}


static int v9_video_sdk_timer_hw_api(u_int32 id, struct tm *tme)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk && sdk->login && sdk->handle >= 0)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		ST_SDKDeviceTime pstDeviceTime;
		pstDeviceTime.nYear = tme->tm_year + 1900;											// 年
		pstDeviceTime.nMonth = tme->tm_mon + 1;											// 月
		pstDeviceTime.nDay = tme->tm_mday;											// 日
		pstDeviceTime.nHour = tme->tm_hour;											// 时
		pstDeviceTime.nMinute = tme->tm_min;										// 分
		pstDeviceTime.nSecond = tme->tm_sec;										// 秒;

		ret = EAIS_SDK_SetDeviceTime(sdk->handle, &pstDeviceTime);
		if(ret != EAIS_SDK_SUCCESS)
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP,"EAIS_SDK_SetDeviceTime Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
			return ERROR;
		}
		V9_SDK_DBGPRF(" Video Board (%d) SDK Set Device Time OK", V9_APP_BOARD_HW_ID(id));
		return OK;
#else
		return OK;
#endif
	}
	if(V9_SDK_DEBUG(WARN))
		zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

int v9_video_sdk_timer_api(u_int32 id)
{
	struct tm tm_tmp;
	time_t time_tmp = 0;

	time_tmp = os_time(NULL);

	localtime_r(&time_tmp, &tm_tmp);
	return v9_video_sdk_timer_hw_api(id, &tm_tmp);
}

int v9_video_sdk_ntp_api(u_int32 id, char *ntps, int TimingInterval)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_video_board && sdk && sdk->login && sdk->handle >= 0)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		ST_SDKNTPInfo pstNTPInfo;
		memset(&pstNTPInfo, 0, sizeof(pstNTPInfo));
		if(TimingInterval > 0)
		{
			pstNTPInfo.nEnable = 1;										// NTP使能 0：不使能，1：使能
			if(ntps)
				strcpy(pstNTPInfo.szDomainName, ntps);			// 域名/ip
			pstNTPInfo.nTimingInterval = TimingInterval;								// 校时间隔(0, 65535]分钟									// 秒;
		}
		else
		{
			pstNTPInfo.nEnable = 0;
		}

		ret = EAIS_SDK_SetNTPInfo(sdk->handle, &pstNTPInfo);
		if(ret != EAIS_SDK_SUCCESS)
		{
			if(V9_SDK_DEBUG(ERROR))
				zlog_err(ZLOG_APP,"EAIS_SDK_SetNTPInfo Video Board (%d) ERROR(%s)", V9_SDK_ID(sdk),v9_video_sdk_errnostr(ret));
			return ERROR;
		}
		V9_SDK_DBGPRF(" Video Board (%d) SDK Set Device NTP OK", V9_APP_BOARD_HW_ID(id));
		return OK;
#else
		return OK;
#endif
	}
	if(V9_SDK_DEBUG(WARN))
		zlog_warn(ZLOG_APP," Video Board (%d) SDK not connect", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}


//LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetDeviceTime(int p_nLoginHandle, ST_SDKDeviceTime* p_pstDeviceTime);
//LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetDeviceTime(int p_nLoginHandle, const ST_SDKDeviceTime* p_pstDeviceTime);

/*
// 查询用户 当用户不存在时返回-3
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetUser(int p_nLoginHandle, char* p_pszUserID, ST_SDKUserInfo* p_pstUserInfo);

// 功能描述：查询一个组内部分人员信息 当组为空时返回-3
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetGroupUsers(int p_nLoginHandle, int p_nGroupID, int p_nPageIndex, int* p_pnALLUserNum, ST_SDKUserData* p_pstUserInfoList);

// 查询组名信息
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_QueryGroupNameList(int p_nLoginHandle, ST_SDKGroupInfoList* p_pstGroupInfoList);

// 设置组名信息
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetGroupNameList(int p_nLoginHandle, const ST_SDKGroupInfoList* p_pstGroupInfoList);
*/

