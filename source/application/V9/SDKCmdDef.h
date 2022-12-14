#ifndef _SDK_CMD_DEF_H_
#define _SDK_CMD_DEF_H_

/************************************************************************************
	SDK����Ŷ���
	�ο� ��EAIS�豸����Э���ĵ��淶.docx��
************************************************************************************/

// Э��CmdID
enum SDK_CMD_DEF
{
	// ͨ��Ӧ��
	SDK_COM_RES					= 1000,			// ͨ��Ӧ��

	// �������ࡿ : 1001~1999
	// ��¼
	SDK_LOGIN_REQ				= 1001,			// ��¼����
	SDK_LOGIN_RES				= 1002,			// ��¼Ӧ��
	// �ǳ�
	SDK_LOGOUT_REQ				= 1003,        	// ע������
	SDK_LOGOUT_RES				= 1004,			// ע����Ӧ	
	// ����
	SDK_HEART_REQ				= 1005,			// ��������
	SDK_HEART_RES				= 1006,			// ������Ӧ
	// �豸����
	SDK_SEARCH_DEVICE_REQ		= 1007,			// �豸��������
	SDK_SEARCH_DEVICE_RES		= 1008,			// �豸������Ӧ
	// �豸����
	SDK_UPDATE_REQ				= 1009,			// �豸��������
	SDK_UPDATE_RES				= 1010,			// �豸������Ӧ
	SDK_UPDATE_DATA				= 1010,			// �豸��������
	SDK_UPDATE_NOTIFY			= 1011,			// �豸����״̬�ϱ���������Ӧ
	// ��������
	SDK_FACTORYTEST_REQ			= 1012,			// ��������������
	SDK_FACTORYTEST_RES			= 1013,			// ������������Ӧ


	// ���豸�����ࡿ : 2001~2999	
	// ��ȡ�豸��Ϣ
	SDK_GET_DEVICEINFO_REQ		= 2001,			// ��ȡ�豸��Ϣ����
	SDK_GET_DEVICEINFO_RES		= 2002,			// ��ȡ�豸��Ϣ��Ӧ
	// RTSP����
	SDK_GET_RTSPCONFIG_REQ		= 2003,			// ��ȡRTSP��������
	SDK_GET_RTSPCONFIG_RES		= 2004,			// ��ȡRTSP������Ӧ
	SDK_SET_RTSPCONFIG_REQ		= 2005,			// ����RTSP��������
	SDK_SET_RTSPCONFIG_RES		= 2006,			// ����RTSP������Ӧ

	SDK_GET_IPCONFIG_REQ		= 2007,			// ��ȡIP��������
	SDK_GET_IPCONFIG_RES		= 2008,			// ��ȡIP������Ӧ
	SDK_SET_IPCONFIG_REQ		= 2009,			// ����IP��������
	SDK_SET_IPCONFIG_RES		= 2010,			// ����IP������Ӧ

	SDK_GET_DEVICETIME_REQ		= 2011,			// ��ȡ�豸ʱ������
	SDK_GET_DEVICETIME_RES		= 2012,			// ��ȡ�豸ʱ����Ӧ
	SDK_SET_DEVICETIME_REQ		= 2013,			// �����豸ʱ������
	SDK_SET_DEVICETIME_RES		= 2014,			// �����豸ʱ����Ӧ

	SDK_DEVICE_REBOOT_REQ		= 2015,			// �豸��������
	SDK_DEVICE_REBOOT_RES		= 2016,			// �豸������Ӧ

	SDK_GET_SNAPSERVERADDR_REQ  = 2017,			// ��ȡץ�ķ�������ַ����
	SDK_GET_SNAPSERVERADDR_RES	= 2018,			// ��ȡץ�ķ�������ַ��Ӧ
	SDK_SET_SNAPSERVERADDR_REQ	= 2019,			// ����ץ�ķ�������ַ����
	SDK_SET_SNAPSERVERADDR_RES	= 2020,			// ����ץ�ķ�������ַ��Ӧ

	// �豸����
	SDK_GET_DEVICENAME_REQ		= 2021,			// ��ȡ�豸��������
	SDK_GET_DEVICENAME_RES		= 2022,			// ��ȡ�豸������Ӧ
	SDK_SET_DEVICENAME_REQ		= 2023,			// �����豸��������
	SDK_SET_DEVICENAME_RES		= 2024,			// �����豸������Ӧ

	// �����û�����
	SDK_SET_PASSWORD_REQ		= 2025,			// �����û���������
	SDK_SET_PASSWORD_RES		= 2026,			// �����û�������Ӧ

