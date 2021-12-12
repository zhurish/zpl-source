/*
 * cmd_voip.c
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */




#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

#include "application.h"


void cmd_app_init(void)
{
#ifdef APP_X5BA_MODULE
	cmd_app_x5b_init();
#endif

#ifdef APP_V9_MODULE
	cmd_app_v9_init();
#endif
}
