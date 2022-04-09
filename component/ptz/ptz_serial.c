#include "auto_include.h"
#include <zplos_include.h>
#include "lib_include.h"

#include "ptz_cmd.h"
#include "ptz_serial.h"



/****************************************************************************/
static int ptz_debug_hex(zpl_int8 *tatil, zpl_uint8 *buf, zpl_uint8 len)
{
    char datastr[512];
    zpl_uint8 i = 0;
    memset(datastr, 0, sizeof(datastr));
    for(i = 0; i < len; i++)
    {
        sprintf(datastr + i*3, "%02x ", buf[i]);
    }
    zlog_debug(MODULE_OSAL, "PTZ-%s [%s]", tatil, datastr);
    return 0;
}
/****************************************************************************/
int ptz_serial_isopen(ptz_channel_t *client)
{
	zassert (client);
	if(os_strlen(client->sttty->devname))
	{
		if((access(client->sttty->devname, 0) == 0) && (client->sttty->fd > 0))
			return OK;
		if(client->sttty->fd > 0)
		{
			modem_sttty_close(client);
		}
		else
		{
		}
	}
	return ERROR;
}

int ptz_serial_isclose(ptz_channel_t *client)
{
	zassert (client);
	if(client->sttty->fd <= 0)
		return OK;
	return ERROR;
}

int ptz_serial_open(ptz_channel_t *client)
{
	zassert (client);
	if(os_strlen(client->sttty->devname))
	{
		if(tty_com_open(client->sttty) == 0)
			return OK;
	}
	return ERROR;
}

int ptz_serial_close(ptz_channel_t *client)
{
	zassert (client);
	if(tty_com_close(client->sttty) == 0)
		return OK;
	return ERROR;
}

int ptz_serial_protocol(ptz_channel_t *client, ptz_protocol_e pro)
{
   	zassert (client);
	client->protocol = pro;
    if(client->protocol == PTZ_PROTOCOL_P)
    {
        client->maxlen = PTZ_CMD_P_LEN;
        memset(&client->ptzdata, 0, sizeof(ptz_data_t));
        client->ptzdata.hdr = PTZ_CMD_P_STX;
        client->ptzdata.end = PTZ_CMD_P_ETX;
        client->ptzdata.len = 0;
        client->ptzdata.flag = 0;
    }
    else if(client->protocol == PTZ_PROTOCOL_D)
    {
        client->maxlen = PTZ_CMD_D_LEN;
        memset(&client->ptzdata, 0, sizeof(ptz_data_t));
        client->ptzdata.hdr = PTZ_CMD_D_STX;
        client->ptzdata.end = 0;
        client->ptzdata.len = 0;
        client->ptzdata.flag = 0;
    }
	return OK;
}

int ptz_serial_address(ptz_channel_t *client, zpl_uint8 address)
{
   	zassert (client);
	client->address = address;
	return OK;
}

static int ptz_serial_read(struct thread *t)
{
    int ret = 0, data = 0;
	ptz_channel_t *client = THREAD_ARG(t);
    if(client)
    {
        client->t_read = NULL;
        ret = tty_com_read(&client->sttty, &data, 1);
        if(ret)
        {
            if(client->ptzdata.hdr && client->ptzdata.hdr == data)
            {
                client->ptzdata.flag = 1;
                client->ptzdata.len = 0;
            }
            if(client->protocol == PTZ_PROTOCOL_P)
            {
                if(client->ptzdata.end && client->ptzdata.end == data)
                {
                    client->ptzdata.flag = 2;
                }
            }
            if(client->ptzdata.flag)
            {
                client->ptzdata.data[client->ptzdata.len] = data;
                client->ptzdata.len++;
                
                if(client->ptzdata.len == client->maxlen)
                {
                    if(client->address)
                    {
                        if(client->address != client->ptzdata.data[0])
                        {
                            client->ptzdata.flag = 0;
                            client->ptzdata.len = 0;
                        }
                    }
                    client->ptzdata.flag = 0;
                    if(client->ptz_data_cb && client->ptzdata.len)
                    {
                        if((client->debug & PTZ_CMD_DEBUG_RECV) && (client->debug & PTZ_CMD_DEBUG_DETAIL))
                            ptz_debug_hex("recv", client->ptzdata.data, client->ptzdata.len);
                        (client->ptz_data_cb)(client, client->ptzdata.data, client->ptzdata.len);
                    }
                }
            }
        }
        if(client->sttty.fd >= 0)
            client->t_read = thread_add_read(t->master, ptz_serial_read, client, client->sttty.fd);
    }
	return OK;
}


