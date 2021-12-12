/*
 * modem_qmi.c
 *
 *  Created on: Jul 28, 2018
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
#include "modem_pppd.h"
#include "modem_qmi.h"

#include "modem_atcmd.h"


static int _qmi_start( char *process, char *apn, char *user, char *passwd, char *pin, int pdp)
{
	int id = 0, i = 0;
	char pdps[16];
	//quectel-CM -s 3gnet carl 1234 0 -p 1234 -f gobinet_log.txt
	char *argv[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	if(apn)
	{
		argv[i++] = "-s";
		argv[i++] = apn;
	}

	if(user && passwd)
	{
		argv[i++] = user;
		argv[i++] = passwd;
	}

	if(pdp >= 0)
	{
		memset(pdps, 0, sizeof(pdps));
		sprintf(pdps, "%d", pdp);
		argv[i++] = pdps;
	}
	if(pin >= 0)
	{
		argv[i++] = "-p";
		argv[i++] = pin;
	}
	//if(pin >= 0)
	{
		argv[i++] = "-f";
		argv[i++] = "gobinet.log";
	}
#ifndef ZPL_TOOLS_PROCESS
	id = child_process_create();
	if(id == 0)
	{
		super_system_execvp(process, argv);
	}
	else
	{
		MODEM_QMI_DEBUG("qmi %d", id);
		return id;
	}
#else
	char path[128];
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "quectel-CM");
	id = os_process_register(PROCESS_START, path, "quectel-CM", zpl_true, argv);
	if(id)
		return id;
#endif
	return 0;
}

static int _qmi_stop(int process)
{
#ifndef ZPL_TOOLS_PROCESS
	MODEM_QMI_DEBUG(" qmi");
	return child_process_destroy(process);
#else
	if(os_process_action(PROCESS_STOP, "quectel-CM", process))
	{
		return OK;
	}
	return OK;
#endif
}


static int _modem_qmi_start(modem_client_t *client)
{
	modem_t *modem = NULL;
	//struct interface *ifp = NULL;
	assert(client);
	assert(client->modem);
	modem = client->modem;
	modem->pid[modem->dialtype] = _qmi_start("quectel-CM", modem->apn,
			NULL, NULL, modem->pin, modem->profile);
	if(modem->pid[modem->dialtype])
	{
		//modem->state = MODEM_STATE_NETWORK_ACTIVE;
		return OK;
	}
	return ERROR;
}

static int _modem_qmi_stop(modem_client_t *client)
{
	modem_t *modem = NULL;
	assert(client);
	assert(client->modem);
	modem = client->modem;
	if(modem->pid[modem->dialtype])
	{
		if(_qmi_stop(modem->pid[modem->dialtype]) == OK)
			modem->pid[modem->dialtype] = 0;
	}
	return OK;
}

zpl_bool modem_qmi_isconnect(modem_t *modem)
{
	assert(modem);
	if(modem->pid[modem->dialtype])
	{
		return zpl_true;
	}
	return zpl_false;
}

zpl_bool modem_qmi_islinkup(modem_t *modem)
{
	assert(modem);
	struct interface *ifp = modem->eth0;
	if(modem && ifp)
	{
		if(modem->pid[modem->dialtype])
		{
			if(pal_interface_ifindex(ifp->k_name))
			{
				if(if_is_running(ifp))
				{
					modem_serial_interface_update_kernel(modem, ifp->k_name);
				}
				return zpl_true;
			}
		}
	}
	return zpl_false;
}


int modem_qmi_start(modem_t *modem)
{
	int ret = 0;
	assert(modem);
	assert(modem->client);
	ret = _modem_qmi_start(modem->client);
	MODEM_QMI_DEBUG(" ERROR: qmi start dial");
	return ERROR;
}


int modem_qmi_stop(modem_t *modem)
{
	int ret = 0;
	assert(modem);
	assert(modem->client);
	ret = _modem_qmi_stop(modem->client);
	MODEM_QMI_DEBUG(" ERROR: qmi stop dial");
	return ret;
}

int modem_qmi_exit(modem_t *modem)
{
	assert(modem);
	assert(modem->client);
	MODEM_QMI_DEBUG(" ERROR: qmi exit");
	return _modem_qmi_stop(modem->client);
}
