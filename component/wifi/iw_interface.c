/*
 * nsm_iw.c
 *
 *  Created on: Jul 15, 2018
 *      Author: zhurish
 */



#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"
#include "nsm_client.h"

#include "iw_config.h"
#include "iw_ap.h"
#include "iw_client.h"
#include "iw_interface.h"



#ifdef IW_ONCE_TASK
static int iw_taskid = 0;
#endif

iw_t * nsm_iw_get(struct interface *ifp)
{
	if(if_is_wireless(ifp) && ifp->ll_type == ZEBRA_LLT_WIRELESS)
	{
		struct nsm_interface *nsm = ifp->info[MODULE_NSM];
		return (iw_t *)nsm->nsm_client[NSM_WIFI];
	}
	return NULL;
}

int nsm_iw_mode_set_api(struct interface *ifp, iw_mode_t mode)
{
	iw_t * iw = nsm_iw_get(ifp);
	if(iw)
	{
		if(iw->mode != mode)
		{
			if(iw->mode == IW_MODE_AP)
			{
				iw_ap_exit(&iw->private.ap);
			}
			if(iw->mode == IW_MODE_MANAGE || iw->mode == IW_MODE_CLIENT)
			{
				iw_client_exit(&iw->private.client);
			}

			if(mode == IW_MODE_AP)
			{
				iw_ap_init(&iw->private.ap, ifp->ifindex);
				iw->private.ap.ifindex = ifp->ifindex;
/*				if(ifp->k_ifindex)
				{
					pal_interface_down(ifp);
					iw_dev_mode_set(ifp, "master");
					pal_interface_up(ifp);
				}*/
			}
			if(mode == IW_MODE_MANAGE || mode == IW_MODE_CLIENT)
			{
				iw_client_init(&iw->private.client, ifp->ifindex);
				iw->private.client.ifindex = ifp->ifindex;
				if(ifp->k_ifindex)
				{
					pal_interface_down(ifp);
					iw_dev_mode_set(ifp, "managed");
					pal_interface_up(ifp);
				}
			}
		}
		iw->mode = mode;
		return OK;
	}
	return ERROR;
}

int nsm_iw_mode_get_api(struct interface *ifp, iw_mode_t *mode)
{
	iw_t * iw = nsm_iw_get(ifp);
	if(iw)
	{
		if(mode)
			*mode = iw->mode;
		return OK;
	}
	return ERROR;
}


iw_mode_t nsm_iw_mode(struct interface *ifp)
{
	iw_t * iw = nsm_iw_get(ifp);
	if(iw)
	{
		return iw->mode;
	}
	return IW_MODE_NONE;
}


int nsm_iw_capabilities_show(struct interface *ifp, struct vty *vty)
{
	if(!ifp || !vty)
		return ERROR;
	if(ifp && ifp->k_ifindex && if_is_wireless(ifp))
	{
		return iw_phy_capabilities_show(ifp, vty);
	}
	return OK;
}

int nsm_iw_ap_info_show(struct interface *ifp, struct vty *vty)
{
	if(!ifp || !vty)
		return ERROR;
	if(ifp && ifp->k_ifindex && if_is_wireless(ifp))
	{
		return iw_ap_dev_information_show(ifp, vty);
	}
	return OK;
}

int nsm_iw_channel_freq_show(struct interface *ifp, struct vty *vty)
{
	if(!ifp || !vty)
		return ERROR;
	if(ifp && ifp->k_ifindex && if_is_wireless(ifp))
	{
		return iw_dev_channel_support_show(ifp, vty);
	}
	return OK;
}

