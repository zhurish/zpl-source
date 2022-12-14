/*
 * application.c
 *
 *  Created on: 2019年11月14日
 *      Author: DELL
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "nsm_include.h"


#include "application.h"

#ifdef APP_X5BA_MODULE

int app_module_init(void)
{
	return 0;//x5b_app_module_init(NULL, 0);
}

int app_module_exit(void)
{
	return x5b_app_module_exit();
}

int app_module_task_init(void)
{
	return x5b_app_module_task_init();
}
int app_module_task_exit(void)
{
	return x5b_app_module_task_exit();
}

#endif
