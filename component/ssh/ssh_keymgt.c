/*
 * ssh_keymgt.c
 *
 *  Created on: Nov 3, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"

#include "ssh_api.h"
#include "sshd_main.h"
#include "ssh_util.h"




ssh_keymgt_t * ssh_keymgt_lookup(ssh_config_t *ssh, char *keyname)
{
	int i = 0;
	char key_name[SSH_KEY_NAME_MAX];
	memset(key_name, 0, sizeof(key_name));
	strcpy(key_name, keyname);
	for(i = 0; i < SSH_KEY_MAX; i++)
	{
		if(ssh->ssh_key[i].used &&
				(memcmp(ssh->ssh_key[i].keyname, key_name, sizeof(key_name)) == 0))
			return &ssh->ssh_key[i];
	}
	return NULL;
}


int ssh_keymgt_delete(ssh_config_t *ssh, char *keyname)
{
	ssh_keymgt_t * sshkey = ssh_keymgt_lookup(ssh, keyname);
	if(sshkey)
	{
		if(sshkey->key)
			SAFE_FREE(sshkey->key);
		memset(sshkey, 0, sizeof(ssh_keymgt_t));
		return OK;
	}
	return ERROR;
}


int ssh_keymgt_add(ssh_config_t *ssh, struct vty *vty, zpl_uint32 type, char *keyname)
{
	zpl_uint32 i = 0;
	char *typestr;
	for(i = 0; i < SSH_KEY_MAX; i++)
	{
		if(ssh->ssh_key[i].used == 0)
		{
			memset(&ssh->ssh_key[i], 0, sizeof(ssh->ssh_key[i]));
			ssh->ssh_key[i].used = 1;
			strcpy(ssh->ssh_key[i].keyname, keyname);
			//ssh->ssh_key[i].key = pkey;
			ssh->ssh_key[i].type = type;
			/*
			 * rsa : length of the key in bits (e.g. 1024, 2048, 4096)
			 * dsa : length of the key in bits (e.g. 1024, 2048, 3072)
			 * ecdsa : bits of the key (e.g. 256, 384, 512)
			 */
			zpl_uint32 parameter = 0;
			switch(type)
			{
			case SSH_KEYTYPE_DSS:
				typestr = "DSA";
				parameter = 2048;
				break;
			case SSH_KEYTYPE_RSA:
				typestr = "RSA";
				parameter = 2048;
				break;
			case SSH_KEYTYPE_RSA1:
				typestr = "RSA1";
				parameter = 2048;
				break;
			case SSH_KEYTYPE_ECDSA:
				typestr = "ECDSA";
				parameter = 512;
				break;
			case SSH_KEYTYPE_ED25519:
				typestr = "ED25519";
				//parameter = 2048;
				break;
			default:
				typestr = "unknown";
				break;
			}
			vty_sync_out(vty, " General Purpose Keys. it will take a few minutes.%s", VTY_NEWLINE);
			vty_sync_out(vty, " % Generating %d bit %s keys, keys will be non-exportable...%s",
					parameter, typestr, VTY_NEWLINE);

			if(ssh_pki_generate(type, parameter, &ssh->ssh_key[i].key) == SSH_OK)
			{
				//vty_sync_out(vty, " ssh key %s is already exist.%s", keyname, VTY_NEWLINE);
				return OK;
			}
			else
			{
				memset(&ssh->ssh_key[i], 0, sizeof(ssh->ssh_key[i]));
			}
		}
	}
	return ERROR;
}


int ssh_keymgt_export_set(ssh_config_t *ssh, char *keyname, zpl_uint32 type, char *filename, char *password)
{
	ssh_keymgt_t * sshkey = ssh_keymgt_lookup(ssh, keyname);
	if(sshkey)
	{
		int ret = SSH_ERROR;
		char file_name[128];
		memset(file_name, 0, sizeof(file_name));
		sprintf(file_name, sizeof(file_name), "%s%s", KEYS_FOLDER, filename);
		if(type == 1 && ssh_key_is_public(sshkey->key))
		{
			ret = ssh_pki_export_pubkey_file(sshkey->key, file_name);
		}
		else if(type == 2 && ssh_key_is_private(sshkey->key))
		{
			ret = ssh_pki_export_privkey_file(sshkey->key, password, NULL, NULL, file_name);
		}
		return (ret == SSH_OK) ? OK:ERROR;
	}
	return ERROR;
}


int ssh_keymgt_import_set(ssh_config_t *ssh, char *keyname, zpl_uint32 type, char *filename, char *password)
{
	ssh_keymgt_t * sshkey = ssh_keymgt_lookup(ssh, keyname);
	if(sshkey)
	{
		int ret = SSH_ERROR;
		char file_name[128];
		memset(file_name, 0, sizeof(file_name));
		sprintf(file_name, sizeof(file_name), "%s%s", KEYS_FOLDER, filename);
		if(type == 1)
		{
			if(sshkey->key)
				SAFE_FREE(sshkey->key);
			ret = ssh_pki_import_pubkey_file(file_name, &sshkey->key);
		}
		else if(type == 2)
		{
			if(sshkey->key)
				SAFE_FREE(sshkey->key);
			ret = ssh_pki_import_privkey_file(file_name, password, NULL, NULL, &sshkey->key);
		}
		return (ret == SSH_OK) ? OK:ERROR;
	}
	return ERROR;
}

/*
int ssh_options_set(ssh_session session, enum ssh_options_e type,
    const void *value)
{
	return ssh_options_set(session, type, value);
}*/

static void sshd_set_default_keys(ssh_bind sshbind,
                             zpl_uint32 rsa_already_set,
                             zpl_uint32 dsa_already_set,
                             zpl_uint32 ecdsa_already_set)
{
    if (!rsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,
                             KEYS_FOLDER "ssh_host_rsa_key");
    }
    if (!dsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY,
                             KEYS_FOLDER "ssh_host_dsa_key");
    }
    if (!ecdsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_ECDSAKEY,
                             KEYS_FOLDER "ssh_host_ecdsa_key");
    }
}

int sshd_set_keys(ssh_config_t *ssh, ssh_bind sshbind)
{
	char keyfile[128];
	memset(keyfile, 0, sizeof(keyfile));
	ssh_bind_key_free(sshbind);
    sshd_set_default_keys(sshbind, 0, 0, 0);
	if(strlen(ssh->hostkey))
	{
		memset(keyfile, 0, sizeof(keyfile));
		snprintf(keyfile, sizeof(keyfile), "%s%s", KEYS_FOLDER, ssh->hostkey);
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_HOSTKEY, keyfile);
	}
	if(strlen(ssh->rsakey))
	{
		memset(keyfile, 0, sizeof(keyfile));
		snprintf(keyfile, sizeof(keyfile), "%s%s", KEYS_FOLDER, ssh->rsakey);
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, keyfile);
	}
	if(strlen(ssh->dsakey))
	{
		memset(keyfile, 0, sizeof(keyfile));
		snprintf(keyfile, sizeof(keyfile), "%s%s", KEYS_FOLDER, ssh->dsakey);
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, keyfile);
	}
	if(strlen(ssh->ecdsakey))
	{
		memset(keyfile, 0, sizeof(keyfile));
		snprintf(keyfile, sizeof(keyfile), "%s%s", KEYS_FOLDER, ssh->ecdsakey);
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_ECDSAKEY, keyfile);
	}
	return OK;
}


