/*
 * modem_main.c
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
#include "modem_error.h"
#include "modem_dhcp.h"
#include "modem_pppd.h"
#include "modem_attty.h"

#include "modem_dialog.h"
#include "modem_message.h"
#include "modem_driver.h"
#include "modem_atcmd.h"
#include "modem_usim.h"
#include "modem_process.h"
#include "modem_usb_driver.h"

static zpl_uint32 modem_task_id = 0;

os_ansync_lst * modem_ansync_lst = NULL;

/*************************************************************************/
/*
static int modem_main_handle(modem_t *modem, void *pVoid)
{
	assert(modem);
	modem_process_t *process = (modem_process_t *)pVoid;
	if(!process)
		return ERROR;
	if(modem->active && process->event)
	{
		modem_event	event = process->event;
		process->event = MODEM_EV_NONE;
		if(event)
		{
			modem_event_process(modem, event);
		}
		return OK;
	}
	return ERROR;
}
*/

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
static int modem_process_handle(modem_process_t *process, void *pVoid)
{
	assert(process);
	if(!process)
		return ERROR;
	assert(process->argv);
	modem_t	*modem = process->argv;


	if(modem->active && !modem->proxy && process->event)
	{
		modem_event	event = process->event;
		process->event = MODEM_EV_NONE;
		if(event)
		{
			if(gModemmain.mutex)
				os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);

			modem_event_process(modem, event);

			if(gModemmain.mutex)
				os_mutex_unlock(gModemmain.mutex);
		}
		return OK;
	}
	return ERROR;
}

static int modem_main_task(void *argv)
{
	os_ansync_t *node;
	os_sleep(5);
	host_config_load_waitting();
	while(modem_ansync_lst)
	{
		while((node = os_ansync_fetch(modem_ansync_lst)))
			os_ansync_execute(modem_ansync_lst, node, OS_ANSYNC_EXECUTE_NONE);
	}
	return 0;
}


int modem_ansync_add(int (*cb)(void *), int fd, char *name)
{
	if(modem_ansync_lst)
	{
		//OS_ANSYNC_DEBUG("%s(fd=%d)", name,fd);
		return _os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_INPUT, cb,
			NULL, fd, name, __FILE__, __LINE__);
	}
	return ERROR;
	//os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_INPUT, c, NULL, v)
}

int modem_ansync_del(int value)
{
	if(modem_ansync_lst)
	{
		//OS_ANSYNC_DEBUG("%s", name);
		return _os_ansync_unregister_api(modem_ansync_lst, OS_ANSYNC_INPUT, NULL,
			NULL, value);
		//return os_ansync_del_api(modem_ansync_lst, value);
	}
	return ERROR;
	//os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_INPUT, c, NULL, v)
}

int modem_ansync_timer_add(int (*cb)(void *), int fd, char *name)
{
	if(modem_ansync_lst)
	{
		//OS_ANSYNC_DEBUG("%s", name);
		return _os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_TIMER, cb,
			NULL, fd, name, __FILE__, __LINE__);
	}
	return ERROR;
	//os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_INPUT, c, NULL, v)
}

int modem_ansync_timer_del(void *value)
{
	if(modem_ansync_lst)
	{
		return _os_ansync_unregister_api(modem_ansync_lst, OS_ANSYNC_TIMER, NULL,
				value, 0);
		//return os_ansync_del_api(modem_ansync_lst, value);
	}
	return ERROR;
}

static int modem_timer_thread(void *argv)
{
	modem_product_detection();

	return modem_process_callback_api(modem_process_handle, NULL);
}

int modem_module_init (void)
{
	modem_main_init();
	modem_ansync_lst = os_ansync_lst_create(ZPL_MODEM_MODULE, 4);
	if(modem_ansync_lst)
	{
		os_ansync_timeout_api(modem_ansync_lst, OS_ANSYNC_SEC(5));
		modem_ansync_timer_add(modem_timer_thread, OS_ANSYNC_SEC(5), "modem_timer_thread");
	}
	return 0;
}

int modem_module_exit (void)
{
	modem_main_exit();
	os_ansync_lst_destroy(modem_ansync_lst);
	return OK;
}

int modem_task_init (void)
{
	if(modem_task_id == 0)
		modem_task_id = os_task_create("modemTask", OS_TASK_DEFAULT_PRIORITY,
	               0, modem_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if(modem_task_id)
		return OK;
	return ERROR;
}


int modem_task_exit (void)
{
	if(modem_task_id == 0)
		return OK;
	os_task_destroy(modem_task_id);
	modem_task_id = 0;
	return OK;
}

struct module_list module_list_modem = 
{ 
	.module=MODULE_MODEM, 
	.name="MODEM", 
	.module_init=modem_module_init, 
	.module_exit=modem_module_exit, 
	.module_task_init=modem_task_init, 
	.module_task_exit=modem_task_exit, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};