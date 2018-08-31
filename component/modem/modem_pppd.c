/*
 * modem_pppd.c
 *
 *  Created on: Jul 27, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"

#include "os_list.h"
#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_machine.h"
#include "modem_event.h"
#include "modem_pppd.h"





static modem_main_pppd_t gModepppd;

/***********************************************************************/
/***********************************************************************/
static int modem_pppd_create_action(modem_pppd_t *pppd);
static int modem_pppd_delete_action(modem_pppd_t *pppd);
static int modem_pppd_default(modem_pppd_t *pppd);
static int modem_pppd_task_disconnect(modem_pppd_t *pppd);
/***********************************************************************/
/***********************************************************************/

int modem_pppd_init(void)
{
	os_memset(&gModepppd, 0, sizeof(modem_pppd_t));
	gModepppd.list = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	if(gModepppd.list)
	{
		gModepppd.mutex = os_mutex_init();
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
			lstFree(gModepppd.list);
		XFREE(MTYPE_MODEM, gModepppd.list);
	}
	if(gModepppd.mutex)
		os_mutex_exit(gModepppd.mutex);
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
			modem_pppd_default(node);
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
		node->modem = NULL;
		XFREE(MTYPE_MODEM, node);
		return OK;
	}
	return ERROR;
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
				ret = modem_pppd_del_node(pppd->modem);
				modem_pppd_delete_action(pppd);
			}
		}
	}

	if(gModepppd.mutex)
		os_mutex_unlock(gModepppd.mutex);
	return ret;
}

int modem_pppd_update_api(modem_pppd_t *pppd)
{
	int ret = ERROR;
	assert(pppd);
	if(gModepppd.mutex)
		os_mutex_lock(gModepppd.mutex, OS_WAIT_FOREVER);
	ret = modem_pppd_create_action(pppd);
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
			"TIMEOUT 5\r\n"
			"ABORT 'NO CARRIER'\r\n"
			"ABORT 'ERROR'\r\n"
			"ABORT 'NODIALTONE'\r\n"
			"ABORT 'BUSY'\r\n"
			"ABORT 'NO ANSWER'\r\n"
			"'' \\rAT\r\n"
			"OK \\rATZ\r\n"
			"OK \\rAT+CSQ\r\n"
			"OK \\r%s\r\n"
			"OK-AT-OK %s\r\n"
			"CONNECT \\d\\c\r\n"
			"\r\n",
			apn ? apn:"AT+CGDCONT=1,,,,,",
			svc ? svc:"ATDT%s*98*1");

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
			"ABORT \"ERROR\"\r\n"
			"ABORT \"NODIALTONE\"\r\n"
			"SAY \"\\nSending break to the modem\\n\"\r\n"
			"'' \"\\K\"\r\n"
			"'' \"+++ATH\"\r\n"
			"SAY \"\\nGoodbay\\n\"\r\n"
			"\r\n");
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
			return OK;
		}
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
			return OK;
		}
	}
	return ERROR;
}

