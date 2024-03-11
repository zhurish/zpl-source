/*
 * modem_pppd.c
 *
 *  Created on: Jul 27, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "zmemory.h"
#include "vty.h"
#include "command.h"
#include "if.h"

#include "checksum.h"
#include "nsm_include.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_machine.h"
#include "modem_event.h"
#include "modem_pppd.h"
#include "modem_split.h"
#include "modem_state.h"



static modem_main_pppd_t gModepppd;

/***********************************************************************/
/***********************************************************************/
static int modem_pppd_create_action(modem_pppd_t *pppd);
static int modem_pppd_delete_action(modem_pppd_t *pppd);
static int modem_pppd_default(modem_t *modem, modem_pppd_t *pppd);
static int modem_pppd_task_disconnect(modem_pppd_t *pppd);
static int modem_pppd_task_connect(modem_pppd_t *pppd);
/***********************************************************************/
static int modem_pppd_cleanall(void);
static int modem_pppd_restart(modem_pppd_t *pppd);
/***********************************************************************/

int modem_pppd_init(void)
{
	os_memset(&gModepppd, 0, sizeof(modem_pppd_t));
	gModepppd.list = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	if(gModepppd.list)
	{
		gModepppd.mutex = os_mutex_name_create("gModepppd.mutex");
		lstInit(gModepppd.list);
		return OK;
	}
	return ERROR;
}

int modem_pppd_exit(void)
{
	if(gModepppd.list)
	{
		if(lstCount(gModepppd.list))
		{
			modem_pppd_cleanall();
			lstFree(gModepppd.list);
		}
		XFREE(MTYPE_MODEM, gModepppd.list);
	}
	if(gModepppd.mutex)
		os_mutex_destroy(gModepppd.mutex);
	return OK;
}

