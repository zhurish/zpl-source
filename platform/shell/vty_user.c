/*
 * vty_user.c
 *
 *  Created on: Nov 27, 2016
 *      Author: zhurish
 */

#include "zebra.h"
#include "command.h"
#include "hash.h"
#include "linklist.h"
#include "memory.h"
#include "version.h"
#include "host.h"
#include "vty.h"

#include "vty_user.h"



char * vty_user_setting (struct vty *vty, const char *name)
{
	if(name == NULL)
		return NULL;
	if(os_strlen(name) > VTY_USERNAME_MAX)
	{
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
	//listnode_add (host.userlist, user);
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
	if(user == NULL || host.userlist == NULL)
		return NULL;
	listnode_add_sort (host.userlist, user);
	return user;
}


static struct vty_user * vty_user_lookup (const char *name)
{
	char lname[VTY_USERNAME_MAX];
	struct listnode *node = NULL;
	struct vty_user *user = NULL;
	if(name == NULL || host.userlist == NULL)
		return NULL;

	os_memset(lname, 0, VTY_USERNAME_MAX);
	os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));

	//VTY_USER_DEBUG_LOG("%s:username:%s\n",__func__,name);
	for (ALL_LIST_ELEMENTS_RO (host.userlist, node, user))
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

BOOL md5_encrypt_empty(unsigned char *ecrypt)
{
    if(os_memcmp(ecrypt,"*#@",3) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

static int encrypt_XCH(unsigned char *pass, unsigned char *password)
{
	int i = 0;
	//char password[VTY_PASSWORD_MAX];
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

int md5_encrypt_password(char *password, unsigned char *ecrypt)
{
    int i;
    unsigned char decrypt[MD5_PASSWORD_MAX];
    unsigned char md5ecrypt[MD5_PASSWORD_MAX];
    if(md5_encrypt_empty(password))
    {
        os_memcpy(ecrypt, password, VTY_PASSWORD_MAX);
        return OK;
    }
    os_memset(decrypt, 0, sizeof(decrypt));
    os_memset(md5ecrypt, 0, sizeof(md5ecrypt));
    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, password, os_strlen(password));
    MD5Final(decrypt, &md5);

    encrypt_XCH(decrypt, md5ecrypt);
    os_memcpy(ecrypt, "*#@", 3);
    os_memcpy(ecrypt + 3, md5ecrypt, MD5_PASSWORD_MAX);
    return OK;
}



int vty_user_encrypt_enable (BOOL encrypt)
{
	struct listnode *node = NULL;
	struct vty_user *user = NULL;
	if(host.userlist == NULL)
		return 0;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	for (ALL_LIST_ELEMENTS_RO (host.userlist, node, user))
	{
		if(user)
		{
			if(/*user->encrypt && */encrypt == FALSE)
			{
				if(md5_encrypt_empty(user->password_encrypt))
					os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));
				if(md5_encrypt_empty(user->enable_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				user->encrypt = FALSE;
			}
			else if(/*user->encrypt == 0 && */encrypt == TRUE)
			{
				if(md5_encrypt_empty(user->password_encrypt))
					os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));
				if(md5_encrypt_empty(user->enable_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				if(user->password)
					md5_encrypt_password(user->password, user->password_encrypt);
				if(user->enable)
					md5_encrypt_password(user->enable, user->enable_encrypt);
				user->encrypt = TRUE;
			}
		}
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return 0;
}

BOOL vty_user_enable_password (struct vty *vty, const char *name)
{
	BOOL ret = FALSE;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			if(user->enable && md5_encrypt_empty(user->enable_encrypt))
				ret = TRUE;
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			return ret;
		}
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return FALSE;
}


int vty_user_getting_privilege (struct vty *vty, char *name)
{
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			vty_out(vty, "%s 0:%s %s",__func__,lname, VTY_NEWLINE);
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			return user->privilege;
		}
	}
/*
	if(vty->user)
	{
		if (host.mutx)
			os_mutex_unlock(host.mutx);
		return vty->user->privilege;
	}
*/

	if (host.mutx)
		os_mutex_unlock(host.mutx);
	vty_out(vty, "%s:vty->user is NULL %s",__func__, VTY_NEWLINE);
	return ENABLE_LEVEL;
}

