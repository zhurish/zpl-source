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
#include "zmemory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"

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



static v9_video_sdk_t *v9_sdk_data = NULL;
static zpl_bool	v9_sdk_initialization = zpl_false;

static zpl_bool v9_video_sdk_isopen(v9_video_sdk_t *);
static int v9_video_sdk_open(v9_video_sdk_t *);
//static int v9_video_sdk_close(v9_video_sdk_t *);


v9_video_sdk_t * v9_video_sdk_lookup(zpl_uint8 id)
{
	zpl_uint32 i =0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_sdk_data[i].id == id)
		{
			return &v9_sdk_data[i];
		}
	}
	return NULL;
}

static zpl_bool v9_video_sdk_isopen(v9_video_sdk_t *sdk)
{
	return sdk? sdk->login:zpl_false;
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
	zlog_warn(MODULE_APP,"v9_video_sdk_device_clk nDeviceType=%d szDeviceIP=%s", p_pstStatusInfo->nDeviceType, p_pstStatusInfo->szDeviceIP);
	//printf("v9_video_sdk_device_clk nDeviceType=%d szDeviceIP=%s\r\n", p_pstStatusInfo->nDeviceType, p_pstStatusInfo->szDeviceIP);
	//if(psdk->type)
	//{
		//zlog_warn(MODULE_APP,"v9_video_sdk_device_clk find");
		//psdk->find = zpl_true;
	//}
	zlog_warn(MODULE_APP,"v9_video_sdk_device_clk");
	//printf("v9_video_sdk_device_clk find\r\n");

	if(strlen(p_pstStatusInfo->szDeviceIP))
	{
		zpl_uint32 i =0;
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			zlog_warn(MODULE_APP,"v9_video_sdk_device_clk [%d]address=%s szDeviceIP=%s", i, v9_sdk_data[i].address, p_pstStatusInfo->szDeviceIP);
			//printf("v9_video_sdk_device_clk [%d]address=%s szDeviceIP=%s\r\n", i, v9_sdk_data[i].address, p_pstStatusInfo->szDeviceIP);

			if(strlen(v9_sdk_data[i].address) && inet_addr(v9_sdk_data[i].address) != 0 &&
					(inet_addr(v9_sdk_data[i].address) == inet_addr(p_pstStatusInfo->szDeviceIP)))
			{
				if(p_pstStatusInfo->nDeviceType == EAIS_DEVICE_TYPE_SNAP)
					v9_sdk_data[i].type = EAIS_DEVICE_TYPE_SNAP;
				else if(p_pstStatusInfo->nDeviceType == EAIS_DEVICE_TYPE_RECOGNIZE)
					v9_sdk_data[i].type = EAIS_DEVICE_TYPE_RECOGNIZE;
				v9_sdk_data[i].find = zpl_true;
				v9_board_set_ready(v9_sdk_data[i].id);
				printf("v9_video_sdk_device_clk find\r\n");
				zlog_warn(MODULE_APP,"v9_video_sdk_device_clk find");
				return EAIS_SDK_SUCCESS;
			}
		}
	}
	return EAIS_SDK_SUCCESS;
}

static int v9_video_sdk_status_clk(ST_SDKStatusInfo* p_pstStatusInfo, void* p_pUserData)
{
	v9_video_channel_t *channel;
	v9_video_sdk_t *psdk = (v9_video_sdk_t *)p_pUserData;
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
	channel = v9_video_board_video_channel_lookup_by_id_and_ch(psdk->id, p_pstStatusInfo->nChannlId);
	if(channel)
	{
		//channel->connect;				//视频流连接状态
		if(p_pstStatusInfo->nStatusBit & 0x01)
			channel->dev_status = p_pstStatusInfo->nESStatus;					// EAIS设备状态，默认离线。 ENUM_EAIS_DEVICE_STATUS
		if(p_pstStatusInfo->nStatusBit & 0x02)
			channel->rtsp_status = p_pstStatusInfo->nRTSPStatus;									// 通道RTSP状态 ENUM_EAIS_DEVICE_STATUS
		if(p_pstStatusInfo->nStatusBit & 0x04)
			channel->decode_status = p_pstStatusInfo->nDecodeStatus;									// 解码状态

		//channel->change;
	}
	zlog_warn(MODULE_APP,"v9_video_sdk_status_clk");
	return EAIS_SDK_SUCCESS;
}

static int v9_video_sdk_snap_clk(ST_SDKStructInfo* p_pstStatusInfo, void* p_pUserData)
{
	v9_video_sdk_t *psdk = (v9_video_sdk_t *)p_pUserData;
	// 如有状态变更， sdk 则会调用该函数
	if(NULL == p_pstStatusInfo || NULL == p_pUserData)
		return EAIS_SDK_ERROR_NULL_POITER;

	// 结构化数据类型

	psdk->datatype = p_pstStatusInfo->nDataType;

	// 方案类型
	if(p_pstStatusInfo->nSolutionType == EAIS_SOLUTION_TYPE_SNAP)
		psdk->mode = EAIS_DEVICE_TYPE_SNAP;
	else if(p_pstStatusInfo->nSolutionType == EAIS_SOLUTION_TYPE_HELMET)
		psdk->mode = EAIS_DEVICE_TYPE_RECOGNIZE;
	else if(p_pstStatusInfo->nSolutionType == EAIS_SOLUTION_TYPE_RECOGNITION)
		psdk->mode = EAIS_DEVICE_TYPE_RECOGNIZE;

	zlog_warn(MODULE_APP,"v9_video_sdk_snap_clk");
	return EAIS_SDK_SUCCESS;
}
#endif


