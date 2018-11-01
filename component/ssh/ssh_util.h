/*
 * ssh_util.h
 *
 *  Created on: Nov 1, 2018
 *      Author: zhurish
 */

#ifndef COMPONENT_SSH_SSH_UTIL_H_
#define COMPONENT_SSH_SSH_UTIL_H_

#include "libssh_config.h"

#include <libssh/callbacks.h>
#include <libssh/poll.h>
#include <libssh/server.h>

#ifdef HAVE_ARGP_H
#include <argp.h>
#endif
#include <fcntl.h>
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#include <signal.h>
#include <stdlib.h>
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <stdio.h>



extern void ssh_log_callback_func(int priority,
        const char *function,
        const char *buffer,
        void *userdata);

#ifdef SSH_STD_REDIST
extern FILE * _ssh_stdout();
extern FILE * _ssh_stderr();
extern FILE * _ssh_stdin();
#endif

extern FILE * ssh_stdin_get(ssh_session session);

extern int ssh_printf(ssh_session session, const char *fmt,...);


extern int verify_knownhost(ssh_session session);
extern int ssh_authenticate_api (ssh_session session, char *password);
extern ssh_session ssh_connect_api(struct vty *vty, const char *host, int port, const char *user, char *password);
extern int do_ssh_copy(struct vty *vty, BOOL download, char *url, char *localfile);



#endif /* COMPONENT_SSH_SSH_UTIL_H_ */
