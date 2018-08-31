/*
 * ip_util.c
 *
 *  Created on: Jul 14, 2018
 *      Author: zhurish
 */


#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "zclient.h"
#include "thread.h"

#include "sigevent.h"

#include "ip_interface.h"


/*int super_system(const char *cmd)
{
	int ret = 0;
	errno = 0;
	//if ( vpn_privs.change (ZPRIVS_RAISE) )
	//	zlog_err ("%s: could not raise privs, %s",__func__,safe_strerror (errno) );

	ret = system(cmd);
	if(ret == -1 || ret == 127)
	{
		zlog_err (ZLOG_PAL, "%s: execute cmd: %s(%s)",__func__,cmd,safe_strerror (errno) );
		return CMD_WARNING;
	}
	//if ( vpn_privs.change (ZPRIVS_LOWER) )
	//	zlog_err ("%s: could not lower privs, %s",__func__,safe_strerror (errno) );
	return ret;
}*/