static modem_pppd_t * modem_pppd_lookup_node(modem_t *modem)
{
	NODE index;
	modem_pppd_t *pstNode = NULL;
	assert(modem);
	for(pstNode = (modem_pppd_t *)lstFirst(gModepppd.list);
			pstNode != NULL;  pstNode = (modem_pppd_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		modem = pstNode->modem;
		if(modem && pstNode->modem && modem == pstNode->modem)
		{
			return pstNode;
		}
	}
	return NULL;
}

static int modem_pppd_add_node(modem_t *modem)
{
	assert(modem);
	if(modem_pppd_lookup_node(modem) == NULL )
	{
		modem_pppd_t *node = XMALLOC(MTYPE_MODEM, sizeof(modem_pppd_t));
		if(node)
		{
			os_memset(node, 0, sizeof(modem_pppd_t));
			modem_pppd_default(modem, node);
			node->modem = modem;
			modem->pppd = node;
			lstAdd(gModepppd.list, (NODE*)node);
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}

static int modem_pppd_del_node(modem_t *modem)
{
	assert(modem);
	modem_pppd_t *node = modem_pppd_lookup_node(modem);
	if(node)
	{
		lstDelete(gModepppd.list, (NODE*)node);
		if(node->modem)
		{
			((modem_t *)node->modem)->pppd = NULL;
		}
		node->modem = NULL;
		XFREE(MTYPE_MODEM, node);
		return OK;
	}
	return ERROR;
}

static int modem_pppd_cleanall(void)
{
	NODE index;
	modem_pppd_t *pstNode = NULL;
	for(pstNode = (modem_pppd_t *)lstFirst(gModepppd.list);
			pstNode != NULL;  pstNode = (modem_pppd_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(gModepppd.list, (NODE*)pstNode);
			if(pstNode->t_time)
			{
				if(os_time_lookup(pstNode->t_time))
				{
					os_time_destroy(pstNode->t_time);
				}
			}
			if(pstNode->taskid)
				modem_pppd_task_disconnect(pstNode);
			if(pstNode->modem)
			{
				((modem_t *)pstNode->modem)->pppd = NULL;
			}
			pstNode->modem = NULL;
			XFREE(MTYPE_MODEM, pstNode);
			pstNode = NULL;
		}
	}
	return OK;
}

int modem_pppd_add_api(modem_t *modem)
{
	int ret = 0;
	assert(modem);
	if(gModepppd.mutex)
		os_mutex_lock(gModepppd.mutex, OS_WAIT_FOREVER);
	ret = modem_pppd_add_node(modem);
	if(gModepppd.mutex)
		os_mutex_unlock(gModepppd.mutex);
	return ret;
}

int modem_pppd_set_api(modem_t *modem, pppd_options_t *pppd_options)
{
	int ret = 0;
	assert(modem);
	assert(pppd_options);
	modem_pppd_t *node = NULL;
	if(gModepppd.mutex)
		os_mutex_lock(gModepppd.mutex, OS_WAIT_FOREVER);
	node = modem_pppd_lookup_node(modem);
	if(node)
	{
		os_memcpy(&node->pppd_options, pppd_options, sizeof(pppd_options_t));
	}
	if(gModepppd.mutex)
		os_mutex_unlock(gModepppd.mutex);
	return ret;
}

int modem_pppd_del_api(modem_pppd_t *pppd)
{
	int ret = ERROR;
	assert(pppd);
	if(gModepppd.mutex)
		os_mutex_lock(gModepppd.mutex, OS_WAIT_FOREVER);

	if(pppd->modem)
	{
		if(modem_pppd_isconnect(pppd->modem))
		{
			if(modem_pppd_task_disconnect(pppd) == OK)
			{
				modem_pppd_delete_action(pppd);
				ret = modem_pppd_del_node(pppd->modem);
			}
		}
	}
	if(gModepppd.mutex)
		os_mutex_unlock(gModepppd.mutex);
	return ret;
}

int modem_pppd_update_api(modem_pppd_t *pppd)
{
	zpl_uint32 checksum = 0;
	int ret = ERROR;
	assert(pppd);
	if(gModepppd.mutex)
		os_mutex_lock(gModepppd.mutex, OS_WAIT_FOREVER);
	checksum = crc_checksum(pppd, sizeof(modem_pppd_t));
	if(pppd->checksum != checksum)
	{
		ret = modem_pppd_create_action(pppd);
		modem_pppd_restart(pppd);
	}
	else
		ret = OK;
	if(gModepppd.mutex)
		os_mutex_unlock(gModepppd.mutex);
	return ret;
}

int modem_pppd_lookup_api(void *modem)
{
	int ret = ERROR;
	assert(modem);
	modem_pppd_t *node = NULL;
	if(gModepppd.mutex)
		os_mutex_lock(gModepppd.mutex, OS_WAIT_FOREVER);
	node = modem_pppd_lookup_node(modem);
	if(gModepppd.mutex)
		os_mutex_unlock(gModepppd.mutex);
	return ret;
}


int modem_pppd_callback_api(modem_pppd_cb cb, void *pVoid)
{
	NODE index;
	modem_pppd_t *pstNode = NULL;
	if(gModepppd.mutex)
		os_mutex_lock(gModepppd.mutex, OS_WAIT_FOREVER);
	for(pstNode = (modem_pppd_t *)lstFirst(gModepppd.list);
			pstNode != NULL;  pstNode = (modem_pppd_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(cb)
			(cb)(pstNode, pVoid);
	}
	if(gModepppd.mutex)
		os_mutex_unlock(gModepppd.mutex);
	return OK;
}

/***********************************************************************/
static int modem_pppd_restart_thread(modem_pppd_t *pppd)
{
	if(pppd && pppd->modem)
	{
		if(modem_pppd_isconnect(pppd->modem))
		{
			modem_pppd_task_disconnect(pppd);
			if(pppd->taskid == 0)
			{
				pppd->taskid = 0;
				pppd->linkup = zpl_false;
			}
			modem_pppd_task_connect(pppd);
		}
		pppd->t_time = 0;
		return OK;
	}
	return OK;
}


static int modem_pppd_restart(modem_pppd_t *pppd)
{
	if(pppd->t_time)
	{
		if(os_time_lookup(pppd->t_time))
		{
			os_time_cancel(pppd->t_time);
			os_time_restart(pppd->t_time, 10000);
			return OK;
		}
	}
	pppd->t_time = os_time_create_once(modem_pppd_restart_thread, pppd, 10000);
	return OK;
}

/***********************************************************************/
static const char * modem_pppd_atcmd_connect(modem_t *modem)
{
	char *apn = NULL, *svc = NULL;
	modem_client_t *client = NULL;
	static char connect[1024];
	assert(modem);
	assert(modem->client);
	client = modem->client;
	assert(client->driver);
	os_memset(connect, 0, sizeof(connect));

	apn = modem_pdp_cmd(modem);
	svc = modem_svc_cmd(modem);
	if(client && client->driver->atcmd.md_pppd_connect)
		(client->driver->atcmd.md_pppd_connect)(client->driver, connect, sizeof(connect));
	else
		os_snprintf(connect, sizeof(connect),
			"TIMEOUT 5\n\n"
			"ABORT 'NO CARRIER'\n\n"
			"ABORT 'ERROR'\n\n"
			"ABORT 'NODIALTONE'\n\n"
			"ABORT 'BUSY'\n\n"
			"ABORT 'NO ANSWER'\n\n"
			"'' \\rAT\n\n"
			"OK \\rATZ\n\n"
			"OK \\rAT+CSQ\n\n"
			"OK \\r%s\n\n"
			"OK-AT-OK %s\n\n"
			"CONNECT \\d\\c\n\n"
			"\n\n",
			apn ? apn:"AT+CGDCONT=1,,,,,",
			svc ? svc:"ATDT*98*1#");

	return connect;
}

static const char * modem_pppd_atcmd_disconnect(modem_t *modem)
{
	modem_client_t *client = NULL;
	static char disconnect[1024];
	assert(modem);
	assert(modem->client);
	client = modem->client;
	assert(client->driver);
	os_memset(disconnect, 0, sizeof(disconnect));
	if(client && client->driver->atcmd.md_pppd_connect)
		(client->driver->atcmd.md_pppd_connect)(client->driver, disconnect, sizeof(disconnect));
	else
		os_snprintf(disconnect, sizeof(disconnect),
			"ABORT \"ERROR\"\n\n"
			"ABORT \"NODIALTONE\"\n\n"
			"SAY \"\\nSending break to the modem\\n\"\n\n"
			"'' \"\\K\"\n\n"
			"'' \"+++ATH\"\n\n"
			"SAY \"\\nGoodbay\\n\"\n\n"
			"\n\n");
	return disconnect;
}

/***********************************************************************/
/***********************************************************************/
static int modem_pppd_create_connect(modem_pppd_t *pppd)
{
	FILE *fp = NULL;
	char filepath[256];
	assert(pppd);
	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_PEERS_BASE, pppd->dial_name, MODEM_PPPD_CONNECT);
	remove(filepath);
	fp = fopen(filepath, "w+");
	if(fp)
	{
		modem_t *modem = pppd->modem;
		if(modem)
		{
			char *at_cmd = modem_pppd_atcmd_connect(modem);
			os_fprintf(fp, "%s", at_cmd);
			fflush(fp);
			fclose(fp);
			MODEM_PPPD_DEBUG("PPPD create connect script");
			return OK;
		}
		fflush(fp);
		fclose(fp);
	}
	return ERROR;
}

static int modem_pppd_create_disconnect(modem_pppd_t *pppd)
{
	FILE *fp = NULL;
	char filepath[256];
	assert(pppd);
	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_PEERS_BASE, pppd->dial_name, MODEM_PPPD_DISCONNECT);
	remove(filepath);
	fp = fopen(filepath, "w+");
	if(fp)
	{
		modem_t *modem = pppd->modem;
		if(modem)
		{
			char *at_cmd = modem_pppd_atcmd_disconnect(modem);
			os_fprintf(fp, "%s", at_cmd);
			fflush(fp);
			fclose(fp);
			MODEM_PPPD_DEBUG("PPPD create disconnect script");
			return OK;
		}
		fflush(fp);
		fclose(fp);
	}
	return ERROR;
}

static int modem_pppd_default(modem_t *modem, modem_pppd_t *pppd)
{
	assert(pppd);
	/*
	 * PPP options
	 */
	pppd->pppd_options.hide_password = zpl_true;
	pppd->pppd_options.show_password = zpl_false;
	pppd->pppd_options.detach = zpl_false;

	//pppd->pppd_options.idle;// <n>
	//pppd->pppd_options.holdoff;// <n>
	//pppd->pppd_options.connect_delay;// <n>
	//pppd->pppd_options.active_filter;// <filter_expression>
	pppd->pppd_options.persist = zpl_true;
#if 0
	pppd->pppd_options.maxfail;// <n>
	pppd->pppd_options.noipx;
	pppd->pppd_options.ip;
	pppd->pppd_options.noip;
	pppd->pppd_options.mn;
	pppd->pppd_options.pc;
#endif
	pppd->pppd_options.vj = zpl_false;

#if 0
	pppd->pppd_options.passive;
	pppd->pppd_options.silent;
	pppd->pppd_options.all;
	pppd->pppd_options.ac;
	pppd->pppd_options.am;
	pppd->pppd_options.proxyarp;
	pppd->pppd_options.login;
	pppd->pppd_options.asyncmap;
#endif
	pppd->pppd_options.auth = zpl_false;
	//pppd->pppd_options.crtscts[PPPD_OPTIONS_MAX];
	//pppd->pppd_options.xonxoff;
	//pppd->pppd_options.escape[PPPD_OPTIONS_MAX];
	pppd->pppd_options.local = zpl_true;
	//pppd->pppd_options.modem;
	pppd->pppd_options.lock = zpl_true;
	pppd->pppd_options.debug = zpl_true;
	//pppd->pppd_options.logfile[MODEM_STRING_MAX];

	/*
	 * IP options
	 */
#if 0
	pppd->pppd_options.mru;
	pppd->pppd_options.mtu;
	pppd->pppd_options.netmask;
	pppd->pppd_options.ms_dns[2];
	pppd->pppd_options.demand;
	pppd->pppd_options.domain[PPPD_OPTIONS_MAX];// <d>
	pppd->pppd_options.name[PPPD_OPTIONS_MAX];// <n>
	pppd->pppd_options.usehostname[PPPD_OPTIONS_MAX];
	pppd->pppd_options.remotename[PPPD_OPTIONS_MAX];// <n>
#endif
	pppd->pppd_options.noipdefault = zpl_true;
	pppd->pppd_options.defaultroute = zpl_true;
	pppd->pppd_options.usepeerdns = zpl_true;

	/*
	 * LCP options
	 */
#if 0
	pppd->pppd_options.lcp_echo_interval; //n
	pppd->pppd_options.lcp_echo_failure; //n
	pppd->pppd_options.lcp_restart; //<n>
	pppd->pppd_options.lcp_max_terminate; //<n>
	pppd->pppd_options.lcp_max_configure; //<n>
	pppd->pppd_options.lcp_max_failure; //<n>
#endif
	/*
	 * IPCP options
	 */
	//pppd->pppd_options.ipcp_restart; //<n>
	//pppd->pppd_options.ipcp_max_terminate;// <n>
	//pppd->pppd_options.ipcp_max_configure;// <n>
	pppd->pppd_options.ipcp_accept_local = zpl_true;
	pppd->pppd_options.ipcp_accept_remote = zpl_true;
#if 0
	/*
	 * PAP options
	 */
	pppd->pppd_options.pap;
	pppd->pppd_options.pap_restart;// <n>
	pppd->pppd_options.pap_max_authreq;// <n>
	pppd->pppd_options.pap_timeout;
	/*
	 * CHAP options
	 */
	pppd->pppd_options.chap;
	pppd->pppd_options.chap_restart;
	pppd->pppd_options.chap_max_challenge;
	pppd->pppd_options.chap_interval;// <n>
#endif
	if(modem)
	{
		//struct interface *ifp = modem->ppp_serial;
		os_memset(pppd->dial_name, 0, sizeof(pppd->dial_name));
		os_snprintf(pppd->dial_name, sizeof(pppd->dial_name), "%s-%s",
				modem->name, modem_network_type_string(modem->network));

		MODEM_PPPD_DEBUG("PPPD create dial script '%s'", pppd->dial_name);

		os_memset(pppd->connect, 0, sizeof(pppd->connect));
		os_snprintf(pppd->connect, sizeof(pppd->connect), "connect-%s", modem_network_type_string(modem->network));

		os_memset(pppd->disconnect, 0, sizeof(pppd->disconnect));
		os_snprintf(pppd->disconnect, sizeof(pppd->disconnect), "disconnect-%s", modem_network_type_string(modem->network));

		os_memset(pppd->secrets, 0, sizeof(pppd->secrets));
		os_snprintf(pppd->secrets, sizeof(pppd->secrets), "secrets-%s", modem_network_type_string(modem->network));

		os_memset(pppd->option, 0, sizeof(pppd->option));
		os_snprintf(pppd->option, sizeof(pppd->option), "option-%s", modem_network_type_string(modem->network));
		return OK;
	}
    return OK;
}
static int modem_pppd_create_options_prefile(void *fp, pppd_options_t *pppd)
{
	assert(fp);
	assert(pppd);

	os_fprintf(fp, "\n");
	os_fprintf(fp, "# modem PPP option\n");
	if(pppd->hide_password)
		os_fprintf(fp, "hide-password\n");
	if(pppd->show_password)
		os_fprintf(fp, "show-password\n");

	if(!pppd->detach)
		os_fprintf(fp, "nodetach\n");
/*
	else
		os_fprintf(fp, "detach\n");
*/

	if(pppd->idle)
		os_fprintf(fp, "idle %d\n", pppd->idle);

	if(pppd->holdoff)
		os_fprintf(fp, "holdoff %d\n", pppd->holdoff);

	if(pppd->connect_delay)
		os_fprintf(fp, "connect-delay %d\n", pppd->connect_delay);

	if(pppd->active_filter)
		os_fprintf(fp, "active-filter %d\n", pppd->active_filter);

	if(pppd->persist)
		os_fprintf(fp, "persist\n");
	else
		os_fprintf(fp, "nopersist\n");

	if(pppd->maxfail)
		os_fprintf(fp, "maxfail %d\n", pppd->maxfail);

	if(pppd->noipx)
		os_fprintf(fp, "noipx\n");
	//else
	//	os_fprintf(fp, "ipx\n");

	if(pppd->ip)
		os_fprintf(fp, "ip\n");
	//else
	//	os_fprintf(fp, "-ip\n");

	if(pppd->noip)
		os_fprintf(fp, "noip\n");
	//else
	//	os_fprintf(fp, "-noip\n");

	if(pppd->mn)
		os_fprintf(fp, "mn\n");
	//else
	//	os_fprintf(fp, "-mn\n");

/*
	if(pppd->pc)
		os_fprintf(fp, "pc\n");
	else
		os_fprintf(fp, "-pc\n");
*/

	if(pppd->vj)
		os_fprintf(fp, "vj\n");
	else
		os_fprintf(fp, "novj\n");

	if(pppd->passive)
		os_fprintf(fp, "passive\n");

	if(pppd->passive)
		os_fprintf(fp, "passive\n");

	if(pppd->silent)
		os_fprintf(fp, "silent\n");

	if(pppd->all)
		os_fprintf(fp, "-all\n");

	if(pppd->ac)
		os_fprintf(fp, "-ac\n");

	if(pppd->am)
		os_fprintf(fp, "-am\n");

	if(pppd->proxyarp)
		os_fprintf(fp, "proxyarp\n");

	if(pppd->login)
		os_fprintf(fp, "login\n");

	if(pppd->asyncmap)
		os_fprintf(fp, "asyncmap\n");

	if(pppd->auth)
		os_fprintf(fp, "auth\n");
	else
		os_fprintf(fp, "noauth\n");

/*
	if(pppd->xonxoff)
		os_fprintf(fp, "xonxoff\n");
	else
		os_fprintf(fp, "-xonxoff\n");
*/

	if(pppd->local)
		os_fprintf(fp, "local\n");
	//else
	//	os_fprintf(fp, "-local\n");

	if(pppd->modem)
		os_fprintf(fp, "modem\n");
	//else
	//	os_fprintf(fp, "-modem\n");

	if(pppd->lock)
		os_fprintf(fp, "lock\n");

	if(pppd->debug)
	{
		os_fprintf(fp, "debug\n");
		if(strlen(pppd->logfile))
			os_fprintf(fp, "logfile /var/log/%s\n",pppd->logfile);
	}
    //char crtscts[PPPD_OPTIONS_MAX];
        //使用硬件流控制来控制串口的数据流向，如RTS/CTS
    //char escape[PPPD_OPTIONS_MAX];
        //指定传输时，需要避免的某些字符
	os_fprintf(fp, "\n");
	os_fprintf(fp, "# modem IP option\n");
	if(pppd->mru)
		os_fprintf(fp, "mru %d\n", pppd->mru);
	if(pppd->mtu)
		os_fprintf(fp, "mtu %d\n", pppd->mtu);
	if(pppd->netmask)
		os_fprintf(fp, "netmask %d\n", pppd->netmask);

	if(pppd->ms_dns[0])
		os_fprintf(fp, "ms-dns %d\n", pppd->ms_dns[0]);
	if(pppd->ms_dns[1])
		os_fprintf(fp, "ms-dns %d\n", pppd->ms_dns[1]);
	if(pppd->demand)
		os_fprintf(fp, "demand %d\n", pppd->demand);

	if(strlen(pppd->domain))
		os_fprintf(fp, "domain %s\n",pppd->domain);

	if(strlen(pppd->name))
		os_fprintf(fp, "name %s\n",pppd->name);

	if(strlen(pppd->usehostname))
		os_fprintf(fp, "usehostname %s\n",pppd->usehostname);

	if(strlen(pppd->remotename))
		os_fprintf(fp, "remotename %s\n",pppd->remotename);

	if(strlen(pppd->domain))
		os_fprintf(fp, "domain %s\n",pppd->domain);

	if(pppd->defaultroute)
		os_fprintf(fp, "defaultroute\n");

	if(pppd->usepeerdns)
		os_fprintf(fp, "usepeerdns\n");

	if(pppd->noipdefault)
		os_fprintf(fp, "noipdefault\n");


	/*
	 * LCP options
	 */
	os_fprintf(fp, "\n");
	os_fprintf(fp, "# modem LCP option\n");
	if(pppd->lcp_echo_interval)
		os_fprintf(fp, "lcp-echo-interval %d\n", pppd->lcp_echo_interval);

	if(pppd->lcp_echo_failure)
		os_fprintf(fp, "lcp-echo-failure %d\n", pppd->lcp_echo_failure);

	if(pppd->lcp_restart)
		os_fprintf(fp, "lcp-restart %d\n", pppd->lcp_restart);

	if(pppd->lcp_max_configure)
		os_fprintf(fp, "lcp-max-configure %d\n", pppd->lcp_max_configure);

	if(pppd->lcp_max_terminate)
		os_fprintf(fp, "lcp-max-terminate %d\n", pppd->lcp_max_terminate);

	if(pppd->lcp_max_failure)
		os_fprintf(fp, "lcp-max-failure %d\n", pppd->lcp_max_failure);



	/*
	 * IPCP options
	 */
	os_fprintf(fp, "\n");
	os_fprintf(fp, "# modem IPCP option\n");
	if(pppd->ipcp_restart)
		os_fprintf(fp, "ipcp-restart %d\n", pppd->ipcp_restart);

	if(pppd->ipcp_max_terminate)
		os_fprintf(fp, "ipcp-max-terminate %d\n", pppd->ipcp_max_terminate);

	if(pppd->ipcp_max_configure)
		os_fprintf(fp, "ipcp-max-configure %d\n", pppd->ipcp_max_configure);

	if(pppd->ipcp_accept_local)
		os_fprintf(fp, "ipcp-accept-local\n");
	if(pppd->ipcp_accept_remote)
		os_fprintf(fp, "ipcp-accept-remote\n");
	/*
	 * PAP options
	 */
	os_fprintf(fp, "\n");
    os_fprintf(fp, "# modem PAP option\n");
	if(pppd->pap)
		os_fprintf(fp, "+pap\n");
	else
		os_fprintf(fp, "-pap\n");

	if(pppd->pap_restart)
		os_fprintf(fp, "pap-restart %d\n", pppd->pap_restart);
	if(pppd->pap_max_authreq)
		os_fprintf(fp, "pap-max-authreq %d\n", pppd->pap_max_authreq);
	if(pppd->pap_timeout)
		os_fprintf(fp, "pap-timeout %d\n", pppd->pap_timeout);


	/*
	 * CHAP options
	 */
	os_fprintf(fp, "\n");
    os_fprintf(fp, "# modem CHAP option\n");
	if(pppd->chap)
		os_fprintf(fp, "+chap\n");
	else
		os_fprintf(fp, "-chap\n");
	if(pppd->chap_restart)
		os_fprintf(fp, "chap-restart %d\n", pppd->chap_restart);
	if(pppd->chap_max_challenge)
		os_fprintf(fp, "chap-max-challenge %d\n", pppd->chap_max_challenge);
	if(pppd->chap_interval)
		os_fprintf(fp, "chap-interval %d\n", pppd->chap_interval);

	os_fprintf(fp, "\n");
	return OK;
}

static int modem_pppd_create_options(modem_pppd_t *pppd)
{
	FILE *fp = NULL;
	char filepath[256];
	assert(pppd);
	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_BASE, pppd->dial_name, MODEM_PPPD_OPTIONS);
	remove(filepath);
	fp = fopen(filepath, "w+");
	if(fp)
	{
		fflush(fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

static int modem_pppd_create_secrets(modem_pppd_t *pppd)
{
	FILE *fp = NULL;
	char filepath[256];
	assert(pppd);
	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_BASE, pppd->dial_name, MODEM_PPPD_SECRETS);
	remove(filepath);
	fp = fopen(filepath, "w+");
	if(fp)
	{
		fflush(fp);
		fclose(fp);
	}
	return ERROR;
}

static int modem_pppd_create_dial_name(modem_pppd_t *pppd)
{
	FILE *fp = NULL;
	char filepath[256];
	assert(pppd);
	assert(	pppd->modem);
	assert(	((modem_t *)pppd->modem)->client);
	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s",MODEM_PPPD_PEERS_BASE, pppd->dial_name);
	remove(filepath);
	fp = fopen(filepath, "w+");
	if(fp)
	{
		//char filepath[256];
		modem_t *modem = pppd->modem;
		modem_client_t *client = NULL;
		struct interface *ifp = modem->ppp_serial;
		char *devname = NULL;
		if(!modem)
		{
			fclose(fp);
			return ERROR;
		}
#if 1
		client = modem->client;
		if(!client)
		{
			fclose(fp);
			return ERROR;
		}
		os_fprintf(fp, "# modem options \n");
		os_fprintf(fp, "# modem dev name\n");
		devname = strrchr(client->pppd->devname, '/');
		if(devname)
			devname++;
		else
			devname = client->pppd->devname;
		os_fprintf(fp, "%s\n", devname);
		os_fprintf(fp, "# modem dev baudrate\n");
		os_fprintf(fp, "%d\n", client->pppd->speed);

		os_fprintf(fp, "# if name \n");
		if(ifp && os_strlen(ifp->ker_name))
		{
			os_fprintf(fp, "\nifname %s\n\n", ifp->ker_name);
			MODEM_PPPD_DEBUG("PPPD interface name '%s'", ifp->ker_name);
		}
		else
			MODEM_PPPD_DEBUG("PPPD interface name NULL");
#endif
		modem_pppd_create_options_prefile(fp, &pppd->pppd_options);

		os_memset(filepath, 0, sizeof(filepath));
		os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_PEERS_BASE,
				pppd->dial_name, MODEM_PPPD_CONNECT);
		os_fprintf(fp, "connect '/usr/sbin/chat -s -v -f %s'\n", filepath);

		os_memset(filepath, 0, sizeof(filepath));
		os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_PEERS_BASE,
				pppd->dial_name, MODEM_PPPD_DISCONNECT);
		os_fprintf(fp, "disconnect '/usr/sbin/chat -s -v -f %s'\n", filepath);

/*
		os_memset(filepath, 0, sizeof(filepath));
		os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_BASE,
				pppd->dial_name, MODEM_PPPD_SECRETS);
		os_fprintf(fp, "secrets '%s'\n", filepath);

		os_memset(filepath, 0, sizeof(filepath));
		os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_BASE,
				pppd->dial_name, MODEM_PPPD_OPTIONS);
		os_fprintf(fp, "options '%s'\n", filepath);
*/

		os_fprintf(fp, "\n");


		fflush(fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

static int modem_pppd_create_action(modem_pppd_t *pppd)
{
	assert(pppd);
	modem_pppd_create_connect(pppd);
	modem_pppd_create_disconnect(pppd);
	modem_pppd_create_dial_name(pppd);
	modem_pppd_create_secrets(pppd);
	modem_pppd_create_options(pppd);
	return OK;
}

static int modem_pppd_delete_action(modem_pppd_t *pppd)
{
	char filepath[256];
	assert(pppd);
	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s",MODEM_PPPD_PEERS_BASE, pppd->dial_name);
	remove(filepath);

	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_BASE, pppd->dial_name, MODEM_PPPD_SECRETS);
	remove(filepath);

	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_BASE, pppd->dial_name, MODEM_PPPD_OPTIONS);
	remove(filepath);

	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_PEERS_BASE, pppd->dial_name, MODEM_PPPD_DISCONNECT);
	remove(filepath);

	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_PEERS_BASE, pppd->dial_name, MODEM_PPPD_CONNECT);
	remove(filepath);

	return OK;
}
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
static int modem_pppd_task_connect(modem_pppd_t *pppd)
{
	assert(pppd);
#ifndef ZPL_TOOLS_PROCESS
	zpl_pid_t pid = child_process_create();
	if(pid < 0)
	{
		  zlog_warn(MODULE_MODEM, "Can't create child process (%s), continuing", ipstack_strerror(ipstack_errno));
		  return -1;
	}
	else if(pid == 0)
	{
		//char filepath[256];
		//os_memset(filepath, 0, sizeof(filepath));
		//os_snprintf(filepath, sizeof(filepath), "pppd call %s", pppd->dial_name);
		char *exeargv[] = {"pppd", "call", pppd->dial_name, NULL};
		exeargv[2] = pppd->dial_name;
		super_system_execvp("pppd", exeargv);
	}
	else
		pppd->taskid = pid;
#else
	char path[128];
	//char *argve[] = {"call", "file", "/etc/ppp/peers/dial-auto", NULL};
	char *argv[4] = {"call", "file", NULL, NULL};
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s%s", MODEM_PPPD_PEERS_BASE, pppd->dial_name);

	if(pppd->taskid)
	{
		if(os_process_action(PROCESS_LOOKUP, NULL, pppd->taskid))
		{
			return OK;
		}
	}
	argv[2] = path;
	MODEM_PPPD_DEBUG("PPPD connect on %s", ((modem_t *)pppd->modem)->name);
	pppd->taskid = os_process_register(PROCESS_START, ((modem_t *)pppd->modem)->name, "pppd", zpl_true, argv);
	if(!pppd->taskid)
		return ERROR;
#endif
	return OK;
}
/*
call: unrecognized option 'dial-auto'
pppd version 2.4.7
Usage: call [ options ], where options are:
        <device>        Communicate over the named device
        <speed>         Set the baud rate to <speed>
        <loc>:<rem>     Set the local and/or remote interface IP
                        addresses.  Either one may be omitted.
        asyncmap <n>    Set the desired async map to hex <n>
        auth            Require authentication from peer
        connect <p>     Invoke shell command <p> to set up the serial line
        crtscts         Use hardware RTS/CTS flow control
        defaultroute    Add default route through interface
        file <f>        Take options from file <f>
        modem           Use modem control lines
        mru <n>         Set MRU value to <n> for negotiation
See pppd(8) for more options.

 */