static int v9_video_sdk_open(v9_video_sdk_t *sdk)
{
	assert(sdk != NULL);
	if(!v9_board_ready(sdk->id))
	{
		zlog_warn(MODULE_APP," board ID=%d is not ready", sdk->id);
		return ERROR;
	}
	if(!strlen(sdk->address))
	{
		v9_address_t * board = v9_board_lookup(sdk->id);
		if(board)
		{
			strcpy(sdk->address, inet_address(board->ip));
		}
		else
		{
			zlog_err(MODULE_APP," v9_video_sdk_open:v9_board_lookup(%d) ",sdk->id);
			return ERROR;
		}
		zlog_warn(MODULE_APP," board ID=%d %s %d %s %s", sdk->id, sdk->address, sdk->port, sdk->username, sdk->password);
	}
	else
		zlog_warn(MODULE_APP," board ID=%d %s %d %s %s", sdk->id, sdk->address, sdk->port, sdk->username, sdk->password);

	//zlog_warn(MODULE_APP," board ID=%d %s %d %s %s", sdk->id, sdk->address, sdk->port, sdk->username, sdk->password);
#ifdef V9_VIDEO_SDK_API
	EAIS_SDK_StartDiscovery(v9_video_sdk_device_clk, NULL);
/*
	// Onvif (获取URL超时单位/秒)
	LIB_SDK_DLL_EXPORT int __stdcall EAIS_ONVIF_StartDiscovery(EAIS_ONVIFDiscoveryCallBack p_fDiscoveryCallback, void* p_pUserData);
*/
	if(!sdk->find)
	{
		zlog_warn(MODULE_APP," di not find board ID=%d %s %d %s %s", sdk->id, sdk->address, sdk->port, sdk->username, sdk->password);
		return ERROR;
	}

	sdk->handle = EAIS_SDK_Login(sdk->address, sdk->port, sdk->username, sdk->password, V9_VIDEO_SDK_TIMEOUT);
	if(sdk->handle < EAIS_SDK_SUCCESS)
	{
		sdk->handle = -1;
		sdk->login = zpl_false;
		sdk->getstate = zpl_false;
		v9_video_board_active(sdk->id, zpl_false);
		zlog_err(MODULE_APP, " EAIS SDK Login failed on Board ID=%s, ipstack_errno=%d", sdk->address, sdk->handle);
		return ERROR;
	}
	zlog_warn(MODULE_APP,"EAIS_SDK_Login OK");

	// 回调注册：设备状态/图片流
	EAIS_SDK_SetStatusCallBack(sdk->handle, v9_video_sdk_status_clk, sdk);
	EAIS_SDK_SetSnapCallBack(sdk->handle, v9_video_sdk_snap_clk, sdk);

	if(EAIS_SDK_SUCCESS == EAIS_SDK_GetDeviceInfo(sdk->handle, sdk->device))
	{
		v9_video_board_active(sdk->id, zpl_true);
		sdk->login = zpl_true;
		zlog_warn(MODULE_APP,"EAIS_SDK_GetDeviceInfo OK");
		return OK;
	}
	zlog_err(MODULE_APP,"EAIS_SDK_GetDeviceInfo ERROR");
	EAIS_SDK_Logout(sdk->handle);
	sdk->login = zpl_false;
	sdk->handle = -1;
	sdk->getstate = zpl_false;
	v9_video_board_active(sdk->id, zpl_false);
	return ERROR;
#else
	return OK;
#endif
}



static int v9_video_sdk_devstatus_get(v9_video_sdk_t *sdk)
{
#ifdef V9_VIDEO_SDK_API
/*	ST_SDKDeviceInfo device;
	memset(&device, 0, sizeof(ST_SDKDeviceInfo));
	if(sdk && sdk->handle >= 0)
	{
		if(EAIS_SDK_SUCCESS == EAIS_SDK_GetDeviceInfo(sdk->handle, &device))
		{
			sdk->getstate = zpl_true;
			return OK;
		}
		return ERROR;
	}
	return ERROR;*/
	if(sdk && sdk->handle >= 0 && sdk->device)
	{
		if(EAIS_SDK_SUCCESS == EAIS_SDK_GetDeviceInfo(sdk->handle, sdk->device))
		{
			sdk->getstate = zpl_true;
			return OK;
		}
		return ERROR;
	}
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
	sdk = ELOOP_ARG(eloop);
	if(!sdk)
	{
		zlog_debug(MODULE_APP,"v9_video_sdk_timeout sdk = NULL");
		return 0;
	}
	sdk->t_timeout = NULL;

	if(!v9_sdk_initialization)
	{
#ifdef V9_VIDEO_SDK_API
		if (EAIS_SDK_SUCCESS != EAIS_SDK_Init(V9_APP_BOARD_MAX + 1))
		{
			EAIS_SDK_UnInit();
			v9_sdk_initialization = zpl_false;
			zlog_err(MODULE_APP,"EAIS_SDK_Init ERROR");
		}
		// EAIS设备搜索、设置设备IP
		//EAIS_SDK_StartDiscovery(v9_video_sdk_device_clk, NULL);
#endif
	}

	if(!v9_video_sdk_isopen(sdk))
		v9_video_sdk_open(sdk);
	else
	{
		v9_video_sdk_devstatus_get(sdk);
	}

	sdk->t_timeout = eloop_add_timer(sdk->master,
			v9_video_sdk_timeout, sdk, sdk->interval);

	//zlog_debug(MODULE_APP,"v9_video_sdk_timeout");
	return 0;
}

