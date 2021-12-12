/*
 * modem_attty->c
 *
 *  Created on: Jul 24, 2018
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_driver.h"
#include "modem_string.h"


//#define TTY_COM_FP
#define AT_LEN_MAX(n)	(128 * (n))
/*
 * 去掉 \r\n
 */
static const char * atcmd_split(char *src, int len, int *out)
{
	char *p = src;
	int i = 0, j = 0, n = 1, slen = len;
	char swapbuf[ATCMD_RESPONSE_MAX];
	if(src == NULL || len <= 0 || len > ATCMD_RESPONSE_MAX)
		return "CMD ERROR(NULL)";
	memset(swapbuf, 0, ATCMD_RESPONSE_MAX);
	for(i = 0; i < len; i++)
	{
		if(!isspace(p[i]))
			break;
	}
	slen -= i;
	p += i;

	for(i = 0; i < slen; i++)
	{
		if(MD_ASCCI(p[i]))
		{
/*			if (MD_abcd(p[i]))
				swapbuf[j++] = p[i] - 32;
			else
			{
				if(p[i] != MD_CTRL_KEY1)*/
					swapbuf[j++] = p[i];
			//}
		}
		else if(p[i] == MD_NL && p[i+1] == MD_CR)
		{
			if(os_strlen(swapbuf) > AT_LEN_MAX(n))
			{
				n++;
				swapbuf[j++] = MD_CR;
				i++;
			}
			else
			{
				swapbuf[j++] = MD_SPACE;
				i++;
			}
		}
		else if(p[i] == MD_NL || p[i] == MD_CR)
		{
			if(os_strlen(swapbuf) > AT_LEN_MAX(n))
			{
				n++;
				swapbuf[j++] = MD_CR;
			}
			else
			{
				swapbuf[j++] = MD_SPACE;
			}
		}
	}
    i = strlen(swapbuf);
	if(swapbuf[j-1] == MD_SPACE)
	{
		if(swapbuf[j-2] == MD_SPACE)
		{
			i--;
			swapbuf[j-2] = '\0';
		}
		swapbuf[j-1] = '\0';
		i--;
	}
	os_memset(src, 0, len);
	slen = i;
	j = 0;
	for(i = 0; i < slen - 1; i++)
	{
		if(swapbuf[i] == MD_SPACE && swapbuf[i+1] == MD_SPACE)
		{
			src[j++] = swapbuf[i];
			i++;
		}
		else
		{
			src[j++] = swapbuf[i];
		}
	}
	src[j++] = swapbuf[slen - 1];
	//os_memcpy(src, swapbuf, i);
	if(out)
		*out = j;
	return (char*)src;
}

static zpl_bool atsms_finish(char *src, int len)
{
	int i = 0;
	char *str = src;
	for (i = 0; *str != '\0' && i < len; str++,i++)
	{
		if (((int) *str) == MD_CTRL_Z)
			return zpl_true;
	}
	return zpl_false;
}

static int tty_attty_fdopen(struct tty_com *attty)
{
#ifdef TTY_COM_FP
	assert (attty);
	attty->fp = fdopen(attty->fd, "r+");
	set_blocking(attty->fd);
	if(attty->fp)
		return OK;
	zlog_debug(MODULE_PAL, "fdopen error:%s", safe_strerror(errno));
	return ERROR;
#else
	assert (attty);
	set_blocking(attty->fd);
	return OK;
#endif
}

static int tty_attty_fclose(struct tty_com *attty)
{
	assert (attty);
#ifdef TTY_COM_FP
	assert (attty->fp);
	fclose(attty->fp);
	attty->fp = NULL;
#endif
	assert (attty);
	if(attty->fd > 0)
	{
		close(attty->fd);
		attty->fd = -1;
	}
	return OK;
}

/*
 * test the AT Channel should be close
 */
