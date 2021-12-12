/*
 * vty_user.c
 *
 *  Created on: Nov 27, 2016
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "vty_include.h"



zpl_char * vty_user_setting (struct vty *vty, const char *name)
{
	if(name == NULL)
		return NULL;
	if(os_strlen(name) > VTY_USERNAME_MAX)
	{
		if(vty)
			vty_out(vty, "ERROR: username is too long.%s", VTY_NEWLINE);
		return NULL;
	}
	if(vty->username)
		free(vty->username);
	vty->username = strdup(name);
	VTY_USER_DEBUG_LOG("%s:username:%s\n",__func__,vty->username);
	return vty->username;
}

static struct vty_user *user_new (const char *name)
{
	struct vty_user *user;
	if(name == NULL)
		return NULL;
	user = XCALLOC (MTYPE_HOST, sizeof (struct vty_user));
	if(user == NULL)
		return NULL;
	os_memset(user, 0, sizeof(struct vty_user));
	os_strncpy(user->username, name, os_strlen(name));
	user->authen_type = AUTHEN_LOCAL;
	//listnode_add (_global_host.userlist, user);
	return user;
}

static void user_free (struct vty_user *user)
{
	if (user)
	{
		if(user->enable)
			XFREE (MTYPE_HOST, user->enable);
		if(user->password)
			XFREE (MTYPE_HOST, user->password);
		XFREE (MTYPE_HOST, user);
	}
}

static struct vty_user *vty_user_add (struct vty_user *user)
{
	if(user == NULL || _global_host.userlist == NULL)
		return NULL;
	listnode_add_sort (_global_host.userlist, user);
	return user;
}


static struct vty_user * vty_user_lookup (const char *name)
{
	zpl_char lname[VTY_USERNAME_MAX];
	struct listnode *node = NULL;
	struct vty_user *user = NULL;
	if(name == NULL || _global_host.userlist == NULL)
		return NULL;

	os_memset(lname, 0, VTY_USERNAME_MAX);
	os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));

	//VTY_USER_DEBUG_LOG("%s:username:%s\n",__func__,name);
	for (ALL_LIST_ELEMENTS_RO (_global_host.userlist, node, user))
	{
		if(user)
		{
			if (os_memcmp (user->username, lname, VTY_USERNAME_MAX) == 0)
			//if(user->name_hash == string_hash_make(name))
				return user;
		}
	}
	return NULL;
}

zpl_bool md5_encrypt_empty(zpl_uchar *ecrypt)
{
    if(os_memcmp(ecrypt,"*#@",3) == 0)
    {
        return zpl_false;
    }
    return zpl_true;
}

static int encrypt_XCH(zpl_uchar *pass, zpl_uchar *password)
{
	zpl_uint32 i = 0;
	//zpl_char password[VTY_PASSWORD_MAX];
	//os_memset(password, '0', MD5_PASSWORD_MAX);
	//可打印字符（0x20-0x7E）
	for(i = 0; i < MD5_PASSWORD_MAX; i++)
	{
		if(pass[i] < 0x21)
			password[i] = pass[i] + 0x21 + i;
		if(pass[i] > 0x7E)
			password[i] = pass[i] - 0x7E + i;

		if(pass[i] == 0x20)
			password[i] = pass[i] + 0x20 + i;

		if(pass[i] == '%')
			password[i] = '*';
		else if(pass[i] == '?')
			password[i] = '@';
		else if(pass[i] == '^')
			password[i] = '$';
		else if(pass[i] == 0x60 || pass[i] == 0x2c ||
				pass[i] == 0x22 || pass[i] == 0x27 ||
				pass[i] == 0x25 || pass[i] == 0x2e)
			password[i] = 0x30 + i;
		else if ((pass[i] >= 0x21) && (pass[i] <= 0x7E))
			password[i] = pass[i];
		else
			password[i] = 0x21 + i;

		if(password[i] < 0x21)
			password[i] = password[i] + 0x21;
		if(password[i] > 0x7E)
			password[i] = password[i] - 0x7E;
		if(password[i] == ' ')
			password[i] = password[i] + i;
	}
	for(i = 0; i < MD5_PASSWORD_MAX; i++)
	{
		if(password[i] < 0x21)
			password[i] = password[i] + 0x21 + i;
		if(password[i] > 0x7E)
			password[i] = password[i] - 0x7E + i;
		if(password[i] == ' ')
			password[i] = password[i] + i;
	}
	return OK;
}

int md5_encrypt_password(zpl_char *password, zpl_uchar *ecrypt)
{
    //zpl_uint32 i;
    zpl_uchar decrypt[VTY_PASSWORD_MAX];
    zpl_uchar md5ecrypt[VTY_PASSWORD_MAX];
    if(!md5_encrypt_empty(password))
    {
        //os_strcpy(ecrypt, password);
		os_memcpy(ecrypt, password, MIN(os_strlen(password), VTY_PASSWORD_MAX));
        return OK;
    }
    os_memset(decrypt, 0, sizeof(decrypt));
    os_memset(md5ecrypt, 0, sizeof(md5ecrypt));
    OS_MD5_CTX md5;
    OS_MD5Init(&md5);
    OS_MD5Update(&md5, password, os_strlen(password));
    OS_MD5Final(decrypt, &md5);

    encrypt_XCH(decrypt, md5ecrypt);
    os_memcpy(ecrypt, "*#@", 3);
    os_memcpy(ecrypt + 3, md5ecrypt, MD5_PASSWORD_MAX);
    return OK;
}



int vty_user_encrypt_enable (zpl_bool encrypt)
{
	struct listnode *node = NULL;
	struct vty_user *user = NULL;
	if(_global_host.userlist == NULL)
		return 0;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	for (ALL_LIST_ELEMENTS_RO (_global_host.userlist, node, user))
	{
		if(user)
		{
			if(/*user->encrypt && */encrypt == zpl_false)
			{
				if(!md5_encrypt_empty(user->password_encrypt))
					os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));
				if(!md5_encrypt_empty(user->enable_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				user->encrypt = zpl_false;
			}
			else if(/*user->encrypt == 0 && */encrypt == zpl_true)
			{
				if(!md5_encrypt_empty(user->password_encrypt))
					os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));
				if(!md5_encrypt_empty(user->enable_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				if(user->password)
					md5_encrypt_password(user->password, user->password_encrypt);
				if(user->enable)
					md5_encrypt_password(user->enable, user->enable_encrypt);
				user->encrypt = zpl_true;
			}
		}
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return 0;
}

