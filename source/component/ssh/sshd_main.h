/*
 * sshd_main.h
 *
 *  Created on: Nov 1, 2018
 *      Author: zhurish
 */

#ifndef COMPONENT_SSH_SSHD_MAIN_H_
#define COMPONENT_SSH_SSHD_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "libssh_autoconfig.h"
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

#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
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



extern int sshd_accept(socket_t fd, zpl_uint32 revents, void *userdata);
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

 
#ifdef __cplusplus
}
#endif
 
#endif /* COMPONENT_SSH_SSHD_MAIN_H_ */