static int modem_pppd_task_disconnect(modem_pppd_t *pppd)
{
#ifndef ZPL_TOOLS_PROCESS
	assert(pppd);
	return child_process_destroy(pppd->taskid);
#else
/*	char path[128];
	char *argv[] = {"call", pppd->dial_name, NULL};
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "pppd");*/
	if(pppd->taskid)
	{
		if(os_process_action(PROCESS_LOOKUP, NULL, pppd->taskid))
		{
			if(os_process_action(PROCESS_STOP, ((modem_t *)pppd->modem)->name, pppd->taskid))
			{
				pppd->taskid = 0;
				return OK;
			}
		}
		pppd->taskid = 0;
		return OK;
	}
	return OK;
#endif
}
/***********************************************************************/
zpl_bool modem_pppd_isconnect(modem_t *modem)
{
	assert(modem);
	struct interface *ifp = modem->ppp_serial;
	modem_pppd_t *pppd = modem->pppd;
	if(pppd && ifp)
	{
		if(pppd->taskid)
		{
			//MODEM_DEBUG("modem_pppd_isconnect zpl_true");
			return zpl_true;
		}
	}
	//MODEM_DEBUG("modem_pppd_isconnect zpl_false");
	return zpl_false;
}

zpl_bool modem_pppd_islinkup(modem_t *modem)
{
	assert(modem);
	struct interface *ifp = modem->ppp_serial;
	modem_pppd_t *pppd = modem->pppd;
	if(pppd && ifp)
	{
		if(pppd->taskid)
		{
			if(nsm_halpal_interface_ifindex(ifp->ker_name))
			{
				//MODEM_DEBUG("modem_pppd_islinkup zpl_true");
				if(pppd->linkup == zpl_false)
				{
					//MODEM_DEBUG("modem_pppd_islinkup UPDATE KERNEL");
					modem_serial_interface_update_kernel(modem, ifp->ker_name);
					pppd->linkup = zpl_true;
				}
				return zpl_true;
			}
		}
		//MODEM_DEBUG("modem_pppd_islinkup zpl_false");
		pppd->linkup = zpl_false;
	}
	return zpl_false;
}

int modem_pppd_connect(modem_t *modem)
{
	assert(modem);
	modem_pppd_t *pppd = modem->pppd;
	if(pppd)
	{
		modem_pppd_create_action(pppd);
		if(pppd->taskid)
		{
			modem_pppd_task_disconnect(pppd);
			pppd->taskid = 0;
		}
		if(pppd->taskid == 0)
		{
			modem_pppd_task_connect(pppd);
			if(pppd->taskid)
				return OK;
		}
	}
	return ERROR;
}

int modem_pppd_disconnect(modem_t *modem)
{
	assert(modem);
	modem_pppd_t *pppd = modem->pppd;
	if(pppd)
	{
		if(pppd->taskid)
		{
			modem_pppd_task_disconnect(pppd);
			if(pppd->taskid == 0)
			{
				pppd->taskid = 0;
				pppd->linkup = zpl_false;
				return OK;
			}
		}
		else
			return OK;
	}
	return ERROR;
}