static md_res_en tty_attty_test_close(struct tty_com *attty)
{
	assert (attty);
	//if(len < 0)
	{
		if ( errno == EPIPE || errno == EBADF || errno == EIO)
		{
			if(tty_com_close(attty) == 0)
				tty_attty_fclose(attty);
			MODEM_TTY_DEBUG("-------- %s (%s)",__func__, safe_strerror(errno));
			return RES_CLOSE;
		}
		else if(errno == EAGAIN || errno == EBUSY || errno == EINTR)
		{
			return RES_AGAIN;
		}
		if(tty_com_close(attty) == 0)
			tty_attty_fclose(attty);
		MODEM_TTY_DEBUG("======= %s (%s)",__func__, safe_strerror(errno));
		return RES_CLOSE;
	}
	return RES_OK;
}



static md_res_en tty_attty_select_wait(struct tty_com *attty, int timeout)
{
	int num = 0;
	fd_set fdset;
	struct timeval timer_wait = { .tv_sec = timeout, .tv_usec = 0 };
	assert (attty);
	FD_ZERO(&fdset);
	FD_SET(attty->fd, &fdset);
	timer_wait.tv_sec = timeout;
	while(1)
	{
		num = select(attty->fd + 1, &fdset, NULL, NULL, &timer_wait);
		if (num < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;
			if (errno == EPIPE || errno == EBADF || errno == EIO)
			{
				return RES_CLOSE;
			}
			return RES_ERROR;
		}
		else if (num == 0)
		{
			//timeout
			return RES_TIMEOUT;
		}

		if (FD_ISSET(attty->fd, &fdset))
		{
			return RES_OK;
		}
	}
	return RES_ERROR;
}


