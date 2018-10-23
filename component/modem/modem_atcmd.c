/*
 * modem_atcmd.c
 *
 *  Created on: Jul 25, 2018
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
#include "modem_atcmd.h"
#include "modem_client.h"
#include "modem_driver.h"
#include "modem_usb_driver.h"

/*
 * AT
 */

int modem_atcmd_isopen(modem_client_t *client)
{
	assert(client);
	//MODEM_CMD_DEBUG("%s", __func__);
	if (modem_attty_isopen(client) == OK)
	{
		return modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT\r\n");
	}
	return ERROR;
}

int modem_echo_atcmd_set(modem_client_t *client, BOOL enable)
{
	assert(client);
	if (client->driver->atcmd.md_echo_cmd)
		return (client->driver->atcmd.md_echo_cmd)(&client->driver);
	else
	{
		if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "ATE%d\r\n", enable)
				== RES_OK)
		{
			return OK;
		}
	}
	return ERROR;
}

int modem_swreset_atcmd_set(modem_client_t *client)
{
	assert(client);
	if (client->driver->atcmd.md_swreset_cmd)
		return (client->driver->atcmd.md_swreset_cmd)(&client->driver);
	else
	{
		if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT\r\n") == RES_OK)
		{
			return OK;
		}
	}
	return ERROR;
}

int modem_reboot_atcmd_set(modem_client_t *client)
{
	assert(client);
	if (client->driver->atcmd.md_reboot_cmd)
		return (client->driver->atcmd.md_reboot_cmd)(&client->driver);
	else
	{
		if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT\r\n") == RES_OK)
		{
			return OK;
		}
	}
	return ERROR;
}

int modem_save_atcmd_set(modem_client_t *client)
{
	assert(client);
	if (client->driver->atcmd.md_save_cmd)
		return (client->driver->atcmd.md_save_cmd)(&client->driver);
	else
	{
		if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT&W\r\n") == RES_OK)
		{
			return OK;
		}
	}
	return ERROR;
}

int modem_open_atcmd_set(modem_client_t *client)
{
	assert(client);
	if (client->driver->atcmd.md_open_cmd)
		return (client->driver->atcmd.md_open_cmd)(&client->driver, 1);
	else
	{
		if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT+CFUN=%d,0\r\n", 1)
				== RES_OK)
		{
			return OK;
		}
	}
	return ERROR;
}

/*
 * AT+CGMI
 * get factory name 获取厂商名称
 */
int modem_factory_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (client->driver->atcmd.md_factory_cmd)
		return (client->driver->atcmd.md_factory_cmd)(&client->driver,
				client->factory_name);
	else
	{
		if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
				"AT+CGMI\r\n") > RES_OK)
		{
			MODEM_CMD_DEBUG("AT+CGMI:%s", client->response->buf);
			os_bzero(client->factory_name, sizeof(client->factory_name));
			os_sscanf(client->response->buf, "%s %*s", client->factory_name);
			return OK;
		}
	}
	return ERROR;
}

/*
 * AT+CGMM
 * get product name 获取模块名称
 */
int modem_product_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (client->driver->atcmd.md_product_cmd)
		return (client->driver->atcmd.md_product_cmd)(&client->driver,
				client->product_name);
	else
	{
		if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
				"AT+CGMM\r\n") > RES_OK)
		{
			os_bzero(client->product_name, sizeof(client->product_name));
			MODEM_CMD_DEBUG("AT+CGMM:%s", client->response->buf);
			os_sscanf(client->response->buf, "%s %*s", client->product_name);
			return OK;
		}
	}
	return ERROR;
}

/*
 * AT+CGMR
 * get product id 获取模块序列号
 */
int modem_product_id_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CGMR\r\n") > RES_OK)
	{
		os_bzero(client->product_iden, sizeof(client->product_iden));
		MODEM_CMD_DEBUG("AT+CGMR:%s", client->response->buf);
		os_sscanf(client->response->buf, "%s %*s", client->product_iden);
		return OK;
	}

	return ERROR;
}

/*
 * AT+CGMR
 * get version 获取模块软件版本
 */
