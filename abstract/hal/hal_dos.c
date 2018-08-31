/*
 * hL_dos.c
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

#include "hal_dos.h"

sdk_dos_t sdk_dos;



int hal_dos_enable(BOOL enable, int type)
{
	if(sdk_dos.sdk_dos_enable_cb)
		return sdk_dos.sdk_dos_enable_cb(enable, type);
	return ERROR;
}

int hal_dos_tcp_hdr_size(int size)
{
	if(sdk_dos.sdk_dos_tcp_hdr_size_cb)
		return sdk_dos.sdk_dos_tcp_hdr_size_cb(size);
	return ERROR;
}

int hal_dos_icmp_size(BOOL ipv6, int size)
{
	if(sdk_dos.sdk_dos_icmp_size_cb)
		return sdk_dos.sdk_dos_icmp_size_cb(ipv6, size);
	return ERROR;
}