static int v9_video_sdk_task(void *argv)
{
	zassert(argv != NULL);
	v9_video_sdk_t *mgt = (v9_video_sdk_t *)argv;
	zassert(mgt != NULL);
	module_setup_task(MODULE_APP_START + 1, os_task_id_self());
	host_waitting_loadconfig();
/*	if(!mgt->enable)
	{
		os_sleep(5);
	}*/

	v9_video_sdk_restart_all();

	// EAIS设备搜索、设置设备IP
	//EAIS_SDK_StartDiscovery(v9_video_sdk_device_clk, NULL);
/*	if(!tty_isopen(mgt->tty))
	{
		tty_com_open(mgt->tty);
	}*/
	eloop_start_running(master_eloop[MODULE_APP_START + 1], MODULE_APP_START + 1);
	return OK;
}

int v9_video_sdk_task_init ()
{
	zassert(v9_sdk_data != NULL);
	if(master_eloop[MODULE_APP_START + 1] == NULL)
		v9_sdk_data->master = master_eloop[MODULE_APP_START + 1] = eloop_master_module_create(MODULE_APP_START + 1);

	zlog_debug(MODULE_APP, "---------%s---------", __func__);
	//v9_sdk_data->enable = zpl_true;
	//v9_sdk_data->task_id =
			os_task_create("appSdk", OS_TASK_DEFAULT_PRIORITY,
	               0, v9_video_sdk_task, v9_sdk_data, OS_TASK_DEFAULT_STACK * 2);
	//if(v9_sdk_data->task_id)
	//	return OK;
	return ERROR;
}


int v9_video_sdk_update_address(zpl_uint32 id, zpl_uint32 ip)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
		memset(sdk->address, 0, sizeof(sdk->address));
		if(ip)
		{
			sprintf(sdk->address, "%s", inet_address(ip));
		}
		return OK;
	}
	else
		return ERROR;
}


int v9_video_sdk_init(void)
{
	zpl_uint32 i = 0;
	if (v9_sdk_data == NULL)
		v9_sdk_data = XMALLOC(MTYPE_VIDEO_SDK, sizeof(v9_video_sdk_t) * V9_APP_BOARD_MAX);
	if (v9_sdk_data)
	{
		v9_sdk_initialization = zpl_false;
		memset(v9_sdk_data, 0, sizeof(v9_video_sdk_t) * V9_APP_BOARD_MAX);
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
#ifdef V9_VIDEO_SDK_API
			v9_sdk_data[i].device = XMALLOC(MTYPE_VIDEO_SDK, sizeof(ST_SDKDeviceInfo));
			if (v9_sdk_data[i].device)
				memset(v9_sdk_data[i].device, 0, sizeof(ST_SDKDeviceInfo));
#endif
			v9_sdk_data[i].id = V9_BOARD_ID(i + APP_BOARD_CALCU_1);
			v9_sdk_data[i].interval = 15;
			//v9_sdk_data[i].cnt = 0;
			v9_sdk_data[i].status = 0;
			v9_sdk_data[i].handle = -1;
			v9_sdk_data[i].port = V9_VIDEO_SDK_PORT;
			strcpy(v9_sdk_data[i].username, V9_VIDEO_SDK_USERNAME);
			strcpy(v9_sdk_data[i].password, V9_VIDEO_SDK_PASSWORD);

			strcpy(v9_sdk_data[i].address, inet_address(APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_1 + i));

			//v9_sdk_data[i].master = m;
			if (v9_sdk_data[i].master)
			{
				if (v9_sdk_data[i].interval)
				{
					v9_sdk_data[i].t_timeout = eloop_add_timer(v9_sdk_data[i].master,
																v9_video_sdk_timeout, &v9_sdk_data[i], 5);

				}
			}
		}
		//初始化最大支持4个EAIS设备
#ifdef V9_VIDEO_SDK_API
		if (EAIS_SDK_SUCCESS != EAIS_SDK_Init(V9_APP_BOARD_MAX + 1))
		{
			v9_sdk_initialization = zpl_false;
			zlog_err(MODULE_APP,"EAIS_SDK_Init ERROR");
			return ERROR;
		}
		// EAIS设备搜索、设置设备IP
		//EAIS_SDK_StartDiscovery(v9_video_sdk_device_clk, NULL);
#endif
		v9_sdk_initialization = zpl_true;
		return OK;
	}
	return ERROR;
}



int v9_video_sdk_restart_all()
{
	zpl_uint32 i = 0;
	if(!v9_sdk_data)
		return ERROR;
	if(master_eloop[MODULE_APP_START + 1] == NULL)
		master_eloop[MODULE_APP_START + 1] = eloop_master_module_create(MODULE_APP_START + 1);

	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		v9_sdk_data[i].master = master_eloop[MODULE_APP_START + 1];
		if (v9_sdk_data[i].master)
		{
			if (v9_sdk_data[i].interval)
			{
				if(v9_sdk_data[i].t_timeout)
				{
					eloop_cancel(v9_sdk_data[i].t_timeout);
					v9_sdk_data[i].t_timeout = NULL;
				}
				v9_sdk_data[i].t_timeout = eloop_add_timer(v9_sdk_data[i].master,
															v9_video_sdk_timeout, &v9_sdk_data[i], 5);
			}
		}
	}
	return OK;
}


