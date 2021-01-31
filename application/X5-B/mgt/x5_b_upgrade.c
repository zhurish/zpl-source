/*
 * x5_b_upgrade.c
 *
 *  Created on: 2019年4月17日
 *      Author: DELL
 */

#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_vrf.h"
#include "nsm_interface.h"
#include "eloop.h"
#include "cJSON.h"
#include "xyz_modem.h"

#include "x5_b_app.h"
#include "x5_b_upgrade.h"
#include "x5_b_cmd.h"
#include "x5_b_web.h"


int x5b_app_update_mode(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(info != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_update_mode_enable(mgt, TRUE, E_CMD_TO_A);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(MODULE_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(MODULE_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(info)
	{
		mgt->sync_ack = TRUE;
		mgt->not_debug = TRUE;
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_UPDATE, E_CMD_UPDATE_MODE), sizeof(x5b_app_update_mode_t), info);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(MODULE_APP, "Update Mode MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
		mgt->upgrade = TRUE;
		//x5b_app_update_mode_enable(mgt, TRUE, E_CMD_TO_A);
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return len;
	}
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return ERROR;
}

int x5b_app_update_mode_exit(x5b_app_mgt_t *app)
{
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	x5b_app_update_mode_enable(mgt, FALSE, E_CMD_TO_A);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	//zlog_debug(MODULE_APP, "----x5b_app_update_mode_exit OK" );
	return OK;
}

int x5b_app_update_data(x5b_app_mgt_t *app, void *info, int inlen, int to)
{
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(info != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(UPDATE))
			zlog_warn(MODULE_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(UPDATE))
			zlog_warn(MODULE_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(info)
	{
		mgt->sync_ack = TRUE;
		mgt->not_debug = TRUE;
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_UPDATE, E_CMD_UPDATE_DATA), inlen, info);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(UPDATE))
			zlog_debug(MODULE_APP, "Update Data MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return len;
	}
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return ERROR;
}









/*
extern int xyz_modem_build_hdr(xyz_modem_t*xyz, xyz_modem_hdr_t *hdr, char *filename, int filesize);
extern int xyz_modem_build_data(xyz_modem_t*xyz, xyz_modem_data_t *hdr, char *data, int len);
extern int xyz_modem_build_data_last(xyz_modem_t*xyz, xyz_modem_data_last_t *hdr, char *data, int len);
extern int xyz_modem_build_finsh(xyz_modem_t*xyz, xyz_modem_data_last_t *hdr);
*/
static int xyz_modem_wait_start_transmit_data(xyz_modem_t *xyz, int timeout)
{
	int ret = 0, cnt = 0;
	char buf[6];
	zassert(xyz != NULL);
	while (1)
	{
		if(cnt == xyzModem_MAX_RETRIES)
			return OS_TIMEOUT;
		cnt++;
		ret = x5b_app_read_msg_timeout (NULL, timeout, buf, 1);
		if (ret == OK)
		{
			if (buf[0] == 'C')
			{
				return OK;
			}
		}
		else
			os_msleep (1000);
	}
	return ERROR;
}

static int xyz_modem_transmit_data(xyz_modem_t *xyz, void *p, int len, int timeout)
{
	int ret = 0, cnt = 0;
	char buf[6];
	zassert(xyz != NULL);
	zassert(p != NULL);
	while(1)
	{
		if(cnt == 5)
			return ERROR;
		cnt++;
		if(x5b_app_update_data(NULL, p, len, E_CMD_TO_A) == OK)
		{
			ret = x5b_app_read_msg_timeout(NULL, timeout, buf, 1);
			if(ret == OK)
			{
				if(buf[0] == ACK)
				{
					if(X5_B_ESP32_DEBUG(UPDATE))
						zlog_debug(MODULE_APP, "Successfully sent packet");
					return OK;
				}
				else if(buf[0] == NAK)
				{
					if(xyz->s_eof == 0)//
					{
						xyz->total_retries++;
						if(X5_B_ESP32_DEBUG(UPDATE))
							zlog_debug(MODULE_APP, "Received NAK, expected ACK. Retrying...");
						continue;
					}
					else if(xyz->s_eof == 1)
					{
						return xyzModem_cancel;
					}
					else if(xyz->s_eof == 2)
					{
						xyz->s_nak = 1;
						return xyzModem_eof;
					}
				}
				else if(buf[0] == 'C')
				{
					if(X5_B_ESP32_DEBUG(UPDATE))
						zlog_debug(MODULE_APP, "Received 'C'");
					return xyzModem_timeout;
				}
			}
			else if(ret == OS_TIMEOUT)
			{
				if(xyz->s_eof == 1)
				{
					return xyzModem_cancel;
				}
				else
					os_msleep(1500);
				xyz->total_retries++;
				continue;
			}
			else
				os_msleep(1500);
			xyz->total_retries++;
			continue;
		}
		else
			os_msleep(1500);
		xyz->total_retries++;
		continue;
	}
    return ERROR;
}

static int ymodem_send_finsh(xyz_modem_t *xyz)
{
	zassert(xyz != NULL);
	zlog_debug(MODULE_APP, "xyzModem: Total len %d Byte, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries",
		xyz->len, xyz->total_SOH, xyz->total_STX,
		xyz->total_CAN, xyz->total_retries);
	return OK;
}
int ymodem_send(int fd, char *filename, int filesize)
{
	int ret = 0;
	char buf[1200];
	xyz_modem_t xyz;
	xyz_modem_data_t data;
	xyz_modem_hdr_t hdr;
	int bytes_read;
	int pos = filesize;
	xyz.sequm = 0;
	zassert(filename != NULL);
	memset(&xyz, 0, sizeof(xyz_modem_t));
	//zlog_debug(MODULE_APP, "waiting to send data...");
	ret = xyz_modem_wait_start_transmit_data(&xyz, xyzModem_CHAR_TIMEOUT);
	if(ret != OK)
	{
		//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
		zlog_debug(MODULE_APP, "waiting timeout or error...");
		return ret;
	}
	//zlog_debug(MODULE_APP, "Sending %u bytes...", pos);

	ret = xyz_modem_build_hdr(&xyz, &hdr, filename, pos);

	ret = xyz_modem_transmit_data(&xyz, &hdr, sizeof(xyz_modem_hdr_t), xyzModem_PACK_TIMEOUT);
	if (ret == ERROR)
	{
		//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
		zlog_debug(MODULE_APP, "Error: No ACK received in 5 attempts\n");
		return ERROR;
	}
	ret = xyz_modem_wait_start_transmit_data(&xyz, xyzModem_CHAR_TIMEOUT);
	if(ret != OK)
	{
		//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
		zlog_debug(MODULE_APP, "waiting timeout or error...");
		return ERROR;
	}

	if(strstr(filename, "esp32") || strstr(filename, "ESP32"))
		ret = xyz_modem_build_finsh_eot(&xyz, 1);
	while ((bytes_read = read(fd, buf, XYZ_MAX_SIZE)) > 0)
	{
		//zlog_debug(MODULE_APP, " ================ read %d byte\n", bytes_read);
		/* Make 10 attempts to sent the packet */
		//if(strstr(filename, "esp32"))
		if(strstr(filename, "esp32") || strstr(filename, "ESP32"))
			ret = xyz_modem_build_finsh_eot(&xyz, 1);
		if(bytes_read > XYZ_MIN_SIZE)
			ret = xyz_modem_build_data(&xyz, &data, buf, bytes_read);
		else
			ret = xyz_modem_build_data_last(&xyz, &hdr, buf, bytes_read);

		if(bytes_read > XYZ_MIN_SIZE)
			ret = xyz_modem_transmit_data(&xyz, &data, sizeof(xyz_modem_data_t), xyzModem_PACK_TIMEOUT);
		else
			ret = xyz_modem_transmit_data(&xyz, &hdr, sizeof(xyz_modem_hdr_t), xyzModem_PACK_TIMEOUT);

		if (ret == ERROR)
		{
			ymodem_send_finsh(&xyz);
			//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
			//if(X5_B_ESP32_DEBUG(UPDATE))
				zlog_debug(MODULE_APP, "Error: No ACK received in 10 attempts\n");
			return ERROR;
		}
		else if (ret == xyzModem_cancel)
		{
			ymodem_send_finsh(&xyz);
			//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
			//if(X5_B_ESP32_DEBUG(UPDATE))
				zlog_debug(MODULE_APP, "Error: Cancel Ymodem\n");
			return ERROR;
		}
		else if (ret == xyzModem_eof)
		{
			ymodem_send_finsh(&xyz);
			//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
			//if(X5_B_ESP32_DEBUG(UPDATE))
				zlog_debug(MODULE_APP, "Error: EOF And Cancel Ymodem\n");
			return ERROR;
		}
		else if (ret == xyzModem_timeout)
		{
			ymodem_send_finsh(&xyz);
			//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
			//if(X5_B_ESP32_DEBUG(UPDATE))
				zlog_debug(MODULE_APP, "Error: EOF Timeout Cancel Ymodem\n");
			return ERROR;
		}
	}
	pos = 0;
	if(X5_B_ESP32_DEBUG(UPDATE))
		zlog_debug(MODULE_APP, "Transmit Update Data finish, Send EOT to cancel ymodem\n");
	while(1)
	{
		buf[0] = EOT;
		ret = xyz_modem_build_finsh_eot(&xyz, 1);
		ret = xyz_modem_transmit_data(&xyz, buf, 1, xyzModem_PACK_TIMEOUT);
		if (ret == OK)
		{
			if(xyz.s_eof == 2)
			{
				ret = xyz_modem_wait_start_transmit_data(&xyz, xyzModem_PACK_TIMEOUT);
				if(ret != OK)
				{
					ret = xyz_modem_build_finsh_empty(&xyz, &hdr);
					ret = xyz_modem_transmit_data(&xyz, &hdr, sizeof(hdr), xyzModem_PACK_TIMEOUT);
					if (ret == OK)
					{
						ymodem_send_finsh(&xyz);
						//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
						return OK;
					}
					else
					{
						ymodem_send_finsh(&xyz);
						//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
						return ERROR;
					}
				}
			}
			ymodem_send_finsh(&xyz);
			//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
			return OK;
		}
		else if (ret == ERROR)
		{
			ymodem_send_finsh(&xyz);
			//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
			//if(X5_B_ESP32_DEBUG(UPDATE))
				zlog_debug(MODULE_APP, "Error: No ACK received in 5 attempts\n");
			return ERROR;
		}
		else if (ret == xyzModem_cancel)
		{
			ymodem_send_finsh(&xyz);
			//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
			return OK;
		}
		else if (ret == xyzModem_eof)
		{
			if(xyz.s_eof == 2)
			{
				continue;
			}
		}
		else if (ret == xyzModem_timeout)
		{
			ymodem_send_finsh(&xyz);
			//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
			return OK;
		}
		pos++;
		if(pos == 5)
			break;
	}
	ymodem_send_finsh(&xyz);
	//x5b_app_update_mode_enable(NULL, FALSE, E_CMD_TO_A);
	return ERROR;
}


#if 0
int x5b_app_A_update_handle(char *buf)
{
	int ret = 0;
	char path[128];
	char *brk = NULL;
	if(strstr(buf, "file"))
	{
		x5b_app_update_mode_t mode;
		brk = strstr(buf, "install-");
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "/tmp/app/tftpboot/%s", brk + strlen("install-"));
		if(X5_B_ESP32_DEBUG(UPDATE))
			zlog_debug(MODULE_APP, "Install '%s' to module A", path);
		if (access (path, F_OK) >= 0)
		{
			if(strstr(path, "esp32") || strstr(path, "ESP32"))
			{
				mode.mode = X5B_APP_UPDATE_ESP32;
			}
			else if(strstr(path, "stm32") || strstr(path, "STM32"))
			{
				mode.mode = X5B_APP_UPDATE_STM32;
			}
			mode.len = htonl(os_file_size(path));
			ret = OK;
			ret = x5b_app_update_mode(NULL, &mode, E_CMD_TO_A);
			//ret = x5b_app_update_mode(NULL, &mode, E_CMD_TO_C);
			if(ret == OK)
			{
				int fd = 0;
				fd = open(path, O_RDONLY);
				if (fd)
				{
					ret = ymodem_send(fd, brk + strlen("install-"), ntohl(mode.len));
					close(fd);
					x5b_app_update_mode_exit(NULL);
					//zlog_debug(MODULE_APP, "----x5b_app_update_mode OK" );
					return ret;
				}
				x5b_app_update_mode_exit(NULL);
				return ERROR;
			}
			x5b_app_update_mode_exit(NULL);
			//zlog_debug(MODULE_APP, "----x5b_app_update_mode error" );
			return ERROR;
		}
		//int x5b_app_update_mode_exit(x5b_app_mgt_t *app)
		//zlog_debug(MODULE_APP, "----update file %s not exist. ", brk + strlen("install-"));
		return ERROR;
	}
	return OK;
}
#endif