	// �豸SN
	SDK_GET_DEVICESN_REQ		= 2027,			// ��ȡ�豸SN����
	SDK_GET_DEVICESN_RES		= 2028,			// ��ȡ�豸SN��Ӧ
	SDK_SET_DEVICESN_REQ		= 2029,			// �����豸SN����
	SDK_SET_DEVICESN_RES		= 2030,			// �����豸SN��Ӧ
	
	// ��������
	SDK_GET_MANUFACTURER_REQ	= 2031,			// ��ȡ�豸������������
	SDK_GET_MANUFACTURER_RES	= 2032,			// ��ȡ�豸����������Ӧ
	SDK_SET_MANUFACTURER_REQ	= 2033,			// �����豸������������
	SDK_SET_MANUFACTURER_RES	= 2034,			// �����豸����������Ӧ

	// ��־���
	SDK_OPEN_DEVICE_LOG_REQ		= 2035,			// ���豸��־����
	SDK_OPEN_DEVICE_LOG_RES		= 2036,			// ���豸��־��Ӧ
	SDK_CLOSE_DEVICE_LOG_REQ	= 2037,			// �ر��豸��־����
	SDK_CLOSE_DEVICE_LOG_RES	= 2038,			// �ر��豸��־��Ӧ
	SDK_GET_DEVICE_LOG_CFG_REQ	= 2039,			// ��ȡ�豸��־��������
	SDK_GET_DEVICE_LOG_CFG_RES	= 2040,			// ��ȡ�豸��־������Ӧ
	SDK_SET_DEVICE_LOG_CFG_REQ	= 2041,			// �����豸��־��������
	SDK_SET_DEVICE_LOG_CFG_RES	= 2042,			// �����豸��־������Ӧ
	SDK_LOAD_LOG_FILE_REQ		= 2043,			// �����豸��־�ļ�����
	SDK_LOAD_LOG_FILE_RES		= 2044,			// �����豸��־�ļ���Ӧ
	SDK_LOAD_LOG_FILE_DATA		= 2045,			// �����豸��־�ļ�����
	SDK_DEVICELOG_NOTIFY		= 2050,			// �ϴ�EAIS��־���ص���

	// ���֡��
	SDK_GET_FRAME_RATE_REQ		= 2051,			// ��ȡ��ǰ���֡������
	SDK_GET_FRAME_RATE_RES		= 2052,			// ��ȡ��ǰ���֡����Ӧ
	SDK_SET_FRAME_RATE_REQ		= 2053,			// ���õ�ǰ���֡������
	SDK_SET_FRAME_RATE_RES		= 2054,			// ���õ�ǰ���֡����Ӧ

	// �ָ���������
	SDK_FACTORY_RESET_REQ		= 2055,			// �ָ�������������
	SDK_FACTORY_RESET_RES		= 2056,			// �ָ�����������Ӧ

	// �豸Mac
	SDK_GET_MAC_REQ				= 2057,			// ��ȡ�豸Mac����
	SDK_GET_MAC_RES				= 2058,			// ��ȡ�豸Mac��Ӧ
	SDK_SET_MAC_REQ				= 2059,			// �����豸Mac����
	SDK_SET_MAC_RES				= 2060,			// �����豸Mac��Ӧ

	// NTP
	SDK_GET_NTP_REQ				= 2061,			// ��ȡ�豸NTP����
	SDK_GET_NTP_RES				= 2062,			// ��ȡ�豸NTP��Ӧ
	SDK_SET_NTP_REQ				= 2063,			// �����豸NTP����
	SDK_SET_NTP_RES				= 2064,			// �����豸NTP��Ӧ

	SDK_GET_VIDEO_NUM_REQ		= 2065,			// ��ȡ��Ƶ·����������
	SDK_GET_VIDEO_NUM_RES		= 2066,			// ��ȡ��Ƶ·��������Ӧ
	SDK_SET_VIDEO_NUM_REQ		= 2067,			// ������Ƶ·����������
	SDK_SET_VIDEO_NUM_RES		= 2068,			// ������Ƶ·��������Ӧ
	


	// ��״̬/���������ϱ��ࡿ : 3001~3999		
	// ״̬
	SDK_DEVICESTATUS_NOTIFY		= 3001,			// �豸״̬����ϱ�

