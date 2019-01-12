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

#ifdef BUILD_X86
#define KEYS_FOLDER 		"/home/zhurish/.ssh/"
#define USER 				"zhurish"
#define PASS 				"centos"
#else
#define KEYS_FOLDER 		"/etc/ssh/"
#define USER 				"root"
#define PASS 				"123456"
#endif

#define BUF_SIZE 			2048
#define SESSION_END 		(SSH_CLOSED | SSH_CLOSED_ERROR)
#define SFTP_SERVER_PATH 	"/usr/lib/sftp-server"

//#define SSH_SCPD_ENABLE


#include "ssh_api.h"


/*typedef struct sshd_s
{
	BOOL			init;
	BOOL			quit;
	BOOL			running;
	int				sshd_taskid;
    ssh_bind 		sshbind;
    ssh_event 		event;
}sshd_t;*/


struct scpd_data
{
	int 	mode;
	char	 *filename;
    socket_t input;
    socket_t output;
    ssh_scp	 scp;
};

typedef struct sshd_client_s
{
	ssh_config_t		*config;
    ssh_session 		session;

    struct ssh_channel_callbacks_struct ssh_channel_cb;

    struct ssh_server_callbacks_struct 	ssh_server_cb;

    struct winsize 		winsize;

    ssh_channel 		channel;

    int 				auth_attempts;
    int 				authenticated;

    enum
	{
    	SSH_C_NONE,
		SSH_C_SHELL,
		SSH_C_SCP,
		SSH_C_SFTP
	} type;

    struct vty 			*vty;
    int 				sock;
#ifdef SSH_SCPD_ENABLE
    struct scpd_data	scp_data;
#endif
}sshd_client_t;

//extern sshd_t sshd_config;


extern int sshd_accept(socket_t fd, int revents, void *userdata);
extern int sshd_task(void *argv);
/*
extern int sshd_enable(char *address, int port);
extern int sshd_disable();

extern int sshd_module_init();
extern int sshd_module_exit();


extern int sshd_module_task_init ();
extern int sshd_module_task_exit ();
*/
/*
 * ssh server close will call
 */
//extern void sshd_session_userdata_close(ssh_session session);


#endif /* COMPONENT_SSH_SSHD_MAIN_H_ */