zpl_bool vty_user_enable_password (struct vty *vty, const char *name)
{
	zpl_bool ret = zpl_false;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		zpl_char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			if(user->enable && !md5_encrypt_empty(user->enable_encrypt))
				ret = zpl_true;
			if (_global_host.mutx)
				os_mutex_unlock(_global_host.mutx);
			return ret;
		}
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return zpl_false;
}


enum cmd_privilege vty_user_getting_privilege (struct vty *vty, zpl_char *name)
{
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		zpl_char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			//vty_out(vty, "%s 0:%s %s",__func__,lname, VTY_NEWLINE);
			if (_global_host.mutx)
				os_mutex_unlock(_global_host.mutx);
			return user->privilege;
		}
	}
/*
	if(vty->user)
	{
		if (_global_host.mutx)
			os_mutex_unlock(_global_host.mutx);
		return vty->user->privilege;
	}
*/

	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	//vty_out(vty, "%s:vty->user is NULL %s",__func__, VTY_NEWLINE);
	return CMD_ENABLE_LEVEL;
}

int vty_user_setting_privilege (struct vty *vty, zpl_char *name, enum cmd_privilege privilege)
{
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		zpl_char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			if(privilege >= CMD_VIEW_LEVEL && privilege <= CMD_ADMIN_LEVEL)
			{
				user->privilege = privilege;
				if (_global_host.mutx)
					os_mutex_unlock(_global_host.mutx);
				return CMD_SUCCESS;
			}
		}
	}
/*	if(vty->user)
	{
		if(privilege >= VIEW_LEVEL && privilege <= ADMIN_LEVEL)
		{
			vty->user->privilege = privilege;
			if (_global_host.mutx)
				os_mutex_unlock(_global_host.mutx);
			return CMD_SUCCESS;
		}
	}*/
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	if(vty)
		vty_out(vty,"%s:Can't find user '%s'%s",__func__,name,VTY_NEWLINE);
	return CMD_WARNING;
}