int vty_user_setting_privilege (struct vty *vty, char *name, int privilege)
{
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			if(privilege >= VIEW_LEVEL && privilege <= ADMIN_LEVEL)
			{
				user->privilege = privilege;
				if (host.mutx)
					os_mutex_unlock(host.mutx);
				return CMD_SUCCESS;
			}
		}
	}
/*	if(vty->user)
	{
		if(privilege >= VIEW_LEVEL && privilege <= ADMIN_LEVEL)
		{
			vty->user->privilege = privilege;
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			return CMD_SUCCESS;
		}
	}*/
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	vty_out(vty,"%s:Can't find user '%s'%s",__func__,name,VTY_NEWLINE);
	return CMD_WARNING;
}

int vty_user_getting_authen_type (struct vty *vty, char *name)
{
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			vty_out(vty, "%s 0:%s %s",__func__,name, VTY_NEWLINE);
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			return user->authen_type;
		}
	}
/*	if(vty->user)
	{
		if (host.mutx)
			os_mutex_unlock(host.mutx);
		return vty->user->authen_type;
	}*/
	vty_out(vty, "%s:vty->user is NULL %s",__func__, VTY_NEWLINE);
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return ENABLE_LEVEL;
}

int vty_user_setting_authen_type (struct vty *vty, char *name, int authen_type)
{
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if(name)
	{
		char lname[VTY_USERNAME_MAX];
		struct vty_user * user = NULL;
		os_memset(lname, 0, VTY_USERNAME_MAX);
		os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
		user = vty_user_lookup (lname);
		if(user)
		{
			if(authen_type >= AUTHEN_LOCAL && authen_type <= AUTHEN_TACACS)
			{
				user->authen_type = authen_type;
				if (host.mutx)
					os_mutex_unlock(host.mutx);
				return CMD_SUCCESS;
			}
		}
	}
/*	if(vty->user)
	{
		if(authen_type >= AUTHEN_LOCAL && authen_type <= AUTHEN_TACACS)
		{
			vty->user->authen_type = authen_type;
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			return CMD_SUCCESS;
		}
	}*/
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	vty_out(vty,"%s:Can't find user '%s'%s",__func__,name,VTY_NEWLINE);
	return CMD_WARNING;
}

int vty_user_authentication (struct vty *vty, char *password)
{
	unsigned char *passwd = NULL;
	//enum node_type next_node = 0;
	int fail = 1;
	unsigned char encrypt[VTY_PASSWORD_MAX];
	struct vty_user * user = NULL;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	user = vty_user_lookup (vty->username);
	if(user)
	{
		VTY_USER_DEBUG_LOG("%s:username:%s==%s\n",__func__,user->username,vty->username);
		switch (vty->node)
		{
			case AUTH_NODE:
				if(user->authen_type == AUTHEN_LOCAL)
				{
					vty_out (vty,"%s loging authenticated%s",user->username,VTY_NEWLINE);
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
					vty_out (vty,"%s enable %s authenticated%s",user->username,user->encrypt? "encrypt":" ",VTY_NEWLINE);
					if (user->encrypt)
						passwd = user->enable_encrypt;
					else
						passwd = user->enable;
					//next_node = ENABLE_NODE;
					vty_out (vty,"%s:%s=%s%s",user->username,user->enable,user->enable_encrypt,VTY_NEWLINE);
				}
				break;
			default:
				if (host.mutx)
					os_mutex_unlock(host.mutx);
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
				if (host.mutx)
					os_mutex_unlock(host.mutx);
				return CMD_WARNING;
			}
			if(fail == 0)
			{
				//vty->user = user;//vty_user_lookup (vty->username);
				if (host.mutx)
					os_mutex_unlock(host.mutx);
				vty_out(vty, "%s:vty->user %s %s",__func__, user->username,VTY_NEWLINE);
				return CMD_SUCCESS;
			}
			else
			{
				if (host.mutx)
					os_mutex_unlock(host.mutx);
				return CMD_WARNING;
			}
		}
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return CMD_WARNING;
}