int x5b_app_upgrade_handle(char *pathdir, char *filename)
{
	int ret = 0;
	char path[128];
	//char *brk = NULL;
	//if(strstr(filename, "file"))
	{
		x5b_app_update_mode_t mode;
		//brk = strstr(buf, "install-");
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "%s/%s", pathdir, filename);
		if(X5_B_ESP32_DEBUG(UPDATE))
			zlog_debug(MODULE_APP, "Install '%s' to module A", path);
		if (access (path, F_OK) >= 0)
		{
			if(strstr(path, "esp32") || strstr(path, "ESP32"))
			{
				mode.mode = X5B_APP_UPDATE_ESP32;
			}
			else if(strstr(path, "stm32") || strstr(path, "STM32"))
			{
				mode.mode = X5B_APP_UPDATE_STM32;
			}
			mode.len = htonl(os_file_size(path));
			ret = OK;
			ret = x5b_app_update_mode(NULL, &mode, E_CMD_TO_A);
			//ret = x5b_app_update_mode(NULL, &mode, E_CMD_TO_C);
			if(ret == OK)
			{
				int fd = 0;
				fd = open(path, O_RDONLY);
				if (fd)
				{
					ret = ymodem_send(fd, filename, ntohl(mode.len));
					close(fd);
					x5b_app_update_mode_exit(NULL);
					//zlog_debug(MODULE_APP, "----x5b_app_update_mode OK" );
					return ret;
				}
				x5b_app_update_mode_exit(NULL);
				return ERROR;
			}
			x5b_app_update_mode_exit(NULL);
			//zlog_debug(MODULE_APP, "----x5b_app_update_mode error" );
			return ERROR;
		}
		//int x5b_app_update_mode_exit(x5b_app_mgt_t *app)
		//zlog_debug(MODULE_APP, "----update file %s not exist. ", filename);
		return ERROR;
	}
	return OK;
}