int v9_video_sdk_start(zpl_uint32 id)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(!v9_sdk_data)
		return ERROR;
	if(!sdk)
		return ERROR;
	if(!v9_video_sdk_isopen(sdk))
		return v9_video_sdk_open(sdk);
	else
		return OK;
	return ERROR;
}


int v9_video_sdk_stop(zpl_uint32 id)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			EAIS_SDK_Logout(sdk->handle);
			v9_video_board_active(id, zpl_false);
			sdk->login = zpl_false;
			sdk->handle = -1;
			sdk->getstate = zpl_false;
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}


static int _v9_video_sdk_show(struct vty * vty, v9_video_sdk_t *sdk, int debug)
{
	if(sdk && sdk->device && sdk->getstate)
	{
#ifdef V9_VIDEO_SDK_API
		char datatype[64];
		memset(datatype, 0, sizeof(datatype));
		ST_SDKDeviceInfo *device = (ST_SDKDeviceInfo *)sdk->device;
		vty_out (vty, "Board Status:%s", VTY_NEWLINE);

		vty_out (vty, " Device Name         : %s%s", device->szDeviceName, VTY_NEWLINE);
		vty_out (vty, " Device ID           : %s%s", device->szDeviceId, VTY_NEWLINE);

		if(device->nDeviceType == EAIS_DEVICE_TYPE_UNKNOW)
			vty_out (vty, " Type                : UNKNOW%s", VTY_NEWLINE);// 未知类型
		else if(device->nDeviceType == EAIS_DEVICE_TYPE_SNAP)
			vty_out (vty, " Type                : SNAP%s", VTY_NEWLINE);// 抓拍类型EAIS设备
		else if(device->nDeviceType == EAIS_DEVICE_TYPE_RECOGNIZE)
			vty_out (vty, " Type                : RECOGNIZE%s", VTY_NEWLINE);// 识别类型EAIS设备

		if(sdk->mode == EAIS_SOLUTION_TYPE_SNAP)
			vty_out (vty, " Mode                : SNAP%s", VTY_NEWLINE);// 未知类型
		else if(sdk->mode == EAIS_SOLUTION_TYPE_HELMET)
			vty_out (vty, " Mode                : HELMET%s", VTY_NEWLINE);// 抓拍类型EAIS设备
		else if(sdk->mode == EAIS_SOLUTION_TYPE_RECOGNITION)
			vty_out (vty, " Mode                : RECOGNIZE%s", VTY_NEWLINE);// 识别类型EAIS设备

		if(sdk->datatype == EAIS_DATA_TYPE_UNKNOW)
			vty_out (vty, " DataType            : UNKNOW%s", VTY_NEWLINE);// 未知类型
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
		return OK;
	}
	vty_out (vty, "Board Status: Not Connected%s", VTY_NEWLINE);
	return ERROR;
}

int v9_video_sdk_show(struct vty * vty, int id, int debug)
{
	if(v9_sdk_data)
	{
		if(id)
		{
			v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
			if(sdk)
				_v9_video_sdk_show(vty, sdk,  debug);
		}
		else
		{
			zpl_uint32 i = 0;
			for(i = 0; i < V9_APP_BOARD_MAX; i++)
			{
				_v9_video_sdk_show(vty, &v9_sdk_data[i],  debug);
			}
		}
	}
	return 0;
}


/*******************************************************************************/
/*******************************************************************************/
int v9_video_sdk_reboot_api(zpl_uint32 id, zpl_bool reset)
{
	zpl_uint32 i =0;
	//v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data)
	{
#ifdef V9_VIDEO_SDK_API

		int ret = 0;
		if(reset)// 恢复出厂
		{
			ST_SDKResetInfo p_pstResetInfo;
/*
			ST_SDKDeviceInfo *device = (ST_SDKDeviceInfo *)sdk->device;
			memset(&p_pstResetInfo, 0, sizeof(ST_SDKResetInfo));
			if(device)
				strcpy(p_pstResetInfo.szDeviceID, device->szDeviceId);
*/
			if(id == 0)
			{
				for(i = 0; i < V9_APP_BOARD_MAX; i++)
				{
					if(/*v9_sdk_data[i].id == id && */v9_sdk_data[i].login && v9_sdk_data[i].handle >= 0)
					{
						ST_SDKDeviceInfo *device = (ST_SDKDeviceInfo *)v9_sdk_data[i].device;
						memset(&p_pstResetInfo, 0, sizeof(ST_SDKResetInfo));
						if(device)
							strcpy(p_pstResetInfo.szDeviceID, device->szDeviceId);

						ret |= EAIS_SDK_FactoryReset(v9_sdk_data[i].handle, &p_pstResetInfo);

						if(ret != EAIS_SDK_SUCCESS)
						{
							zlog_err(MODULE_APP,"EAIS_SDK_FactoryReset ERROR");
							v9_video_board_active(id, zpl_false);
							v9_sdk_data[i].login = zpl_false;
							v9_sdk_data[i].handle = -1;
							v9_sdk_data[i].getstate = zpl_false;
							return ERROR;
						}
					}
				}
				return OK;
			}
			else
			{
				v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
				if(sdk && sdk->login && sdk->handle >= 0)
				{
					ST_SDKDeviceInfo *device = (ST_SDKDeviceInfo *)sdk->device;
					memset(&p_pstResetInfo, 0, sizeof(ST_SDKResetInfo));
					if(device)
						strcpy(p_pstResetInfo.szDeviceID, device->szDeviceId);
					ret |= EAIS_SDK_FactoryReset(v9_sdk_data[i].handle, &p_pstResetInfo);
					if(ret != EAIS_SDK_SUCCESS)
					{
						zlog_err(MODULE_APP,"EAIS_SDK_FactoryReset ERROR");
						v9_video_board_active(id, zpl_false);
						v9_sdk_data[i].login = zpl_false;
						v9_sdk_data[i].handle = -1;
						v9_sdk_data[i].getstate = zpl_false;
						return ERROR;
					}
					return OK;
				}
				return ERROR;
			}
		}
		else // EAIS设备重启
		{
			if(id == 0)
			{
				for(i = 0; i < V9_APP_BOARD_MAX; i++)
				{
					if(/*v9_sdk_data[i].id == id && */v9_sdk_data[i].login && v9_sdk_data[i].handle >= 0)
					{
						ret |= EAIS_SDK_DeviceReboot(v9_sdk_data[i].handle);

						if(ret != EAIS_SDK_SUCCESS)
						{
							zlog_err(MODULE_APP,"EAIS_SDK_DeviceReboot ERROR");
							v9_video_board_active(id, zpl_false);
							v9_sdk_data[i].login = zpl_false;
							v9_sdk_data[i].handle = -1;
							v9_sdk_data[i].getstate = zpl_false;
							return ERROR;
						}
					}
				}
				return OK;
			}
			else
			{
				v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
				if(sdk && sdk->login && sdk->handle >= 0)
				{
					ret |= EAIS_SDK_DeviceReboot(v9_sdk_data[i].handle);
					if(ret != EAIS_SDK_SUCCESS)
					{
						zlog_err(MODULE_APP,"EAIS_SDK_DeviceReboot ERROR");
						v9_video_board_active(id, zpl_false);
						v9_sdk_data[i].login = zpl_false;
						v9_sdk_data[i].handle = -1;
						v9_sdk_data[i].getstate = zpl_false;
						return ERROR;
					}
					return OK;
				}
				return ERROR;
			}
		}
		return ERROR;
#else
		return OK;
#endif
	}
	return ERROR;
}