static int modem_pppd_default(modem_pppd_t *pppd)
{
	assert(pppd);
	/*
	 * PPP options
	 */
	pppd->pppd_options.hide_password = TRUE;
	pppd->pppd_options.show_password = FALSE;
	pppd->pppd_options.detach = FALSE;

	//pppd->pppd_options.idle;// <n>
	//pppd->pppd_options.holdoff;// <n>
	//pppd->pppd_options.connect_delay;// <n>
	//pppd->pppd_options.active_filter;// <filter_expression>
	pppd->pppd_options.persist = TRUE;
#if 0
	pppd->pppd_options.maxfail;// <n>
	pppd->pppd_options.noipx;
	pppd->pppd_options.ip;
	pppd->pppd_options.noip;
	pppd->pppd_options.mn;
	pppd->pppd_options.pc;
#endif
	pppd->pppd_options.vj = FALSE;

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
	pppd->pppd_options.auth = FALSE;
	//pppd->pppd_options.crtscts[PPPD_OPTIONS_MAX];
	//pppd->pppd_options.xonxoff;
	//pppd->pppd_options.escape[PPPD_OPTIONS_MAX];
	pppd->pppd_options.local = TRUE;
	//pppd->pppd_options.modem;
	pppd->pppd_options.lock = TRUE;
	pppd->pppd_options.debug = TRUE;
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
	pppd->pppd_options.noipdefault = TRUE;
	pppd->pppd_options.defaultroute = TRUE;
	pppd->pppd_options.usepeerdns = TRUE;

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
	pppd->pppd_options.ipcp_accept_local = TRUE;
	pppd->pppd_options.ipcp_accept_remote = TRUE;
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
    return OK;
}
static int modem_pppd_create_options_prefile(void *fp, pppd_options_t *pppd)
{
	assert(fp);
	assert(pppd);
	os_fprintf(fp, "\r\n");
	os_fprintf(fp, "# modem PPP option\r\n");
	if(pppd->hide_password)
		os_fprintf(fp, "hide-password\r\n");
	if(pppd->show_password)
		os_fprintf(fp, "show-password\r\n");

	if(pppd->detach)
		os_fprintf(fp, "nodetach\r\n");
	else
		os_fprintf(fp, "detach\r\n");

	if(pppd->idle)
		os_fprintf(fp, "idle %d\r\n", pppd->idle);

	if(pppd->holdoff)
		os_fprintf(fp, "holdoff %d\r\n", pppd->holdoff);

	if(pppd->connect_delay)
		os_fprintf(fp, "connect-delay %d\r\n", pppd->connect_delay);

	if(pppd->active_filter)
		os_fprintf(fp, "active-filter %d\r\n", pppd->active_filter);

	if(pppd->persist)
		os_fprintf(fp, "persist\r\n");
	else
		os_fprintf(fp, "nopersist\r\n");

	if(pppd->maxfail)
		os_fprintf(fp, "maxfail %d\r\n", pppd->maxfail);

	if(pppd->noipx)
		os_fprintf(fp, "noipx\r\n");
	else
		os_fprintf(fp, "ipx\r\n");

	if(pppd->ip)
		os_fprintf(fp, "ip\r\n");
	else
		os_fprintf(fp, "-ip\r\n");

	if(pppd->noip)
		os_fprintf(fp, "noip\r\n");
	else
		os_fprintf(fp, "-noip\r\n");

	if(pppd->mn)
		os_fprintf(fp, "mn\r\n");
	else
		os_fprintf(fp, "-mn\r\n");

	if(pppd->pc)
		os_fprintf(fp, "pc\r\n");
	else
		os_fprintf(fp, "-pc\r\n");

	if(pppd->vj)
		os_fprintf(fp, "vj\r\n");
	else
		os_fprintf(fp, "novj\r\n");

	if(pppd->passive)
		os_fprintf(fp, "passive\r\n");

	if(pppd->passive)
		os_fprintf(fp, "passive\r\n");

	if(pppd->silent)
		os_fprintf(fp, "silent\r\n");

	if(pppd->all)
		os_fprintf(fp, "-all\r\n");

	if(pppd->ac)
		os_fprintf(fp, "-ac\r\n");

	if(pppd->am)
		os_fprintf(fp, "-am\r\n");

	if(pppd->proxyarp)
		os_fprintf(fp, "proxyarp\r\n");

	if(pppd->login)
		os_fprintf(fp, "login\r\n");

	if(pppd->asyncmap)
		os_fprintf(fp, "asyncmap\r\n");

	if(pppd->auth)
		os_fprintf(fp, "auth\r\n");
	else
		os_fprintf(fp, "noauth\r\n");

	if(pppd->xonxoff)
		os_fprintf(fp, "xonxoff\r\n");
	else
		os_fprintf(fp, "-xonxoff\r\n");

	if(pppd->local)
		os_fprintf(fp, "local\r\n");
	else
		os_fprintf(fp, "-local\r\n");

	if(pppd->modem)
		os_fprintf(fp, "modem\r\n");
	else
		os_fprintf(fp, "-modem\r\n");

	if(pppd->lock)
		os_fprintf(fp, "lock\r\n");

	if(pppd->debug)
	{
		os_fprintf(fp, "debug\r\n");
		if(strlen(pppd->logfile))
			os_fprintf(fp, "logfile /var/log/%s\r\n",pppd->logfile);
	}
    //char crtscts[PPPD_OPTIONS_MAX];
        //使用硬件流控制来控制串口的数据流向，如RTS/CTS
    //char escape[PPPD_OPTIONS_MAX];
        //指定传输时，需要避免的某些字符
	os_fprintf(fp, "\r\n");
	os_fprintf(fp, "# modem IP option\r\n");
	if(pppd->mru)
		os_fprintf(fp, "mru %d\r\n", pppd->mru);
	if(pppd->mtu)
		os_fprintf(fp, "mtu %d\r\n", pppd->mtu);
	if(pppd->netmask)
		os_fprintf(fp, "netmask %d\r\n", pppd->netmask);

	if(pppd->ms_dns[0])
		os_fprintf(fp, "ms-dns %d\r\n", pppd->ms_dns[0]);
	if(pppd->ms_dns[1])
		os_fprintf(fp, "ms-dns %d\r\n", pppd->ms_dns[1]);
	if(pppd->demand)
		os_fprintf(fp, "demand %d\r\n", pppd->demand);

	if(strlen(pppd->domain))
		os_fprintf(fp, "domain %s\r\n",pppd->domain);

	if(strlen(pppd->name))
		os_fprintf(fp, "name %s\r\n",pppd->name);

	if(strlen(pppd->usehostname))
		os_fprintf(fp, "usehostname %s\r\n",pppd->usehostname);

	if(strlen(pppd->remotename))
		os_fprintf(fp, "remotename %s\r\n",pppd->remotename);

	if(strlen(pppd->domain))
		os_fprintf(fp, "domain %s\r\n",pppd->domain);

	if(pppd->defaultroute)
		os_fprintf(fp, "defaultroute\r\n");

	if(pppd->usepeerdns)
		os_fprintf(fp, "usepeerdns\r\n");

	if(pppd->noipdefault)
		os_fprintf(fp, "noipdefault\r\n");


	/*
	 * LCP options
	 */
	os_fprintf(fp, "\r\n");
	os_fprintf(fp, "# modem LCP option\r\n");
	if(pppd->lcp_echo_interval)
		os_fprintf(fp, "lcp-echo-interval %d\r\n", pppd->lcp_echo_interval);

	if(pppd->lcp_echo_failure)
		os_fprintf(fp, "lcp-echo-failure %d\r\n", pppd->lcp_echo_failure);

	if(pppd->lcp_restart)
		os_fprintf(fp, "lcp-restart %d\r\n", pppd->lcp_restart);

	if(pppd->lcp_max_configure)
		os_fprintf(fp, "lcp-max-configure %d\r\n", pppd->lcp_max_configure);

	if(pppd->lcp_max_terminate)
		os_fprintf(fp, "lcp-max-terminate %d\r\n", pppd->lcp_max_terminate);

	if(pppd->lcp_max_failure)
		os_fprintf(fp, "lcp-max-failure %d\r\n", pppd->lcp_max_failure);



	/*
	 * IPCP options
	 */
	os_fprintf(fp, "\r\n");
	os_fprintf(fp, "# modem IPCP option\r\n");
	if(pppd->ipcp_restart)
		os_fprintf(fp, "ipcp-restart %d\r\n", pppd->ipcp_restart);

	if(pppd->ipcp_max_terminate)
		os_fprintf(fp, "ipcp-max-terminate %d\r\n", pppd->ipcp_max_terminate);

	if(pppd->ipcp_max_configure)
		os_fprintf(fp, "ipcp-max-configure %d\r\n", pppd->ipcp_max_configure);

	if(pppd->ipcp_accept_local)
		os_fprintf(fp, "ipcp-accept-local\r\n");
	if(pppd->ipcp_accept_remote)
		os_fprintf(fp, "ipcp-accept-remote\r\n");
	/*
	 * PAP options
	 */
	os_fprintf(fp, "\r\n");
    os_fprintf(fp, "# modem PAP option\r\n");
	if(pppd->pap)
		os_fprintf(fp, "+pap\r\n");
	else
		os_fprintf(fp, "-pap\r\n");

	if(pppd->pap_restart)
		os_fprintf(fp, "pap-restart %d\r\n", pppd->pap_restart);
	if(pppd->pap_max_authreq)
		os_fprintf(fp, "pap-max-authreq %d\r\n", pppd->pap_max_authreq);
	if(pppd->pap_timeout)
		os_fprintf(fp, "pap-timeout %d\r\n", pppd->pap_timeout);


	/*
	 * CHAP options
	 */
	os_fprintf(fp, "\r\n");
    os_fprintf(fp, "# modem CHAP option\r\n");
	if(pppd->chap)
		os_fprintf(fp, "+chap\r\n");
	else
		os_fprintf(fp, "-chap\r\n");
	if(pppd->chap_restart)
		os_fprintf(fp, "chap-restart %d\r\n", pppd->chap_restart);
	if(pppd->chap_max_challenge)
		os_fprintf(fp, "chap-max-challenge %d\r\n", pppd->chap_max_challenge);
	if(pppd->chap_interval)
		os_fprintf(fp, "chap-interval %d\r\n", pppd->chap_interval);

	os_fprintf(fp, "\r\n");
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

	}
	return ERROR;
}