int modem_version_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (client->driver->atcmd.md_vetsion_cmd)
		return (client->driver->atcmd.md_vetsion_cmd)(&client->driver,
				client->version);
	else
	{
		if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
				"AT+CGMR\r\n") > RES_OK)
		{
			os_bzero(client->version, sizeof(client->version));
			MODEM_CMD_DEBUG("AT+CGMR:%s", client->response->buf);
			os_sscanf(client->response->buf, "%s %*s", client->version);
			return OK;
		}
	}
	return ERROR;
}

/*
 * AT+CGSN -> IMEI   AT+GSN  -> IMEI
 * get IMEI 获取模块 IMEI
 */
int modem_serial_number_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (client->driver->atcmd.md_serial_number_cmd)
		return (client->driver->atcmd.md_serial_number_cmd)(&client->driver,
				client->serial_number);
	else
	{
		if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
				"AT+CGSN\r\n") > RES_OK)
		{
			os_bzero(client->serial_number, sizeof(client->serial_number));
			MODEM_CMD_DEBUG("AT+CGSN:%s", client->response->buf);
			os_sscanf(client->response->buf, "%s %*s", client->serial_number);
			return OK;
		}
	}
	return ERROR;
}

int modem_IMEI_atcmd_get(modem_client_t *client)
{
	assert(client);
	{
		if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
				"AT+GSN\r\n") > RES_OK)
		{
			os_bzero(client->IMEI_number, sizeof(client->IMEI_number));
			MODEM_CMD_DEBUG("AT+GSN:%s", client->response->buf);
			os_sscanf(client->response->buf, "%s %*s", client->IMEI_number);
			return OK;
		}
	}
	return ERROR;
}

/*
 * AT+CQS
 */
int modem_signal_atcmd_get(modem_client_t *client)
{
	assert(client);
	/*	if(client->driver->atcmd.md_signal_cmd)
	 return (client->driver->atcmd.md_signal_cmd)(&client->driver, &client->signal, &client->bit_error);
	 else*/

	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CSQ\r\n") > RES_OK)
	{
		//char tmp[512];
		//os_memset(tmp, 0, sizeof(tmp));
		//os_bzero(client->serial_number, sizeof(client->serial_number));
		os_sscanf(client->response->buf, "%*s%d,%d", &client->signal, &client->bit_error);
		MODEM_CMD_DEBUG("AT+CSQ:%s (%d,%d)", client->response->buf, client->signal,
				client->bit_error);
		return OK;
	}

	return ERROR;
}

int modem_activity_atcmd_get(modem_client_t *client)
{
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CPAS\r\n") > RES_OK)
	{
		MODEM_CMD_DEBUG("AT+CPAS:%s", client->response->buf);
		os_sscanf(client->response->buf, "%*s: %d", client->activity);
		return OK;
	}

	return ERROR;
}

/* Network status control
 * AT+QCFG
 */
int modem_gprsattach_atcmd_set(modem_client_t *client)
{
	assert(client);
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+QCFG=\"gprsattach\",%d\r\n", 0) == RES_OK)
	{
		//os_sscanf(buf,"+%s: %d", client->activity);
		return OK;
	}

	return ERROR;
}