/******************************/
#if 0
static int tty_attty_read(struct tty_com *attty, char *buf, int len)
{
	int ret = 0;
	int bytes = 0, offset = 0,readlen = 0;
	assert (attty);
	//atcmd_response_t response;
	if(ioctl(attty->fd, FIONREAD, &bytes) != 0)
	{
		if (errno == EPIPE || errno == EBADF || errno == EIO)
		{
			zlog_err(MODULE_MODEM, " Can not get bytes to read(%s)", safe_strerror(errno));
			return RES_CLOSE;
		}
		zlog_err(MODULE_MODEM, " Can not get bytes to read(%s)", safe_strerror(errno));
		return RES_ERROR;
	}
	if(bytes > 0 && bytes < ATCMD_RESPONSE_MAX)
	{
		while(1)
		{
			//ret = read(attty->fd, buf + offset, len - offset);
			ret = tty_com_read(attty, buf + offset, len - offset);
			if(ret > 0)
			{
				offset += ret;
				if(offset != bytes)
					continue;
				atcmd_split(buf, offset, &readlen);
#ifdef _MODEM_TTY_DEBUG
				fprintf(stdout, "read(%d) (%d)byte:(%s)\r\n", os_task_gettid(),readlen, buf);
				fflush(stdout);
#endif
				//os_msleep(10);
				return readlen;
			}
			else
			{
				if (errno == EINTR || errno == EAGAIN)
					continue;
				else if (errno == EPIPE || errno == EBADF || errno == EIO)
				{
					return RES_CLOSE;
				}
				else
				{
					if(ret < 0)
					{
						zlog_err(MODULE_MODEM, " Can not read (%s)", safe_strerror(errno));
						return RES_ERROR;
					}
				}
			}
		}
	}
	if(bytes > ATCMD_RESPONSE_MAX)
		zlog_warn(MODULE_MODEM, " Too much data to be read(%d bytes)", bytes);
	else
		zlog_warn(MODULE_MODEM, " There no data to be read(%d bytes)", bytes);
	return RES_ERROR;
}
#else
static int tty_attty_read(struct tty_com *attty, char *buf, int len)
{
	int ret = 0, already = 0;
	int bytes = 0, offset = 0, readlen = 0;
	assert (attty);
	assert (buf);
try_again:

	if(already)
	{
#ifdef _MODEM_TTY_DEBUG
		fprintf(stdout, "try_again (%d)\r\n", os_task_gettid());
		fflush(stdout);
#endif
		ret = tty_attty_select_wait(attty, 1);
		if(ret != RES_OK)
			return ret;
	}
	//atcmd_response_t response;
	if(ioctl(attty->fd, FIONREAD, &bytes) != 0)
	{
		if (errno == EPIPE || errno == EBADF || errno == EIO)
		{
			zlog_err(MODULE_MODEM, " Can not get bytes to read(%s)", safe_strerror(errno));
			return RES_CLOSE;
		}
		zlog_err(MODULE_MODEM, " Can not get bytes to read(%s)", safe_strerror(errno));
		return RES_ERROR;
	}
	if(bytes > 0 && bytes < ATCMD_RESPONSE_MAX)
	{
		while(1)
		{
			//ret = read(attty->fd, buf + offset, len - offset);
			ret = tty_com_read(attty, buf + offset, len - offset);
			if(ret > 0)
			{
				offset += ret;
				if(offset != (bytes + already))
					continue;
#if 1
				if( ((buf[offset-2] == MD_CR) && (buf[offset-1] == MD_NL)) ||
					((buf[offset-2] == MD_SMS_KEY) && (buf[offset-1] == MD_SPACE)) )
				{
					atcmd_split(buf, offset, &readlen);
#ifdef _MODEM_TTY_DEBUG
					fprintf(stdout, "read(%d) (%d)byte:(%s)\r\n", os_task_gettid(),readlen, buf);
					fflush(stdout);
#endif
					if(MODEM_IS_DEBUG(ATCMD))
						zlog_debug(MODULE_MODEM, " AT CMD: <<<<< :%s", buf);
					//os_msleep(10);
					return readlen;
				}
#else
				if(strstr(buf, "OK")
						|| strstr(buf, "ERROR")
						|| strstr(buf, ">")
						|| strstr(buf, "PIN")
						|| strstr(buf, "PUK")
						|| strstr(buf, "READY"))
				{
					atcmd_split(buf, offset, &readlen);
#ifdef _MODEM_TTY_DEBUG
					fprintf(stdout, "read(%d) (%d)byte:(%s)\r\n", os_task_gettid(),readlen, buf);
					fflush(stdout);
#endif
					//os_msleep(10);
					return readlen;
				}
#endif
				else
				{
					already = offset;
					goto try_again;
				}
			}
			else
			{
				if (errno == EINTR || errno == EAGAIN)
					continue;
				else if (errno == EPIPE || errno == EBADF || errno == EIO)
				{
					return RES_CLOSE;
				}
				else
				{
					if(ret < 0)
					{
						zlog_err(MODULE_MODEM, " Can not read (%s)", safe_strerror(errno));
						return RES_ERROR;
					}
				}
			}
		}
	}
	if(bytes > ATCMD_RESPONSE_MAX)
		zlog_warn(MODULE_MODEM, " Too much data to be read(%d bytes)", bytes);
	else
		zlog_warn(MODULE_MODEM, " There no data to be read(%d bytes)", bytes);
	return RES_ERROR;
}
#endif
/****************************************************************************/
static int tty_attty_wait_response(modem_client_t *client, int timeout)
{
	md_res_en ret = RES_ERROR;
	assert (client);
	ret = tty_attty_select_wait(client->attty, timeout);
	switch(ret)
	{
	case RES_OK:
		ret = tty_attty_read(client->attty, client->response->buf, ATCMD_RESPONSE_MAX);
		if(ret > 0)
		{
			if((strncasecmp(client->response->buf, "at", 2) == 0) &&
					(os_strlen(client->response->buf) > client->atcmd->len))
			{
				memmove(client->response->buf, client->response->buf + client->atcmd->len, ret - client->atcmd->len);
				ret -= client->atcmd->len;
				memset(client->response->buf + ret, 0,  ATCMD_RESPONSE_MAX - ret);
			}
#ifdef _MODEM_TTY_DEBUG
			fprintf(stdout, "response(%d) %d byte:(%s)\r\n", os_task_gettid(),ret, client->response->buf);
			fflush(stdout);
#endif
		}
		client->response->len = ret;
		break;
	case RES_TIMEOUT:
		//break;
	case RES_ERROR:
		//break;
	case RES_CLOSE:
		{
			MODEM_TTY_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
			zlog_err(MODULE_MODEM, " AT CMD ERROR and close AT Channel(%s)",
					safe_strerror(errno));
#ifdef _MODEM_TTY_DEBUG
			fprintf(stdout, " %s:%d (%s)", __func__, __LINE__,
					safe_strerror(errno));
#endif
		}
		break;
	case RES_AGAIN:
	default:
		break;
	}
	return ret;
}
/****************************************************************************/
/****************************************************************************/
static int modem_attty_cmd_buffer_alloc(modem_client_t *client)
{
	if(client)
	{
		if(client->atcmd == NULL)
			client->atcmd = os_malloc(sizeof(atcmd_request_t));
		if(client->response == NULL)
			client->response = os_malloc(sizeof(atcmd_response_t));

		if(client->response == NULL || client->atcmd == NULL)
			return ERROR;

		os_memset(client->atcmd, 0, sizeof(atcmd_request_t));
		os_memset(client->response, 0, sizeof(atcmd_response_t));
		return OK;
	}
	return ERROR;
}