int ptz_serial_read_start(zpl_void *master, ptz_channel_t *client)
{
    zassert(master);
    zassert(client);
    if(master && client && client->sttty.fd)
        client->t_read = thread_add_read(master, ptz_serial_read, client, client->sttty.fd);

	return OK;    
}

int ptz_serial_read_stop(ptz_channel_t *client)
{
    zassert(client);  
    if(client && client->t_read)
    {
        thread_cancel(client->t_read);
        client->t_read = NULL;
    }
    return OK; 
}



static int ptz_serial_preset_positions_set_cmd_make_d(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    zpl_uint32 crc = 0;
    zpl_uchar buf[8] = {0xff,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buf[1] = addr;
    buf[3] = 0x03;
    buf[5] = cmd;
    crc = buf[1] + buf[2] + buf[3] + buf[4] + buf[5];
    buf[6] = (crc%256) & 0xff;
    if((client->debug & PTZ_CMD_DEBUG_SEND) && (client->debug & PTZ_CMD_DEBUG_DETAIL))
        ptz_debug_hex("send", buf, client->maxlen);
    return tty_com_write(&client->sttty, (zpl_uchar *)buf, client->maxlen);
}

static int ptz_serial_preset_positions_clear_cmd_make_d(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    zpl_uint32 crc = 0;
    zpl_uchar buf[8] = {0xff,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buf[1] = addr;
    buf[3] = 0x05;
    buf[5] = cmd;
    crc = buf[1] + buf[2] + buf[3] + buf[4] + buf[5];
    buf[6] = (crc%256) & 0xff;
    if((client->debug & PTZ_CMD_DEBUG_SEND) && (client->debug & PTZ_CMD_DEBUG_DETAIL))
        ptz_debug_hex("send", buf, client->maxlen);
    return tty_com_write(&client->sttty, (zpl_uchar *)buf, client->maxlen);
}

static int ptz_serial_preset_positions_call_cmd_make_d(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    zpl_uint32 crc = 0;
    zpl_uchar buf[8] = {0xff,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buf[1] = addr;
    buf[3] = 0x07;
    buf[5] = cmd;
    crc = buf[1] + buf[2] + buf[3] + buf[4] + buf[5];
    buf[6] = (crc%256) & 0xff;
    if((client->debug & PTZ_CMD_DEBUG_SEND) && (client->debug & PTZ_CMD_DEBUG_DETAIL))
        ptz_debug_hex("send", buf, client->maxlen);
    return tty_com_write(&client->sttty, (zpl_uchar *)buf, client->maxlen);
}


static int ptz_serial_preset_positions_set_cmd_make_p(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    zpl_uint32 crc = 0;
    zpl_uchar buf[8] = {0xA0,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buf[1] = addr;
    buf[3] = 0x03;
    buf[5] = cmd;
    buf[6] = 0XAF;
    crc = buf[1] ^ buf[2] ^ buf[3] ^ buf[4] ^ buf[5];
    buf[7] = (crc%256) & 0xff;
    if((client->debug & PTZ_CMD_DEBUG_SEND) && (client->debug & PTZ_CMD_DEBUG_DETAIL))
        ptz_debug_hex("send", buf, client->maxlen);
    return tty_com_write(&client->sttty, (zpl_uchar *)buf, client->maxlen);
}

static int ptz_serial_preset_positions_clear_cmd_make_p(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    zpl_uint32 crc = 0;
    zpl_uchar buf[8] = {0xff,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buf[1] = addr;
    buf[3] = 0x05;
    buf[5] = cmd;
    buf[6] = 0XAF;
    crc = buf[1] ^ buf[2] ^ buf[3] ^ buf[4] ^ buf[5];
    buf[7] = (crc%256) & 0xff;
    if((client->debug & PTZ_CMD_DEBUG_SEND) && (client->debug & PTZ_CMD_DEBUG_DETAIL))
        ptz_debug_hex("send", buf, client->maxlen);
    return tty_com_write(&client->sttty, (zpl_uchar *)buf, client->maxlen);
}

static int ptz_serial_preset_positions_call_cmd_make_p(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    zpl_uint32 crc = 0;
    zpl_uchar buf[8] = {0xff,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    buf[1] = addr;
    buf[3] = 0x07;
    buf[5] = cmd;
    buf[6] = 0XAF;
    crc = buf[1] ^ buf[2] ^ buf[3] ^ buf[4] ^ buf[5];
    buf[7] = (crc%256) & 0xff;
    if((client->debug & PTZ_CMD_DEBUG_SEND) && (client->debug & PTZ_CMD_DEBUG_DETAIL))
        ptz_debug_hex("send", buf, client->maxlen);
    return tty_com_write(&client->sttty, (zpl_uchar *)buf, client->maxlen);
}

int ptz_serial_preset_positions_set_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    if(client->protocol == PTZ_PROTOCOL_D)
    {
        return ptz_serial_preset_positions_set_cmd_make_d(client, addr, cmd);
    }
    else if(client->protocol == PTZ_PROTOCOL_P)
    {
        return ptz_serial_preset_positions_set_cmd_make_p(client, addr, cmd);
    }
    return ERROR;
}

int ptz_serial_preset_positions_clear_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    if(client->protocol == PTZ_PROTOCOL_D)
    {
        return ptz_serial_preset_positions_clear_cmd_make_d(client, addr, cmd);
    }
    else if(client->protocol == PTZ_PROTOCOL_P)
    {
        return ptz_serial_preset_positions_clear_cmd_make_p(client, addr, cmd);
    }
    return ERROR;
}

int ptz_serial_preset_positions_call_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd)
{
    if(client->protocol == PTZ_PROTOCOL_D)
    {
        return ptz_serial_preset_positions_call_cmd_make_d(client, addr, cmd);
    }
    else if(client->protocol == PTZ_PROTOCOL_P)
    {
        return ptz_serial_preset_positions_call_cmd_make_p(client, addr, cmd);
    }
    return ERROR;
}

static int ptz_serial_cmd_write(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 cmd1, zpl_uint8 cmd2, zpl_uint8 data1, zpl_uint8 data2)
{
    zpl_uint32 crc = 0;
    zpl_uchar buf[32];
    ptz_cmd_pelco_p_t *cmd_data = (ptz_cmd_pelco_p_t *)buf;
    zassert(client); 
    zassert(client->maxlen);  
    zassert(client->sttty.fd);   

    cmd_data->addr = addr;
    cmd_data->cmd1 = cmd1;
    cmd_data->cmd2 = cmd2;
    cmd_data->data1 = data1;
    cmd_data->data2 = data2;

    if(client->protocol == PTZ_PROTOCOL_D)
    {
        ptz_cmd_pelco_d_t *cmd_ddata = (ptz_cmd_pelco_d_t *)buf;
        cmd_ddata->stx = PTZ_CMD_D_STX;
        crc = buf[1] + buf[2] + buf[3] + buf[4] + buf[5];
        cmd_ddata->mod = (crc%256) & 0xff;
    }
    else
    {
        cmd_data->stx = PTZ_CMD_P_STX;
        cmd_data->etx = PTZ_CMD_P_ETX;
        crc = buf[1] ^ buf[2] ^ buf[3] ^ buf[4] ^ buf[5];
        cmd_data->xor = crc & 0xff;
    }
    if((client->debug & PTZ_CMD_DEBUG_SEND) && (client->debug & PTZ_CMD_DEBUG_DETAIL))
        ptz_debug_hex("send", buf, client->maxlen);
    return tty_com_write(&client->sttty, (zpl_uchar *)buf, client->maxlen);
}

int ptz_serial_trun_down_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_TILT_DOWN, 0x00, updown);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_TILT_DOWN, 0x00, updown);
}

