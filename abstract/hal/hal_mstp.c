/*
 * hal_mstp.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include <log.h>
#include "os_list.h"

#include "nsm_client.h"

#include "hal_mstp.h"

sdk_mstp_t sdk_mstp;


int hal_mstp_enable(BOOL enable)
{
	if(sdk_mstp.sdk_mstp_enable_cb)
		return sdk_mstp.sdk_mstp_enable_cb(enable);
	return ERROR;
}

int hal_mstp_age(int age)
{
	if(sdk_mstp.sdk_mstp_age_cb)
		return sdk_mstp.sdk_mstp_age_cb(age);
	return ERROR;
}

int hal_mstp_bypass(BOOL enable, int type)
{
	if(sdk_mstp.sdk_mstp_bypass_cb)
		return sdk_mstp.sdk_mstp_bypass_cb(enable, type);
	return ERROR;
}

int hal_mstp_state(ifindex_t ifindex, int mstp, int state)
{
	if(sdk_mstp.sdk_mstp_state_cb)
		return sdk_mstp.sdk_mstp_state_cb(ifindex, mstp, state);
	return ERROR;
}

int hal_stp_state(ifindex_t ifindex, int state)
{
	if(sdk_mstp.sdk_stp_state_cb)
		return sdk_mstp.sdk_stp_state_cb(ifindex, state);
	return ERROR;
}
