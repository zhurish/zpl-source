/*
 * service.h
 *
 *  Created on: Oct 28, 2018
 *      Author: zhurish
 */

#ifndef __SYSTOOLS_H__
#define __SYSTOOLS_H__

#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "thread.h"
#include "eloop.h"
#include "log.h"
#include "network.h"
#include "prefix.h"
#include "vty.h"
#include "vty_user.h"
#include "host.h"

#ifndef BUFSIZE
#define BUFSIZE 512
#endif

#define MAX_FILENAME_LENGTH 128
#define MAX_LOGIN_NAME_LEN 	128
#define MAX_DIR_NAME_LEN 	128



extern const char *ftpd_hostname(void);

extern int service_task_init (void);
extern int service_task_exit (void);
extern int service_module_init (void);
extern int service_module_exit (void);
extern int service_clicmd_init (void);


#endif /* __SYSTOOLS_H__ */