static int modem_attty_cmd_buffer_free(modem_client_t *client)
{
	if(client)
	{
		if(client->atcmd)
			os_free(client->atcmd);
		client->atcmd = NULL;
		if(client->response)
			os_free(client->response);
		client->response = NULL;
	}
	return OK;
}
/****************************************************************************/
int modem_attty_isopen(modem_client_t *client)
{
	assert (client);
	if(os_strlen(client->attty->devname))
	{
		if((access(client->attty->devname, 0) == 0) && (client->attty->fd > 0))
			return OK;
		if(client->attty->fd > 0)
		{
			modem_attty_close(client);
		}
		else
		{
#ifdef _MODEM_TTY_DEBUG
			fprintf(stdout, "client->attty->fd is not open\r\n");
#endif
		}
	}
#ifdef _MODEM_TTY_DEBUG
	fprintf(stdout, "client->attty->devname is NULL\r\n");
#endif
	return ERROR;
}

int modem_attty_isclose(modem_client_t *client)
{
	assert (client);
	if(client->attty->fd <= 0)
		return OK;
	return ERROR;
}

int modem_attty_open(modem_client_t *client)
{
	assert (client);
	if(os_strlen(client->attty->devname))
	{
		MODEM_TTY_DEBUG("%s:%s",__func__, client->attty->devname);
		if(modem_attty_cmd_buffer_alloc(client) == OK)
		{
			if(tty_com_open(client->attty) == 0)
				return tty_attty_fdopen(client->attty);
		}
		MODEM_TTY_DEBUG("%s: Can not alloc cmd buffer for AT Channel(%s)",__func__, client->product_name);
	}
	return ERROR;
}

int modem_attty_close(modem_client_t *client)
{
	assert (client);
	MODEM_TTY_DEBUG("%s:%s",__func__, client->attty->devname);
	modem_attty_cmd_buffer_free(client);
	if(tty_com_close(client->attty) == 0)
		return tty_attty_fclose(client->attty);
	return ERROR;
}

static md_res_en modem_attty_write_one(modem_client_t *client)
{
	assert (client);
    client->atcmd->len = tty_com_write(client->attty, client->atcmd->buf, client->atcmd->len);
    if(client->atcmd->len <= 0)
    {
		if(tty_attty_test_close(client->attty) != RES_OK)
			return RES_CLOSE;
    }
    if(client->atcmd->buf[client->atcmd->len-1] == '\n')
    	client->atcmd->buf[client->atcmd->len-1] = '\0';
    if(client->atcmd->buf[client->atcmd->len-2] == '\r')
    	client->atcmd->buf[client->atcmd->len-2] = '\0';

	if(MODEM_IS_DEBUG(ATCMD))
		zlog_debug(MODULE_MODEM, " AT CMD: >>>>> :%s", client->atcmd->buf);

	return RES_OK;
}


