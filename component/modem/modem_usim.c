/*
 * modem_sim.c
 *
 *  Created on: Jul 28, 2018
 *      Author: zhurish
 */



#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"

#include "os_list.h"
#include "os_util.h"
#include "tty_com.h"
#include "modem_enum.h"
#include "modem.h"
#include "modem_client.h"
#include "modem_attty.h"
#include "modem_machine.h"
#include "modem_event.h"
#include "modem_pppd.h"
#include "modem_usim.h"
#include "modem_atcmd.h"



/*
 *
1.AT+CLCK="SC",1,"1234"	使用SIM卡锁功能,密码为为上次设置的(或初始密码),或者使用CPWD更改后的密码

2.AT+CLCK="SC",2	查询SIM卡锁功能

3.AT+CLCK="SC",0,"1234"	关闭SIM卡锁功能

4.AT+CPWD="SC","1234","4321"	修改PIN码

5.AT+CPIN?	查询PIN码的状态,是否需要输入PIN码

6.AT+CPIN="1234"	输入PIN码

7.AT+CPIN="88139522","1234"	输入PUK码和新的PIN码
 *
 */

static int modem_cpin_lock_atcmd_get(modem_client_t *client, BOOL *enable)
{
	assert(client);
	assert(enable);
	if(modem_attty_respone(client, MODEM_TIMEOUT(5), NULL,
			0, "AT+CLCK=\"SC\",2\r\n") > RES_OK)
	{
		MODEM_CMD_DEBUG("AT+CLCK=\"SC\",2:%s",client->response->buf);
		//enable = buf + os_strlen("+CLCK: ");
		sscanf(client->response->buf,"%*s %d", enable);
		return OK;
	}
	return ERROR;

}

/*static int modem_cpin_lock_atcmd_set(modem_client_t *client, BOOL enable, char *pin)
{
	assert(client);
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT+CLCK=\"SC\",%d,\"%s\"",
			enable, pin) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}


static int modem_cpin_passwd_atcmd_set(modem_client_t *client, char *old, char *pin)
{
	assert(client);
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+CPWD=\"SC\",\"%s\",\"%s\"", old, pin) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}*/


static int modem_cpin_pin_atcmd_set(modem_client_t *client, char *pin)
{
	assert(client);
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT+CPIN=\"%s\"", pin)
			== RES_OK)
	{
		return OK;
	}
	return ERROR;
}

static int modem_cpin_puk_pin_atcmd_set(modem_client_t *client, char *puk, char *pin)
{
	assert(client);
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT+CPIN=\"%s\",\"%s\"",
			puk, pin) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

/*
 * AT+CPIN
 */
static int modem_cpin_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CPIN?\r\n") > RES_OK)
	{
		MODEM_CMD_DEBUG("AT+CPIN?:%s", client->response->buf);
		modem_pin_state_split(client, client->response->buf);
		return OK;
	}
	return ERROR;
}


static int modem_sim_pin_control(modem_t *modem)
{
	int ret = OK;
	modem_client_t *client = NULL;
	assert(modem);
	if(!modem || !modem->client)
		return ERROR;
	client = modem->client;
	if(client->pin_lock)
	{
		switch(client->cpin_status)
		{
		case CPIN_NONE:
			break;
		case CPIN_READY:
			break;
		case CPIN_SIM_PIN:
			if(os_strlen(modem->pin))
			{
				ret = modem_cpin_pin_atcmd_set(modem->client, modem->pin);
				if(ret == OK)
					ret = modem_cpin_atcmd_get(modem->client);
			}
			break;
		case CPIN_SIM_PUK:
			if(os_strlen(modem->puk) && os_strlen(modem->pin))
			{
				ret = modem_cpin_puk_pin_atcmd_set(modem->client, modem->puk, modem->pin);
				if(ret == OK)
					ret = modem_cpin_atcmd_get(modem->client);
			}
			break;
		case CPIN_SIM_PIN2:
			break;
		case CPIN_SIM_PUK2:
			break;
		case CPIN_PH_NET_PIN:
			break;
		case CPIN_PH_NET_PUK:
			break;
		case CPIN_PH_NET_SUB_PIN:
			break;
		case CPIN_PH_NET_SUB_PUK:
			break;
		case CPIN_PH_SP_PIN:
			break;
		case CPIN_PH_SP_PUK:
			break;
		case CPIN_PH_CORP_PIN:
			break;
		case CPIN_PH_CORP_PUK:
			break;
		default:
			break;
		}
	}
	if(ret == ERROR)
		return ERROR;
	return OK;
}





int modem_usim_detection(modem_t *modem)
{
	//int ret = OK;
	modem_client_t *client = NULL;
	assert(modem);
	if(!modem || !modem->client)
		return ERROR;
	client = modem->client;
	if(modem_cpin_atcmd_get(modem->client) != OK)
		return ERROR;
	if(client->cpin_status == CPIN_NONE)
		return ERROR;
	if(modem_cpin_lock_atcmd_get(modem->client, &client->pin_lock))
		return ERROR;

	if(modem_sim_pin_control(modem))
		return ERROR;

	if(modem_IMSI_atcmd_get(client) != OK)
		return ERROR;

	if(modem_CCID_atcmd_get(client) != OK)
		return ERROR;
	return OK;
}


/*int modem_cpin_atcmd_set(modem_client_t *client)
{
	char buf[1024];
	os_memset(buf, 0, sizeof(buf));
	if(client->driver.atcmd.md_cpin_set_cmd)
	{
		int ret = 0;
		ret = (client->driver.atcmd.md_cpin_set_cmd)(&client->driver, buf);

		return ret;
	}
	else
	{
		char cmd[512];
		os_memset(cmd, 0, sizeof(cmd));
		modem_t * modem = client->modem;
		if(os_strlen(modem->puk) && os_strlen(modem->pin))
			snprintf(cmd, sizeof(cmd), "AT+CPIN=%s,%s\r\n", modem->puk, modem->pin);
		else
			snprintf(cmd, sizeof(cmd), "AT+CPIN=%s\r\n", modem->pin);

		if(modem_attty(client, 60, "OK", cmd) == RES_OK)
		{
			return OK;
		}
	}
	return ERROR;
}*/


/*
 * Network service
 */
/*
 * AT+CIMI  IMSI OF SIM
 */
