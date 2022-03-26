/*
 * modem.c
 *
 *  Created on: Jul 24, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "modem_enum.h"
#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_message.h"
#include "modem_event.h"
#include "modem_machine.h"
#include "modem_dhcp.h"
#include "modem_pppd.h"
#include "modem_serial.h"
#include "modem_process.h"
#include "modem_usb_driver.h"
#include "modem_product.h"


zpl_uint32 modem_debug_conf = 0;

modem_main_t gModemmain;

static int modem_main_cleanall(void);

static int modem_main_list_init(void)
{
	os_memset(&gModemmain, 0, sizeof(modem_main_t));
	gModemmain.list = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	if(gModemmain.list)
	{
		gModemmain.mutex = os_mutex_init();
		lstInit(gModemmain.list);
		return OK;
	}
	return ERROR;
}


static int modem_main_list_exit(void)
{
	if(gModemmain.list)
	{
		if(lstCount(gModemmain.list))
		{
			modem_main_cleanall();
			lstFree(gModemmain.list);
		}
		XFREE(MTYPE_MODEM, gModemmain.list);
	}
	if(gModemmain.mutex)
		os_mutex_exit(gModemmain.mutex);
	return OK;
}


static modem_t * modem_main_lookup_node(char *name)
{
	NODE index;
	modem_t *pstNode = NULL;
	assert(name);
	char pname[MODEM_STRING_MAX];
	os_memset(pname, 0, MODEM_STRING_MAX);
	os_strcpy(pname, name);
	for(pstNode = (modem_t *)lstFirst(gModemmain.list);
			pstNode != NULL;  pstNode = (modem_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(os_memcmp(pname, pstNode->name, MODEM_STRING_MAX) == 0)
		{
			return pstNode;
		}
	}
	return NULL;
}

static int modem_main_default_node(modem_t *modem)
{
	assert(modem);
	modem->active 		= zpl_true;
	modem->dialtype 	= MODEM_DIAL_NONE;
	modem->ipstack 		= MODEM_IPV4;
	modem->bSecondary 	= zpl_false;
	modem->profile		= 1;
	modem->network = NETWORK_AUTO;

	modem->state = MODEM_MACHINE_STATE_NONE;

	modem->delay = MODEM_DELAY_CHK_TIME;
	modem->time_base = os_time (NULL);
	modem->time_axis = modem->time_base;

	modem->dedelay = MODEM_DECHK_TIME;
	modem->detime_base = os_time (NULL);
	modem->detime_axis = modem->detime_base;
	modem->mutex = os_mutex_init();
	return OK;
}

static int modem_main_add_node(char *name)
{
	assert(name);
	if(name)
	{
		modem_t *node = XMALLOC(MTYPE_MODEM, sizeof(modem_t));
		if(node)
		{
			os_memset(node, 0, sizeof(modem_t));

			os_strcpy(node->name, name);

			modem_main_default_node(node);
			lstAdd(gModemmain.list, (NODE*)node);
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}

static int modem_main_del_node(modem_t *node)
{
	assert(node);
	//modem_t *node = modem_main_lookup_node(client);
	if(node)
	{
		if(node->mutex)
			os_mutex_exit(node->mutex);
		lstDelete(gModemmain.list, (NODE*)node);
		modem_mgtlayer_remove(node);
		//modem_dhcpc_exit(node);
		//node->client = NULL;
/*		if(node->mutex)
			s_mutex_exit(node->mutex);*/
		os_memset(node, 0, sizeof(modem_t));
		XFREE(MTYPE_MODEM, node);
		return OK;
	}
	return ERROR;
}

static int modem_main_cleanall(void)
{
	NODE index;
	modem_t *pstNode = NULL;
	for(pstNode = (modem_t *)lstFirst(gModemmain.list);
			pstNode != NULL;  pstNode = (modem_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			modem_main_del_node(pstNode);
		}
	}
	return OK;
}

int modem_main_add_api(char *name)
{
	int ret = 0;
	assert(name);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(!modem_main_lookup_node(name))
		ret = modem_main_add_node(name);
	else
		ret = ERROR;
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return ret;
}

int modem_main_del_api(char *name)
{
	int ret = ERROR;
	assert(name);
	modem_t *node = modem_main_lookup_node(name);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	ret = modem_main_del_node(node);
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return ret;
}