//获取当前正常连接的视频路数
int v9_video_sdk_getvch_api(zpl_uint32 id)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup (id);
	if (v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if (sdk->handle >= 0)
		{
			ST_SDKRTSPConfig p_pstRTSPConfig;
			memset (&p_pstRTSPConfig, 0, sizeof(ST_SDKRTSPConfig));
			if (EAIS_SDK_GetRtspConfig (sdk->handle,
										&p_pstRTSPConfig) == EAIS_SDK_SUCCESS)
			{
				zpl_uint32 i = 0, vch = 0;
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
				zlog_err(MODULE_APP, "EAIS_SDK_GetRtspConfig ERROR");
				return ERROR;
			}
			return 0;
		}
#else
		return 0;
#endif
	}
	return 0;
}

//把上层配置的视频URL下发到SDK
int v9_video_sdk_set_vch_api(zpl_uint32 id, int cnum, void *p)
{
	v9_video_channel_t *stream = p;
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk && stream)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0, i =0;
			ST_SDKRTSPConfig stRTSPConfig;
			memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
			stRTSPConfig.nRTSPInfoNum = cnum;
			for(i = 0; i < cnum; i++)
			{
				if(strlen(stream->video_url))
				{
					strcpy(stRTSPConfig.stRTSPInfoArr[i].szChanRtspUrl, stream->video_url);
				}
			}
			ret = EAIS_SDK_SetRtspConfig(sdk->handle, &stRTSPConfig);

			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetRtspConfig ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_add_vch_api(zpl_uint32 id, int ch, char *url)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk && url)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKRTSPConfig stRTSPConfig;
			memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
			stRTSPConfig.nRTSPInfoNum = 1;
			stRTSPConfig.stRTSPInfoArr[0].nChannleId = ch;
			strcpy(stRTSPConfig.stRTSPInfoArr[0].szChanRtspUrl, url);
			ret = EAIS_SDK_SetRtspConfig(sdk->handle, &stRTSPConfig);

			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetRtspConfig ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_del_vch_api(zpl_uint32 id, int ch)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKRTSPConfig stRTSPConfig;
			memset(&stRTSPConfig, 0, sizeof(stRTSPConfig));
			stRTSPConfig.nRTSPInfoNum = 1;
			stRTSPConfig.stRTSPInfoArr[0].nChannleId = ch;
			//strcpy(stRTSPConfig.stRTSPInfoArr[0].szChanRtspUrl, url);
			ret = EAIS_SDK_SetRtspConfig(sdk->handle, &stRTSPConfig);

			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetRtspConfig ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_open_snap_api(zpl_uint32 id, zpl_uint32 type)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ret = EAIS_SDK_OpenSnapStream(sdk->handle, type);

			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_OpenSnapStream ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_close_snap_api(zpl_uint32 id)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ret = EAIS_SDK_CloseSnapStream(sdk->handle);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_CloseSnapStream ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_set_snap_dir_api(zpl_uint32 id, zpl_bool http, char *address, int port,
								  char *user, char *pass, char *dir)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
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
				zlog_err(MODULE_APP,"EAIS_SDK_SetSnapServerAddr ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

// 获取/设置人脸识别配置参数
int v9_video_sdk_recognize_config_set_api(zpl_uint32 id, int nOutSimilarity,
									  int nRegisterQuality, zpl_bool nOpenUpload)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKRecognizeConfig stRecognizeConfig;
			memset(&stRecognizeConfig, 0, sizeof(ST_SDKRecognizeConfig));

			stRecognizeConfig.nOutSimilarity = nOutSimilarity;
			stRecognizeConfig.nRegisterQuality = nRegisterQuality;
			stRecognizeConfig.nOpenUpload = nOpenUpload;
			ret = EAIS_SDK_SetRecognizeCofig(sdk->handle, &stRecognizeConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetRecognizeCofig ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_recognize_config_get_api(zpl_uint32 id, int *nOutSimilarity,
										  int *nRegisterQuality, zpl_bool *nOpenUpload)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKRecognizeConfig stRecognizeConfig;
			memset(&stRecognizeConfig, 0, sizeof(ST_SDKRecognizeConfig));

			ret = EAIS_SDK_GetRecognizeCofig(sdk->handle, &stRecognizeConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetRecognizeCofig ERROR");
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
#else
		return OK;
#endif
	}
	return ERROR;
}

// 安全帽配置
int v9_video_sdk_helmet_config_set_api(zpl_uint32 id, zpl_uint32 ch, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk && data)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKHelmetConfig stHelmetConfig;
			memset(&stHelmetConfig, 0, sizeof(ST_SDKHelmetConfig));

			stHelmetConfig.nHelmetInfoNum = 1;
			memcpy(&stHelmetConfig.stHelmetInfoArr[0], data, sizeof(ST_SDKHelmetInfo));

			ret = EAIS_SDK_SetHelmetConfig(sdk->handle, &stHelmetConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetHelmetConfig ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}
int v9_video_sdk_helmet_config_get_api(zpl_uint32 id, zpl_uint32 ch, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk && data)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKHelmetConfig stHelmetConfig;
			memset(&stHelmetConfig, 0, sizeof(ST_SDKHelmetConfig));

			ret = EAIS_SDK_GetHelmetConfig(sdk->handle, &stHelmetConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetHelmetConfig ERROR");
				return ERROR;
			}
			for(ret = 0; ret < EAIS_SDK_MAX_CHANNEL_SUPPORT; ret++)
			{
				if(stHelmetConfig.stHelmetInfoArr[ret].nChannelId == ch)
				{
					memcpy(data, &stHelmetConfig.stHelmetInfoArr[ret], sizeof(ST_SDKHelmetInfo));
					return OK;
				}
			}
			return ERROR;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

// 抓拍策略配置
int v9_video_sdk_snap_config_set_api(zpl_uint32 id, zpl_uint32 ch, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk && data)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKSnapConfig stHelmetConfig;
			memset(&stHelmetConfig, 0, sizeof(ST_SDKSnapConfig));

			stHelmetConfig.nSnapInfoNum = 1;

			memcpy(&stHelmetConfig.stSnapInfoArr[0], data, sizeof(ST_SDKSnapInfo));
			stHelmetConfig.stSnapInfoArr[0].nChannelId = ch;
			ret = EAIS_SDK_SetHelmetConfig(sdk->handle, &stHelmetConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetHelmetConfig ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}
int v9_video_sdk_snap_config_get_api(zpl_uint32 id, zpl_uint32 ch, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk && data)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKSnapConfig stHelmetConfig;
			memset(&stHelmetConfig, 0, sizeof(ST_SDKSnapConfig));
			ret = EAIS_SDK_GetHelmetConfig(sdk->handle, &stHelmetConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_SetHelmetConfig ERROR");
				return ERROR;
			}
			for(ret = 0; ret < EAIS_SDK_MAX_CHANNEL_SUPPORT; ret++)
			{
				if(stHelmetConfig.stSnapInfoArr[ret].nChannelId == ch)
				{
					memcpy(data, &stHelmetConfig.stSnapInfoArr[ret], sizeof(ST_SDKSnapInfo));
					return OK;
				}
			}
			return ERROR;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

//原图输出
int v9_video_sdk_original_pic_enable_set_api(zpl_uint32 id, zpl_bool enable)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKSnapConfig stSnapConfig;
			memset(&stSnapConfig, 0, sizeof(ST_SDKSnapConfig));
			ret = EAIS_SDK_GetSnapConfig(sdk->handle, &stSnapConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_GetSnapConfig ERROR");
				return ERROR;
			}
			stSnapConfig.nOriginalPicEnable = enable;
			ret = EAIS_SDK_SetSnapConfig(sdk->handle, &stSnapConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_GetSnapConfig ERROR");
				return ERROR;
			}
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_original_pic_enable_get_api(zpl_uint32 id, zpl_bool *enable)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		if(sdk->handle >= 0)
		{
			int ret = 0;
			ST_SDKSnapConfig stSnapConfig;
			memset(&stSnapConfig, 0, sizeof(ST_SDKSnapConfig));
			ret = EAIS_SDK_GetSnapConfig(sdk->handle, &stSnapConfig);
			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_GetSnapConfig ERROR");
				return ERROR;
			}
			if(enable)
				*enable = stSnapConfig.nOriginalPicEnable;
			return OK;
		}