	// ��ʵʱ/�ط���Ƶ�ࡿ : 4001~4999		
	SDK_GET_DISPLAYMODE_REQ		= 4001,			// ��ȡ��ʾģʽ����
	SDK_GET_DISPLAYMODE_RES		= 4002,			// ��ȡ��ʾģʽ��Ӧ
	SDK_SET_DISPLAYMODE_REQ		= 4003,			// ������ʾģʽ����
	SDK_SET_DISPLAYMODE_RES		= 4004,			// ������ʾģʽ��Ӧ

	// ��ץ���ࡿ : 5001~5999	
	// ץ������
	SDK_GET_SNAPCONFIG_REQ		= 5001,			// ��ȡץ����������
	SDK_GET_SNAPCONFIG_RES		= 5002,			// ��ȡץ��������Ӧ
	SDK_SET_SNAPCONFIG_REQ		= 5003,			// ����ץ����������
	SDK_SET_SNAPCONFIG_RES		= 5004,			// ����ץ��������Ӧ
	// ��/�ر�ץ����
	SDK_OPENSNAP_REQ			= 5005,			// ��ץ��������
	SDK_OPENSNAP_RES			= 5006,			// ��ץ������Ӧ
	SDK_CLOSESNAP_REQ			= 5007,			// �ر�ץ��������
	SDK_CLOSESNAP_RES			= 5008,			// �ر�ץ������Ӧ
	// ץ�Ľṹ�������ϱ�
	SDK_STRUCTDATA_NOTIFY		= 5009,			// ץ�Ľṹ�������ϱ�
	// ��ȡ/����ץ�ı������
	SDK_GET_FACEENCODE_REQ		= 5010,			// ��ȡץ�ı����������
	SDK_GET_FACEENCODE_RES		= 5011,			// ��ȡץ�ı��������Ӧ
	SDK_SET_FACEENCODE_REQ		= 5012,			// ����ץ�ı����������
	SDK_SET_FACEENCODE_RES		= 5013,			// ����ץ�ı��������Ӧ

	// ��ȡ��������ֵ
	SDK_GET_FACEFEA_REQ			= 5014,			// ��ȡͼƬ��������
	SDK_GET_FACEFEA_RES			= 5015,			// ��ȡͼƬ����������Ӧ
	SDK_FACE_DATA				= 5016,			// ͼƬ����
	SDK_FACEFEA_NOTIFY			= 5017,			// ���������ϱ�

	// ��ȫñ����
	SDK_GET_HELMETCONFIG_REQ    = 5018,			// ��ȡ��ȫñ��������
	SDK_GET_HELMETCONFIG_RES    = 5019,			// ��ȡ��ȫñ������Ӧ
	SDK_SET_HELMETCONFIG_REQ    = 5020,			// ���ð�ȫñ��������
	SDK_SET_HELMETCONFIG_RES    = 5021,			// ���ð�ȫñ������Ӧ

	// HDMI
	SDK_GET_HDMISTATUS_REQ		= 5022,			// ��ȡHDMI״̬����
	SDK_GET_HDMISTATUS_RES		= 5023,			// ��ȡHDMI״̬��Ӧ
	SDK_SET_HDMISTATUS_REQ		= 5024,			// ����HDMI״̬����
	SDK_SET_HDMISTATUS_RES		= 5025,			// ����HDMI״̬��Ӧ
	
	// �Ƿ�ʹ��ģ���������״̬
	SDK_GET_MODELSTATUS_REQ		= 5026,			// ��ȡģ��״̬����
	SDK_GET_MODELSTATUS_RES		= 5027,			// ��ȡģ��״̬��Ӧ
	SDK_SET_MODELSTATUS_REQ		= 5028,			// ����ģ��״̬����
	SDK_SET_MODELSTATUS_RES		= 5029,			// ����ģ��״̬��Ӧ

	// ��ʶ���ࡿ : 6001~6999	
	SDK_GET_GROUPUSER_REQ       = 6001,			// ��ȡ������Ա��Ϣ����
	SDK_GET_GROUPUSER_RES		= 6002,			// ��ȡ������Ա��Ϣ��Ӧ
	SDK_GROUPUSER_DATA			= 6003,			// ������Ա��Ϣ����

	// �û���ɾ�Ĳ�
	SDK_EDIT_USER_REQ           = 6005,			// ��/����������
	SDK_EDIT_USER_RES           = 6006,			// ��/��������Ӧ

	SDK_DELETE_USER_REQ			= 6007,			// ɾ����������
	SDK_DELETE_USER_RES			= 6008,			// ɾ��������Ӧ
	
