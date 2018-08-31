/*
 * modem_attty.c
 *
 *  Created on: Jul 24, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"

#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_driver.h"
#include "modem_string.h"


//#define TTY_COM_FP


static int tty_attty_fdopen(struct tty_com *attty)
{
#ifdef TTY_COM_FP
	assert (attty);
	attty->fp = fdopen(attty->fd, "r+");
	set_blocking(attty->fd);
	if(attty->fp)
		return OK;
	zlog_debug(ZLOG_PAL, "fdopen error:%s", safe_strerror(errno));
	return ERROR;
#else
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
	if(attty->fd > 0)
	{
		close(attty->fd);
		attty->fd = 0;
	}
	return OK;
}

/*
static int tty_com_atcmd_format(struct tty_com *com, const char *format, ...)
{
    int len=0;
    va_list args;
    va_start (args, format);
    len = vfprintf (com->fp, format, args);
    fflush(com->fp);
    va_end (args);
    return len;
}

static int tty_com_atcmd_write(struct tty_com *com, const char *format, ...)
{
    int len=0;
    va_list args;
    char cmd[1024];
    os_memset(cmd, 0, sizeof(cmd));
    va_start (args, format);
    len = vsprintf (cmd, format, args);
    va_end (args);
    len = write(com->fd, cmd, len);
    return len;
}

static int tty_com_atcmd_fscanf(struct tty_com *com, char *buf, int len)
{
	return fscanf(com->fp, "%s", buf);
}
*/

static int tty_attty_shuld_close(struct tty_com *attty, int len)
{

	if(len < 0)
	{
		if ( errno == EPIPE || errno == ENOENT ||
				errno == EBADF || errno == EEXIST || errno == ENODEV)
		{
			MODEM_DEBUG("-------- %s len = %d (%s)",__func__, len, safe_strerror(errno));
			modem_attty_close(attty);
			return RES_CLOSE;
		}
		MODEM_DEBUG("======= %s len = %d (%s)",__func__, len, safe_strerror(errno));
		modem_attty_close(attty);
		return RES_CLOSE;
	}
	return RES_OK;
}


