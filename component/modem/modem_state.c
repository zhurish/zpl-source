/*





























 * modem_state.c
 *
 *  Created on: Aug 12, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_state.h"
//#include "modem_machine.h"



const char *modem_dial_string(modem_dial_type type)
{
	switch(type)
	{
	case MODEM_DIAL_NONE:
		return " ";
		break;
	case MODEM_DIAL_DHCP:
		return "DHCP";
		break;
	case MODEM_DIAL_QMI:
		return "QMI";
		break;
	case MODEM_DIAL_GOBINET:
		return "GOBINET";
		break;
	case MODEM_DIAL_PPP:
		return "PPP";
		break;
	default:
		break;
	}
	return "UNKNOW";
}

const char *modem_network_type_string(modem_network_type type)
{
	switch(type)
	{
	case NETWORK_AUTO:
		return "auto";
		break;
	case NETWORK_GSM:
		return "gsm";
		break;
	case NETWORK_CDMA1X:
		return "cdma1x";
		break;
	case NETWORK_GPRS:
		return "gprs";
		break;
	case NETWORK_HSCSD:
		return "hscsd";
		break;
	case NETWORK_WAP:
		return "wap";
		break;
	case NETWORK_EDGE:
		return "edge";
		break;
	case NETWORK_TD_SCDMA:
		return "td-scdma";
		break;
	case NETWORK_WCDMA:
		return "wcdma";
		break;
	case NETWORK_EVDO:
		return "evdo";
		break;
	case NETWORK_TD_LTE:
		return "td-lte";
		break;
	case NETWORK_FDD_LTE:
		return "fdd-lte";
		break;
	default:
		break;
	}
	return "UNKNOW";
}

modem_network_type modem_network_type_get(const char * type)
{
	if(strncasecmp( "auto", type, 3) == 0)
		return NETWORK_AUTO;
	else if(strncasecmp( "gsm", type, 3) == 0)
		return NETWORK_GSM;
	else if(strncasecmp( "cdma1x", type, 3) == 0)
		return NETWORK_CDMA1X;
	else if(strncasecmp( "gprs", type, 3) == 0)
		return NETWORK_GPRS;
	else if(strncasecmp( "hscsd", type, 3) == 0)
		return NETWORK_HSCSD;
	else if(strncasecmp( "wap", type, 3) == 0)
		return NETWORK_WAP;
	else if(strncasecmp( "edge", type, 3) == 0)
		return NETWORK_EDGE;
	else if(strncasecmp( "wcdma", type, 3) == 0)
		return NETWORK_WCDMA;
	else if(strncasecmp( "evdo", type, 3) == 0)
		return NETWORK_EVDO;
	else
	{
		if(strlen(type) >=4)
		{
			if(strncasecmp( "td-scdma", type, 3) == 0)
				return NETWORK_TD_SCDMA;
			else if(strncasecmp( "td-lte", type, 3) == 0)
				return NETWORK_TD_LTE;
			else if(strncasecmp( "fdd-lte", type, 3) == 0)
				return NETWORK_FDD_LTE;
		}
	}
	return NETWORK_AUTO;
}

int modem_network_type_id(modem_network_type type)
{
	switch(type)
	{
	case NETWORK_AUTO:
		return 0;
		break;
	case NETWORK_GSM:
		return 1;
		break;
	case NETWORK_CDMA1X:
		return 6;
		break;
	case NETWORK_GPRS:
		return 0;
		break;
	case NETWORK_HSCSD:
		return 0;
		break;
	case NETWORK_WAP:
		return 0;
		break;
	case NETWORK_EDGE:
		return 0;
		break;
	case NETWORK_TD_SCDMA:
		return 4;
		break;
	case NETWORK_WCDMA:
		return 2;
		break;
	case NETWORK_EVDO:
		return 0;
		break;
	case NETWORK_TD_LTE:
		return 3;
		break;
	case NETWORK_FDD_LTE:
		return 3;
		break;
	default:
		break;
	}
	return 0;
}
/*
AT+CSQ
Signal Quality 信号强度指示.返回信号强度和信道误码率:+ CSQ:<rssi>,<ber>.
<rssi>
0        <=-113dBm
1        -111dBm
2-30   -109到-53dBm
31      -51dBm or greater
99      未知或不可检测
<ber> 0 …..7 表示误码率由低到高,99 表示未知.
 0 BER < 0,2 %
 1 0,2 % < BER < 0,4 %
 2 0,4 % < BER < 0,8 %
 3 0,8 % < BER < 1,6 %
 4 1,6 % < BER < 3,2 %
 5 3,2 % < BER < 6,4 %
 6 6,4 % < BER < 12,8 %
 7 12,8 % < BER
99 未知或不可测
AT+CSQ=?	返回信号强度的范围,比如:+CSQ: (0-31,99),(0-7,99)
AT+CSQ	查询当前的信号强度
*/
int modem_signal_state_update(modem_client_t *client)
{
	if(client)
	{
		MODEM_SIGNAL_CLR(client->signal_state);

		if(client->signal && client->signal < MODEM_SIGNAL_LOW)
			MODEM_SIGNAL_SET(client->signal_state, MODEM_STATE_SIGNAL_READY);

		else if(client->signal > MODEM_SIGNAL_LOW )
			MODEM_SIGNAL_SET(client->signal_state, MODEM_STATE_SIGNAL_LOSS);
		else
			MODEM_SIGNAL_SET(client->signal_state, MODEM_STATE_SIGNAL_NONE);


		if(client->signal > MODEM_BITERR_HIGH )
			MODEM_SIGNAL_SET(client->signal_state, MODEM_STATE_BITERR_HIGH);
		else
			MODEM_SIGNAL_SET(client->signal_state, MODEM_STATE_BITERR_READY);
	}
	return OK;
}


modem_signal_state modem_signal_state_get(modem_client_t *client)
{
	if(client)
	{
		return client->signal_state;
	}
	return MODEM_STATE_SIGNAL_NONE;
}

/*modem_machine modem_signal_state(modem_client_t *client)
{
	if(client)
	{
		if( MODEM_SIGNAL_GET(client->signal_state, MODEM_STATE_SIGNAL_NONE)
				|| MODEM_SIGNAL_GET(client->signal_state, MODEM_STATE_SIGNAL_LOSS)
				|| MODEM_SIGNAL_GET(client->signal_state, MODEM_STATE_BITERR_HIGH) )
			return MDMS(LOSS_SIGNAL);
		return client->signal_state;
	}
	return MDMS(NONE);
}*/


int modem_register_state(modem_client_t *client, zpl_uint32 code)
{
	assert(client);
	client->nw_register_state = (nw_register_state)code;
	return OK;
}

/* CPIN */
modem_cpin_en modem_usim_state(modem_t *modem)
{
	assert(modem);
	if(modem && modem->client)
		return ((modem_client_t *)modem->client)->cpin_status;
	return CPIN_NONE;
}