int user_authentication (char *username, char *password)
{
	unsigned char *passwd = NULL;
	int fail = ERROR;
	unsigned char encrypt[VTY_PASSWORD_MAX];
	struct vty_user * user = NULL;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
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
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			return ERROR;
		}
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return fail;
}

int vty_user_authorization (struct vty *vty, char *cmd)
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
	if(vty == NULL || host.userlist == NULL)
		return 0;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if(vty->type != VTY_FILE && vty->username)
	{
		vty_out(vty,"! current login user:%s %s", vty->username, VTY_NEWLINE);
		vty_out(vty,"!%s", VTY_NEWLINE);
	}
	for (ALL_LIST_ELEMENTS_RO (host.userlist, node, user))
	{
		if(user)
		{
			if((user->password == NULL) && !md5_encrypt_empty(user->password_encrypt))
				vty_out(vty,"username %s%s", user->username, VTY_NEWLINE);
			else
			{
				if(user->encrypt && md5_encrypt_empty(user->password_encrypt))
					vty_out(vty,"username %s password %s%s", user->username,
							(user->password_encrypt), VTY_NEWLINE);
				else if(user->password)
					vty_out(vty,"username %s password %s%s", user->username,
								user->password, VTY_NEWLINE);
			}
			if(user->encrypt && md5_encrypt_empty(user->enable_encrypt))
				vty_out(vty,"username %s enable password %s%s", user->username,
						(user->enable_encrypt), VTY_NEWLINE);
			else if(user->enable)
				vty_out(vty,"username %s enable password %s%s", user->username,
							user->enable, VTY_NEWLINE);
			if(user->privilege)
				vty_out(vty,"username %s privilege %d%s", user->username, user->privilege, VTY_NEWLINE);

		}
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return CMD_SUCCESS;
}

int vty_user_create(struct vty *vty, char *name, char *password, BOOL enable, BOOL encrypt)
{
	char lname[VTY_USERNAME_MAX];
	struct vty_user * user = NULL;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	os_memset(lname, 0, VTY_USERNAME_MAX);
	os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
	user = vty_user_lookup (lname);
	if(password == NULL)
	{
		if(user)
		{
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			vty_out(vty,"user '%s' is already exist%s",password,VTY_NEWLINE);
			return CMD_WARNING;
		}
		/*
		 * create user
		 */
		user = user_new (lname);
		user->privilege = ENABLE_LEVEL;
		vty_user_add (user);
		if (host.mutx)
			os_mutex_unlock(host.mutx);
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

				if(md5_encrypt_empty(user->enable_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));

				if(encrypt)
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

				if(md5_encrypt_empty(user->password_encrypt))
					os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				if(encrypt)
					vty_out(vty,"setting password encrypt for username %s %s", user->username, VTY_NEWLINE);

				user->password = XSTRDUP (MTYPE_HOST,  (password));
				if(encrypt)
					md5_encrypt_password(password, user->password_encrypt);
					//user->password_encrypt = XSTRDUP (MTYPE_HOST, vty_user_crypt(password));
			}
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			return CMD_SUCCESS;
		}
		/*
		 * create user
		 */
		user = user_new (lname);
		user->privilege = ENABLE_LEVEL;
		if(enable)
		{
			/*
			 * setting enable password
			 */
			if(user->enable)
				XFREE(MTYPE_HOST, user->enable);

			if(md5_encrypt_empty(user->enable_encrypt))
				os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				//XFREE(MTYPE_HOST, user->enable_encrypt);
			if(encrypt)
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

			if(md5_encrypt_empty(user->password_encrypt))
				os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));

			if(encrypt)
				vty_out(vty,"setting password encrypt for username %s %s", user->username, VTY_NEWLINE);

			user->password = XSTRDUP (MTYPE_HOST,  (password));
			if(encrypt)
				md5_encrypt_password(password, user->password_encrypt);
		}
		vty_user_add (user);
		if (host.mutx)
			os_mutex_unlock(host.mutx);
		return CMD_SUCCESS;
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	vty_out(vty,"%s:Can't find user '%s'%s",__func__,name,VTY_NEWLINE);
	return CMD_WARNING;
}

