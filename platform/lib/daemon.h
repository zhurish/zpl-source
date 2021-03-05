/*
 * daemon.h
 *
 *  Created on: Oct 24, 2018
 *      Author: zhurish
 */

#ifndef __LIB_DAEMON_H__
#define __LIB_DAEMON_H__

#ifdef __cplusplus
extern "C" {
#endif


ospl_pid_t pid_output (const char *);
ospl_pid_t pid_input (const char *);

#ifndef HAVE_DAEMON
int daemon(ospl_bool, ospl_bool);
#endif


 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_DAEMON_H__ */