	SDK_DELETE_GROUP_REQ		= 6009,			// ɾ������������
	SDK_DELETE_GROUP_RES		= 6010,			// ɾ����������Ӧ

	SDK_DELETE_USER_LIST_REQ	= 6011,			// ɾ�������б�����
	SDK_DELETE_USER_LIST_RES	= 6012,			// ɾ�������б���Ӧ

	SDK_INQURE_USER_REQ			= 6013,			// ��ѯ��������
	SDK_INQURE_USER_RES			= 6014,			// ��ѯ������Ӧ

	SDK_GET_RECOGNIZE_REQ		= 6015,			// ��ȡʶ����������
	SDK_GET_RECOGNIZE_RES       = 6016,			// ��ȡʶ��������Ӧ
	SDK_SET_RECOGNIZE_REQ		= 6017,			// ����ʶ����������
	SDK_SET_RECOGNIZE_RES		= 6018,			// ����ʶ��������Ӧ

	// ��ѯ����ͳ��
	SDK_QUERY_PEOPLE_REQ		= 6019,			// ��ѯ��������
	SDK_QUERY_PEOPLE_RES		= 6020,			// ��ѯ������Ӧ

	// ����Ϣ
	SDK_QUERY_GROUP_NAME_REQ	= 6021,			// ��ѯ����������
	SDK_QUERY_GROUP_NAME_RES	= 6022,			// ��ѯ��������Ӧ

	SDK_SET_GROUP_NAME_REQ		= 6023,			// ��������������
	SDK_SET_GROUP_NAME_RES		= 6024,			// ������������Ӧ


	// ʶ�����ƶ��Ż���ֵ
	SDK_GET_OPTIMIZE_REQ		= 6031,			// ��ȡʶ�����ƶ��Ż���ֵ����
	SDK_GET_OPTIMIZE_RES		= 6032,			// ��ȡʶ�����ƶ��Ż���ֵ��Ӧ
	SDK_SET_OPTIMIZE_REQ		= 6033,			// ����ʶ�����ƶ��Ż���ֵ����
	SDK_SET_OPTIMIZE_RES		= 6034,			// ����ʶ�����ƶ��Ż���ֵ��Ӧ

	// ��������
	SDK_GET_ALARM_REQ			= 6035,			// ��ȡ������������
	SDK_GET_ALARM_RES			= 6036,			// ��ȡ����������Ӧ
	SDK_SET_ALARM_REQ			= 6037,			// ���ñ�����������
	SDK_SET_ALARM_RES			= 6038,			// ���ñ���������Ӧ
	
	// ����ROI
	SDK_GET_ROI_REQ				= 6039,			// ��ȡROI����
	SDK_GET_ROI_RES				= 6040,			// ��ȡROI��Ӧ
	SDK_SET_ROI_REQ				= 6041,			// ����ROI����
	SDK_SET_ROI_RES				= 6042,			// ����ROI��Ӧ


	// ��ɭ
	SDK_GET_ALGORITHM_REQ		= 6043,			// ��ȡ���������
	SDK_GET_ALGORITHM_RES       = 6044,			// ��ȡ�������Ӧ

    SDK_GET_INFRAREDDETECT_REQ  = 6045,			// ��ȡ������������
	SDK_GET_INFRAREDDETECT_RES  = 6046,			// ��ȡ����������Ӧ

	// ˫·�Ž�
	SDK_GET_ALLATTENDANCE_REQ	= 6047,			// ��ȡȫԱ��������
	SDK_GET_ALLATTENDANCE_RES	= 6048,			// ��ȡȫԱ������Ӧ

	SDK_GET_SINGLEATTEN_REQ     = 6049,			// ��ȡ���˿�������
	SDK_GET_SINGLEATTEN_RES     = 6050,			// ��ȡ���˿�����Ӧ

	SDK_GET_CLOUDCONFIG_REQ		= 6051,			// ��ȡ��������Ϣ����
	SDK_GET_CLOUDCONFIG_RES		= 6052,			// ��ȡ��������Ϣ��Ӧ
	SDK_SET_CLOUDCONFIG_REQ     = 6053,			// ������������Ϣ����
	SDK_SET_CLOUDCONFIG_RES		= 6054,			// ������������Ϣ��Ӧ

	SDK_INIT_DB_REQ				= 6055,			// ��ʼ�����ݿ�����
	SDK_INIT_DB_RES				= 6056,			// ��ʼ�����ݿ���Ӧ
};

#endif // _SDK_CMD_DEF_H_