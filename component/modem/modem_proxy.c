/*
 * modem_proxy.c
 *
 *  Created on: Nov 13, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "os_util.h"
#include "tty_com.h"
#include "os_time.h"
#include "os_ansync.h"


#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_driver.h"
#include "modem_string.h"
#include "modem_serial.h"
#include "modem_proxy.h"

typedef struct modem_proxy_s
{
	int 	fd;
	int 	wfd;
	BOOL	bClose;
	int		offset;
	char 	input[512];
	char 	output[512];
	modem_t *modem;
	modem_client_t *client;
	modem_serial_t * channel;

}modem_proxy_t;


extern os_ansync_lst * modem_ansync_lst;


static modem_client_t * modem_client_proxy_lookup(const char *name, u_int8 hw_channel)
{
	modem_serial_t * channel = modem_serial_lookup_api(name, hw_channel);
	if(channel)
	{
		if(channel->client)
		{
/*			if(channel->modem)
			{
				modem_t *modem = channel->modem;
				modem->active = FALSE;
			}*/
			return (modem_client_t *)channel->client;
		}
	}
	return NULL;
}

static int modem_proxy_open(modem_client_t *client)
{
	if(client->modem)
	{
		modem_t *modem = client->modem;
		modem->proxy = TRUE;
	}
	else
	{
		//create modem and bind to client
		if(modem_main_add_api("test") == OK)
			modem_main_bind_api("test", ((modem_serial_t *)client->serial)->name);
		if(client->modem)
		{
			modem_t *modem = client->modem;
			modem->active = FALSE;
		}
	}
	if(client->attty)
	{
		if(client->attty->fd)
			return OK;
		return modem_attty_open(client);
	}
	return ERROR;
}

static int modem_proxy_close(modem_client_t *client)
{
	modem_t *modem = client->modem;
	if(client->attty)
	{
		if(client->attty->fd)
		{
			if(modem && modem->active == FALSE && modem->proxy == TRUE)
				modem_attty_close(client);
		}
	}

	if(modem && modem->active == FALSE && modem->proxy == TRUE)
	{
		if(modem->proxy_data)
			free(modem->proxy_data);
		modem->proxy_data = NULL;
		//create modem and bind to client
		modem_main_unbind_api("test", ((modem_serial_t *)client->serial)->name);
		modem_main_del_api("test");
	}
	return ERROR;
}



static int modem_proxy_thread(os_ansync_t *value)
{
	unsigned char buf[512];
	int i = 0, ret = 0, nbytes = 0;
	modem_proxy_t *mdproxy = value->pVoid;
	if(!mdproxy)
		return ERROR;
	os_bzero(buf, sizeof(buf));
	nbytes = ip_read(mdproxy->fd, buf, sizeof(buf));
	if (nbytes < 0)
	{
		if (!ERRNO_IO_RETRY(errno))
		{
			os_ansync_unlock(modem_ansync_lst);
			os_ansync_unregister_api(modem_ansync_lst, OS_ANSYNC_INPUT, modem_proxy_thread, mdproxy, mdproxy->fd);
			os_ansync_lock(modem_ansync_lst);
			if (mdproxy->bClose)
			{
				ip_close(mdproxy->fd);
				mdproxy->fd = 0;
			}
			modem_proxy_close(mdproxy->client);
			return ERROR;
		}
	}
	else if (nbytes == 0)
	{
		os_ansync_unlock(modem_ansync_lst);
		os_ansync_unregister_api(modem_ansync_lst, OS_ANSYNC_INPUT, modem_proxy_thread, mdproxy, mdproxy->fd);
		os_ansync_lock(modem_ansync_lst);
		if (mdproxy->bClose)
		{
			ip_close(mdproxy->fd);
			mdproxy->fd = 0;
		}
		modem_proxy_close(mdproxy->client);
		return ERROR;
	}
	else
	{
		for (i = 0; i < nbytes; i++)
		{
			switch (buf[i])
			{
			case '\n':
			case 0x0D:	// \n
			case 0X1A: 	// CTRL+Z
				//case 26:
				if (buf[i] == 0X1A)
					mdproxy->input[mdproxy->offset++] = buf[i];

				ret = modem_attty_proxy_respone(mdproxy->client, 10,
						mdproxy->output, sizeof(mdproxy->output),
						mdproxy->input, mdproxy->offset);

				if (ret > 0)
				{
					if(write(mdproxy->wfd, mdproxy->output, ret) <= 0)
					{
						if (!ERRNO_IO_RETRY(errno))
						{
							os_ansync_unlock(modem_ansync_lst);
							os_ansync_unregister_api(modem_ansync_lst, OS_ANSYNC_INPUT, modem_proxy_thread, mdproxy, mdproxy->fd);
							os_ansync_lock(modem_ansync_lst);
							if (mdproxy->bClose)
							{
								ip_close(mdproxy->fd);
								mdproxy->fd = 0;
							}
							modem_proxy_close(mdproxy->client);
							return ERROR;
						}
					}
				}
				if(ret == RES_CLOSE || ret == RES_ERROR)
				{
					os_ansync_unlock(modem_ansync_lst);
					os_ansync_unregister_api(modem_ansync_lst, OS_ANSYNC_INPUT, modem_proxy_thread, mdproxy, mdproxy->fd);
					os_ansync_lock(modem_ansync_lst);
					if (mdproxy->bClose)
					{
						ip_close(mdproxy->fd);
						mdproxy->fd = 0;
					}
					modem_proxy_close(mdproxy->client);
					return ERROR;
				}
				/*						RES_OK  = 0,
				 RES_ERROR = -1,
				 RES_TIMEOUT  = -2,
				 RES_CLOSE  = -3,
				 RES_AGAIN  = -4,*/

				mdproxy->offset = 0;
				break;
			default:
				mdproxy->input[mdproxy->offset++] = buf[i];
				break;
			}
		}
		return OK;
	}
	return ERROR;
}


int modem_proxy_enable(const char *name, int fd, BOOL close)
{
	modem_client_t * client = modem_client_proxy_lookup(name, 0);
	if(client)
	{
		modem_proxy_t *mdproxy = NULL;
		mdproxy = malloc(sizeof(modem_proxy_t));
		if(!mdproxy)
		{
			return ERROR;
		}
		memset(mdproxy, 0 , sizeof(modem_proxy_t));
		mdproxy->fd = fd;
		mdproxy->wfd = fd;
		if(modem_proxy_open(client) == OK)
		{
			mdproxy->modem = client->modem;
			mdproxy->client = client;
			mdproxy->channel = client->serial;
			mdproxy->modem->proxy_data = mdproxy;
			//os_ansync_unlock(modem_ansync_lst);
			os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_INPUT, modem_proxy_thread, mdproxy, mdproxy->fd);
			//os_ansync_lock(modem_ansync_lst);
			return OK;
		}
		else
		{
			free(mdproxy);
		}
	}
	return ERROR;
}


int modem_proxy_disable(const char *name)
{
	modem_client_t * client = modem_client_proxy_lookup(name, 0);
	if(client)
	{
		if(client->modem)
		{
			modem_t *modem = client->modem;
			if(modem->proxy == TRUE)
			{
				modem_proxy_t *mdproxy = modem->proxy_data;
				if(mdproxy)
				{
					//os_ansync_unlock(modem_ansync_lst);
					os_ansync_unregister_api(modem_ansync_lst, OS_ANSYNC_INPUT, modem_proxy_thread, mdproxy, mdproxy->fd);
					//os_ansync_lock(modem_ansync_lst);
				}
				modem_proxy_close(client);
				return OK;
			}
		}
	}
	return ERROR;
}
//modem->active
