/*
 * modem_split.c
 *
 *  Created on: Aug 3, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_driver.h"

#include "modem_split.h"





const char * modem_module_name(modem_t *modem)
{
	assert(modem);
	if(modem && modem->client)
	{
		modem_client_t *client = modem->client;
		return (const char *)client->module_name;
	}
	return (const char *)"Unknown";
}


const char * modem_client_module_name(modem_client_t *client)
{
	assert(client);
	return (const char *)client->module_name;
}









int modem_operator_split(modem_client_t *client, char *buf)
{
	//+COPS: 0,0,"CHINA MOBILE",7
	//+COPS: 0
	//+COPS: 2
	int stat = 0, mode = 0, act = 0, format = 0;
	assert(client);
	assert(buf);
	if(os_strlen(buf) <= os_strlen("+COPS: 2"))
	{
		os_sscanf(buf,"%*s %d", &stat);
		client->nw_state = mode;
	}
	if(strchr_count(buf, ',') == 3)
	{
		//+COPS: 0,0,\"CHINA MOBILE\",7  OK"
		//
		zpl_uint32 offset = strchr_step(buf, ':', 1);
		if(offset)
		{
			//fprintf(stdout, "=========0=====%s\r\n", buf + offset + 1);
			mode = os_atoi(buf + offset + 1);
		}
		offset = strchr_step(buf, ',', 1);
		if(offset)
		{
			//fprintf(stdout, "========1======%s\r\n", buf + offset + 1);
			format = os_atoi(buf + offset + 1);
		}
		//os_sscanf(buf,"%*s %d,%d", &mode, &format);
		if(format == OP_LONG_FORMAT)
		{
			if(os_strstr(buf, "MOBILE"))
				client->operator = OPERATOR_MOBILE;
			else if(os_strstr(buf, "TELECOM"))
				client->operator = OPERATOR_TELECOM;
			else if(os_strstr(buf, "UNICOM"))
				client->operator = OPERATOR_UNICOM;

			offset = strchr_step(buf, ',', 3);
			if(offset)
				os_sscanf(buf + offset + 1, "%d", &act);
		}
		else if(format == OP_SHORT_FORMAT)
		{
			if(os_strstr(buf, "MOBILE"))
				client->operator = OPERATOR_MOBILE;
			else if(os_strstr(buf, "TELECOM"))
				client->operator = OPERATOR_TELECOM;
			else if(os_strstr(buf, "UNICOM"))
				client->operator = OPERATOR_UNICOM;

			offset = strchr_step(buf, ',', 3);
			if(offset)
				os_sscanf(buf + offset + 1, "%d", &act);
		}
		else if(format == OP_DIGIT_FORMAT)
		{
			int oper = 0;
			offset = strchr_step(buf, ',', 2);
			if(offset)
				os_sscanf(buf + offset + 1,"%x %d", &oper, &act);

			if(oper == 0x46000)
				client->operator = OPERATOR_MOBILE;
			else if(oper == 0x46003)
				client->operator = OPERATOR_TELECOM;
			else if(oper == 0x46001)
				client->operator = OPERATOR_UNICOM;
			else
				client->operator = oper;
		}
		//client->operator;		//当前注册运营商
		client->nw_state = mode;		//网络状态标识
		client->rat_state = (nw_rat_state)act;		//无线接入技术 Wireless access technology
	}
	return OK;
}

int modem_qnwinfo_split(modem_client_t *client, char *buf)
{
	assert(buf);
	assert(client);
	if(os_strstr(buf, "No Service"))
	{
		os_bzero(client->nw_act, sizeof(client->nw_act));
		os_bzero(client->nw_band, sizeof(client->nw_band));
		client->nw_channel = 0;
		client->operator = 0;
		return OK;
	}
	if(strchr_count(buf, ',') == 3)
	{
		//+QNWINFO: "GSM","46000","GSM 900",66
		//+QNWINFO: "TDD LTE","46000","LTE BAND 38",38098  OK
		//+QNWINFO: TDD LTE,46000,LTE BAND 41,40936  OK
		int oper = 0;
		//char tmp[64];
		strchr_empty(buf, '"');

		os_sscanf(buf,"%*[^ ] %[^,],%x,%[^,],%d", client->nw_act, &oper,
				client->nw_band, &client->nw_channel);
		client->operator = oper;

		//MODEM_DEBUG("split :%s,%x,%s,%d", client->nw_act, client->operator, client->nw_band, client->nw_channel);
		//strchr_empty(client->nw_band, ' ');
		//strchr_empty_step(client->nw_act, ' ', 1);
/*
“NONE”
“CDMA1X”
“CDMA1X AND HDR”
“CDMA1X AND EHRPD”
“HDR”
“HDR-EHRPD”
“GSM”
“GPRS”
“EDGE”
“WCDMA”
“HSDPA”
“HSUPA”
“HSPA+”
“TDSCDMA”
“TDD LTE”
“FDD LTE”
*/
	}
	return OK;
}

/*
 * +CREG: 2,1,"27A6","2CBCB17",7  OK
 * +QENG: "servingcell","NOCONN","LTE","TDD",
 * 		460,00,2CBCB17,15,38098,38,5,5,27A6,-88,-9,-58,4,35  OK
 */

