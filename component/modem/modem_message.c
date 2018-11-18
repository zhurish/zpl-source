/*
 * modem_message.c
 *
 *  Created on: Jul 26, 2018
 *      Author: zhurish
 */



#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"

#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_dialog.h"
#include "modem_attty.h"
#include "modem_message.h"

#if 0
/*
 * AT+CMGF
 * set Message Format
 */
static int modem_message_format_set(modem_client_t *client)
{
	char cmd[1024];
	char buf[1024];
	assert(client);
	os_memset(buf, 0, sizeof(buf));
	os_memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "AT+CMGF=0");
	if(modem_attty_respone(client, 60, buf, sizeof(buf), cmd) == RES_OK)
	{
		sscanf(buf,"%s %*s", client->CCID_number);
		return OK;
	}
	return ERROR;
}

/*
 * AT+CPMS
 * set Preferred Message Storage
 */
static int modem_message_storage_set(modem_client_t *client)
{
	char cmd[1024];
	char buf[1024];
	assert(client);
	os_memset(buf, 0, sizeof(buf));
	os_memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "AT+CPMS=\"SM\",\"SM\",\"SM\"");
	if(modem_attty_respone(client, 60, buf, sizeof(buf), cmd) == RES_OK)
	{
		sscanf(buf,"%s %*s", client->CCID_number);
		return OK;
	}
	return ERROR;
}


/*
 * AT+CNMI
 * set Message report type
 */
static int modem_message_report_set(modem_client_t *client)
{
	char cmd[1024];
	char buf[1024];
	assert(client);
	os_memset(buf, 0, sizeof(buf));
	os_memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "AT+CNMI=\"SM\",\"SM\",\"SM\"");
	if(modem_attty_respone(client, 60, buf, sizeof(buf), cmd) == RES_OK)
	{
		sscanf(buf,"%s %*s", client->CCID_number);
		return OK;
	}
	return ERROR;
}

/*
 * AT+CSCA
 * set Message Server Center address
 */
static int modem_message_center_address_set(modem_client_t *client)
{
	char cmd[1024];
	char buf[1024];
	assert(client);
	os_memset(buf, 0, sizeof(buf));
	os_memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "AT+CSCA=\"SM\",\"SM\",\"SM\"");
	if(modem_attty_respone(client, 60, buf, sizeof(buf), cmd) == RES_OK)
	{
		sscanf(buf,"%s %*s", client->CCID_number);
		return OK;
	}
	return ERROR;
}

/*
 * AT+CMGS
 * set Message send
 */
static int modem_message_send_set(modem_client_t *client)
{
	char cmd[1024];
	char buf[1024];
	assert(client);
	os_memset(buf, 0, sizeof(buf));
	os_memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "AT+CNMI=\"SM\",\"SM\",\"SM\"");
	if(modem_attty_respone(client, 60, buf, sizeof(buf), cmd) == RES_OK)
	{
		sscanf(buf,"%s %*s", client->CCID_number);
		return OK;
	}
	return ERROR;
}


/*
 * AT+CMGL
 * set Message read
 */
static int modem_sms_read_set(modem_client_t *client)
{
	char cmd[1024];
	char buf[1024];
	assert(client);
	os_memset(buf, 0, sizeof(buf));
	os_memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "AT+CMGL=\"SM\",\"SM\",\"SM\"");
	if(modem_attty_respone(client, 60, buf, sizeof(buf), cmd) == RES_OK)
	{
		sscanf(buf,"%s %*s", client->CCID_number);
		return OK;
	}
	return ERROR;
}
#endif