md_res_en modem_attty_write(modem_client_t *client, const char *format, ...)
{
    va_list args;
    assert (client);
    if(modem_attty_isclose(client) == RES_OK)
    {
    	return RES_ERROR;
    }
    if(!client->bSms)
    	modem_main_lock(client->modem);
    os_memset(client->atcmd, 0, sizeof(atcmd_request_t));
    va_start (args, format);
    client->atcmd->len = vsnprintf (client->atcmd->buf, ATCMD_RESPONSE_MAX, format, args);
    va_end (args);
    if(client->atcmd->len <= 0)
    {
    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return RES_ERROR;
    }
    client->atcmd->len = tty_com_write(client->attty, client->atcmd->buf, client->atcmd->len);

    if(client->atcmd->len <= 0)
    {
		if(tty_attty_test_close(client->attty) != RES_OK)
		{
			modem_main_unlock(client->modem);
			return RES_CLOSE;
		}
    }
    if(client->bSms)
    {
    	if(atsms_finish(client->atcmd->buf, client->atcmd->len))
    		client->bSms = zpl_false;
    }
    if(client->atcmd->buf[client->atcmd->len-1] == '\n')
    	client->atcmd->buf[client->atcmd->len-1] = '\0';
    if(client->atcmd->buf[client->atcmd->len-2] == '\r')
    	client->atcmd->buf[client->atcmd->len-2] = '\0';

	if(MODEM_IS_DEBUG(ATCMD))
		zlog_debug(MODULE_MODEM, " AT CMD: >>>>> :%s", client->atcmd->buf);
	if(!client->bSms)
		modem_main_unlock(client->modem);
	return RES_OK;
}


md_res_en modem_attty(modem_client_t *client,
		zpl_uint32 timeout, const char *waitkey, const char *format, ...)
{
	md_res_en res = RES_ERROR;
    va_list args;
    assert (client);
    if(modem_attty_isclose(client) == RES_OK)
    {
    	return RES_ERROR;
    }
    if(!client->bSms)
    	modem_main_lock(client->modem);
    os_memset(client->atcmd, 0, sizeof(atcmd_request_t));
    va_start (args, format);
    client->atcmd->len = vsnprintf (client->atcmd->buf, ATCMD_RESPONSE_MAX, format, args);
    va_end (args);
    if(client->atcmd->len <= 0)
    {
    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return RES_ERROR;
    }
    res = modem_attty_write_one(client);
    if( res != RES_OK)
    {
    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return res;
    }
    if(client->bSms)
    {
    	if(atsms_finish(client->atcmd->buf, client->atcmd->len))
    		client->bSms = zpl_false;
    }
	os_msleep(50);
    os_memset(client->response, 0, sizeof(atcmd_response_t));
    if(tty_attty_wait_response(client, timeout) > 0)
    {
    	if(waitkey && strstr(client->response->buf, waitkey))
    		res =  RES_OK;
    	if(strstr(client->response->buf, "OK"))
    		res =  RES_OK;
    	if(strstr(client->response->buf, ">"))
    	{
    		client->bSms = zpl_true;
    		res =  RES_OK;
    	}
    	if(strstr(client->response->buf, "ERROR"))
    		res =  RES_ERROR;
    }
    else
    {
		if(tty_attty_test_close(client->attty) != RES_OK)
		{
			modem_main_unlock(client->modem);
			return RES_CLOSE;
		}
    }
    if(!client->bSms)
    	modem_main_unlock(client->modem);
    return res;
}

md_res_en modem_attty_respone(modem_client_t *client,
		zpl_uint32 timeout, char *buf, zpl_size_t size, const char *format, ...)
{
	md_res_en res = RES_ERROR;
    va_list args;
    //assert (buf);
    assert (client);
    if(modem_attty_isclose(client) == RES_OK)
    {
    	return RES_ERROR;
    }
    if(!client->bSms)
    	modem_main_lock(client->modem);
    os_memset(client->atcmd, 0, sizeof(atcmd_request_t));
    va_start (args, format);
    client->atcmd->len = vsnprintf (client->atcmd->buf, ATCMD_RESPONSE_MAX, format, args);
    va_end (args);
    if(client->atcmd->len <= 0)
    {
    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return RES_ERROR;
    }
    res = modem_attty_write_one(client);
    if( res != RES_OK)
    {
    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return res;
    }
    if(client->bSms)
    {
    	if(atsms_finish(client->atcmd->buf, client->atcmd->len))
    		client->bSms = zpl_false;
    }
	os_msleep(50);
    os_memset(client->response, 0, sizeof(atcmd_response_t));
    if(tty_attty_wait_response(client, timeout) > 0)
    {
    	if(buf && size)
    		os_memcpy(buf, client->response->buf, MIN(client->response->len, size));

    	if(strstr(client->response->buf, ">"))
    	{
    		client->bSms = zpl_true;
    	}

    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return client->response->len;
    }
    else
    {
		if(tty_attty_test_close(client->attty) != RES_OK)
		{
			modem_main_unlock(client->modem);
			return RES_CLOSE;
		}
    }
    if(!client->bSms)
    	modem_main_unlock(client->modem);
    return RES_ERROR;
}


