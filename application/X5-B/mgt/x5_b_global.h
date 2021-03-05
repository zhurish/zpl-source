/*
 * x5_b_global.h
 *
 *  Created on: 2019年7月16日
 *      Author: DELL
 */

#ifndef __X5_B_MGT_X5_B_GLOBAL_H__
#define __X5_B_MGT_X5_B_GLOBAL_H__


#ifdef __cplusplus
extern "C" {
#endif

//#define X5B_APP_DATABASE			1
//#define X5B_APP_IO_LOG			1



#define X5B_APP_DEVICE_NAME_MAX			64
#define X5B_APP_DEVICE_IP_MAX			32



typedef enum
{
	OPEN_NONE,
	OPEN_CARD,
	OPEN_FACE,
	OPEN_FACE_AND_CARD,
	OPEN_FACE_OR_CARD,
} x5b_app_opentype_t ;			//开门类型

typedef enum
{
	CUSTOMIZER_NONE,
	CUSTOMIZER_SECOM,
	CUSTOMIZER_HUIFU,
} x5b_app_customizer_t;		//客户定制

typedef enum
{
	APP_SCENE_HOUSING,		//小区场景
	APP_SCENE_BUSSINESS,		//办公场景
	APP_SCENE_COMMERCIAL,		//商业楼宇场景
} x5b_app_scene_t;		//安装场景类型

typedef enum
{
	HOUSING_NONE,
	HOUSING_UNIT,
	HOUSING_WALL,
	HOUSING_PL_BUILDING,
} x5b_app_housing_t;		//小区场景下安装位置


typedef struct
{
	ospl_uint32 waitopen;
	ospl_uint32 openhold;
	ospl_bool	openalarm;
	ospl_uint32 waitclose;
	x5b_app_opentype_t opentype;
	ospl_uint8 wiggins;
	ospl_bool outrelay;
	ospl_bool	tamperalarm;

}x5b_app_open_t;


typedef struct
{
	//yaw角和pitch角
	int 			faceYawLeft;
	int 			faceYawRight;
	int 			facePitchUp;
	int 			facePitchDown;

	//ROI大小
	int 			faceRoiWidth;
	int 			faceRoiHeight;

	//Filter大小
	//int 			faceFilterWidth;
	//int 			faceFilterHeight;

	int 			faceRecordWidth;		//录入人脸大小
	int 			faceRecordHeight;

	int 			faceRecognizeWidth;		//识别人脸大小
	int 			faceRecognizeHeight;

	double  		similarRecord;			//录入阈值
	double  		similarRecognize;		//识别阈值
	double  		similarSecRecognize;	//二级识别阈值
	double  		similarLiving;			//活体阈值 ospl_float
	ospl_bool			livenessSwitch;			//活体检测开关

	int 			faceOKContinuousTime;		//连续识别间隔时间（两次识别间隔成功）
	int 			faceERRContinuousTime;		//连续识别间隔时间（两次识别间隔失败）
}make_face_config_t;


typedef struct x5b_app_global_s
{
	ospl_bool X5CM;
	ospl_bool bluetooth;
	ospl_bool nfc_enable;
	x5b_app_opentype_t		opentype;
	x5b_app_customizer_t	customizer;
	x5b_app_scene_t 		install_scene;		//安装场景类型
	x5b_app_housing_t			housing;
	ospl_bool 					doorcontact;//是否有门磁
	char devicename[X5B_APP_DEVICE_NAME_MAX];		//设备名称
	char location_address[X5B_APP_DEVICE_NAME_MAX];		//设备安装地址

	ospl_bool out_direction;		//进出方向
	char docking_platform_address[X5B_APP_DEVICE_IP_MAX];		//对接平台地址
	char docking_platform_address1[X5B_APP_DEVICE_IP_MAX];		//对接平台地址
	char docking_platform_address2[X5B_APP_DEVICE_IP_MAX];		//对接平台地址

	int	topf;//TOPF检测距离
}x5b_app_global_t;

extern x5b_app_open_t *x5b_app_open;
extern make_face_config_t *x5b_app_face;

extern x5b_app_global_t *x5b_app_global;

extern int x5b_app_global_mode_load();
extern int x5b_app_global_mode_free();
extern int x5b_app_global_config_load(void);
extern int x5b_app_global_mode_config_save(void);

extern int x5b_app_open_config_load(x5b_app_open_t *fct);
extern int x5b_app_open_config_save(void);

extern int x5b_app_face_config_load(make_face_config_t *fct);
extern int x5b_app_face_config_save(void);

extern int x5b_app_global_device_config(void *app, int to);

extern ospl_bool x5b_app_mode_X5CM();
extern int x5b_app_open_mode();
extern int x5b_app_customizer();

int x5b_app_mode_set_api(ospl_bool X5CM);
int x5b_app_open_mode_set_api(x5b_app_opentype_t opentype);
int x5b_app_customizer_set_api(x5b_app_customizer_t customizer);

int x5b_app_bluetooth_set_api(ospl_bool bluetooth);
int x5b_app_bluetooth_get_api();
int x5b_app_nfc_set_api(ospl_bool nfc_enable);
int x5b_app_nfc_get_api();

int x5b_app_housing_set_api(x5b_app_housing_t housing);
x5b_app_housing_t x5b_app_housing_get_api(void);

int x5b_app_scene_set_api(x5b_app_scene_t install_scene);
x5b_app_scene_t x5b_app_scene_get_api(void);

int x5b_app_devicename_set_api(char * devicename);
char * x5b_app_devicename_get_api(void);

int x5b_app_location_address_set_api(char * location_address);
char * x5b_app_location_address_get_api(void);

int x5b_app_out_direction_set_api(ospl_bool out_direction);
ospl_bool x5b_app_out_direction_get_api(void);

int x5b_app_docking_platform_address_set_api(char * address);
char * x5b_app_docking_platform_address_get_api(void);

int x5b_app_docking_platform_address1_set_api(char * address);
char * x5b_app_docking_platform_address1_get_api(void);

int x5b_app_docking_platform_address2_set_api(char * address);
char * x5b_app_docking_platform_address2_get_api(void);

int x5b_app_show_param(struct vty *vty, ospl_uint32 type);

#ifdef __cplusplus
}
#endif

#endif /* __X5_B_MGT_X5_B_GLOBAL_H__ */
