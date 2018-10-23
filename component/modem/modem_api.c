/*
 * modem_api.c
 *
 *  Created on: Jul 29, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "if.h"
#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_message.h"
#include "modem_event.h"
#include "modem_machine.h"
#include "modem_dhcp.h"
#include "modem_pppd.h"
#include "modem_api.h"
#include "modem_usb_driver.h"


int modem_main_apn_set_api(modem_t *modem, char *apn)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(apn)
		{
			os_memset(modem->apn, 0, sizeof(modem->apn));
			os_strcpy(modem->apn, apn);
		}
		else
			os_memset(modem->apn, 0, sizeof(modem->apn));
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_ip_set_api(modem_t *modem, modem_stack_type type)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		modem->ipstack = type;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_ip_get_api(modem_t *modem, modem_stack_type *type)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(type)
			*type = modem->ipstack;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_secondary_set_api(modem_t *modem, BOOL bSecondary)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		modem->bSecondary = bSecondary;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_secondary_get_api(modem_t *modem, BOOL *bSecondary)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(bSecondary)
			*bSecondary = modem->bSecondary;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}
int modem_main_svc_set_api(modem_t *modem, char *svc)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(svc)
		{
			os_memset(modem->svc, 0, sizeof(modem->svc));
			os_strcpy(modem->svc, svc);
		}
		else
			os_memset(modem->svc, 0, sizeof(modem->svc));
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}


int modem_main_pin_set_api(modem_t *modem, char *pin)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(pin)
		{
			os_memset(modem->pin, 0, sizeof(modem->pin));
			os_strcpy(modem->pin, pin);
		}
		else
			os_memset(modem->pin, 0, sizeof(modem->pin));
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_puk_set_api(modem_t *modem, char *puk)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(puk)
		{
			os_memset(modem->puk, 0, sizeof(modem->puk));
			os_strcpy(modem->puk, puk);
		}
		else
			os_memset(modem->puk, 0, sizeof(modem->puk));
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}


int modem_main_profile_set_api(modem_t *modem, int profile)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		modem->profile = profile;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_profile_get_api(modem_t *modem, int *profile)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(profile)
			*profile = modem->profile;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_dial_set_api(modem_t *modem, modem_dial_type type)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(modem->dialtype != type)
		{
			if(modem->dialtype == MODEM_DIAL_PPP && type != MODEM_DIAL_PPP)
			{
				if(modem->pppd && modem_pppd_del_api(modem->pppd) != OK)
				{
					if(gModemmain.mutex)
						os_mutex_unlock(gModemmain.mutex);
					return ERROR;
				}
				modem->dialtype = type;
			}
			if(modem->dialtype != MODEM_DIAL_PPP && type == MODEM_DIAL_PPP)
			{
				if(modem->pppd == NULL && modem_pppd_add_api(modem) != OK)
				{
					if(gModemmain.mutex)
						os_mutex_unlock(gModemmain.mutex);
					return ERROR;
				}
				modem->dialtype = type;
			}
		}
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_dial_get_api(modem_t *modem, modem_dial_type *type)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(type)
			*type = modem->dialtype;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}


int modem_main_network_set_api(modem_t *modem, modem_network_type profile)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		modem->network = profile;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

int modem_main_network_get_api(modem_t *modem, modem_network_type *profile)
{
	assert (modem);
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(profile)
			*profile = modem->network;
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}




static modem_t * modem_lookup_by_ifp(struct interface *ifp)
{
	NODE index;
	modem_t *pstNode = NULL;
	assert (ifp);
	if(lstCount(gModemmain.list) ==  0)
		return NULL;
	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	for(pstNode = (modem_t *)lstFirst(gModemmain.list);
			pstNode != NULL;  pstNode = (modem_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(if_is_serial(ifp))
		{
			if(pstNode && pstNode->ppp_serial == ifp)
				break;
			if(pstNode && pstNode->dial_serial == ifp)
				break;
			if(pstNode && pstNode->test_serial == ifp)
				break;
		}
		else
		{
			if(pstNode && pstNode->eth0 == ifp)
				break;
			if(pstNode && pstNode->eth1 == ifp)
				break;
			if(pstNode && pstNode->eth2 == ifp)
				break;
		}
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return pstNode;
}


modem_t * modem_lookup_by_interface(char *name)
{
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name(name);
	if(ifp)
		return modem_lookup_by_ifp(ifp);
	return NULL;
}
/*
 * interface modem 0/1/1
 *  bind modem-profile <name>
 */