static int modem_pppd_create_dial_name(modem_pppd_t *pppd)
{
	FILE *fp = NULL;
	char filepath[256];
	assert(pppd);
	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "%s%s",MODEM_PPPD_PEERS_BASE, pppd->dial_name);
	remove(filepath);
	fp = fopen(filepath, "w+");
	if(fp)
	{
		char filepath[256];
		modem_t *modem = pppd->modem;
		modem_client_t *client = NULL;
		char *devname = NULL;
		if(!modem)
		{
			fclose(fp);
			return ERROR;
		}
#if 0
		client = modem->client;
		if(!client)
		{
			fclose(fp);
			return ERROR;
		}
		os_fprintf(fp, "# modem options \r\n");
		os_fprintf(fp, "# modem dev name\r\n");
		devname = strrchr(client->pppd.devname, '/');
		if(devname)
			devname++;
		else
			devname = client->pppd.devname;
		os_fprintf(fp, "%s\r\n", devname);
		os_fprintf(fp, "# modem dev baudrate\r\n");
		os_fprintf(fp, "%d\r\n", client->pppd.speed);
#endif
		modem_pppd_create_options_prefile(fp, &pppd->pppd_options);

		os_memset(filepath, 0, sizeof(filepath));
		os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_PEERS_BASE,
				pppd->dial_name, MODEM_PPPD_CONNECT);
		os_fprintf(fp, "connect '/usr/sbin/chat -s -v -f %s'\r\n", filepath);

		os_memset(filepath, 0, sizeof(filepath));
		os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_PEERS_BASE,
				pppd->dial_name, MODEM_PPPD_DISCONNECT);
		os_fprintf(fp, "disconnect '/usr/sbin/chat -s -v -f %s'\r\n", filepath);

		os_memset(filepath, 0, sizeof(filepath));
		os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_BASE,
				pppd->dial_name, MODEM_PPPD_SECRETS);
		os_fprintf(fp, "secrets '%s'\r\n", filepath);

		os_memset(filepath, 0, sizeof(filepath));
		os_snprintf(filepath, sizeof(filepath), "%s%s%s",MODEM_PPPD_BASE,
				pppd->dial_name, MODEM_PPPD_OPTIONS);
		os_fprintf(fp, "option '%s'\r\n", filepath);

		os_fprintf(fp, "\r\n");


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
#if 1
	int pid = child_process_create();
	if(pid < 0)
	{
		  zlog_warn(ZLOG_DEFAULT, "Can't create child process (%s), continuing", safe_strerror(errno));
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
	char filepath[256];
	os_memset(filepath, 0, sizeof(filepath));
	os_snprintf(filepath, sizeof(filepath), "pppd call %s&", pppd->dial_name);
	pppd->taskid = super_system(filepath);
#endif
	return OK;
}

static int modem_pppd_task_disconnect(modem_pppd_t *pppd)
{
	assert(pppd);
	return child_process_destroy(pppd->taskid);
}
/***********************************************************************/
BOOL modem_pppd_isconnect(modem_t *modem)
{
	assert(modem);
	modem_pppd_t *pppd = modem->pppd;
	if(pppd)
	{
		if(pppd->taskid)
		{
			return TRUE;
		}
	}
	return FALSE;
}

int modem_pppd_connect(modem_t *modem)
{
	assert(modem);
	modem_pppd_t *pppd = modem->pppd;
	if(pppd)
	{
		if(pppd->taskid)
		{
			modem_pppd_task_disconnect(pppd);
			pppd->taskid = 0;
		}
		if(pppd->taskid == 0)
		{
			pppd->taskid = modem_pppd_task_connect(pppd);
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
			pppd->taskid = 0;
			return OK;
		}
		else
			return OK;
	}
	return ERROR;
}



