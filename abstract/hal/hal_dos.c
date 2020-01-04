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
#include "nsm_dos.h"

#include "hal_dos.h"
#include "hal_driver.h"


int hal_dos_enable(BOOL enable, dos_type_en type)
{
	if(hal_driver && hal_driver->dos_tbl && hal_driver->dos_tbl->sdk_dos_enable_cb)
		return hal_driver->dos_tbl->sdk_dos_enable_cb(hal_driver->driver, enable, type);
	return ERROR;
}

int hal_dos_tcp_hdr_size(int size)
{
	if(hal_driver && hal_driver->dos_tbl && hal_driver->dos_tbl->sdk_dos_tcp_hdr_size_cb)
		return hal_driver->dos_tbl->sdk_dos_tcp_hdr_size_cb(hal_driver->driver, size);
	return ERROR;
}

int hal_dos_icmp_size(BOOL ipv6, int size)
{
	if(hal_driver && hal_driver->dos_tbl && hal_driver->dos_tbl->sdk_dos_icmp_size_cb)
		return hal_driver->dos_tbl->sdk_dos_icmp_size_cb(hal_driver->driver, ipv6, size);
	return ERROR;
}



