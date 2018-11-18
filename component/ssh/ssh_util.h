/*
 * ssh_util.h
 *
 *  Created on: Nov 1, 2018
 *      Author: zhurish
 */

#ifndef COMPONENT_SSH_SSH_UTIL_H_
#define COMPONENT_SSH_SSH_UTIL_H_

#include "zebra.h"
#include "vty.h"

#include "libssh_config.h"

#include <libssh/callbacks.h>
#include <libssh/poll.h>
#include <libssh/server.h>

#include "ssh_api.h"
#include "sshd_main.h"

typedef struct ssh_scp_connect
{
  BOOL 			is_ssh;
  char 			*user;
  char 			*host;
  int			port;
  char 			*path;
  char 			*password;
  ssh_session 	session;
  ssh_scp 		scp;
  FILE 			*file;

}ssh_sftp_connect;



/*
 * util
 */
extern void ssh_log_callback_func(int priority,
        const char *function,
        const char *buffer,
        void *userdata);


extern int ssh_get_input(int fd, char *buf, int len);


extern int ssh_url_setup(struct ssh_scp_connect *dest,  os_url_t *spliurl);

//extern FILE * ssh_stdin_get(ssh_session session);
extern int ssh_stdin_get(ssh_session session);
extern int ssh_stdout_set(void *v);

extern int ssh_printf(ssh_session session, const char *fmt,...);

extern BOOL sshd_acl_action(ssh_config_t *ssh, ssh_session session);
/*
 * auth
 */
extern int ssh_authenticate_kbdint(int fd, ssh_session session, const char *password);
extern int ssh_verify_knownhost(int fd, ssh_session session);
extern int ssh_authenticate_api (ssh_session session, char *password);

/*
 * connect
 */
extern ssh_session ssh_connect_api(struct vty *vty, const char *host, int port, const char *user, char *password);
extern ssh_session ssh_client_connect_api(ssh_session session, struct vty *vty, const char *host, int port,
		const char *user, char *password);

/*
 * ssh scp client
 */
extern int ssh_scp_download(struct vty *vty, BOOL download, char *url, char *localfile);
extern int ssh_scp_upload(struct vty *vty, BOOL download, char *url, char *localfile);

/*
 *
 */

extern int ssh_scpd_init(sshd_client_t *sshclient, char *cmd);
extern int ssh_scpd_exit(sshd_client_t *sshclient);

/*
 * ssh client
 */
extern int ssh_client(struct vty *vty, char *host, int port, char *user, char *pasword);


/*
 * sftp client
 */

extern int sftp_action(struct vty *vty, BOOL download, char *url, char *localfile);


/*
 * key mgt
 */
//extern void sshd_set_default_keys(ssh_bind sshbind,
//                             int rsa_already_set,
//                             int dsa_already_set,
//                             int ecdsa_already_set);
extern int sshd_set_keys(ssh_config_t *ssh, ssh_bind sshbind);

extern ssh_keymgt_t * ssh_keymgt_lookup(ssh_config_t *ssh, char *keyname);
extern int ssh_keymgt_delete(ssh_config_t *ssh, char *keyname);
extern int ssh_keymgt_add(ssh_config_t *ssh, struct vty *vty, int type, char *keyname);
extern int ssh_keymgt_export_set(ssh_config_t *ssh, char *keyname, int type, char *filename, char *password);
extern int ssh_keymgt_import_set(ssh_config_t *ssh, char *keyname, int type, char *filename, char *password);


#endif /* COMPONENT_SSH_SSH_UTIL_H_ */