#else
		return OK;
#endif
	}
	return ERROR;
}



int v9_video_sdk_update_api(zpl_uint32 id, char *filename)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		int nDataLen = 0;
		char* pszPacketData = NULL;
		nDataLen = os_file_size(filename);
		if(nDataLen != ERROR && nDataLen > 0)
		{
			pszPacketData = malloc(nDataLen + 1);
			if(!pszPacketData)
				return ERROR;
			memset(pszPacketData, 0, nDataLen + 1);
			ret = os_read_file(filename, pszPacketData, nDataLen);
			if(ret != OK)
			{
				free(pszPacketData);
				return ERROR;
			}
			ret = EAIS_SDK_Update(sdk->address, V9_VIDEO_SDK_UPDATE_PORT,
								  sdk->username, sdk->password, pszPacketData, nDataLen);

			free(pszPacketData);

			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_Update ERROR");
				return ERROR;
			}
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_query_api(zpl_uint32 id, zpl_uint32 ch, zpl_uint32 nStartTime, zpl_uint32 nEndTime, void *data)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk && data)
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
			zlog_err(MODULE_APP,"EAIS_SDK_QueryPeopleCount ERROR");
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
int v9_video_sdk_add_user_api(zpl_uint32 id, zpl_bool gender, int group, char *user, char *ID, char *pic, zpl_bool edit)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		int nDataLen = 0;
		ST_SDKUserInfo pstUserInfo;
		memset(&pstUserInfo, 0, sizeof(ST_SDKUserInfo));
		nDataLen = os_file_size(pic);
		if(nDataLen != ERROR && nDataLen > 0)
		{
			pstUserInfo.szPictureData = malloc(nDataLen + 1);
			if(!pstUserInfo.szPictureData)
				return ERROR;
			memset(pstUserInfo.szPictureData, 0, nDataLen + 1);
			ret = os_read_file(pic, pstUserInfo.szPictureData, nDataLen);
			if(ret != OK)
			{
				free(pstUserInfo.szPictureData);
				return ERROR;
			}

			pstUserInfo.nGroupID = group;										// 所属组ID  0： 黑名单 1： 白名单
			strcpy(pstUserInfo.szUserName, user);			// 姓名
			strcpy(pstUserInfo.szUserID, ID);				// 证件号
			pstUserInfo.nGender = gender;										// 人员性别  0： 女 1： 男
			pstUserInfo.nFaceLen = nDataLen;										// 人脸图片数据长度（1, 1024 * 1024]字节
			//pstUserInfo.szPictureData;									// 图片内容
			//pstUserInfo.szComment[EAIS_SDK_USER_FACE_COMMENT_LEN];		// 用户自定义备注字段
			//pstUserInfo.nRegisterTime;									// 注册时间，从1970-01-01 00:00:00 (utc) 开始计时的秒数
			//pstUserInfo.szReserved[256];								// 预留位，便于拓展，默认置空

			ret = EAIS_SDK_EditUser(sdk->handle, &pstUserInfo, edit ? zpl_false:zpl_true);

			free(pstUserInfo.szPictureData);

			if(ret != EAIS_SDK_SUCCESS)
			{
				zlog_err(MODULE_APP,"EAIS_SDK_EditUser ERROR");
				return ERROR;
			}
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}