static md_res_en tty_attty_select_wait(struct tty_com *attty, int timeout)
{
	int num = 0;
	fd_set fdset;
	struct timeval timer_wait = { .tv_sec = timeout, .tv_usec = 0 };
	FD_ZERO(&fdset);
	FD_SET(attty->fd, &fdset);
	timer_wait.tv_sec = timeout;
	while(1)
	{
		num = select(attty->fd + 1, &fdset, NULL, NULL, &timer_wait);
		if (num < 0)
		{
			if (errno == EINTR)
				continue;
			if (errno == EPIPE)
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
static int tty_attty_read(struct tty_com *attty, char *buf, int len)
{
	int ret = 0;
	int bytes = 0, offset = 0;
	atcmd_response_t response;
	if(ioctl(attty->fd, FIONREAD, &bytes) != 0)
	{
		os_sleep(1);
		zlog_err(ZLOG_MODEM, " Can not get bytes to read(%s)", safe_strerror(errno));
		return RES_ERROR;
	}
	if(bytes > 0 && bytes < 1024)
	{
		while(1)
		{
			ret = read(attty->fd, buf + offset, len - offset);
			if(ret > 0)
			{
				offset += ret;
				if(offset != bytes)
					continue;
				atcmd_split(buf, bytes, &response);
				fprintf(stdout, "read(%d) %d (%d)byte:(%s)\r\n", os_task_gettid(),ret, bytes, response.buf);
				fflush(stdout);
				//os_msleep(10);
				return offset;
			}
			else
			{
				if ( errno == EPIPE || errno == ENOENT ||
						errno == EBADF || errno == EEXIST || errno == ENODEV)
				{
					modem_attty_close(attty);
					return RES_CLOSE;
				}
				else
				{
					//os_sleep(1);
					zlog_err(ZLOG_MODEM, " Can not read (%s)", safe_strerror(errno));
					return RES_ERROR;
				}
			}
		}
	}
	zlog_warn(ZLOG_MODEM, " Too much data to be read(%d bytes)", bytes);
	return RES_ERROR;
}

static int tty_attty_read_line(struct tty_com *attty,int timeout, char *buf, int len)
{
	int ret = 0;
	int bytes = 0, offset = 0;
	atcmd_response_t response;
	while(1)
	{
		ret = tty_attty_select_wait(attty,  timeout);
		if(ret == RES_OK)
		{
			if(ioctl(attty->fd, FIONREAD, &bytes) != 0)
			{
				zlog_err(ZLOG_MODEM, " Can not get bytes to read(%s)", safe_strerror(errno));
				return RES_ERROR;
			}
			if(bytes > 0 && bytes < 1024)
			{
				while(1)
				{
					ret = read(attty->fd, buf + offset, len - offset);
					if(ret > 0)
					{
						offset += ret;
						if(offset != bytes)
							continue;
						atcmd_split(buf, bytes, &response);
						fprintf(stdout, "read %d (%d)byte:(%s)\r\n", ret, bytes, response.buf);
						fflush(stdout);
						return offset;
					}
					else
					{
						if ( errno == EPIPE || errno == ENOENT ||
								errno == EBADF || errno == EEXIST || errno == ENODEV)
						{
							modem_attty_close(attty);
							return RES_CLOSE;
						}
						zlog_err(ZLOG_MODEM, " Can not read (%s)", safe_strerror(errno));
						return RES_ERROR;
					}
				}
			}
			else
			{
				zlog_warn(ZLOG_MODEM, " Too much data to be read(%d bytes)", bytes);
				return RES_ERROR;
			}
		}
	}
	zlog_warn(ZLOG_MODEM, " Too much data to be read(%d bytes)", bytes);
	return RES_ERROR;
}
/****************************************************************************/
/****************************************************************************/
int modem_attty_isopen(struct tty_com *attty)
{
	assert (attty);
	if(os_strlen(attty->devname))
	{
		if((access(attty->devname, 0) == 0) && attty->fd)
			return OK;
		if(attty->fd)
		{
			modem_attty_close(attty);
		}
	}
	return ERROR;
}

int modem_attty_open(struct tty_com *attty)
{
	assert (attty);
	if(os_strlen(attty->devname))
	{
		MODEM_DEBUG("%s:%s",__func__, attty->devname);
		if(tty_com_open(attty) == 0)
			return tty_attty_fdopen(attty);
	}
	return ERROR;
}

int modem_attty_close(struct tty_com *attty)
{
	assert (attty);
	MODEM_DEBUG("%s",__func__);
	if(tty_com_close(attty) == 0)
		return tty_attty_fclose(attty);
	return ERROR;
}

md_res_en modem_attty(struct tty_com *attty,
		int timeout, const char *waitkey, const char *format, ...)
{
    int len = 0, fail = 0;
    char cmd[1024];
    md_res_en ret = 0;
    atcmd_response_t response;
#ifdef TTY_COM_FP
    va_list args;
    va_start (args, format);
    len = vfprintf (attty->fp, format, args);
    fflush(attty->fp);
#else
    va_list args;
    va_start (args, format);
    os_memset(cmd, 0, sizeof(cmd));
    len = vsnprintf (cmd, sizeof(cmd), format, args);
    va_end (args);
    if(len <= 0)
    	return RES_ERROR;
    len = write(attty->fd, cmd, len);
    if(tty_attty_shuld_close(attty, len) != RES_OK)
    	return RES_CLOSE;
	if(MODEM_IS_DEBUG(ATCMD))
		zlog_debug(ZLOG_MODEM, " AT CMD: >>>>> :%s", atcmd_split(cmd, len, &response));
#endif
    os_memset(cmd, 0, sizeof(cmd));
    while(1)
    {
    	ret = tty_attty_select_wait(attty,  timeout);
    	if(ret == RES_OK)
    	{
    		len = tty_attty_read(attty, cmd, sizeof(cmd));
    		if(len > 0)
    		{
    			atcmd_split(cmd, len, &response);
    			if(MODEM_IS_DEBUG(ATCMD))
    				zlog_debug(ZLOG_MODEM, " AT CMD: <<<<< :%s", response.buf);
    			if(strstr(response.buf, "ERROR"))
    				return RES_ERROR;
    			if(strstr(response.buf, waitkey))
    				return RES_OK;
    		}
    		else
    		{
        		if(fail == 5)
        		{
            		modem_attty_close(attty);
            		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
            		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
            		return RES_CLOSE;
        		}
        		os_msleep(100);
        		fail++;
    		}
    	}
    	else if(ret == RES_TIMEOUT)
    	{
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    		if(fail == 5)
    		{
        		modem_attty_close(attty);
        		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
        		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
        		return ret;
    		}
    		fail++;
    		continue;
    	}
    	else if(ret == RES_CLOSE)
    	{
    		modem_attty_close(attty);
    		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		return ret;
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    	}
    	else if(ret == RES_ERROR)
    	{
    		modem_attty_close(attty);
    		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		return ret;
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    	}
    }
    return ret;
}

int modem_attty_respone(struct tty_com *attty,
		int timeout, char *buf, int size, const char *format, ...)
{
    int len = 0, fail = 0;
    char cmd[1024];
    md_res_en ret = 0;
    atcmd_response_t response;
#ifdef TTY_COM_FP
    va_list args;
    va_start (args, format);
    len = vfprintf (attty->fp, format, args);
    fflush(attty->fp);
#else
    va_list args;
    va_start (args, format);
    os_memset(cmd, 0, sizeof(cmd));
    len = vsnprintf (cmd, sizeof(cmd), format, args);
    va_end (args);
    if(len <= 0)
    	return RES_ERROR;
    len = write(attty->fd, cmd, len);
    if(tty_attty_shuld_close(attty, len) != RES_OK)
    	return RES_CLOSE;
	if(MODEM_IS_DEBUG(ATCMD))
		zlog_debug(ZLOG_MODEM, " AT CMD: >>>>> :%s", atcmd_split(cmd, len, &response));
#endif

    os_memset(cmd, 0, sizeof(cmd));
    while(1)
    {
    	ret = tty_attty_select_wait(attty,  timeout);
    	if(ret == RES_OK)
    	{
    		len = tty_attty_read(attty, cmd, sizeof(cmd));
    		if(len > 0)
    		{
    			atcmd_split(cmd, len, &response);
    			if(MODEM_IS_DEBUG(ATCMD))
    				zlog_debug(ZLOG_MODEM, " AT CMD: <<<<< :%s", response.buf);
    			if(strstr(response.buf, "ERROR"))
    				return RES_ERROR;
    			os_memcpy(buf, response.buf, MIN(response.len, size));
    			return len;
    		}
    		else
    		{
        		if(fail == 5)
        		{
            		modem_attty_close(attty);
            		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
            		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
            		return RES_CLOSE;
        		}
        		os_msleep(100);
        		fail++;
    		}
    	}
    	else if(ret == RES_TIMEOUT)
    	{
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    		if(fail == 5)
    		{
        		modem_attty_close(attty);
        		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
        		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
        		return ret;
    		}
    		fail++;
    		continue;
    	}
    	else if(ret == RES_CLOSE)
    	{
    		modem_attty_close(attty);
    		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		return ret;
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    	}
    	else if(ret == RES_ERROR)
    	{
    		modem_attty_close(attty);
    		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		return ret;
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    	}
    }
    return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
int modem_client_attty_isopen(modem_client_t *client)
{
	assert (client);
	if(modem_attty_isopen(&client->attty) == OK)
	{
		client->at_state = RES_OK;
		return OK;//modem_attty_isopen(&client->attty);
	}
	return ERROR;
#if 0
	if(os_strlen(client->attty.devname))
	{
		if(client->attty.fd)
			return OK;
	}
	return ERROR;
#endif
}

int modem_client_attty_open(modem_client_t *client)
{
	assert (client);
	return modem_attty_open(&client->attty);
#if 0
	if(os_strlen(client->attty.devname))
	{
		if(tty_com_open(&client->attty) == 0)
			return tty_attty_fdopen(&client->attty);
	}
	return ERROR;
#endif
}

int modem_client_attty_close(modem_client_t *client)
{
	assert (client);
	client->at_state = RES_CLOSE;
	return modem_attty_close(&client->attty);
#if 0
	return tty_attty_fclose(&client->attty);
#endif
}



md_res_en modem_client_attty(modem_client_t *client,
		int timeout, const char *waitkey, const char *format, ...)
{
    int len = 0, fail = 0, retlen = 0;
    char cmd[1024];
    md_res_en ret = 0;
#ifdef TTY_COM_FP
    va_list args;
    va_start (args, format);
    len = vfprintf (client->attty.fp, format, args);
    fflush(client->attty.fp);
#else
    va_list args;
    va_start (args, format);
    os_memset(cmd, 0, sizeof(cmd));
    len = vsnprintf (cmd, sizeof(cmd), format, args);
    va_end (args);
    if(len <= 0)
    	return RES_ERROR;

    retlen = tty_com_write(&client->attty, cmd, len);
    //retlen = write(client->attty.fd, cmd, len);
    if(tty_attty_shuld_close(&client->attty, retlen) != RES_OK)
    {
    	client->at_state = RES_CLOSE;
    	return RES_CLOSE;
    }
	if(MODEM_IS_DEBUG(ATCMD))
		zlog_debug(ZLOG_MODEM, " (%s) AT CMD: >>>>> :%s", modem_client_module_name(client),
				(char*)atcmd_split(cmd, len, (atcmd_response_t *)&(client->response)));
	os_msleep(50);
	//tcflush(client->attty.fd, TCIOFLUSH);
#endif
    os_memset(cmd, 0, sizeof(cmd));
    while(1)
    {
    	ret = tty_attty_select_wait(&client->attty,  timeout);
    	if(ret == RES_OK)
    	{
    		len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    		if(len)
    		{
    			atcmd_split(cmd, len, (atcmd_response_t *)&client->response);
    			if(MODEM_IS_DEBUG(ATCMD))
    				zlog_debug(ZLOG_MODEM, " (%s) AT CMD: <<<<< :%s",
    						modem_client_module_name(client),
							(char*)client->response.buf);
    			if(strstr(client->response.buf, "ERROR"))
    			{
    				fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
    				return RES_ERROR;
    			}
    			if(strstr(client->response.buf, waitkey))
    				return RES_OK;
    		}
    		else
    		{
        		if(fail == 5)
        		{
        			client->at_state = RES_CLOSE;
        			modem_client_attty_close(client);
        			MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
            		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
            		fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
            		return RES_CLOSE;
        		}
        		os_msleep(100);
        		fail++;
    		}
    	}
    	else if(ret == RES_TIMEOUT)
    	{
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    		if(fail == 5)
    		{
    			client->at_state = RES_CLOSE;
    			modem_client_attty_close(client);
    			MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
        		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
        		fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
        		return ret;
    		}
    		fail++;
    		continue;
    	}
    	else if(ret == RES_CLOSE)
    	{
    		client->at_state = RES_CLOSE;
    		modem_client_attty_close(client);
    		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
    		return ret;
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    	}
    	else if(ret == RES_ERROR)
    	{
    		client->at_state = RES_CLOSE;
    		modem_client_attty_close(client);
    		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
    		return ret;
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    	}
    }
    return ret;
}

int modem_client_attty_respone(modem_client_t *client,
		int timeout, char *buf, int size, const char *format, ...)
{
    int len = 0, fail = 0;
    char cmd[1024];
    md_res_en ret = 0;
#ifdef TTY_COM_FP
    va_list args;
    va_start (args, format);
    len = vfprintf (client->attty.fp, format, args);
    fflush(client->attty.fp);
#else
    va_list args;
    va_start (args, format);
    os_memset(cmd, 0, sizeof(cmd));
    len = vsnprintf (cmd, sizeof(cmd), format, args);
    va_end (args);
    if(len <= 0)
    	return RES_ERROR;
    len = tty_com_write(&client->attty, cmd, len);
    //len = write(client->attty.fd, cmd, len);
    if(tty_attty_shuld_close(&client->attty, len) != RES_OK)
    {
    	client->at_state = RES_CLOSE;
    	return RES_CLOSE;
    }
	if(MODEM_IS_DEBUG(ATCMD))
		zlog_debug(ZLOG_MODEM, " (%s) AT CMD: >>>>> :%s", modem_client_module_name(client),
				(char*)atcmd_split(cmd, len, &client->response));
	os_msleep(50);
	//tcflush(client->attty.fd, TCIOFLUSH);
#endif

    os_memset(cmd, 0, sizeof(cmd));
    while(1)
    {
    	ret = tty_attty_select_wait(&client->attty,  timeout);
    	if(ret == RES_OK)
    	{
    		len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    		if(len)
    		{
    			atcmd_split(cmd, len, &client->response);
    			if(MODEM_IS_DEBUG(ATCMD))
    				zlog_debug(ZLOG_MODEM, " (%s) AT CMD: <<<<< :%s",
    						modem_client_module_name(client),
							(char*)client->response.buf);
    			if(strstr(client->response.buf, "ERROR"))
    			{
    				fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
    				return RES_ERROR;
    			}
    			os_memcpy(buf, client->response.buf, MIN(client->response.len, size));
    			return len;
    		}
    		else
    		{
        		if(fail == 5)
        		{
        			client->at_state = RES_CLOSE;
        			modem_client_attty_close(client);
        			MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
            		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
            		fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
            		return RES_CLOSE;
        		}
        		os_msleep(100);
        		fail++;
    		}
    	}
    	else if(ret == RES_TIMEOUT)
    	{
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    		if(fail == 5)
    		{
    			client->at_state = RES_CLOSE;
    			modem_client_attty_close(client);
    			MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
        		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
        		fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
        		return ret;
    		}
    		fail++;
    		continue;
    	}
    	else if(ret == RES_CLOSE)
    	{
    		client->at_state = RES_CLOSE;
    		modem_client_attty_close(client);
    		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
    		return ret;
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    	}
    	else if(ret == RES_ERROR)
    	{
    		client->at_state = RES_CLOSE;
    		modem_client_attty_close(client);
    		MODEM_DEBUG(" AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		zlog_err(ZLOG_MODEM, " AT CMD ERROR and close AT Channel(%s)",safe_strerror(errno));
    		fprintf(stdout, " %s:%d (%s)",__func__,__LINE__,safe_strerror(errno));
    		return ret;
    		//len = tty_attty_read(&client->attty, cmd, sizeof(cmd));
    	}
    }
    return ret;
}

/*extern int tty_com_open(struct tty_com *com);
extern int tty_com_close(struct tty_com *com);
extern int tty_com_update_option(struct tty_com *com);*/


