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

#include "zpl_include.h"
#include "vty.h"
#include "md5.h"
#include "command.h"
//#define VTY_USER_DEBUG

#define VTY_USERNAME_DEFAULT	"admin"
#define VTY_PASSWORD_DEFAULT	"admin"

#define VTY_USERNAME_MAX	32
#define VTY_PASSWORD_MAX	32
#define MD5_PASSWORD_MAX	16



enum vty_authen_type {
	AUTHEN_LOCAL = 1,
	AUTHEN_RUDIUS = 2,
	AUTHEN_TACACS = 3,
};

struct vty_user
{
  zpl_char username[VTY_USERNAME_MAX + 1];

  enum vty_authen_type authen_type;

  zpl_bool			encrypt;
  zpl_uchar *password;
  zpl_uchar password_encrypt[VTY_PASSWORD_MAX];

  zpl_uchar *enable;
  zpl_uchar enable_encrypt[VTY_PASSWORD_MAX];

  enum cmd_privilege privilege;
  //enum { VIEW_LEVEL = 1, ENABLE_LEVEL = 2, CONFIG_LEVEL = 3, ADMIN_LEVEL = 4 } privilege;
};




//extern struct vty_user * vty_user_lookup (const char *name);
extern zpl_char * vty_user_setting (struct vty *, const char *);

extern enum vty_authen_type vty_user_getting_authen_type (struct vty *, zpl_char *);
extern int vty_user_setting_authen_type (struct vty *, zpl_char *, enum vty_authen_type );
extern enum cmd_privilege vty_user_getting_privilege (struct vty *, zpl_char *);
extern int vty_user_setting_privilege (struct vty *, zpl_char *, enum cmd_privilege );
extern int vty_user_encrypt_enable (zpl_bool );

extern zpl_bool vty_user_enable_password (struct vty *, const char *);

extern int user_authentication (zpl_char *, zpl_char *);
//authentication authorization accounting
extern int vty_user_authentication (struct vty *, zpl_char *);
extern int vty_user_authorization (struct vty *, zpl_char *);
extern int vty_user_accounting_start (struct vty *);
extern int vty_user_accounting_stop (struct vty *);

extern int config_write_vty_user (struct vty *);

extern int vty_user_create(struct vty *, zpl_char *, zpl_char *, zpl_bool , zpl_bool );
extern int vty_user_delete(struct vty *, zpl_char *, zpl_bool , zpl_bool );
extern int vty_user_change(struct vty *, zpl_char *);
extern int vty_user_foreach (int (*cb)(void *user, void *p), void *p);

extern zpl_char * vty_user_get(struct vty *);


extern zpl_bool md5_encrypt_empty(zpl_uchar *);
//extern int encrypt_XCH(zpl_uchar *pass, zpl_uchar *password);
extern int md5_encrypt_password(zpl_char *, zpl_uchar *);

extern int vty_user_init(void);
extern int cmd_vty_user_init(void);

#ifdef VTY_USER_DEBUG
#define VTY_USER_DEBUG_LOG	zlog_debug
#else
#define VTY_USER_DEBUG_LOG(msg,fmt...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LIB_VTY_USER_H__ */
