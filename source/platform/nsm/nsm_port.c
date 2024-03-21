/*
 * nsm_port.c
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zpl_type.h"
#include "os_util.h"
#include "os_sem.h"
#include "module.h"
#include "route_types.h"
#include "zmemory.h"
#include "prefix.h"
#include "log.h"
#include "template.h"
#ifdef ZPL_SHELL_MODULE
#include "vty_include.h"
#endif
#include "nsm_interface.h"
#include "nsm_port.h"
#include "hal_include.h"


static nsm_port_t * _nsm_port_get(struct interface *ifp)
{
	return (nsm_port_t *)nsm_intf_module_data(ifp, NSM_INTF_PORT);
}


static int nsm_port_interface_create_api(struct interface *ifp)
{
	nsm_port_t *port = NULL;
	if(if_is_ethernet(ifp))
	{
		port = (nsm_port_t *)nsm_intf_module_data(ifp, NSM_INTF_PORT);
		if(port == NULL)
		{
			port = XMALLOC(MTYPE_PORT, sizeof(nsm_port_t));
			if(port == NULL)
				return ERROR;
			os_memset(port, 0, sizeof(nsm_port_t));
			if (port->mutex == NULL)
				port->mutex = os_mutex_name_create(os_name_format("%s-port_mutex", ifp->name));
			IF_NSM_PORT_DATA_LOCK(port);
			port->ifp = ifp;
			port->learning_enable = zpl_true;
			nsm_intf_module_data_set(ifp, NSM_INTF_PORT, port);
			IF_NSM_PORT_DATA_UNLOCK(port);
		}
	}
	return OK;
}


static int nsm_port_interface_del_api(struct interface *ifp)
{
	nsm_port_t *port = NULL;
	port = (nsm_port_t *)nsm_intf_module_data(ifp, NSM_INTF_PORT);
	if(if_is_ethernet(ifp) && port)
	{
		if(port->mutex)
		{
			os_mutex_destroy(port->mutex);
			port->mutex = NULL;
		}
		XFREE(MTYPE_PORT, port);
		port = NULL;
		nsm_intf_module_data_set(ifp, NSM_INTF_PORT, NULL);
	}
	return OK;
}


int nsm_port_jumbo_set_api(struct interface *ifp, zpl_bool value)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_jumbo_set(ifp->ifindex, value);
#else
		ret = OK;
#endif
		if(ret == OK)
			port->jumbo_enable = value;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}

int nsm_port_jumbo_get_api(struct interface *ifp, zpl_bool *value)
{
	int ret = OK;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
		if(value)
			*value = port->jumbo_enable;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}


int nsm_port_loopback_set_api(struct interface *ifp, zpl_bool value)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_loop_set(ifp->ifindex, value);
#else
		ret = OK;
#endif
		if(ret == OK)
			port->loopback = value;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}
int nsm_port_loopback_get_api(struct interface *ifp, zpl_bool *value)
{
	int ret = OK;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
		if(value)
			*value = port->loopback;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}

int nsm_port_learning_set_api(struct interface *ifp, zpl_bool value)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_learning_set(ifp->ifindex, value);
#else
		ret = OK;
#endif
		if(ret == OK)
			port->learning_enable = value;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}
int nsm_port_learning_get_api(struct interface *ifp, zpl_bool *value)
{
	int ret = OK;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
		if(value)
			*value = port->learning_enable;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}

int nsm_port_sw_learning_set_api(struct interface *ifp, zpl_bool value)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_software_learning_set(ifp->ifindex, value);
#else
		ret = OK;
#endif
		if(ret == OK)
			port->sw_learning_enable = value;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}
int nsm_port_sw_learning_get_api(struct interface *ifp, zpl_bool *value)
{
	int ret = OK;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
		if(value)
			*value = port->sw_learning_enable;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}

int nsm_port_pause_set_api(struct interface *ifp, zpl_bool tx, zpl_bool rx)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_pause_set(ifp->ifindex, tx, rx);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			port->pause_tx = tx;
			port->pause_rx = rx;
		}
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}
int nsm_port_pause_get_api(struct interface *ifp, zpl_bool *tx, zpl_bool *rx)
{
	int ret = OK;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
		if(tx)
			*tx = port->pause_tx;
		if(rx)
			*rx = port->pause_rx;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}

int nsm_port_protect_set_api(struct interface *ifp, zpl_bool value)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_protected_set(ifp->ifindex, value);
#else
		ret = OK;
#endif
		if(ret == OK)
			port->protect = value;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}
int nsm_port_protect_get_api(struct interface *ifp, zpl_bool *value)
{
	int ret = OK;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
		if(value)
			*value = port->protect;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}

int nsm_port_flowcontrol_set_api(struct interface *ifp, zpl_bool tx, zpl_bool rx)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_flow_set(ifp->ifindex, tx, rx);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			port->flowcontrol_tx = tx;
			port->flowcontrol_rx = rx;
		}
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}
int nsm_port_flowcontrol_get_api(struct interface *ifp, zpl_bool *tx, zpl_bool *rx)
{
	int ret = OK;
	zassert(ifp);
	nsm_port_t *port = (nsm_port_t *)_nsm_port_get(ifp);
	zassert(port);
	IF_NSM_PORT_DATA_LOCK(port);
	if(port)
	{
		if(tx)
			*tx = port->flowcontrol_tx;
		if(rx)
			*rx = port->flowcontrol_rx;
	}
	IF_NSM_PORT_DATA_UNLOCK(port);
	return ret;
}

#ifdef ZPL_SHELL_MODULE
static int nsm_port_interface_write_config(struct vty *vty, struct interface *ifp)
{
	nsm_port_t *port = _nsm_port_get(ifp);
	if(port && if_is_ethernet(ifp))
	{
		IF_NSM_PORT_DATA_LOCK(port);
		if(port->learning_enable != NSM_PORT_LEARNING_DEFAULT)
			vty_out(vty, " mac-address learning %s%s", port->learning_enable?"enable":"disable",VTY_NEWLINE);

		if(port->sw_learning_enable != NSM_PORT_SWLEARNING_DEFAULT)
			vty_out(vty, " mac-address learning software %s%s", port->sw_learning_enable?"enable":"disable",VTY_NEWLINE);

		if(port->jumbo_enable != NSM_PORT_JUMBO_DEFAULT)
			vty_out(vty, " jumboframe %s%s", port->jumbo_enable?"enable":"disable",VTY_NEWLINE);

		if(port->loopback != NSM_PORT_LOOPBACK_DEFAULT)
			vty_out(vty, " port-loopback%s", VTY_NEWLINE);

		if(port->pause_tx != NSM_PORT_PUASE_TX_DEFAULT)
			vty_out(vty, " port-pause transmit%s", VTY_NEWLINE);
		if(port->pause_rx != NSM_PORT_PUASE_RX_DEFAULT)
			vty_out(vty, " port-pause receive%s",VTY_NEWLINE);
		
		if(port->protect != NSM_PORT_PROTECT_DEFAULT)
			vty_out(vty, " port-protect%s", VTY_NEWLINE);
		
		if(port->flowcontrol_tx != NSM_PORT_FLOWCONTROL_TX_DEFAULT)
			vty_out(vty, " flowcontrol transmit%s",VTY_NEWLINE);
		if(port->flowcontrol_rx != NSM_PORT_FLOWCONTROL_RX_DEFAULT)
			vty_out(vty, " flowcontrol receive%s",VTY_NEWLINE);//flowcontrol {send|receive} {on|off}
		IF_NSM_PORT_DATA_UNLOCK(port);	
	}
	return OK;
}
#endif


int nsm_port_init(void)
{
	nsm_interface_hook_add(NSM_INTF_PORT, nsm_port_interface_create_api, nsm_port_interface_del_api);
	nsm_interface_write_hook_add(NSM_INTF_PORT, nsm_port_interface_write_config);
	return OK;
}

int nsm_port_exit(void)
{
	return OK;
}

int nsm_port_start(void)
{
	return OK;
}