int modem_nwscanmode_atcmd_set(modem_client_t *client)
{
	assert(client);

	modem_t *modem = client->modem;
	if (!modem)
		return ERROR;
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+QCFG=\"nwscanmode\",%d,1\r\n", modem_network_type_id(modem->network)) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_nwscanseq_atcmd_set(modem_client_t *client)
{
	assert(client);

	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+QCFG=\"nwscanseq\",%d\r\n", 0) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_roamservice_atcmd_set(modem_client_t *client)
{
	assert(client);

	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+QCFG=\"roamservice\",%d,1\r\n", 0) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_servicedomain_atcmd_set(modem_client_t *client)
{
	assert(client);

	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+QCFG=\"servicedomain\",%d,1\r\n", 0) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_nwband_atcmd_set(modem_client_t *client)
{
	assert(client);

	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+QCFG=\"band\",%d,%d,%d,1\r\n", 0, 0, 0) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_hsdpacat_atcmd_set(modem_client_t *client)
{
	assert(client);

	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+QCFG=\"hsdpacat\",%d\r\n", 0) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_hsupacat_atcmd_set(modem_client_t *client)
{
	assert(client);

	if (modem_attty(client, MODEM_TIMEOUT(5), "OK",
			"AT+QCFG=\"hsupacat\",%d\r\n", 0) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_sgsn_atcmd_set(modem_client_t *client)
{
	assert(client);

	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT+QCFG=\"sgsn\",%d\r\n",
			0) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_tdscsq_atcmd_set(modem_client_t *client)
{
	assert(client);

	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT+QCFG=\"tdscsq\",%d\r\n",
			0) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_nwpdp_atcmd_set(modem_client_t *client)
{
	assert(client);
	assert(client->modem);
	modem_t *modem = client->modem;
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "%s\r\n",
			modem_pdp_cmd(modem)) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_nwpdp_atcmd_enable(modem_client_t *client, BOOL enable)
{
	assert(client);
	//modem_t *modem = client->modem;
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT+CGACT=%d,1\r\n",
			enable) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_nwinfo_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+QNWINFO\r\n") > RES_OK)
	{
		//+QNWINFO: "GSM","46000","GSM 900",66
		MODEM_CMD_DEBUG("AT+QNWINFO:%s", client->response->buf);
		modem_qnwinfo_split(client, client->response->buf);
		//os_sscanf(buf,"+%s: %d", client->activity);
		return OK;
	}
	return ERROR;
}


int modem_nwaddr_atcmd_get(modem_client_t *client)
{
	assert(client);
	//if(!modem)
	//	return ERROR;
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CGPADDR=%d\r\n", 1) > RES_OK)
	{
		//+CGPADDR: 1,"10.76.51.180"
		int offset = 0;
		char address[128];
		char *brk = NULL;

		if(os_strstr(client->response->buf, "ERROR"))
		{
			os_bzero(&client->prefix, sizeof(struct prefix));
			return ERROR;
		}
		strchr_empty(client->response->buf, '"');
		MODEM_CMD_DEBUG("AT+CQPADDR:%s", client->response->buf);
		if(os_strlen(client->response->buf) >= os_strlen("+CGPADDR: 1,1.1.1.1"))
		{
			offset = strchr_step(client->response->buf, ',', 1);
			brk = client->response->buf + offset + 1;
			if(brk)
			{
				os_memset(address, 0, sizeof(address));
				os_sscanf(brk, "%[^ ]", address);
				if(os_strlen(address))
				{
					strcat(address, "/32");
					str2prefix(address, &client->prefix);
					return OK;
				}
			}
		}
		return OK;
	}
	return ERROR;
}

int modem_nwservingcell_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+QENG=\"servingcell\"\r\n") > RES_OK)
	{
		//+QENG: "servingcell","LIMSRV","GSM",460,00,27A6,10ED,22,66,-,-48,255,255,0,57,181,1,-,-,-,-,-,-,-,-,-,"-"
		MODEM_CMD_DEBUG("AT+QENG:%s", client->response->buf);
		//os_sscanf(buf,"+%s: %d", client->activity);
		return OK;
	}
	return ERROR;
}

int modem_nwcell_atcmd_set(modem_client_t *client, BOOL enable)
{
	assert(client);

	if(enable == FALSE)
	{
		if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT$QCRMCALL=0,1\r\n") == RES_OK)
		{
			return OK;
		}
		return ERROR;
	}
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT$QCRMCALL=1,1,1,2,1\r\n") == RES_OK)
	{
		//+QENG: "servingcell","LIMSRV","GSM",460,00,27A6,10ED,22,66,-,-48,255,255,0,57,181,1,-,-,-,-,-,-,-,-,-,"-"
		//MODEM_CMD_DEBUG("AT+QNWINFO:%s",buf);
		//os_sscanf(buf,"+%s: %d", client->activity);
		return OK;
	}
	return ERROR;
}