// 删除用户
// 根据用户ID删除用户
int v9_video_sdk_del_user_api(zpl_uint32 id, char *ID)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		ret = EAIS_SDK_DelUser(sdk->handle, ID);
		if(ret != EAIS_SDK_SUCCESS)
		{
			zlog_err(MODULE_APP,"EAIS_SDK_DelUser ERROR");
			return ERROR;
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}


// 批量删除某个库的用户
//LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_DelGroupUserList(int p_nLoginHandle,  ST_SDKUserList* p_pstUserList);

int v9_video_sdk_del_group_user_api(zpl_uint32 id, void *p_pstUserList)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		ret = EAIS_SDK_DelGroupUserList(sdk->handle, p_pstUserList);
		if(ret != EAIS_SDK_SUCCESS)
		{
			zlog_err(MODULE_APP,"EAIS_SDK_DelGroup ERROR");
			return ERROR;
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}

// 删除库 p_nGroupID : -1 删除所有分组人脸库信息， 0 删除黑名单所有人脸信息，1 删除白名单所有人脸信息
int v9_video_sdk_del_group_api(zpl_uint32 id, int group)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		ret = EAIS_SDK_DelGroup(sdk->handle, group);
		if(ret != EAIS_SDK_SUCCESS)
		{
			zlog_err(MODULE_APP,"EAIS_SDK_DelGroup ERROR");
			return ERROR;
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_get_user_api(zpl_uint32 id, char* ID, void* UserInfo)
{
	v9_video_sdk_t *sdk = v9_video_sdk_lookup(id);
	if(v9_sdk_data && sdk)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		ret = EAIS_SDK_GetUser(sdk->handle, ID, UserInfo);
		if(ret != EAIS_SDK_SUCCESS)
		{
			zlog_err(MODULE_APP,"EAIS_SDK_DelGroup ERROR");
			return ERROR;
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}

// 修改(新增)用户 p_bNewUser false:修改用户 true:新用户
int v9_video_sdk_add_user_all_api(zpl_bool gender, int group, char *user, char *ID, char *pic, zpl_bool edit)
{
	zpl_uint32 i = 0;
	if(v9_sdk_data)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0;
		int nDataLen = 0;
		ST_SDKUserInfo pstUserInfo;
		memset(&pstUserInfo, 0, sizeof(ST_SDKUserInfo));
		nDataLen = os_file_size(pic);
		if(nDataLen != ERROR && nDataLen > 0)
		{
			pstUserInfo.szPictureData = malloc(nDataLen + 1);
			if(!pstUserInfo.szPictureData)
				return ERROR;
			memset(pstUserInfo.szPictureData, 0, nDataLen + 1);
			ret = os_read_file(pic, pstUserInfo.szPictureData, nDataLen);
			if(ret != OK)
			{
				free(pstUserInfo.szPictureData);
				return ERROR;
			}

			pstUserInfo.nGroupID = group;										// 所属组ID  0： 黑名单 1： 白名单
			strcpy(pstUserInfo.szUserName, user);			// 姓名
			strcpy(pstUserInfo.szUserID, ID);				// 证件号
			pstUserInfo.nGender = gender;										// 人员性别  0： 女 1： 男
			pstUserInfo.nFaceLen = nDataLen;										// 人脸图片数据长度（1, 1024 * 1024]字节
			//pstUserInfo.szPictureData;									// 图片内容
			//pstUserInfo.szComment[EAIS_SDK_USER_FACE_COMMENT_LEN];		// 用户自定义备注字段
			//pstUserInfo.nRegisterTime;									// 注册时间，从1970-01-01 00:00:00 (utc) 开始计时的秒数
			//pstUserInfo.szReserved[256];								// 预留位，便于拓展，默认置空

			for(i = 0; i < V9_APP_BOARD_MAX; i++)
			{
				if(v9_sdk_data[i].login && v9_sdk_data[i].handle >= 0)
				{
					ret |= EAIS_SDK_EditUser(v9_sdk_data[i].handle, &pstUserInfo, edit ? zpl_false:zpl_true);
					if(ret != EAIS_SDK_SUCCESS)
					{
						zlog_err(MODULE_APP,"EAIS_SDK_EditUser ERROR");
						free(pstUserInfo.szPictureData);
						return ERROR;
					}
				}
			}
			free(pstUserInfo.szPictureData);
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}

// 删除用户
// 根据用户ID删除用户
int v9_video_sdk_del_user_all_api(char *ID)
{
	if(v9_sdk_data)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0, i = 0;
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if(v9_sdk_data[i].login && v9_sdk_data[i].handle >= 0)
			{
				ret |= EAIS_SDK_DelUser(v9_sdk_data[i].handle, ID);
				if(ret != EAIS_SDK_SUCCESS)
				{
					zlog_err(MODULE_APP,"EAIS_SDK_DelUser ERROR");
					return ERROR;
				}
			}
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}


// 批量删除某个库的用户
//LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_DelGroupUserList(int p_nLoginHandle,  ST_SDKUserList* p_pstUserList);

int v9_video_sdk_del_group_user_all_api(void *p_pstUserList)
{
	if(v9_sdk_data )
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0, i = 0;
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if(v9_sdk_data[i].login && v9_sdk_data[i].handle >= 0)
			{
				ret |= EAIS_SDK_DelGroupUserList(v9_sdk_data[i].handle, p_pstUserList);
				if(ret != EAIS_SDK_SUCCESS)
				{
					zlog_err(MODULE_APP,"EAIS_SDK_DelGroup ERROR");
					return ERROR;
				}
			}
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}

// 删除库 p_nGroupID : -1 删除所有分组人脸库信息， 0 删除黑名单所有人脸信息，1 删除白名单所有人脸信息
int v9_video_sdk_del_group_all_api(int group)
{
	if(v9_sdk_data)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0, i = 0;
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if(v9_sdk_data[i].login && v9_sdk_data[i].handle >= 0)
			{
				ret |= EAIS_SDK_DelGroup(v9_sdk_data[i].handle, group);
				if(ret != EAIS_SDK_SUCCESS)
				{
					zlog_err(MODULE_APP,"EAIS_SDK_DelGroup ERROR");
					return ERROR;
				}
			}
		}
		return OK;
#else
		return OK;
#endif
	}
	return ERROR;
}

int v9_video_sdk_get_user_all_api(char* ID, void* UserInfo)
{
	if(v9_sdk_data)
	{
#ifdef V9_VIDEO_SDK_API
		int ret = 0, i = 0;
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if(v9_sdk_data[i].login && v9_sdk_data[i].handle >= 0)
			{
				ret |= EAIS_SDK_GetUser(v9_sdk_data[i].handle, ID, UserInfo);
				if(ret == EAIS_SDK_SUCCESS)
				{
					return OK;
				}
			}
		}
/*		int ret = 0;
		ret = EAIS_SDK_GetUser(sdk->handle, ID, UserInfo);
		if(ret != EAIS_SDK_SUCCESS)
		{
			zlog_err(MODULE_APP,"EAIS_SDK_GetUser ERROR");
		}*/
		zlog_err(MODULE_APP,"EAIS_SDK_GetUser ERROR");
		return ERROR;
#else
		return OK;
#endif
	}
	return ERROR;
}

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


#if 0
int v9_video_sdk_time_api(zpl_uint32 timesp)
{
#ifdef V9_EAIS_SDK
	if(v9_sdk && v9_sdk->fd >= 0)
	{
	    struct tm *p_tm = NULL;
	    zpl_uint32 ptimesp = timesp;
	    ST_SDKDeviceTime p_pstDeviceTime;
	    p_tm = localtime(&ptimesp);

	    p_pstDeviceTime.nYear = (p_tm->tm_year + 1900);
	    p_pstDeviceTime.nMonth = (p_tm->tm_mon + 1);
	    p_pstDeviceTime.nDay = (p_tm->tm_mday);
	    p_pstDeviceTime.nHour = (p_tm->tm_hour);
	    p_pstDeviceTime.nMinute = (p_tm->tm_min);
	    p_pstDeviceTime.nSecond = (p_tm->tm_sec);

		if(EAIS_SDK_SetDeviceTime(v9_sdk->fd, &p_pstDeviceTime) == EAIS_SDK_SUCCESS)
			return OK;
		else
		{
			os_util_error("EAIS_SDK_SetDeviceTime ERROR");
		}
	}
	return ERROR;
#else
	return OK;
#endif
}
#endif