int ptz_serial_trun_up_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_TILT_UP, 0x00, updown);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_TILT_UP, 0x00, updown);
}

int ptz_serial_trun_left_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 leftright)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_PAN_LEFT, leftright, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_PAN_LEFT, leftright, 0x00);
}

int ptz_serial_trun_right_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 leftright)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_PAN_RIGHT, leftright, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_PAN_RIGHT, leftright, 0x00);
}


int ptz_serial_trun_left_down_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown, zpl_uint8 leftright)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_TILT_DOWN|PTZ_CMD_D_PAN_LEFT, leftright, updown);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_TILT_DOWN|PTZ_CMD_P_PAN_LEFT, leftright, updown);
}

int ptz_serial_trun_left_up_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown, zpl_uint8 leftright)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_TILT_UP|PTZ_CMD_D_PAN_LEFT, leftright, updown);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_TILT_UP|PTZ_CMD_P_PAN_LEFT, leftright, updown);
}

int ptz_serial_trun_right_down_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown, zpl_uint8 leftright)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_TILT_DOWN|PTZ_CMD_D_PAN_RIGHT, leftright, updown);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_TILT_DOWN|PTZ_CMD_P_PAN_RIGHT, leftright, updown);
}

int ptz_serial_trun_right_up_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 updown, zpl_uint8 leftright)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_TILT_UP|PTZ_CMD_D_PAN_RIGHT, leftright, updown);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_TILT_UP|PTZ_CMD_P_PAN_RIGHT, leftright, updown);
}

