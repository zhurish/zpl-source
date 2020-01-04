/*
 * web_ppp_jst.c
 *
 *  Created on: 2019年8月3日
 *      Author: zhurish
 */




#include "zebra.h"
#include "module.h"
#include "memory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

int web_ppp_jst_init(void)
{
	//websDefineJst("jst_port_connect", jst_port_connect);
	return 0;
}
