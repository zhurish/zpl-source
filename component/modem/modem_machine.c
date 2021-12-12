/*
 * modem_machine.c
 *
 *  Created on: Jul 26, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_machine.h"
#include "modem_event.h"
#include "modem_state.h"



static modem_machine modem_machine_state_update(modem_t *modem);

static modem_machine_action modem_machine_tbl[MODEM_MACHINE_STATE_MAX][MODEM_MACHINE_STATE_MAX] =
{
	{
		{MDMS(NONE),		MDMS(NONE),				MODEM_EV_NONE},
		{MDMS(NONE),		MDMS(NO_USIM_CARD),		MODEM_EV_NONE},
		{MDMS(NONE),		MDMS(NO_SIGNAL),		MODEM_EV_NONE},
		{MDMS(NONE),		MDMS(NO_ADDR),		MODEM_EV_NONE},
		{MDMS(NONE),		MDMS(NO_SERVICE),		MODEM_EV_NONE},
		{MDMS(NONE),		MDMS(NETWORK_ACTIVE),	MODEM_EV_NONE},
	},
	{
		{MDMS(NO_USIM_CARD),		MDMS(NONE),				MODEM_EV_NONE},
		{MDMS(NO_USIM_CARD),		MDMS(NO_USIM_CARD),		MODEM_EV_NONE},
		{MDMS(NO_USIM_CARD),		MDMS(NO_SIGNAL),		MODEM_EV_NONE},
		{MDMS(NO_USIM_CARD),		MDMS(NO_ADDR),		MODEM_EV_NONE},
		{MDMS(NO_USIM_CARD),		MDMS(NO_SERVICE),		MODEM_EV_NONE},
		{MDMS(NO_USIM_CARD),		MDMS(NETWORK_ACTIVE),	MODEM_EV_NONE},
	},
	{
		{MDMS(NO_SIGNAL),		MDMS(NONE),				MODEM_EV_NONE},
		{MDMS(NO_SIGNAL),		MDMS(NO_USIM_CARD),		MODEM_EV_REMOVE_CARD},
		{MDMS(NO_SIGNAL),		MDMS(NO_SIGNAL),		MODEM_EV_NONE},
		{MDMS(NO_SIGNAL),		MDMS(NO_ADDR),		MODEM_EV_NONE},
		{MDMS(NO_SIGNAL),		MDMS(NO_SERVICE),		MODEM_EV_NONE},
		{MDMS(NO_SIGNAL),		MDMS(NETWORK_ACTIVE),	MODEM_EV_NONE},
	},
	{
		{MDMS(NO_ADDR),		MDMS(NONE),				MODEM_EV_NONE},
		{MDMS(NO_ADDR),		MDMS(NO_USIM_CARD),		MODEM_EV_REMOVE_CARD},
		{MDMS(NO_ADDR),		MDMS(NO_SIGNAL),		MODEM_EV_NONE},
		{MDMS(NO_ADDR),		MDMS(NO_ADDR),		MODEM_EV_NONE},
		{MDMS(NO_ADDR),		MDMS(NO_SERVICE),		MODEM_EV_NONE},
		{MDMS(NO_ADDR),		MDMS(NETWORK_ACTIVE),	MODEM_EV_NONE},
	},
	{
		{MDMS(NO_SERVICE),		MDMS(NONE),				MODEM_EV_NONE},
		{MDMS(NO_SERVICE),		MDMS(NO_USIM_CARD),		MODEM_EV_REMOVE_CARD},
		{MDMS(NO_SERVICE),		MDMS(NO_SIGNAL),		MODEM_EV_NONE},
		{MDMS(NO_SERVICE),		MDMS(NO_ADDR),		MODEM_EV_NONE},
		{MDMS(NO_SERVICE),		MDMS(NO_SERVICE),		MODEM_EV_NONE},
		{MDMS(NO_SERVICE),		MDMS(NETWORK_ACTIVE),	MODEM_EV_NONE},
	},
	{
		{MDMS(NETWORK_ACTIVE),		MDMS(NONE),				MODEM_EV_NONE},
		{MDMS(NETWORK_ACTIVE),		MDMS(NO_USIM_CARD),		MODEM_EV_REMOVE_CARD},
		{MDMS(NETWORK_ACTIVE),		MDMS(NO_SIGNAL),		MODEM_EV_OFFLINE},
		{MDMS(NETWORK_ACTIVE),		MDMS(NO_ADDR),		MODEM_EV_OFFLINE},
		{MDMS(NETWORK_ACTIVE),		MDMS(NO_SERVICE),		MODEM_EV_OFFLINE},
		{MDMS(NETWORK_ACTIVE),		MDMS(NETWORK_ACTIVE),	MODEM_EV_NONE},
	},
};


const char *modem_machine_state_string(modem_machine state)
{
	switch(state)
	{
	case MDMS(NONE): //检测
		return "NONE";
		break;
	case MDMS(NO_USIM_CARD): //检测
		return "NO USIM CARD";
		break;
	case MDMS(NO_SIGNAL): //检测
		return "NO SIGNAL";
		break;

	case MDMS(NO_ADDR): //检测
		return "NO ADDRESS";
		break;
	case MDMS(NO_SERVICE): //检测
		return "NO SERVICE";
		break;
	case MDMS(NETWORK_ACTIVE):	//模块拔除
		return "NETWORK ACTIVE";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

int	modem_machine_state_action(modem_t *modem)
{
	modem_event event = MODEM_EV_NONE;
	assert(modem);

	modem->newstate = modem_machine_state_update(modem);

	if(modem->state == modem->newstate)
		return OK;

	event = modem_machine_tbl[modem->state][modem->newstate].event;

	MODEM_DEBUG("state %s > %s",
			modem_machine_state_string(modem->state), modem_machine_state_string(modem->newstate));

	if(modem->state != MDMS(NETWORK_ACTIVE) && modem->newstate == MDMS(NETWORK_ACTIVE))
	{
		modem->delay = MODEM_DELAY_CHK_TIME;
		modem->dedelay = MODEM_DELAY_CHK_TIME;
		modem_event_add_api(modem, MODEM_EV_DELAY, zpl_false);
	}
	if(modem->state == MDMS(NETWORK_ACTIVE) && modem->newstate != MDMS(NETWORK_ACTIVE))
	{
		modem->delay = MODEM_DELAY_CHK_TIME;
		modem->dedelay = MODEM_DECHK_TIME;
	}

	if(event != MODEM_EV_NONE)
	{
		modem_event_add_api(modem, event, zpl_false);
		//modem->state = modem->newstate;

		MODEM_DEBUG("event > %s",modem_event_string(event));
	}
	else
		modem->state = modem->newstate;

	return OK;
}

int modem_machine_state_set(modem_t *modem, modem_machine newstate)
{
	assert(modem);
	modem->state = newstate;
	return OK;
}

modem_machine modem_machine_state_get(modem_t *modem)
{
	assert(modem);
	return modem->state;
}



static modem_machine modem_machine_state_update(modem_t *modem)
{
	assert(modem);
	assert(modem->client);
	modem_client_t *client = modem->client;
	if(modem_usim_state(modem) == CPIN_NONE)	//USIM 状态
		return MDMS(NO_USIM_CARD);

	if( MODEM_SIGNAL_GET(client->signal_state, MODEM_STATE_SIGNAL_NONE)
			|| MODEM_SIGNAL_GET(client->signal_state, MODEM_STATE_SIGNAL_LOSS)
			|| MODEM_SIGNAL_GET(client->signal_state, MODEM_STATE_BITERR_HIGH) )
		return MDMS(NO_SIGNAL);

/*
	if( modem->dialtype == MODEM_DIAL_PPP )
		return MODEM_MACHINE_STATE_NETWORK_ACTIVE;
*/

	if(!client->prefix.family)
		return MDMS(NO_ADDR);

	if(!client->operator)
		return MDMS(NO_SERVICE);

	return MODEM_MACHINE_STATE_NETWORK_ACTIVE;
}