int vty_user_delete(struct vty *vty, char *name, BOOL password, BOOL enable)
{
	char lname[VTY_USERNAME_MAX];
	struct vty_user * user = NULL;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	os_memset(lname, 0, VTY_USERNAME_MAX);
	os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
	user = vty_user_lookup (lname);

	if(user)
	{
		if(password == FALSE)
		{
			/*
			 * delete user
			 */
			if (user->password)
				XFREE (MTYPE_HOST, user->password);
			user->password = NULL;
			if (md5_encrypt_empty(user->password_encrypt))
				os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));
				//XFREE (MTYPE_HOST, user->password_encrypt);
			if (user->enable)
				XFREE (MTYPE_HOST, user->enable);
			user->enable = NULL;
			if (md5_encrypt_empty(user->enable_encrypt))
				os_memset(user->enable_encrypt, 0, sizeof(user->enable_encrypt));
				//XFREE (MTYPE_HOST, user->enable_encrypt);
			listnode_delete (host.userlist, user);
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
				if (md5_encrypt_empty(user->enable_encrypt))
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
				if (md5_encrypt_empty(user->password_encrypt))
					os_memset(user->password_encrypt, 0, sizeof(user->password_encrypt));
					//XFREE (MTYPE_HOST, user->password_encrypt);
			}
		}
		if (host.mutx)
			os_mutex_unlock(host.mutx);
		return CMD_SUCCESS;
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	vty_out(vty,"%s:Can't find user '%s'%s",__func__,lname,VTY_NEWLINE);
	return CMD_WARNING;
}

int vty_user_change(struct vty *vty, char *name)
{
	char lname[VTY_USERNAME_MAX];
	struct vty_user * user = NULL;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	os_memset(lname, 0, VTY_USERNAME_MAX);
	os_memcpy(lname, name, MIN(os_strlen(name), VTY_USERNAME_MAX));
	user = vty_user_lookup (lname);

	if (user) {
		vty_user_setting(vty, name);
		//vty_userticated (vty);
		if (host.mutx)
			os_mutex_unlock(host.mutx);
		return CMD_SUCCESS;
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	vty_out(vty, "Can't find user '%s'%s", name, VTY_NEWLINE);
	return CMD_WARNING;

}

char * vty_user_get(struct vty *vty)
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
	if(host.mutx == NULL)
		host.mutx = os_mutex_init();
	if(host.userlist == NULL)
		host.userlist = list_new ();
	user = user_new (VTY_USERNAME_DEFAULT);
	if(user)
	{
		if(host.encrypt)
		{
			user->encrypt = host.encrypt;
			md5_encrypt_password(VTY_PASSWORD_DEFAULT, user->password_encrypt);
			//user->password_encrypt = strdup(vty_user_crypt(VTY_PASSWORD_DEFAULT));
		}
		user->password = strdup(VTY_PASSWORD_DEFAULT);
		user->privilege = ADMIN_LEVEL;
		user->authen_type = AUTHEN_LOCAL;
		vty_user_add (user);

/*		host.username = user->username;
		host.password = user->password;
		host.password_encrypt = user->password_encrypt;
		host.enable = user->enable;
		host.enable_encrypt = user->enable_encrypt;*/
		//printf("%s\n",zencrypt ("admin"));
	}
	return OK;
}