int modem_nwreq_addr_atcmd_get(modem_client_t *client)
{
	assert(client);
	assert(client->modem);
	modem_t *modem = client->modem;
	if(!modem)
		return ERROR;
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CQPADDR=%d\r\n", modem->profile) > RES_OK)
	{
		//+CGPADDR: 1,"10.76.51.180"
		int offset = 0;
		char *brk = NULL;
		if(os_strstr(client->response->buf, "ERROR"))
		{
			os_bzero(&client->prefix, sizeof(struct prefix));
			return ERROR;
		}
		strchr_empty(client->response->buf, '"');
		if(os_strlen(client->response->buf) >= os_strlen("+CGPADDR: 1,1.1.1.1"))
		{
			offset = strchr_step(client->response->buf, ',', 1);
			brk = client->response->buf + offset + 1;
			MODEM_CMD_DEBUG("AT+CQPADDR:%s", client->response->buf);

			if(brk && atoi(brk))
			{
				str2prefix(brk, &client->prefix);
				return OK;
			}
		}
		return ERROR;
	}
	return ERROR;
}

/*
 * (U)SIM
 */
/*
 * AT+CIMI  IMSI OF SIM
 */
int modem_IMSI_atcmd_get(modem_client_t *client)
{
	assert(client);

	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CIMI\r\n") > RES_OK)
	{
		//char tmp[512];
		//os_memset(tmp, 0, sizeof(tmp));
		os_bzero(client->IMSI_number, sizeof(client->IMSI_number));
		MODEM_CMD_DEBUG("AT+CIMI:%s", client->response->buf);
		os_sscanf(client->response->buf, "%s %*s", client->IMSI_number);
		return OK;
	}
	return ERROR;
}

/*
 * AT+CIMI
 */
int modem_CCID_atcmd_get(modem_client_t *client)
{
	assert(client);

	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CCID\r\n") > RES_OK)
	{
		os_bzero(client->CCID_number, sizeof(client->CCID_number));
		MODEM_CMD_DEBUG("AT+CCID:%s", client->response->buf);
		os_sscanf(client->response->buf + os_strlen("+CCID: "), "%s", client->CCID_number);
		return OK;
	}
	return ERROR;
}

/*
 * AT+CREG?
 * 获取小区信息
 */

int modem_cell_information_atcmd_set(modem_client_t *client)
{
	assert(client);
	if (modem_attty(client, MODEM_TIMEOUT(5), "OK", "AT+CREG=%d\r\n",
			client->nw_register_type) == RES_OK)
	{
		return OK;
	}
	return ERROR;
}

int modem_cell_information_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+CREG?\r\n") > RES_OK)
	{
		int state = 0, num = 0;
		//+CREG: 2,1
		//+CREG: 2,1,"27A6","2CBCB2B",7
		//int lac = 0, ci = 0;
		strchr_empty(client->response->buf, '"');
		if (strchr_count(client->response->buf, ',') == 4)
		{
			os_bzero(client->LAC, sizeof(client->LAC));
			os_bzero(client->CI, sizeof(client->CI));
			//+CREG: 2,1,27A6,2CBCB17,7
			os_sscanf(client->response->buf, "%*[^ ] %*d,%d,%[^,],%[^,],%d", &state, client->LAC,
					client->CI, &num);
			//strchr_empty(client->LAC, '"');
			//strchr_empty(client->CI, '"');
			//os_snprintf(client->LAC, sizeof(client->LAC), "%x", lac);
			//os_snprintf(client->CI, sizeof(client->CI), "%x", ci);

			MODEM_CMD_DEBUG("AT+CREG?:%s(%s,%s)", client->response->buf, client->LAC, client->CI);
			modem_register_state(client, state);
		}
		else if (strchr_count(client->response->buf, ',') == 1)
		{
			os_sscanf(client->response->buf, "%*s %*d,%d", &state);
			MODEM_CMD_DEBUG("AT+CREG?:%s", client->response->buf);
			modem_register_state(client, state);
		}
		//MODEM_CMD_DEBUG("AT+CREG?===:%s", buf);
		return OK;
	}
	return ERROR;
}

/*
 * AT+COPS?
 * 获取运营商信息
 */
int modem_operator_atcmd_get(modem_client_t *client)
{
	assert(client);
	if (modem_attty_respone(client, MODEM_TIMEOUT(5), NULL, 0,
			"AT+COPS?\r\n") > RES_OK)
	{
		modem_operator_split(client, client->response->buf);
		MODEM_CMD_DEBUG("AT+COPS?:%s", client->response->buf);
		return OK;
	}

	return ERROR;
}