int modem_bind_interface_api(modem_t *modem, char *name, int number)
{
	int ret = ERROR;
	assert (name);
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name(name);
	if(!ifp)
	{
		if(modem_interface_add_api(name) == OK)
		{
			ifp = if_lookup_by_name(name);
		}
	}
	if(!ifp)
		return ERROR;

	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		if(if_is_serial(ifp))
		{
			switch(number)
			{
			case 1:
				modem->ppp_serial = ifp;
				ret = modem_pppd_add_api(modem);
				//ifp->ll_type = ZEBRA_LLT_MODEM;
				break;
			case 2:
				modem->dial_serial = ifp;
				//ifp->ll_type = ZEBRA_LLT_MODEM;
				ret = OK;
				break;
			case 3:
				modem->test_serial = ifp;
				//ifp->ll_type = ZEBRA_LLT_MODEM;
				ret = OK;
				break;
			default:
				break;
			}
		}
		else
		{
			switch(number)
			{
			case 1:
				modem->eth0 = ifp;
				ifp->ll_type = ZEBRA_LLT_MODEM;
				ret = OK;
				break;
			case 2:
				modem->eth1 = ifp;
				ifp->ll_type = ZEBRA_LLT_MODEM;
				ret = OK;
				break;
			case 3:
				modem->eth2 = ifp;
				ifp->ll_type = ZEBRA_LLT_MODEM;
				ret = OK;
				break;
			default:
				break;
			}
		}
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}

/*
 * interface modem 0/1/1
 *  no bind modem-profile
 */
int modem_unbind_interface_api(modem_t *modem, BOOL ppp, int number)
{
	int ret = ERROR;
	//assert (name);
	struct interface *ifp = NULL;
/*
	ifp = if_lookup_by_name(name);
	if(!ifp)
	{
		if(modem_interface_add_api(name) == OK)
		{
			ifp = if_lookup_by_name(name);
		}
	}
	if(!ifp)
		return ERROR;
*/

	if(gModemmain.mutex)
		os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);
	if(modem)
	{
		switch(number)
		{
		case 1:
			if(ppp && modem->ppp_serial)
			{
				ret = modem_pppd_del_api(modem);
			}
			if(!ppp && modem->eth0)
			{
				modem_mgtlayer_network_offline(modem);
				if_make_llc_type(modem->eth0);
				modem->eth0 = NULL;
				ret = OK;
			}
			break;
		case 2:
			if(ppp && modem->dial_serial)
			{
				ret = modem_pppd_del_api(modem);
			}
			if(!ppp && modem->eth1)
			{
				modem_mgtlayer_network_offline(modem);
				if_make_llc_type(modem->eth1);
				modem->eth1 = NULL;
				ret = OK;
			}
			break;
		case 3:
			modem->test_serial = NULL;
			if(ppp && modem->test_serial)
			{
				ret = modem_pppd_del_api(modem);
			}
			if(!ppp && modem->eth2)
			{
				modem_mgtlayer_network_offline(modem);
				if_make_llc_type(modem->eth2);
				modem->eth2 = NULL;
				ret = OK;
			}
			break;
		default:
			break;
		}
	}
	if(gModemmain.mutex)
		os_mutex_unlock(gModemmain.mutex);
	return OK;
}


int modem_interface_add_api(char *name)
{
	int i = 0;
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name(name);
	if(ifp && ifp->ll_type == ZEBRA_LLT_MODEM)
		return OK;
	if(ifp && ifp->ll_type != ZEBRA_LLT_MODEM)
		return ERROR;

	if_new_llc_type_mode(ZEBRA_LLT_MODEM, IF_MODE_L3);

	ifp = if_create_dynamic (name, strlen(name));
	if(ifp)
	{
		//ifp->ll_type = ZEBRA_LLT_MODEM;
		//ifp->if_mode = IF_MODE_L3;
		//ifp->dynamic = TRUE;
/*		if_manage_kernel_update(ifp);
		zebra_interface_add_update(ifp);*/
		return OK;
	}
	return ERROR;
}

/*struct interface * modem_interface_del_api(char *name)
{

}*/


/*
 * modem-profile <name>
 *  bind serial <aa>
 * exit
 *
 * serial-profile <name>
 *  bind aa
 * exit
 */
