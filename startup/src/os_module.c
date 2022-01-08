/*
 * os_module.c
 *
 *  Created on: Jun 10, 2017
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "bmgt.h"
#include "module.h"
#include "os_start.h"
#include "os_module.h"

#ifdef ZPL_APP_MODULE
#include "application.h"
#endif



int os_module_init(void)
{
	unit_board_init();

	pl_module_init(MODULE_NSM);
	pl_module_init(MODULE_HAL);
	pl_module_init(MODULE_PAL);
	
	pl_module_allinit();

	
	pl_module_init(MODULE_SDK);

	os_msleep(50);
	return OK;
}

int os_module_exit(void)
{
	pl_module_allexit();
	hal_bsp_exit();
	return OK;
}

int os_module_task_init(void)
{
	pl_module_task_init(MODULE_NSM);
	pl_module_task_init(MODULE_HAL);
	pl_module_task_init(MODULE_PAL);
	
	pl_module_task_allinit();

	pl_module_task_init(MODULE_SDK);
	
	os_msleep(50);
	//hal_bsp_task_init();
	return OK;
}

int os_module_task_exit(void)
{
#ifdef ZPL_TOOLS_PROCESS
	os_process_stop();
#endif

	pl_module_task_allexit();

	//hal_bsp_task_exit();

	os_time_exit();
	os_job_exit();

	os_task_exit();
	return OK;
}

int os_module_cmd_init(void)
{
#ifdef ZPL_NSM_MODULE
	nsm_module_cmd_init();
#endif

#ifdef ZPL_ZPLMEDIA_MODULE
	zpl_media_cmd_init();
#endif

#ifdef OS_START_TEST
	/*
	 * test module
	 */
	extern int os_test();
	os_test();
#ifdef ZPL_HAL_MODULE
	hal_test_init();
#endif
#endif
	os_msleep(50);
	return OK;
}

int os_module_cmd_exit(void)
{
	vrf_terminate();
	vty_terminate();
	cmd_terminate();

	if (zlog_default)
		closezlog(zlog_default);
	return OK;
}