enum vty_authen_type vty_user_getting_authen_type (struct vty *vty, zpl_char *name)
{
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		zpl_char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			if(vty)
				vty_out(vty, "%s 0:%s %s",__func__,name, VTY_NEWLINE);
			if (_global_host.mutx)
				os_mutex_unlock(_global_host.mutx);
			return user->authen_type;
		}
	}
/*	if(vty->user)
	{
		if (_global_host.mutx)
			os_mutex_unlock(_global_host.mutx);
		return vty->user->authen_type;
	}*/
	if(vty)
		vty_out(vty, "%s:vty->user is NULL %s",__func__, VTY_NEWLINE);
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return CMD_ENABLE_LEVEL;
}

int vty_user_setting_authen_type (struct vty *vty, zpl_char *name, enum vty_authen_type authen_type)
{
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		zpl_char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			if(authen_type >= AUTHEN_LOCAL && authen_type <= AUTHEN_TACACS)
			{
				user->authen_type = authen_type;
				if (_global_host.mutx)
					os_mutex_unlock(_global_host.mutx);
				return CMD_SUCCESS;
			}
		}
	}
/*	if(vty->user)
	{
		if(authen_type >= AUTHEN_LOCAL && authen_type <= AUTHEN_TACACS)
		{
			vty->user->authen_type = authen_type;
			if (_global_host.mutx)
				os_mutex_unlock(_global_host.mutx);
			return CMD_SUCCESS;
		}
	}*/
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	if(vty)
		vty_out(vty,"%s:Can't find user '%s'%s",__func__,name,VTY_NEWLINE);
	return CMD_WARNING;
}