//变倍+
int ptz_serial_zoom_wide_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_ZOOM_WIDE, 0x00, speed);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_ZOOM_WIDE, 0x00, speed);
}
//变倍-
int ptz_serial_zoom_tele_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_ZOOM_TELE, 0x00, speed);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_P_ZOOM_TELE, 0x00, speed);
}

//聚焦+
int ptz_serial_focus_near_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, PTZ_CMD_D_FOCUS_NEAR, 0x00, 0x00, speed);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, PTZ_CMD_P_FOCUS_NEAR, 0x00, 0x00, speed);
}
//聚焦-
int ptz_serial_focus_far_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, PTZ_CMD_D_FOCUS_FAR, 0x00, speed);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, PTZ_CMD_P_FOCUS_FAR, 0x00, 0x00, speed);
}


//光圈+
int ptz_serial_iris_close_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, PTZ_CMD_D_IRIS_CLOSE, 0x00, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, PTZ_CMD_D_IRIS_CLOSE, 0x00, 0x00, 0x00);
}
//光圈-
int ptz_serial_iris_open_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, PTZ_CMD_D_IRIS_OPEN, 0x00, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, PTZ_CMD_P_IRIS_OPEN, 0x00, 0x00, 0x00);
}


int ptz_serial_stop_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x00, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x00, 0x00, 0x00);
}
//灯光关
int ptz_serial_lighting_close_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x0b, 0x00, 0x00);
    return ERROR;
}
//灯光开
int ptz_serial_lighting_open_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x09, 0x00, 0x00);
    return ERROR;
}

//自动巡航
int ptz_serial_cruise_control_open_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x96, 0x00, 0x00);
    return ERROR;
}
//关闭自动巡航
int ptz_serial_cruise_control_close_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x99, 0x00, 0x00);
    return ERROR;    
}


int ptz_serial_zoom_speed_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x25, 0x00, speed);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x25, 0x00, speed);
}

int ptz_serial_focus_speed_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 speed)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x27, 0x00, speed);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x27, 0x00, speed);
}

int ptz_serial_alarm_ack_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 no)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x19, 0x00, no);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x19, 0x00, no);
}


int ptz_serial_flip_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x07, 0x00, 0x21);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x07, 0x00, 0x21);
}

int ptz_serial_zero_pan_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x07, 0x00, 0x22);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x07, 0x00, 0x22);
}

int ptz_serial_remote_reset_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x0f, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x0f, 0x00, 0x00);
}

int ptz_serial_set_aux_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 val)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x09, 0x00, val);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x09, 0x00, val);
}

int ptz_serial_clear_aux_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 val)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x0b, 0x00, val);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x0b, 0x00, val);
}

int ptz_serial_zone_start_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 val)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x11, 0x00, val);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x11, 0x00, val);
}

int ptz_serial_zone_stop_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 val)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x13, 0x00, val);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x13, 0x00, val);
}

int ptz_serial_screen_show_cmd(ptz_channel_t *client, zpl_uint8 addr, zpl_uint8 x, zpl_uint8 val)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x15, x, val);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x15, x, val);
}

int ptz_serial_screen_clear_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x17, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x17, 0x00, 0x00);
}

int ptz_serial_zone_scan_on_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x1b, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x1b, 0x00, 0x00);
}

int ptz_serial_zone_scan_off_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x1d, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x1d, 0x00, 0x00);
}

int ptz_serial_pattern_start_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x1f, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x1f, 0x00, 0x00);
}

int ptz_serial_pattern_stop_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x21, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x21, 0x00, 0x00);
}


int ptz_serial_pattern_running_cmd(ptz_channel_t *client, zpl_uint8 addr)
{
    if(client->protocol == PTZ_PROTOCOL_D)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x23, 0x00, 0x00);
    if(client->protocol == PTZ_PROTOCOL_P)
        return ptz_serial_cmd_write(client,  addr, 0x00, 0x23, 0x00, 0x00);
}