int modem_main_bind_api(char *name, char *serialname)
{
	int ret = 0;
	assert(name);
	assert(serialname);
	modem_t *modem = NULL;
	modem_serial_t * serial = NULL;
	modem_client_t * client = NULL;
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	modem = modem_main_lookup_node(name);
	serial = modem_serial_lookup_api(serialname, 0);
	if(modem && serial)
	{
		serial->modem = modem;
		client = serial->client;
		if(client)
		{
			client->modem = modem;
			if(client->serial == NULL)
				client->serial = serial;
		}
		strcpy(modem->serialname, serialname);
		modem->serial = serial;
		modem->client = serial->client;
		//modem->ifp = serial->ifp;
		modem->state = MODEM_MACHINE_STATE_NONE;
		//modem->event = MODEM_EV_NONE;
		modem->active = zpl_true;
		//modem->pppdActive = zpl_false;
		if(client && client->driver && client->driver->modem_driver_probe)
		{
			(client->driver->modem_driver_probe)(client->driver);
			//modem_process_add_api(MODEM_EV_INSTER, modem, zpl_true);
		}
	}
	else
		ret = ERROR;
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return ret;
}

int modem_main_unbind_api(char *name, char *serialname)
{
	int ret = ERROR;
	modem_t *modem = NULL;
	modem_serial_t * serial = NULL;
	assert(name);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	modem = modem_main_lookup_node(name);
	if(modem)
	{
		if(modem->client)
		{
			modem_client_t * client = modem->client;
			if(client && client->driver && client->driver->modem_driver_exit)
			{
				(client->driver->modem_driver_exit)(client->driver);
				//modem_process_add_api(MODEM_EV_INSTER, modem, zpl_true);
			}
			modem_event_del_api(modem, MODEM_EV_MAX, zpl_true);
			((modem_client_t *)modem->client)->modem = NULL;
			//modem_event_remove(modem, MODEM_EV_REMOVE);
			//modem_process_add_api(MODEM_EV_REMOVE, modem, zpl_true);
		}
		serial = modem->serial;
		if(serial)
			serial->modem = NULL;
		memset(modem->serialname, 0, sizeof(modem->serialname));
		modem->serial = NULL;
		modem->client = NULL;
		modem->state = MODEM_MACHINE_STATE_NONE;
		//modem->event = MODEM_EV_NONE;
		modem->active = zpl_false;
		//modem->ifp = NULL;
		//modem->pppdActive = zpl_false;
	}
	else
		ret = ERROR;
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return ret;
}

#if 0
int modem_interface_bind_api(char *name, char *ifname)
{
	int ret = 0;
	assert(name);
	assert(ifname);
	modem_t *modem = NULL;
	struct interface * ifp = NULL;
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	modem = modem_main_lookup_node(name);
	ifp = if_lookup_by_name(ifname);
	if(modem && ifp)
	{
		modem->ifp = ifp;
	}
	else
		ret = ERROR;
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return ret;
}

int modem_interface_unbind_api(char *name)
{
	int ret = ERROR;
	modem_t *modem = NULL;
	//modem_serial_t * serial = NULL;
	assert(name);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	modem = modem_main_lookup_node(name);
	if(modem)
	{
		modem->ifp = NULL;
	}
	else
		ret = ERROR;
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return ret;
}
#endif

modem_t * modem_main_lookup_api(char *name)
{
	//int ret = ERROR;
	modem_t *node = NULL;
	assert(name);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	node = modem_main_lookup_node(name);
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return node;
}