int vty_user_authentication (struct vty *vty, zpl_char *password)
{
	zpl_uchar *passwd = NULL;
	//enum node_type next_node = 0;
	int fail = 1;
	zpl_uchar encrypt[VTY_PASSWORD_MAX];
	struct vty_user * user = NULL;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	user = vty_user_lookup (vty->username);
	if(user)
	{
		VTY_USER_DEBUG_LOG("%s:username:%s==%s\n",__func__,user->username,vty->username);
		switch (vty->node)
		{
			case AUTH_NODE:
				if(user->authen_type == AUTHEN_LOCAL)
				{
					//vty_out (vty,"%s loging authenticated%s",user->username,VTY_NEWLINE);
					if (user->encrypt)
						passwd = user->password_encrypt;
					else
						passwd = user->password;
					//next_node = VIEW_NODE;
				}
				break;
			case AUTH_ENABLE_NODE:
				if(user->authen_type == AUTHEN_LOCAL)
				{
					//vty_out (vty,"%s enable %s authenticated%s",user->username,user->encrypt? "encrypt":" ",VTY_NEWLINE);
					if (user->encrypt)
						passwd = user->enable_encrypt;
					else
						passwd = user->enable;
					//next_node = ENABLE_NODE;
					//vty_out (vty,"%s:%s=%s%s",user->username,user->enable,user->enable_encrypt,VTY_NEWLINE);
				}
				break;
			default:
				if (_global_host.mutx)
					os_mutex_unlock(_global_host.mutx);
				return CMD_WARNING;
				break;
		}
		if(user->authen_type == AUTHEN_LOCAL)
		{
			if (passwd)
			{
				os_memset(encrypt, 0, sizeof(encrypt));
				md5_encrypt_password(password, encrypt);
				if (user->encrypt)
					fail = memcmp (encrypt, passwd, VTY_PASSWORD_MAX);
					//fail = strcmp (crypt(password, passwd), passwd);
				else
					fail = strcmp (password, passwd);
			}
			else
			{
				if (_global_host.mutx)
					os_mutex_unlock(_global_host.mutx);
				return CMD_WARNING;
			}
			if(fail == 0)
			{
				//vty_user_getting_privilege (struct vty *vty, zpl_char *name)
				//vty->user = user;//vty_user_lookup (vty->username);
				vty->privilege = user->privilege;
				if (_global_host.mutx)
					os_mutex_unlock(_global_host.mutx);
				//vty_out(vty, "%s:vty->user %s %s",__func__, user->username,VTY_NEWLINE);
				return CMD_SUCCESS;
			}
			else
			{
				if (_global_host.mutx)
					os_mutex_unlock(_global_host.mutx);
				return CMD_WARNING;
			}
		}
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return CMD_WARNING;
}

int user_authentication (zpl_char *username, zpl_char *password)
{
	zpl_uchar *passwd = NULL;
	int fail = ERROR;
	zpl_uchar encrypt[VTY_PASSWORD_MAX];
	struct vty_user * user = NULL;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	user = vty_user_lookup (username);
	if(user)
	{
		if (user->encrypt)
			passwd = user->password_encrypt;
		else
			passwd = user->password;
		if (passwd)
		{
			if (user->encrypt)
			{
				os_memset(encrypt, 0, sizeof(encrypt));
				md5_encrypt_password(password, encrypt);
				fail = memcmp (encrypt, passwd, VTY_PASSWORD_MAX);
			}
			else
				fail = strcmp (password, passwd);
		}
		else
		{
			if (_global_host.mutx)
				os_mutex_unlock(_global_host.mutx);
			return ERROR;
		}
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return fail;
}

int vty_user_authorization (struct vty *vty, zpl_char *cmd)
{
	return CMD_SUCCESS;
}

int vty_user_accounting_start (struct vty *vty)
{
	return CMD_SUCCESS;
}

int vty_user_accounting_stop (struct vty *vty)
{
	return CMD_SUCCESS;
}




int vty_user_config_write (struct vty *vty)
{
	struct listnode *node;
	struct vty_user *user;
	if(vty == NULL || _global_host.userlist == NULL)
		return 0;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if(vty->type != VTY_FILE && vty->username)
	{
		vty_out(vty,"! current login user:%s %s", vty->username, VTY_NEWLINE);
		vty_out(vty,"!%s", VTY_NEWLINE);
	}
	for (ALL_LIST_ELEMENTS_RO (_global_host.userlist, node, user))
	{
		if(user)
		{
			if((user->password == NULL) && md5_encrypt_empty(user->password_encrypt))
				vty_out(vty,"username %s%s", user->username, VTY_NEWLINE);
			else
			{
				if(user->encrypt && !md5_encrypt_empty(user->password_encrypt))
					vty_out(vty,"username %s password %s%s", user->username,
							(user->password_encrypt), VTY_NEWLINE);
				else if(user->password)
					vty_out(vty,"username %s password %s%s", user->username,
								user->password, VTY_NEWLINE);
			}
			if(user->encrypt && !md5_encrypt_empty(user->enable_encrypt))
				vty_out(vty,"username %s enable password %s%s", user->username,
						(user->enable_encrypt), VTY_NEWLINE);
			else if(user->enable)
				vty_out(vty,"username %s enable password %s%s", user->username,
							user->enable, VTY_NEWLINE);
			if(user->privilege)
				vty_out(vty,"username %s privilege %d%s", user->username, user->privilege, VTY_NEWLINE);

		}
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return CMD_SUCCESS;
}

int vty_user_create(struct vty *vty, zpl_char *name, zpl_char *password, zpl_bool enable, zpl_bool encrypt)
{
	zpl_char lname[VTY_USERNAME_MAX];
	struct vty_user * user = NULL;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);

	os_memset(lname, 0, VTY_USERNAME_MAX);
	os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
	user = vty_user_lookup (lname);
	if(password == NULL)
	{
		if(user)
		{
			if (_global_host.mutx)
				os_mutex_unlock(_global_host.mutx);
			if(vty)
				vty_out(vty,"user '%s' is already exist%s",password,VTY_NEWLINE);
			return CMD_WARNING;
		}
		/*
		 * create user
		 */
		user = user_new (lname);
		user->privilege = CMD_ENABLE_LEVEL;
		vty_user_add (user);
		if (_global_host.mutx)
			os_mutex_unlock(_global_host.mutx);
		return CMD_SUCCESS;
	}
	else if(password)
	{
		if(user)
		{
			//vty_out(vty,"%s %s%s",argv[1],vty_user_crypt (argv[1]),VTY_NEWLINE);
			if(enable)
			{
				/*
				 * setting enable password
				 */
				if(user->enable)
					XFREE(MTYPE_HOST, user->enable);

				if(!md5_encrypt_empty(user->enable_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));

				if(encrypt && vty)
					vty_out(vty,"setting enable password encrypt for username %s %s", user->username, VTY_NEWLINE);
				//user->enable = XSTRDUP (MTYPE_HOST, password);
				user->enable = XSTRDUP (MTYPE_HOST,  (password));
				if(encrypt)
				{
					md5_encrypt_password(password, user->enable_encrypt);
				}
			}
			else
			{
				/*
				 * setting loging password
				 */
				if(user->password)
					XFREE(MTYPE_HOST, user->password);

				if(!md5_encrypt_empty(user->password_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				if(encrypt && vty)
					vty_out(vty,"setting password encrypt for username %s %s", user->username, VTY_NEWLINE);

				user->password = XSTRDUP (MTYPE_HOST,  (password));
				if(encrypt)
					md5_encrypt_password(password, user->password_encrypt);
					//user->password_encrypt = XSTRDUP (MTYPE_HOST, vty_user_crypt(password));
			}
			if (_global_host.mutx)
				os_mutex_unlock(_global_host.mutx);
			return CMD_SUCCESS;
		}
		/*
		 * create user
		 */
		user = user_new (lname);
		user->privilege = CMD_ENABLE_LEVEL;
		if(enable)
		{
			/*
			 * setting enable password
			 */
			if(user->enable)
				XFREE(MTYPE_HOST, user->enable);

			if(!md5_encrypt_empty(user->enable_encrypt))
				os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				//XFREE(MTYPE_HOST, user->enable_encrypt);
			if(encrypt && vty)
				vty_out(vty,"setting enable password encrypt for username %s %s", user->username, VTY_NEWLINE);

			user->enable = XSTRDUP (MTYPE_HOST,  (password));
			if(encrypt)
				md5_encrypt_password(password, user->enable_encrypt);
				//user->enable_encrypt = XSTRDUP (MTYPE_HOST, vty_user_crypt(password));
		}
		else
		{
			/*
			 * setting loging password
			 */
			if(user->password)
				XFREE(MTYPE_HOST, user->password);

			if(!md5_encrypt_empty(user->password_encrypt))
				os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));

			if(encrypt && vty)
				vty_out(vty,"setting password encrypt for username %s %s", user->username, VTY_NEWLINE);

			user->password = XSTRDUP (MTYPE_HOST,  (password));
			if(encrypt)
				md5_encrypt_password(password, user->password_encrypt);
		}
		vty_user_add (user);
		if (_global_host.mutx)
			os_mutex_unlock(_global_host.mutx);
		return CMD_SUCCESS;
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	if(vty)
		vty_out(vty,"%s:Can't find user '%s'%s",__func__,name,VTY_NEWLINE);
	return CMD_WARNING;
}

int vty_user_delete(struct vty *vty, zpl_char *name, zpl_bool password, zpl_bool enable)
{
	zpl_char lname[VTY_USERNAME_MAX];
	struct vty_user * user = NULL;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);

	os_memset(lname, 0, VTY_USERNAME_MAX);
	os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
	user = vty_user_lookup (lname);

	if(user)
	{
		if(password == zpl_false)
		{
			/*
			 * delete user
			 */
			if (user->password)
				XFREE (MTYPE_HOST, user->password);
			user->password = NULL;
			if (!md5_encrypt_empty(user->password_encrypt))
				os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));
				//XFREE (MTYPE_HOST, user->password_encrypt);
			if (user->enable)
				XFREE (MTYPE_HOST, user->enable);
			user->enable = NULL;
			if (!md5_encrypt_empty(user->enable_encrypt))
				os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				//XFREE (MTYPE_HOST, user->enable_encrypt);
			listnode_delete (_global_host.userlist, user);
			user_free (user);
		}
		else
		{
			if(enable)
			{
				/*
				 * delete user enable password
				 */
				if (user->enable)
					XFREE (MTYPE_HOST, user->enable);
				user->enable = NULL;
				if (!md5_encrypt_empty(user->enable_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
					//XFREE (MTYPE_HOST, user->enable_encrypt);
			}
			else
			{
				/*
				 * delete user loging password
				 */
				if (user->password)
					XFREE (MTYPE_HOST, user->password);
				user->password = NULL;
				if (!md5_encrypt_empty(user->password_encrypt))
					os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));
					//XFREE (MTYPE_HOST, user->password_encrypt);
			}
		}
		if (_global_host.mutx)
			os_mutex_unlock(_global_host.mutx);
		return CMD_SUCCESS;
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	if(vty)
		vty_out(vty,"%s:Can't find user '%s'%s",__func__,lname,VTY_NEWLINE);
	return CMD_WARNING;
}

int vty_user_change(struct vty *vty, zpl_char *name)
{
	zpl_char lname[VTY_USERNAME_MAX];
	struct vty_user * user = NULL;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);

	os_memset(lname, 0, VTY_USERNAME_MAX);
	os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
	user = vty_user_lookup (lname);

	if (user) {
		vty_user_setting(vty, name);
		//vty_userticated (vty);
		if (_global_host.mutx)
			os_mutex_unlock(_global_host.mutx);
		return CMD_SUCCESS;
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	if(vty)
		vty_out(vty, "Can't find user '%s'%s", name, VTY_NEWLINE);
	return CMD_WARNING;

}

int vty_user_foreach (int (*cb)(void *user, void *p), void *p)
{
	struct listnode *node;
	struct vty_user *user;
	if(_global_host.userlist == NULL)
		return 0;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);

	for (ALL_LIST_ELEMENTS_RO (_global_host.userlist, node, user))
	{
		if(user)
		{
			if(cb)
				(cb)(user, p);
/*			if((user->password == NULL) && md5_encrypt_empty(user->password_encrypt))
				vty_out(vty,"username %s%s", user->username, VTY_NEWLINE);
			else
			{
				if(user->encrypt && !md5_encrypt_empty(user->password_encrypt))
					vty_out(vty,"username %s password %s%s", user->username,
							(user->password_encrypt), VTY_NEWLINE);
				else if(user->password)
					vty_out(vty,"username %s password %s%s", user->username,
								user->password, VTY_NEWLINE);
			}
			if(user->encrypt && !md5_encrypt_empty(user->enable_encrypt))
				vty_out(vty,"username %s enable password %s%s", user->username,
						(user->enable_encrypt), VTY_NEWLINE);
			else if(user->enable)
				vty_out(vty,"username %s enable password %s%s", user->username,
							user->enable, VTY_NEWLINE);
			if(user->privilege)
				vty_out(vty,"username %s privilege %d%s", user->username, user->privilege, VTY_NEWLINE);*/

		}
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return CMD_SUCCESS;
}

zpl_char * vty_user_get(struct vty *vty)
{
	if(vty)
	{
		return vty->username;
	}
	return VTY_USERNAME_DEFAULT;
}


int vty_user_init(void)
{
	struct vty_user * user = NULL;
	if(_global_host.mutx == NULL)
		_global_host.mutx = os_mutex_init();
	if(_global_host.userlist == NULL)
		_global_host.userlist = list_new ();
	user = user_new (VTY_USERNAME_DEFAULT);
	if(user)
	{
		if(_global_host.encrypt)
		{
			user->encrypt = _global_host.encrypt;
			md5_encrypt_password(VTY_PASSWORD_DEFAULT, user->password_encrypt);
			//user->password_encrypt = strdup(vty_user_crypt(VTY_PASSWORD_DEFAULT));
		}
		user->password = strdup(VTY_PASSWORD_DEFAULT);
		user->privilege = CMD_ADMIN_LEVEL;
		user->authen_type = AUTHEN_LOCAL;
		vty_user_add (user);

/*		_global_host.username = user->username;
		_global_host.password = user->password;
		_global_host.password_encrypt = user->password_encrypt;
		_global_host.enable = user->enable;
		_global_host.enable_encrypt = user->enable_encrypt;*/
		//printf("%s\n",zencrypt ("admin"));
	}
	return OK;
}


