#ifndef __MODEM_CLIENT_H__
#define __MODEM_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "zplos_include.h"


#define PTZ_CMD_DEBUG_SEND      0X01
#define PTZ_CMD_DEBUG_RECV      0X02
#define PTZ_CMD_DEBUG_DETAIL    0X80

typedef enum ptz_protocol_s
{
	PTZ_PROTOCOL_P = 0,
	PTZ_PROTOCOL_D = 1,
}ptz_protocol_e;


#define PTZ_CMD_D_LEN      7
#define PTZ_CMD_P_LEN      8


/* CMD1 */
#define PTZ_CMD_D_SENSE           1<<7
#define PTZ_CMD_D_AUTOSCAN_ON     1<<4
#define PTZ_CMD_D_CAMERA_ONOFF    1<<3
#define PTZ_CMD_D_IRIS_CLOSE      1<<2
#define PTZ_CMD_D_IRIS_OPEN       1<<1
#define PTZ_CMD_D_FOCUS_NEAR      1<<0

/* CMD2 */
#define PTZ_CMD_D_FOCUS_FAR       1<<7
#define PTZ_CMD_D_ZOOM_WIDE       1<<6
#define PTZ_CMD_D_ZOOM_TELE       1<<5
#define PTZ_CMD_D_TILT_DOWN       1<<4
#define PTZ_CMD_D_TILT_UP         1<<3
#define PTZ_CMD_D_PAN_LEFT        1<<2
#define PTZ_CMD_D_PAN_RIGHT       1<<1


typedef struct 
{
#define PTZ_CMD_D_STX        0XFF
    zpl_uint8 stx;		    //同步开始
	zpl_uint8 addr;		    //解码地址
	zpl_uint8 cmd1;		    //命令1
	zpl_uint8 cmd2;	        //指令2
	zpl_uint8 data1; 	    //数据1
	zpl_uint8 data2;        //数据2
	zpl_uint8 mod;
} ptz_cmd_pelco_d_t;

/* CMD1 */
#define PTZ_CMD_P_CAMERA_ON       1<<6
#define PTZ_CMD_P_AUTOSCAN_ON     1<<5
#define PTZ_CMD_P_CAMERA_ONOFF    1<<4
#define PTZ_CMD_P_IRIS_CLOSE      1<<3
#define PTZ_CMD_P_IRIS_OPEN       1<<2
#define PTZ_CMD_P_FOCUS_NEAR      1<<1
#define PTZ_CMD_P_FOCUS_FAR       1<<0
/* CMD2 */
#define PTZ_CMD_P_ZOOM_WIDE       1<<6
#define PTZ_CMD_P_ZOOM_TELE       1<<5
#define PTZ_CMD_P_TILT_DOWN       1<<4
#define PTZ_CMD_P_TILT_UP         1<<3
#define PTZ_CMD_P_PAN_LEFT        1<<2
#define PTZ_CMD_P_PAN_RIGHT       1<<1

typedef struct 
{
#define PTZ_CMD_P_STX        0XA0
#define PTZ_CMD_P_ETX        0XAF
    zpl_uint8 stx;		    //同步开始
	zpl_uint8 addr;		    //解码地址
	zpl_uint8 cmd1;		    //命令1
	zpl_uint8 cmd2;	        //指令2
	zpl_uint8 data1; 	    //数据1
	zpl_uint8 data2;        //数据2
    zpl_uint8 etx;		    //同步结束
    zpl_uint8 xor;		    //校验码
} ptz_cmd_pelco_p_t;


typedef struct 
{
    zpl_uint8       hdr;
    zpl_uint8       end;
    zpl_uint8       data[16];
    zpl_uint8       len;
    zpl_uint8       flag;
}ptz_data_t;

typedef struct 
{
    int             istty;      //串口
    zpl_uint8       address;    //本机地址
    struct tty_com 	sttty;      //串口通道数据
    void            *t_read;
    ptz_protocol_e  protocol;
    zpl_uint8       maxlen;
    ptz_data_t      ptzdata;
    int             (*ptz_data_cb)(void *, zpl_uint8 *, zpl_uint8);

    zpl_uint32      debug;    
}ptz_channel_t;