int modem_main_callback_api(modem_cb cb, void *pVoid)
{
	NODE index;
	modem_t *pstNode = NULL;
	if(lstCount(gModemmain.list) ==  0)
		return OK;
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	for(pstNode = (modem_t *)lstFirst(gModemmain.list);
			pstNode != NULL;  pstNode = (modem_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(cb)
			(cb)(pstNode, pVoid);
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}


int modem_main_lock(modem_t *modem)
{
	if(modem && modem->mutex)
		os_mutex_lock(modem->mutex, OS_WAIT_FOREVER);
	return OK;
}


int modem_main_unlock(modem_t *modem)
{
	if(modem && modem->mutex)
		os_mutex_unlock(modem->mutex);
	return OK;
}
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
int modem_main_trywait(zpl_uint32 s)
{
	static zpl_uint32 time_base = 0;
	if(time_base == 0)
	{
		time_base = os_time(NULL);
	}
	else
	{
		zpl_uint32 now = os_time(NULL);
		if((now - time_base) > 1)
		{
			time_base = now;
			return OK;
		}
		os_sleep(s);
	}
	return OK;
}
/*************************************************************************/
/*************************************************************************/
static int modem_interface_system_up(zpl_bool up,  char *name)
{
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "ifconfig %s %s", name, up ? "up":"down");
	super_system(cmd);
	return OK;
}
/*************************************************************************/
int modem_interface_update_kernel(modem_t *modem, char *name)
{
	if(modem && modem->eth0)
	{
		struct interface *ifp = modem->eth0;
		MODEM_DEBUG("modem update kernel interface %s -> %s", ifp->name, name);
		modem_interface_system_up(zpl_true, name);
		if_kname_set(modem->eth0, name);
		//((struct interface *)modem->eth0)->k_ifindex = nsm_halpal_interface_ifindex(name);
		nsm_halpal_interface_up(modem->eth0);
		nsm_interface_update_kernel(modem->eth0, name);
		if(!(ifp->flags & IPSTACK_IFF_NOARP))
		{
			ifp->flags |= IPSTACK_IFF_NOARP;
			nsm_halpal_interface_update_flag(ifp, ifp->flags);
		}
	}
	return OK;
}

int modem_serial_interface_update_kernel(modem_t *modem, char *name)
{
	if(modem && modem->ppp_serial)
	{
		MODEM_DEBUG("modem update kernel interface %s -> %s", ((struct interface *)modem->ppp_serial)->name, name);
		modem_interface_system_up(zpl_true, name);
		if_kname_set(modem->ppp_serial, name);
		//((struct interface *)modem->eth0)->k_ifindex = nsm_halpal_interface_ifindex(name);
		nsm_halpal_interface_up(modem->ppp_serial);
		nsm_serial_interface_kernel(modem->ppp_serial, name);
	}
	return OK;
}

int modem_serial_devname_update_kernel(modem_t *modem, char *name)
{
	if(modem && modem->ppp_serial)
	{
		MODEM_DEBUG("modem update devname interface %s -> %s", ((struct interface *)modem->ppp_serial)->name, name);
		nsm_serial_interface_devname(modem->ppp_serial, name);
	}
	return OK;
}
/*************************************************************************/
/*************************************************************************/
int modem_bind_interface_update(modem_t *modem)
{
	modem_client_t *client = NULL;
	if(!modem)
		return ERROR;
	client = modem->client;
	switch(modem->dialtype)
	{
	case MODEM_DIAL_NONE:
	case MODEM_DIAL_DHCP:
	case MODEM_DIAL_QMI:
	case MODEM_DIAL_GOBINET:
		if(modem->eth0 && strlen(client->driver->eth_name))
			modem_interface_update_kernel(modem, client->driver->eth_name);
		break;
	case MODEM_DIAL_PPP:
		if(modem->ppp_serial && strlen(client->driver->eth_name))
			modem_serial_interface_update_kernel(modem, client->driver->eth_name);

		if(modem->ppp_serial && strlen(client->driver->pppd->devname))
			modem_serial_devname_update_kernel(modem, client->driver->pppd->devname);

		break;
	}
	return OK;
}
/*************************************************************************/
/*************************************************************************/
int modem_main_init(void)
{
	modem_debug_conf = MODEM_DEBUG_ATCMD;//0XFFFFFFF;
	modem_main_list_init();
	modem_pppd_init();
	modem_serial_init();
	modem_client_init();
	modem_process_init();

	modem_usb_driver_init();

	modem_product_init();

#if 1//def __MODEM_DEBUG
	modem_serial_add_api("quectelec20");
#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
	modem_serial_channel_api("quectelec20", 5);
#else
	modem_serial_channel_api("quectelec20", 1);
#endif
	modem_main_add_api("ec25");
	modem_main_bind_api("ec25", "quectelec20");
#endif
	return OK;
}

int modem_main_exit(void)
{
	modem_process_exit();
	modem_pppd_exit();
	modem_main_list_exit();
	modem_serial_exit();
	modem_client_exit();
	modem_usb_driver_exit();
	return OK;
}
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

#ifdef __MODEM_DEBUG
void modem_debug_printf(void *fp, char *func, zpl_uint32 line,  const char *format, ...)
{
	va_list args;
	va_start(args, format);
	fprintf(fp, "%s(%d)",func,line);
	vfprintf(fp, format, args);
	fprintf(fp, "\n");
	fflush(fp);
	va_end(args);
}
#endif
