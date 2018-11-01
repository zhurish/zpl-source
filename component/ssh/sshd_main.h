/*
 * sshd_main.h
 *
 *  Created on: Nov 1, 2018
 *      Author: zhurish
 */

#ifndef COMPONENT_SSH_SSHD_MAIN_H_
#define COMPONENT_SSH_SSHD_MAIN_H_

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


#include "zebra.h"


#define SSH_SHELL_PROXY_ENABLE



#ifndef KEYS_FOLDER
#ifdef _WIN32
#define KEYS_FOLDER
#else
#ifdef BUILD_X86
#define KEYS_FOLDER 		"/home/zhurish/.ssh/"
#else
#define KEYS_FOLDER 		"/root/.ssh/"
#endif
#endif
#endif

#define USER 				"zhurish"
#define PASS 				"centos"
#define BUF_SIZE 			2048
#define SESSION_END 		(SSH_CLOSED | SSH_CLOSED_ERROR)
#define SFTP_SERVER_PATH 	"/usr/lib/sftp-server"



typedef struct ssh_config_s
{
	BOOL			init;
	BOOL			quit;
	int				sshd_taskid;
    ssh_bind 		sshbind;
    ssh_event 		event;
}ssh_config_t;


/* A userdata struct for channel. */
struct channel_data_struct {
	struct vty *vty;
    struct winsize *winsize;
};


/* A userdata struct for session. */
struct session_data_struct {
    /* Pointer to the channel the session will allocate. */
    ssh_channel channel;
    int auth_attempts;
    int authenticated;
};


typedef struct ssh_config_client_s
{
    ssh_config_t		*config;
    ssh_session 		session;

    struct vty 			*vty;

#ifndef SSH_SHELL_PROXY_ENABLE
    int					timeval;
#else
    int 				sock;
#endif

    struct winsize wsize;

    /* Our struct holding information about the channel. */
    struct channel_data_struct cdata;

    /* Our struct holding information about the session. */
    struct session_data_struct sdata;

    struct ssh_channel_callbacks_struct channel_cb;

    struct ssh_server_callbacks_struct server_cb;

}ssh_config_client_t;



extern int sshd_enable(char *address, int port);
extern int sshd_disable();

extern int sshd_module_init();
extern int sshd_module_exit();


extern int sshd_module_task_init ();
extern int sshd_module_task_exit ();



#endif /* COMPONENT_SSH_SSHD_MAIN_H_ */
