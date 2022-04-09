/*
 * ssh_api.h
 *
 *  Created on: Oct 30, 2018
 *      Author: zhurish
 */

#ifndef COMPONENT_SSH_SSH_API_H_
#define COMPONENT_SSH_SSH_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "zplos_include.h"
#include "prefix.h"

#include "libssh_autoconfig.h"

#include "libssh/priv.h"
#include "libssh/libssh.h"
#include "libssh/poll.h"
#include "libssh/socket.h"
#include "libssh/session.h"
#include "libssh/misc.h"
#ifdef WITH_SERVER
#include "libssh/server.h"
#endif
#include "libssh/libssh.h"


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
	zpl_uint32				sshd_taskid;
    ssh_bind 		sshbind;
    ssh_event 		event;

    ssh_keymgt_t	ssh_key[SSH_KEY_MAX];
    ssh_keymgt_t	*key;
}ssh_config_t;

extern zpl_bool ssh_is_enable_api(void);
extern zpl_bool ssh_is_running_api(void);

extern int ssh_enable_api(zpl_bool enable);
extern int ssh_login_api(zpl_bool enable);
extern int ssh_bind_address_api(struct prefix *address);
extern int ssh_bind_port_api(zpl_uint16 port);
extern int ssh_version_api(zpl_uint32 version);
extern int ssh_debug_api(zpl_uint32 type, zpl_uint32 debug);

extern int ssh_authentication_type_api(zpl_uint32 value);
extern int ssh_authentication_waitting_api(zpl_uint32 value);
extern int ssh_authentication_retries_api(zpl_uint32 value);

extern int ssh_keyfile_api(zpl_uint32 type, char * value);

extern int ssh_generate_key_api(struct vty *vty, zpl_uint32 type, char * keyname);
extern int ssh_key_delete_api(char * keyname);
extern int ssh_keymgt_export_api(char *keyname, zpl_uint32 type, char *filename, char *password);
extern int ssh_keymgt_import_api(char *keyname, zpl_uint32 type, char *filename, char *password);

extern int show_ssh_keymgt(struct vty *vty);
extern int ssh_write_config(struct vty *vty);


extern int ssh_module_init();
extern int ssh_module_exit();

extern int ssh_module_task_init ();
extern int ssh_module_task_exit ();

 
#ifdef __cplusplus
}
#endif
 
#endif /* COMPONENT_SSH_SSH_API_H_ */