static int nsm_iw_create_interface(struct interface *ifp)
{
	int kmode = 0;
	iw_t * iw = NULL;
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(nsm && if_is_wireless(ifp) && ifp->ll_type == ZEBRA_LLT_WIRELESS)
	{
		if(!nsm->nsm_client[NSM_WIFI])
			nsm->nsm_client[NSM_WIFI] = XMALLOC(MTYPE_WIFI, sizeof(iw_t));
		zassert(nsm->nsm_client[NSM_WIFI]);
		os_memset(nsm->nsm_client[NSM_WIFI], 0, sizeof(iw_t));
		iw = nsm->nsm_client[NSM_WIFI];
		iw->ifp = ifp;
/*		"Auto",
		"Ad-Hoc",
		"Managed",
		"Master",
		"Repeater",
		"Secondary",
		"Monitor",
		"Unknown/bug"*/
#if 0
		/* Modes of operation */
		#define IW_MODE_AUTO	0	/* Let the driver decides */
		#define IW_MODE_ADHOC	1	/* Single cell network */
		#define IW_MODE_INFRA	2	/* Multi cell network, roaming, ... */
		#define IW_MODE_MASTER	3	/* Synchronisation master or Access Point */
		#define IW_MODE_REPEAT	4	/* Wireless Repeater (forwarder) */
		#define IW_MODE_SECOND	5	/* Secondary master/repeater (backup) */
		#define IW_MODE_MONITOR	6	/* Passive monitor (listen only) */
#endif
		kmode = iw_dev_mode(ifp);
		switch(kmode)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
			nsm_iw_mode_set_api(ifp, IW_MODE_MANAGE);
			break;
		case 3:
			nsm_iw_mode_set_api(ifp, IW_MODE_AP);
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		default:
			break;
		}
	}
	return OK;
}


static int nsm_iw_delete_interface(struct interface *ifp)
{
	if(ifp && if_is_wireless(ifp) && ifp->ll_type == ZEBRA_LLT_WIRELESS)
	{
		iw_t * iw = nsm_iw_get(ifp);
		if(iw)
		{
			struct nsm_interface *nsm = ifp->info[MODULE_NSM];
			if(iw->mode == IW_MODE_AP)
			{
				iw_ap_exit(&iw->private.ap);
			}
			if(iw->mode == IW_MODE_MANAGE || iw->mode == IW_MODE_CLIENT)
			{
				iw_client_exit(&iw->private.client);
			}

			XFREE(MTYPE_WIFI, iw);
			nsm->nsm_client[NSM_WIFI] = NULL;
		}
	}
	return OK;
}


static int nsm_iw_write_config_ap(struct vty *vty, iw_t * iw)
{
	if (iw)
	{
		iw_ap_config(&iw->private.ap, vty);
/*		if (os_strlen(iw->private.ap.SSID))
			vty_out(vty, " ap-ssid name %s%s", (iw->private.ap.SSID),VTY_NEWLINE);

		if (os_strlen(iw->private.ap.password))
			vty_out(vty, " ap-password %s%s", iw->private.ap.encrypt_password, VTY_NEWLINE);

		vty_out(vty, " authentication-method %s%s",
				iw_auth_string(iw->private.ap.auth), VTY_NEWLINE);
		vty_out(vty, " encryption-method %s%s",
				iw_encryption_string(iw->private.ap.encryption), VTY_NEWLINE);
		vty_out(vty, " radio-type %s%s",
				iw_network_string(iw->private.ap.network), VTY_NEWLINE);

		if (iw->private.ap.channel)
			vty_out(vty, " work-channel %d%s", iw->private.ap.channel, VTY_NEWLINE);
		else
			vty_out(vty, " work-channel auto%s", VTY_NEWLINE);

		//vty_out(vty, " wireless freq %d%s", iw->private.ap.freq, VTY_NEWLINE);

		if (iw->private.ap.power)
			vty_out(vty, " power-level %d%s", iw->private.ap.power, VTY_NEWLINE);
		else
			vty_out(vty, " power-level auto%s", VTY_NEWLINE);*/


/*		vty_out(vty, " beacon-interval %s%s", (iw->private.ap.power),
				VTY_NEWLINE);
		vty_out(vty, " wireless power %s%s", (iw->private.ap.power),
				VTY_NEWLINE);*/

	}
	return OK;
}

