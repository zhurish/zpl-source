/*
 * voip_osip.c
 *
 *  Created on: Jan 22, 2019
 *      Author: zhurish
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>

#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>


struct eXosip_t *context_eXosip;

int voip_osip_module_init()
{
	context_eXosip = eXosip_malloc ();
	if (eXosip_init (context_eXosip)) {
/*		syslog_wrapper (LOG_ERR, "eXosip_init failed");
		exit (1);*/
	}
	return 0;
}