int modem_machine_state(modem_t *modem)
{
	modem_machine newstate = MODEM_MACHINE_STATE_NONE;
	assert(modem);
	assert(modem->client);
	//modem_client_t *client = modem->client;
	newstate = modem_machine_state_update(modem);

	modem_machine_state_set(modem,  newstate);
	return OK;
}


int modem_machine_state_show(modem_t *modem, struct vty *vty, zpl_bool detail)
{
	assert(modem);
	assert(modem->client);
	modem_client_t *client = modem->client;
	vty_out(vty, "state--------------------:%d%s",modem->state, VTY_NEWLINE);
	if(detail && client)
	{
		vty_out(vty, "activity-----------------:%d%s",client->activity, VTY_NEWLINE);
		vty_out(vty, "cpin_status--------------:%d%s",client->cpin_status, VTY_NEWLINE);
		vty_out(vty, "signal_state-------------:0x%x%s",client->signal_state, VTY_NEWLINE);
		vty_out(vty, "register_state-----------:0x%x%s",client->nw_register_state, VTY_NEWLINE);
		vty_out(vty, "operator-----------------:0x%x%s",client->operator, VTY_NEWLINE);
		vty_out(vty, "nw_state-----------------:0x%x%s",client->nw_state, VTY_NEWLINE);
		vty_out(vty, "address family-----------:0x%x%s",client->prefix.family, VTY_NEWLINE);
	}
	return OK;
}

