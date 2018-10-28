/*
 * daemon.h
 *
 *  Created on: Oct 24, 2018
 *      Author: zhurish
 */

#ifndef __LIB_DAEMON_H__
#define __LIB_DAEMON_H__


pid_t pid_output (const char *);
pid_t pid_input (const char *);

#ifndef HAVE_DAEMON
int daemon(int, int);
#endif



#endif /* __LIB_DAEMON_H__ */
