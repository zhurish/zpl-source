/*****************************************************************************

		版权所有 (C),2018-2025  开放智能机器（上海）有限公司
							OPEN AI LAB

*****************************************************************************
 文 件 名   : EAIS_SDKApi.h
 作    者   : 陈润城
 生成日期   : 2019年7月6日
 功能描述   : 定义EAIS SDK对外接口
******************************************************************************/

#ifndef __EAIS_SDKAPI_H__
#define __EAIS_SDKAPI_H__

#include "EAIS_SDKDef.h"

/************************************************
	回调函数：抓拍/状态/EAIS设备搜索/Onvif设备搜索
	1) 回调函数不能有阻塞和耗时操作	
	2) 不能在回调函数内再调用SDK接口
************************************************/
typedef int (__stdcall* EAIS_SDKSnapCallBack)		(ST_SDKStructInfo* p_pstStructInfo, void* p_pUserData);
typedef int (__stdcall* EAIS_SDKStatusCallBack)		(ST_SDKStatusInfo* p_pstStatusInfo, void* p_pUserData);
typedef int (__stdcall* EAIS_SDKDiscoveryCallBack)	(ST_SDKDeviceInfo* p_pstDeviceInfo, void* p_pUserData);
typedef int (__stdcall* EAIS_ONVIFDiscoveryCallBack)(ST_OnvifSearchInfo* p_pstSearchInfo, void* p_pUserData);

// 获取SDK版本号
LIB_SDK_DLL_EXPORT char* __stdcall EAIS_SDK_GetSDKVersion();

// 初始化/去初始化
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_Init(int p_nMaxDevNum);
LIB_SDK_DLL_EXPORT void __stdcall EAIS_SDK_UnInit();

// 回调注册：设备状态/图片流
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetStatusCallBack(int p_nLoginHandle, EAIS_SDKStatusCallBack p_fStatusCallBack, void* p_pUserData);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetSnapCallBack(int p_nLoginHandle, EAIS_SDKSnapCallBack p_fSnapCallback, void* p_pUserData);

// 登录/登出
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_Login(const char* p_pszDeviceIP, int p_nPort, const char* p_pszUserName, const char* p_pszPWD, int p_nTimeOut);
LIB_SDK_DLL_EXPORT void __stdcall EAIS_SDK_Logout(int p_nLoginHandle);

// 升级：同步接口，预计6~10秒完成，设备自动重启，默认升级端口号为：40002
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_Update(const char* p_pszDeviceIP, int p_nPort, const char* p_pszUserName, const char* p_pszPWD, const char* p_pszPacketData, int p_nDataLen);

// EAIS设备搜索、设置设备IP
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_StartDiscovery(EAIS_SDKDiscoveryCallBack p_fDiscoveryCallback, void* p_pUserData);
LIB_SDK_DLL_EXPORT void __stdcall EAIS_SDK_StopDiscovery();
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_UDPSetIpConfig(const ST_SDKIPConfig* p_pstIPConfig);
// Onvif (获取URL超时单位/秒)
LIB_SDK_DLL_EXPORT int __stdcall EAIS_ONVIF_StartDiscovery(EAIS_ONVIFDiscoveryCallBack p_fDiscoveryCallback, void* p_pUserData);
LIB_SDK_DLL_EXPORT void __stdcall EAIS_ONVIF_StopDiscovery();
LIB_SDK_DLL_EXPORT int __stdcall EAIS_ONVIF_GetRtspStreamUri(const char* p_pszDeviceSerAddr, const char* p_pszUserName, const char* p_pszPWD, int p_nStreamId, int p_nTimeout, char* p_pszStreamUri);

// 请求/关闭抓拍信息
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_OpenSnapStream(int p_nLoginHandle, int p_nDataTypePolicy);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_CloseSnapStream(int p_nLoginHandle);

// 获取EAIS设备信息
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetDeviceInfo(int p_nLoginHandle, ST_SDKDeviceInfo* p_pstDeviceInfo);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetDeviceName(int p_nLoginHandle, char* p_pszDeviceName);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetDeviceName(int p_nLoginHandle, const char* p_pszDeviceName);

// 获取/设置RTSP参数: 获取默认全部, 设置支持单个通道、多个通道及全部通道
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetRtspConfig(int p_nLoginHandle, ST_SDKRTSPConfig* p_pstRTSPConfig);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetRtspConfig(int p_nLoginHandle, const ST_SDKRTSPConfig* p_pstRTSPConfig);

// 获取/设置抓拍图片上传服务器地址参数: 通过p_nType区分 0:ftp，1：http
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetSnapServerAddr(int p_nLoginHandle, int p_nType, ST_SDKSnapServerAddr* p_pstSnapServerAddr);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetSnapServerAddr(int p_nLoginHandle, int p_nType, const ST_SDKSnapServerAddr* p_pstSnapServerAddr);