md_res_en modem_attty_proxy_respone(modem_client_t *client,
		zpl_uint32 timeout, char *buf, zpl_size_t size, const char *format, zpl_size_t len)
{
	md_res_en res = RES_ERROR;
    //assert (buf);
    assert (client);
    if(modem_attty_isclose(client) == RES_OK)
    {
    	return RES_ERROR;
    }
    if(!client->bSms)
    	modem_main_lock(client->modem);
    os_memset(client->atcmd, 0, sizeof(atcmd_request_t));
    client->atcmd->len = len;
    os_memcpy(client->atcmd->buf, format, client->atcmd->len);
    if(client->atcmd->len <= 0)
    {
    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return RES_ERROR;
    }
    res = modem_attty_write_one(client);
    if( res != RES_OK)
    {
    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return res;
    }
    if(client->bSms)
    {
    	if(atsms_finish(client->atcmd->buf, client->atcmd->len))
    		client->bSms = zpl_false;
    }
	os_msleep(50);
    os_memset(client->response, 0, sizeof(atcmd_response_t));
    if(tty_attty_wait_response(client, timeout) > 0)
    {
    	if(buf && size)
    		os_memcpy(buf, client->response->buf, MIN(client->response->len, size));

    	if(strstr(client->response->buf, ">"))
    	{
    		client->bSms = zpl_true;
    	}

    	if(!client->bSms)
    		modem_main_unlock(client->modem);
    	return client->response->len;
    }
    else
    {
		if(tty_attty_test_close(client->attty) != RES_OK)
		{
			modem_main_unlock(client->modem);
			return RES_CLOSE;
		}
    }
    if(!client->bSms)
    	modem_main_unlock(client->modem);
    return RES_ERROR;
}
md_res_en modem_attty_massage_respone(modem_client_t *client,
		zpl_uint32 timeout, const char *msg_cmd, const char *buf, zpl_size_t size)
{
	//char msg_cmd[128];
	md_res_en res = RES_ERROR;
    assert (client);
    assert (msg_cmd);
    if(modem_attty_isclose(client) == RES_OK)
    {
    	return RES_ERROR;
    }
/*    os_memset(msg_cmd, 0, msg_cmd);
    vsnprintf (msg_cmd, sizeof(msg_cmd), "AT+CMGS=\"%s\"", phone);*/
    res = modem_attty(client, 5, NULL, msg_cmd);
    if(res == RES_OK && client->bSms)
    {
    	//modem_main_lock(client->modem);
        os_memset(client->atcmd, 0, sizeof(atcmd_request_t));
        os_memcpy(client->atcmd->buf, buf, size);
        client->atcmd->buf[size - 1] = MD_CTRL_Z;
        client->atcmd->len = size + 1;
    	res = modem_attty_write_one(client);
        if(res == RES_OK)
        {
            os_memset(client->response, 0, sizeof(atcmd_response_t));
            if(tty_attty_wait_response(client, timeout) > 0)
            {
            	if(strstr(client->response->buf, "OK"))
            		res =  RES_OK;
            	if(strstr(client->response->buf, "ERROR"))
            		res =  RES_ERROR;
            	modem_main_unlock(client->modem);
            	client->bSms = zpl_false;
            }
            else
            {
        		if(tty_attty_test_close(client->attty) != RES_OK)
        		{
        			modem_main_unlock(client->modem);
        			return RES_CLOSE;
        		}
        		//modem_main_unlock(client->modem);
        		return RES_ERROR;
            }
        }
        else
        {
        	//modem_main_unlock(client->modem);
        	return RES_ERROR;
        }
    }
    else
    	modem_main_unlock(client->modem);
    return res;
}
