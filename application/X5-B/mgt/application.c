/*
 * application.c
 *
 *  Created on: 2019年11月14日
 *      Author: DELL
 */


#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"

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
