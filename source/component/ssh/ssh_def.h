/**
 * @file      : ssh_def.h
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-21
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */

#ifndef COMPONENT_SSH_SSH_DEF_H_
#define COMPONENT_SSH_SSH_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "libssh_autoconfig.h"
#include <libssh/callbacks.h>
#include "libssh/priv.h"
#include "libssh/libssh.h"
#include "libssh/poll.h"
#include "libssh/scp.h"
#include "libssh/socket.h"
#include "libssh/session.h"
#include "libssh/misc.h"
#include "libssh/pki.h"
#ifdef WITH_SERVER
#include "libssh/server.h"
#endif
#include "os_ipstack.h"
#include "prefix.h"
#include "vty.h"

#define SSH_PORT_DEFAULT		4022
#define SSH_KEY_FILENAME_MAX	64
#define SSH_KEY_NAME_MAX		32
#define SSH_KEY_MAX				8

#define SSH_DEBUG_EVENT		0x0800
#define SSH_DEBUG_PACKET	0x0400
#define SSH_DEBUG_PROTO		0x0200
#define SSH_DEBUG_ERROR		0x0300

typedef struct ssh_keymgt_s
{
	char		keyname[SSH_KEY_NAME_MAX];
	char		used;
	enum ssh_keytypes_e type;
	ssh_key 	key;
}ssh_keymgt_t;

enum
{
	SSH_NONE_CMD,
	SSH_STOP_CMD,
	SSH_START_CMD,
};

typedef struct ssh_config_s
{
	zpl_bool			enable;
	zpl_bool			shell_enable;
	zpl_uint16				bindport;
	struct prefix	bind_prefix;

	char			hostkey[SSH_KEY_FILENAME_MAX];
	char			rsakey[SSH_KEY_FILENAME_MAX];
	char			dsakey[SSH_KEY_FILENAME_MAX];
	char			ecdsakey[SSH_KEY_FILENAME_MAX];

	zpl_uint8	auth_retries;
	zpl_uint8	auth_waitting;
	//zpl_uint8	ssh_connect_timeout;

	enum
	{
		SSH_AUTH_NONE,
		SSH_AUTH_AUTO,
		SSH_AUTH_PASSWORD,
		SSH_AUTH_PUBLIC_KEY,
		SSH_AUTH_RSA,
		SSH_AUTH_HOSTBASE,	//host
		SSH_AUTH_KB,		//keyboard
		SSH_AUTH_GSSAPI,
	}auth_type;

	zpl_uint32				ssh_debug;
	char			ssh_version;

	void			*ssh_acl;

	int				sock;
	int				ctlfd;

	zpl_bool			initialized;
	zpl_bool			quit;
	zpl_bool			running;
	zpl_taskid_t				sshd_taskid;
  ssh_bind 		sshbind;
  ssh_event 		event;

  ssh_keymgt_t	ssh_key[SSH_KEY_MAX];
  ssh_keymgt_t	*key;
}ssh_config_t;

typedef struct ssh_scp_connect
{
  zpl_bool 			is_ssh;
  char 			*user;
  char 			*host;
  zpl_uint16			port;
  char 			*path;
  char 			*password;
  ssh_session 	session;
  ssh_scp 		scp;
  FILE 			*file;

}ssh_sftp_connect;

struct scpd_data
{
	zpl_uint32 	mode;
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

    zpl_uint32 				auth_attempts;
    zpl_uint32 				authenticated;

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


#ifdef __cplusplus
}
#endif
 
#endif /* COMPONENT_SSH_SSH_DEF_H_ */