int modem_pin_state_split(modem_client_t *client, char *buf)
{
	assert(buf);
	assert(client);
	if (buf)
	{
		if (os_strstr(buf, "ERROR"))
			client->cpin_status = CPIN_NONE;
		else if (os_strstr(buf, "READY"))
			client->cpin_status = CPIN_READY;
		else if (os_strstr(buf, "SIM PIN"))
			client->cpin_status = CPIN_SIM_PIN;
		else if (os_strstr(buf, "SIM PUK"))
			client->cpin_status = CPIN_SIM_PUK;
		else if (os_strstr(buf, "SIM PIN2"))
			client->cpin_status = CPIN_SIM_PIN2;
		else if (os_strstr(buf, "SIM PUK2"))
			client->cpin_status = CPIN_SIM_PUK2;
		else if (os_strstr(buf, "PH-NET PIN"))
			client->cpin_status = CPIN_PH_NET_PIN;
		else if (os_strstr(buf, "PH-NET PUK"))
			client->cpin_status = CPIN_PH_NET_PUK;
		else if (os_strstr(buf, "PH-NETSUB PIN"))
			client->cpin_status = CPIN_PH_NET_SUB_PIN;
		else if (os_strstr(buf, "PH-NETSUB PUK"))
			client->cpin_status = CPIN_PH_NET_SUB_PUK;
		else if (os_strstr(buf, "PH-SP PUK"))
			client->cpin_status = CPIN_PH_SP_PIN;
		else if (os_strstr(buf, "PH-SP PUK"))
			client->cpin_status = CPIN_PH_SP_PUK;
		else if (os_strstr(buf, "PH-CORP PUK"))
			client->cpin_status = CPIN_PH_CORP_PIN;
		else if (os_strstr(buf, "PH-CORP PUK"))
			client->cpin_status = CPIN_PH_CORP_PUK;
		return OK;
	}
	return ERROR;
}

/*
 *
 */
const char *modem_operator_svc_cmd(modem_t *modem)
{
	static char cmd[512];
	assert(modem);
	os_memset(cmd, 0, sizeof(cmd));
	modem_client_t *client = NULL;
	if(!modem || !modem->client)
		return (const char *)NULL;
	client = modem->client;
	switch(client->operator)
	{
	case OPERATOR_NONE:
		break;
	case OPERATOR_MOBILE:		//移动
		os_sprintf(cmd, "ATDT*98*%d#", modem->profile);
		break;
	case OPERATOR_UNICOM:		//联通
		os_sprintf(cmd, "ATDT%s", "*99#");
		break;
	case OPERATOR_TELECOM:		//电信
		os_sprintf(cmd, "ATDT%s", "#777");
		break;
	case OPERATOR_ATANDT:		//AT&T
		break;
	default:
		break;
	}
	if(os_strlen(cmd))
		return (const char *)cmd;
	return NULL;
}


const char *modem_operator_apn_cmd(modem_t *modem)
{
	static char cmd[512];
	assert(modem);
	os_memset(cmd, 0, sizeof(cmd));
	modem_client_t *client = NULL;
	if(!modem || !modem->client)
		return NULL;
	client = modem->client;
	switch(client->operator)
	{
	case OPERATOR_NONE:
		break;
	case OPERATOR_MOBILE:		//移动
		os_sprintf(cmd, "%s", "CMNET");
		break;
	case OPERATOR_UNICOM:		//联通
		os_sprintf(cmd, "%s", "3GNET");
		break;
	case OPERATOR_TELECOM:		//电信
		os_sprintf(cmd, "%s", "CTNET");
		break;
	case OPERATOR_ATANDT:		//AT&T
		break;
	}
	return cmd;
}

const char *modem_pdp_cmd(modem_t *modem)
{
	//AT+CGDCONT=1,,,,,
	static char cmd[512];
	char ip[64];
	assert(modem);
	//char apn[128];
	os_memset(cmd, 0, sizeof(cmd));
	os_memset(ip, 0, sizeof(ip));
	//os_memset(apn, 0, sizeof(apn));

	if(modem->ipstack == MODEM_BOTH)
		os_sprintf(ip, "%s", "IPV4V6");
	else if(modem->ipstack == MODEM_IPV4)
		os_sprintf(ip, "%s", "IP");
	else if(modem->ipstack == MODEM_IPV6)
		os_sprintf(ip, "%s", "IPV6");
	else
		os_sprintf(ip, "%s", "PPP");

	if(os_strlen(modem->apn))
		os_sprintf(cmd, "AT+CGDCONT=%d,\"%s\",\"%s\",,,", modem->profile, ip, modem->apn);
	else
		os_sprintf(cmd, "AT+CGDCONT=%d,\"%s\",,,,", modem->profile, ip);

	return cmd;
}

const char *modem_svc_cmd(modem_t *modem)
{
	static char cmd[512];
	assert(modem);
	os_memset(cmd, 0, sizeof(cmd));
	if(os_strlen(modem->svc))
		os_sprintf(cmd, "ATDT%s", modem->svc);
	else
	{
		return modem_operator_svc_cmd(modem);
	}
	return cmd;
}