int ptz_serial_isopen(ptz_channel_t *client);
int ptz_serial_isclose(ptz_channel_t *client);
int ptz_serial_open(ptz_channel_t *client);
int ptz_serial_close(ptz_channel_t *client);

int ptz_serial_protocol(ptz_channel_t *client, ptz_protocol_e pro);
int ptz_serial_address(ptz_channel_t *client, zpl_uint8 address);


int ptz_serial_read_start(zpl_void *master, ptz_channel_t *client);
int ptz_serial_read_stop(ptz_channel_t *client);

/* 设置，清除，调用预置点命令 */
int ptz_serial_preset_positions_set_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd);
int ptz_serial_preset_positions_clear_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd);
int ptz_serial_preset_positions_call_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd);

int ptz_serial_trun_down_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown);
int ptz_serial_trun_up_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown);
int ptz_serial_trun_left_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 leftright);
int ptz_serial_trun_right_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 leftright);
int ptz_serial_trun_left_down_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown, zpl_uint8 leftright);
int ptz_serial_trun_left_up_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown, zpl_uint8 leftright);
int ptz_serial_trun_right_down_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown, zpl_uint8 leftright);
int ptz_serial_trun_right_up_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown, zpl_uint8 leftright);


//变倍+
int ptz_serial_zoom_wide_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed);
//变倍-
int ptz_serial_zoom_tele_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed);

//聚焦+
int ptz_serial_focus_near_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed);
//聚焦-
int ptz_serial_focus_far_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed);

//光圈+
int ptz_serial_iris_close_cmd(ptz_channel_t *client, zpl_uint8 addr);
//光圈-
int ptz_serial_iris_open_cmd(ptz_channel_t *client, zpl_uint8 addr);


int ptz_serial_stop_cmd(ptz_channel_t *client, zpl_uint8 addr);

//灯光关
int ptz_serial_lighting_close_cmd(ptz_channel_t *client, zpl_uint8 addr);
//灯光开
int ptz_serial_lighting_open_cmd(ptz_channel_t *client, zpl_uint8 addr);

//自动巡航
int ptz_serial_cruise_control_open_cmd(ptz_channel_t *client, zpl_uint8 addr);
//关闭自动巡航
int ptz_serial_cruise_control_close_cmd(ptz_channel_t *client, zpl_uint8 addr);



int ptz_serial_zoom_speed_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed);
int ptz_serial_focus_speed_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed);
int ptz_serial_alarm_ack_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 no);

int ptz_serial_flip_cmd(ptz_channel_t *client, zpl_uint8 addr);
int ptz_serial_zero_pan_cmd(ptz_channel_t *client, zpl_uint8 addr);
int ptz_serial_remote_reset_cmd(ptz_channel_t *client, zpl_uint8 addr);

int ptz_serial_set_aux_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 val);
int ptz_serial_clear_aux_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 val);

int ptz_serial_zone_start_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 val);
int ptz_serial_zone_stop_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 val);

int ptz_serial_screen_show_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 x, zpl_uint8 val);
int ptz_serial_screen_clear_cmd(ptz_channel_t *client, zpl_uint8 addr);

int ptz_serial_zone_scan_on_cmd(ptz_channel_t *client, zpl_uint8 addr);
int ptz_serial_zone_scan_off_cmd(ptz_channel_t *client, zpl_uint8 addr);

int ptz_serial_pattern_start_cmd(ptz_channel_t *client, zpl_uint8 addr);
int ptz_serial_pattern_stop_cmd(ptz_channel_t *client, zpl_uint8 addr);
int ptz_serial_pattern_running_cmd(ptz_channel_t *client, zpl_uint8 addr);





#ifdef __cplusplus
}
#endif


#endif /* __MODEM_CLIENT_H__ */