// 获取/设置抓拍配置: 获取默认全部, 设置支持单个通道、多个通道及全部通道
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetSnapConfig(int p_nLoginHandle, ST_SDKSnapConfig* p_pstSnapConfig);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetSnapConfig(int p_nLoginHandle, const ST_SDKSnapConfig* p_pstSnapConfig);

// 获取/设置EAIS设备IPAddr信息: 获取默认全部, 设置支持单个通道、多个通道及全部通道
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetIpConfig(int p_nLoginHandle, ST_SDKIPConfig* p_pstIPConfig);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetIpConfig(int p_nLoginHandle, const ST_SDKIPConfig* p_pstIPConfig);

// 获取/设置EAIS设备时间
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetDeviceTime(int p_nLoginHandle, ST_SDKDeviceTime* p_pstDeviceTime);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetDeviceTime(int p_nLoginHandle, const ST_SDKDeviceTime* p_pstDeviceTime);

// 设置显示模式： 0表示显示全通道画面, 其他则表示单个通道
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetDisplayMode(int p_nLoginHandle, int p_nChannelId);

// EAIS设备重启
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_DeviceReboot(int p_nLoginHandle);

// 修改用户密码
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetPassWord(int p_nLoginHandle, const ST_Account* p_pstAccount);

// 恢复出厂
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_FactoryReset(int p_nLoginHandle, const ST_SDKResetInfo* p_pstResetInfo);

// NTP配置
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetNTPInfo(int p_nLoginHandle, ST_SDKNTPInfo* p_pstNTPInfo);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetNTPInfo(int p_nLoginHandle, const ST_SDKNTPInfo* p_pstNTPInfo);

// 获取图片特征向量
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetPictureFeature(int p_nLoginHandle, const ST_SDKPictureInfo* p_pstPictureInfo, ST_SDKSnapFea* p_pstSnapFea);

// 获取特征值余弦相似度
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetSosineSimilarity(const float* p_fFirstArray, const float* p_fSecondArray, int p_nlength, float* p_fResult);

// 获取/设置安全帽配置: 获取默认全部, 设置支持单个通道、多个通道及全部通道
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetHelmetConfig(int p_nLoginHandle, ST_SDKHelmetConfig* p_pstHelmetConfig);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetHelmetConfig(int p_nLoginHandle, const ST_SDKHelmetConfig* p_pstHelmetConfig);

// 抓拍识别方案
// 用户增删改查
// 修改(新增)用户 p_bNewUser false:修改用户 true:新用户
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_EditUser(int p_nLoginHandle, ST_SDKUserInfo* p_pstUserInfo, bool p_bNewUser); 

// 删除用户
// 根据用户ID删除用户
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_DelUser(int p_nLoginHandle, char* p_pszUserID);

// 批量删除某个库的用户
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_DelGroupUserList(int p_nLoginHandle,  ST_SDKUserList* p_pstUserList);

// 删除库 p_nGroupID : -1 删除所有分组人脸库信息， 0 删除黑名单所有人脸信息，1 删除白名单所有人脸信息
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_DelGroup(int p_nLoginHandle, int p_nGroupID);

// 查询用户 当用户不存在时返回-3
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetUser(int p_nLoginHandle, char* p_pszUserID, ST_SDKUserInfo* p_pstUserInfo);

// 功能描述：查询一个组内部分人员信息 当组为空时返回-3
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetGroupUsers(int p_nLoginHandle, int p_nGroupID, int p_nPageIndex,  ST_SDKUserData* p_pstUserInfoList);

// 查询组名信息
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_QueryGroupNameList(int p_nLoginHandle, ST_SDKGroupInfoList* p_pstGroupInfoList);

// 设置组名信息
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetGroupNameList(int p_nLoginHandle, const ST_SDKGroupInfoList* p_pstGroupInfoList);

// 获取/设置人脸识别配置参数
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetRecognizeCofig(int p_nLoginHandle, ST_SDKRecognizeConfig* p_pstRecognizeConfig);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetRecognizeCofig(int p_nLoginHandle, const ST_SDKRecognizeConfig* p_pstRecognizeConfig);

// 查询人数统计信息
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_QueryPeopleCount(int p_nLoginHandle, const ST_SDKQueryPeopleReq* p_pstQueryPeopleReq, ST_SDKPeopleCount* p_pstPeopleCount);

// 获取/设置报警配置: p_pChannelID: -1代表获取所有通道
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetAlarmConfig(int p_nLoginHandle, int p_pChannelID, ST_SDKAlarmConfig* p_pstAlarmConfig);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetAlarmConfig(int p_nLoginHandle, const ST_SDKAlarmConfig* p_pstAlarmConfig);

// 设置ROI
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetROIArea(int p_nLoginHandle, int p_pChannelID, ST_SDKROIArea* p_pstROIArea);
LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_SetROIArea(int p_nLoginHandle, const ST_SDKROIArea* p_pstROIArea);

#endif
