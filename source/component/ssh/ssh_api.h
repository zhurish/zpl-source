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

extern int ssh_cmd_init(void);
extern int ssh_module_init(void);
extern int ssh_module_exit(void);

extern int ssh_module_task_init(void);
extern int ssh_module_task_exit(void);

#ifdef __cplusplus
}
#endif
 
#endif /* COMPONENT_SSH_SSH_API_H_ */