static int nsm_iw_write_config_client(struct vty *vty, iw_t * iw)
{
	if (iw)
	{
		if (os_strlen(iw->private.client.cu.SSID))
		{
			vty_out(vty, " network-name %s%s", (iw->private.client.cu.SSID), VTY_NEWLINE);

			//vty_out(vty, " ap-name %s", (iw->private.client.cu.SSID));

			if (os_strlen(iw->private.client.cu.encrypt_password))
				vty_out(vty, " authentication password %s%s", (iw->private.client.cu.encrypt_password), VTY_NEWLINE);
			//else
			//	vty_out(vty, "%s", VTY_NEWLINE);
		}

		if ((iw->private.client.scan_max))
			vty_out(vty, " scan-num %d%s", (iw->private.client.scan_max), VTY_NEWLINE);
		if ((iw->private.client.scan_interval))
			vty_out(vty, " scan-interval %d%s", (iw->private.client.scan_interval), VTY_NEWLINE);
		//vty_out(vty, " scan-interval %d%s", (iw->private.ap.SSID));
	}
	return OK;
}

static int nsm_iw_write_config_interface(struct vty *vty, struct interface *ifp)
{
	if(ifp && if_is_wireless(ifp) && ifp->ll_type == ZEBRA_LLT_WIRELESS)
	{
		iw_t * iw = nsm_iw_get(ifp);
		if(iw)
		{
			vty_out(vty, " work-mode %s %s", iw_mode_string(iw->mode), VTY_NEWLINE);
			switch(iw->mode)
			{
			case IW_MODE_IBSS:
				break;
			case IW_MODE_MANAGE:
			//case IW_MODE_CLIENT:
				nsm_iw_write_config_client(vty, iw);
				break;
			case IW_MODE_AP:
				nsm_iw_write_config_ap(vty, iw);
				break;
			case IW_MODE_WDS:
				break;
			case IW_MODE_MONITOR:
				break;
			case IW_MODE_MESH:
				break;
/*			case IW_MODE_CLIENT:
				nsm_iw_write_config_client(vty, iw);
				break;*/
			default:
				break;
			}
		}
	}
	return OK;
}

int nsm_iw_debug_write_config(struct vty *vty)
{
	if (IW_DEBUG(EVENT))
		vty_out(vty, "debug wireless event %s%s", IW_DEBUG(DETAIL)? "detail":" ", VTY_NEWLINE);
	if (IW_DEBUG(DB))
		vty_out(vty, "debug wireless db %s%s", IW_DEBUG(DETAIL)? "detail":" ", VTY_NEWLINE);
	if (IW_DEBUG(SCAN))
		vty_out(vty, "debug wireless scan %s%s", IW_DEBUG(DETAIL)? "detail":" ", VTY_NEWLINE);
	if (IW_DEBUG(AP))
		vty_out(vty, "debug wireless ap %s%s", IW_DEBUG(DETAIL)? "detail":" ", VTY_NEWLINE);
	if (IW_DEBUG(AP_ACCEPT))
		vty_out(vty, "debug wireless ap-accept %s%s", IW_DEBUG(DETAIL)? "detail":" ", VTY_NEWLINE);
	return OK;
}

#ifdef IW_ONCE_TASK
static int iw_task(void *p)
{
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	os_start_running(NULL, MODULE_WIFI);
	return OK;
}


static int iw_task_start(void)
{
	master_thread[MODULE_WIFI] = thread_master_module_create (MODULE_WIFI);

	if(iw_taskid == 0)
		iw_taskid = os_task_create("iwApTask", OS_TASK_DEFAULT_PRIORITY,
	               0, iw_task, NULL, OS_TASK_DEFAULT_STACK);
	if(iw_taskid)
		return OK;
	return ERROR;
}

static int iw_task_exit(void)
{
	if(iw_taskid)
	{
		//iw_ap_stop(iw_ap);
		thread_master_free(master_thread[MODULE_WIFI]);
		if(os_task_destroy(iw_taskid)==OK)
			iw_taskid = 0;
	}
	return OK;
}
#endif

int nsm_iw_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->notify_add_cb = nsm_iw_create_interface;
	nsm->notify_delete_cb = nsm_iw_delete_interface;
	nsm->interface_write_config_cb = nsm_iw_write_config_interface;
	nsm_client_install (nsm, NSM_WIFI);
#ifdef IW_ONCE_TASK
	//iw_task_start();
#endif
	return OK;
}

int nsm_iw_client_exit()
{
	iw_task_exit();
	struct nsm_client *nsm = nsm_client_lookup (NSM_WIFI);
	nsm_client_free (nsm);
	return OK;
}
