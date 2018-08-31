/*
 * vty_user.h
 *
 *  Created on: Nov 27, 2016
 *      Author: zhurish
 */

#ifndef LIB_VTY_USER_H_
#define LIB_VTY_USER_H_


#include "md5.h"
//#define VTY_USER_DEBUG

#define VTY_USERNAME_DEFAULT	"admin"
#define VTY_PASSWORD_DEFAULT	"admin"

#define VTY_USERNAME_MAX	16
#define VTY_PASSWORD_MAX	20
#define MD5_PASSWORD_MAX	16

enum vty_privilege {
	VIEW_LEVEL = 1,
	ENABLE_LEVEL = 2,
	CONFIG_LEVEL = 3,
	ADMIN_LEVEL = 4
};

enum vty_authen_type {
	AUTHEN_LOCAL = 1,
	AUTHEN_RUDIUS = 2,
	AUTHEN_TACACS = 3,
};

struct vty_user
{
  char username[VTY_USERNAME_MAX + 1];

  enum vty_authen_type authen_type;

  BOOL			encrypt;
  unsigned char *password;
  unsigned char password_encrypt[VTY_PASSWORD_MAX];

  unsigned char *enable;
  unsigned char enable_encrypt[VTY_PASSWORD_MAX];

  enum vty_privilege privilege;
  //enum { VIEW_LEVEL = 1, ENABLE_LEVEL = 2, CONFIG_LEVEL = 3, ADMIN_LEVEL = 4 } privilege;
};



struct vty;

//extern struct vty_user * vty_user_lookup (const char *name);

extern char * vty_user_setting (struct vty *vty, const char *name);
extern int vty_user_getting_authen_type (struct vty *vty, char *name);
extern int vty_user_setting_authen_type (struct vty *vty, char *name, int authen_type);
extern int vty_user_getting_privilege (struct vty *vty, char *name);
extern int vty_user_setting_privilege (struct vty *vty, char *name, int privilege);
extern int vty_user_encrypt_enable (BOOL encrypt);

extern BOOL vty_user_enable_password (struct vty *vty, const char *name);

//authentication authorization accounting
extern int vty_user_authentication (struct vty *vty, char *password);
extern int vty_user_authorization (struct vty *vty, char *cmd);
extern int vty_user_accounting_start (struct vty *vty);
extern int vty_user_accounting_stop (struct vty *vty);

extern int vty_user_config_write (struct vty *vty);

extern int vty_user_create(struct vty *vty, char *name, char *password, BOOL enable, BOOL encrypt);
extern int vty_user_delete(struct vty *vty, char *name, BOOL password, BOOL enable);
extern int vty_user_change(struct vty *vty, char *name);


extern int vty_user_init(void);


#ifdef VTY_USER_DEBUG
#define VTY_USER_DEBUG_LOG	zlog_debug
#else
#define VTY_USER_DEBUG_LOG(msg,fmt...)
#endif


#endif /* LIB_VTY_USER_H_ */
