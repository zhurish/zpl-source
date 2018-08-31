/*
 * nsm_wifi.h
 *
 *  Created on: Jul 15, 2018
 *      Author: zhurish
 */

#ifndef __NSM_WIFI_H__
#define __NSM_WIFI_H__

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
#include "vrf.h"
#include "nsm_mac.h"

#ifndef CONFIG_IW_TOOLS
#define WIFI_FILE_NAME	"wifi-local"
#define WIFI_FILE_NAME_MAX	64
#endif


typedef enum wifi_mode_s
{
	WIFI_IBSS = 1,
	WIFI_MANAGE,
	WIFI_AP,
	WIFI_AP_VLAN,
	WIFI_MONITOR,
	WIFI_MESH,
}wifi_mode_t;


typedef struct wifi_interface_s
{
	ifindex_t	ifindex;
#ifndef CONFIG_IW_TOOLS
	char	filename[WIFI_FILE_NAME_MAX];
#endif
	struct vty *vty;

}wifi_interface_t;




typedef struct wifi_cmd_s
{
	NODE		node;
	void		*cmd;
}wifi_cmd_t;


int iw_cmd_init(void);
int iw_cmd_exit(void);
int iw_cmd_add(void *data);


//#define for_each_cmd(_cmd)	\
		NODE index; \
		wifi_cmd_t *value; \
	for ( value = (wifi_cmd_t *)lstFirst(&iw_cmd); value != NULL, _cmd = value->cmd; \
		value = (wifi_cmd_t *)lstNext((NODE*)&index) )



#ifdef CONFIG_IW_TOOLS
extern int iw_printf(const char *format, ...);
extern int iw_fprintf(void *fp, const char *format, ...);
#endif


#endif /* __NSM_WIFI_H__ */
