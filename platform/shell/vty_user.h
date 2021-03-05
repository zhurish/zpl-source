/*
 * vty_user.h
 *
 *  Created on: Nov 27, 2016
 *      Author: zhurish
 */

#ifndef __LIB_VTY_USER_H__
#define __LIB_VTY_USER_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "zebra.h"
#include "vty.h"
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
  ospl_char username[VTY_USERNAME_MAX + 1];

  enum vty_authen_type authen_type;

  ospl_bool			encrypt;
  ospl_uchar *password;
  ospl_uchar password_encrypt[VTY_PASSWORD_MAX];

  ospl_uchar *enable;
  ospl_uchar enable_encrypt[VTY_PASSWORD_MAX];

  enum vty_privilege privilege;
  //enum { VIEW_LEVEL = 1, ENABLE_LEVEL = 2, CONFIG_LEVEL = 3, ADMIN_LEVEL = 4 } privilege;
};




//extern struct vty_user * vty_user_lookup (const char *name);
extern ospl_char * vty_user_setting (struct vty *, const char *);

extern enum vty_authen_type vty_user_getting_authen_type (struct vty *, ospl_char *);
extern int vty_user_setting_authen_type (struct vty *, ospl_char *, enum vty_authen_type );
extern enum vty_privilege vty_user_getting_privilege (struct vty *, ospl_char *);
extern int vty_user_setting_privilege (struct vty *, ospl_char *, enum vty_privilege );
extern int vty_user_encrypt_enable (ospl_bool );

extern ospl_bool vty_user_enable_password (struct vty *, const char *);

extern int user_authentication (ospl_char *, ospl_char *);
//authentication authorization accounting
extern int vty_user_authentication (struct vty *, ospl_char *);
extern int vty_user_authorization (struct vty *, ospl_char *);
extern int vty_user_accounting_start (struct vty *);
extern int vty_user_accounting_stop (struct vty *);

extern int vty_user_config_write (struct vty *);

extern int vty_user_create(struct vty *, ospl_char *, ospl_char *, ospl_bool , ospl_bool );
extern int vty_user_delete(struct vty *, ospl_char *, ospl_bool , ospl_bool );
extern int vty_user_change(struct vty *, ospl_char *);
extern int vty_user_foreach (int (*cb)(void *user, void *p), void *p);

extern ospl_char * vty_user_get(struct vty *);


extern ospl_bool md5_encrypt_empty(ospl_uchar *);
//extern int encrypt_XCH(ospl_uchar *pass, ospl_uchar *password);
extern int md5_encrypt_password(ospl_char *, ospl_uchar *);

extern int vty_user_init(void);


#ifdef VTY_USER_DEBUG
#define VTY_USER_DEBUG_LOG	zlog_debug
#else
#define VTY_USER_DEBUG_LOG(msg,fmt...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LIB_VTY_USER_H__ */
