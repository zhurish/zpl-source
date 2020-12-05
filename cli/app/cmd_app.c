/*
 * cmd_voip.c
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
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
#include "nsm_vrf.h"
#include "nsm_interface.h"
#include "template.h"

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
