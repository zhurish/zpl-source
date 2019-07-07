/*
 * x5_b_app_ac.c
 *
 *  Created on: 2019年4月3日
 *      Author: DELL
 */
#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"
#include "eloop.h"
#include "cJSON.h"

#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "x5_b_app.h"
#include "x5_b_json.h"
#include "x5_b_cmd.h"
#include "web_x5b.h"

#ifdef PL_VOIP_MODULE
#include "voip_def.h"
#include "voip_app.h"
#endif


/**********************************************/
/**********************************************/
#ifndef X5B_APP_IMG_ANSYNC
int base64_encode(char *in_str, int in_len, char **out_str)
{
	BIO *b64, *bio;
	BUF_MEM *bptr = NULL;
	size_t size = 0;

	if (in_str == NULL || out_str == NULL)
		return -1;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_write(bio, in_str, in_len);
	BIO_flush(bio);

	BIO_get_mem_ptr(bio, &bptr);
	if(out_str)
	{
		*out_str = malloc(bptr->length + 1);
		if(*out_str)
		{
			memset(*out_str, '\0', bptr->length + 1);
			memcpy(*out_str, bptr->data, bptr->length);
		}
	}
	size = bptr->length;

	BIO_free_all(bio);
	return size;
}

int base64_decode(char *in_str, int in_len, char **output)
{
	int out_length = 0;
	BIO* b64 = NULL;
	BIO* bio = NULL;
	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bio = BIO_new_mem_buf(in_str, in_len);
	bio = BIO_push(b64, bio);
	if(output)
	{
		*output = (char*)malloc(in_len);
		if(*output)
		{
			memset(*output, '\0', in_len);
			out_length = BIO_read(bio, *output, in_len);
		}
	}
	BIO_free_all(bio);
	BIO_free_all(b64);
	return out_length;
}
/**********************************************/
int x5b_face_img_base64_enc(char *input, char **out)
{
	int maxlen = 0;
	char filename[128];
	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "/tmp/app/img/%s", input);
	maxlen = os_file_size(filename);
	if(maxlen != ERROR)
	{
		char *buf = malloc(maxlen + 1);
		if(buf)
		{
			memset(buf, 0, maxlen + 1);
			if(os_read_file(filename, buf, maxlen) == OK)
			{
				maxlen = base64_encode(buf, maxlen, out);
				free(buf);
				if(maxlen > 0 && maxlen != -1)
				{
					return maxlen;
				}
				if(out)
				{
					free(*out);
					*out = NULL;
				}
				return ERROR;
			}
			free(buf);
			return ERROR;
		}
	}
	return ERROR;
}

/*int x5b_face_img_base64_dec(char *input, int len, char **out)
{
	int base64_decode(char *in_str, int in_len, char **output)

	int maxlen = 0;
	char filename[128];
	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "/tmp/app/img/%s", input);
	maxlen = os_file_size(filename);
	if(maxlen != ERROR)
	{
		char *buf = malloc(maxlen + 1);
		if(buf)
		{
			memset(buf, 0, maxlen + 1);
			if(os_read_string(filename, buf, maxlen) == OK)
			{
				maxlen = base64_encode(buf, maxlen, out);
				free(buf);
				if(maxlen > 0 && maxlen != -1)
				{
					return maxlen;
				}
				if(out)
				{
					free(*out);
					*out = NULL;
				}
				return ERROR;
			}
			free(buf);
			return ERROR;
		}
	}
	return ERROR;
}*/
#endif

/**********************************************************************************/
/**********************************************************************************/
/* C 模块制卡 */
static int _x5b_app_add_card_json(char *output, int mlen, uci_face_card_t *card)
{
	char* szJSON = NULL;
	cJSON* pRoot = cJSON_CreateObject();
	cJSON* pArray = cJSON_CreateArray();
	cJSON* pItem = cJSON_CreateObject();
	if(pItem && pArray && pRoot && card )
	{
		cJSON_AddItemToObject(pRoot, "make_card", pArray);
		if(strlen(card->username))
			cJSON_AddStringToObject(pItem, "username", card->username);
		if(strlen(card->user_id))
			cJSON_AddStringToObject(pItem, "userid", card->user_id);

		if(card->make_card && card->make_face)
			cJSON_AddNumberToObject(pItem, "type", 2);
		else if(card->make_face)
			cJSON_AddNumberToObject(pItem, "type", 0);
		else if(card->make_card)
			cJSON_AddNumberToObject(pItem, "type", 1);

		if(card->make_card)
		{
			if(strlen(card->cardid))
			{
				cJSON_AddStringToObject(pItem, "cardid", card->cardid);
				cJSON_AddNumberToObject(pItem, "time1", card->start_date);
				cJSON_AddNumberToObject(pItem, "time2", card->stop_date);
				zlog_debug(ZLOG_APP, "==================_x5b_app_add_card_json start:%d", card->start_date);
				zlog_debug(ZLOG_APP, "==================_x5b_app_add_card_json stop:%d", card->stop_date);
				if(strlen(card->cardtype))
					cJSON_AddStringToObject(pItem, "cardtype", card->cardtype);
			}
		}
		if(card->make_face && strlen(card->face_img))
		{
			char imgpath[256];
			memset(imgpath, 0, sizeof(imgpath));
			cJSON_AddNumberToObject(pItem, "faceid", 1);
			//busybox wget -q -O - http://admin:admin@10.10.10.254/adm/status.shtml
			snprintf(imgpath, sizeof(imgpath), "http://root:admintsl@10.10.10.254/app/%s", card->face_img);
			cJSON_AddStringToObject(pItem, "img", imgpath);
		}
		cJSON_AddItemToArray(pArray, pItem);

		szJSON = cJSON_Print(pRoot);
		if(szJSON)
		{
			if(output)
				strncpy(output, szJSON, MIN(mlen, strlen(szJSON)));
			cjson_free(szJSON);
		}
		cJSON_Delete(pRoot);
		return OK;
	}
	return ERROR;
}
/**********************************************************************************/
static int _x5b_app_delete_card_json(char *output, int mlen, uci_face_card_t *card)
{
	char* szJSON = NULL;
	cJSON* pRoot = cJSON_CreateObject();
	cJSON* pArray = cJSON_CreateArray();
	cJSON* pItem = cJSON_CreateObject();
	if(pItem && pArray && pRoot && card)
	{
		cJSON_AddItemToObject(pRoot, "delete_card", pArray);
		if(strlen(card->username))
			cJSON_AddStringToObject(pItem, "username", card->username);
		if(strlen(card->user_id))
			cJSON_AddStringToObject(pItem, "userid", card->user_id);

		if(card->make_card && card->make_face)
			cJSON_AddNumberToObject(pItem, "type", 2);
		else if(card->make_face)
			cJSON_AddNumberToObject(pItem, "type", 0);
		else if(card->make_card)
			cJSON_AddNumberToObject(pItem, "type", 1);

		if(card->make_card)
		{
			if(strlen(card->cardid))
				cJSON_AddStringToObject(pItem, "cardid", card->cardid);
		}
		if(card->make_face && card->face_id)
			cJSON_AddNumberToObject(pItem, "faceid", card->face_id);

		cJSON_AddItemToArray(pArray, pItem);
		szJSON = cJSON_Print(pRoot);
		if(szJSON)
		{
			if(output)
				strncpy(output, szJSON, MIN(mlen, strlen(szJSON)));
			cjson_free(szJSON);
		}
		cJSON_Delete(pRoot);
		return OK;
	}
	return ERROR;
}


int x5b_app_add_card_json(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	char output[1024];
	uci_face_card_t *card = (uci_face_card_t *)info;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(card != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Add Card CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	memset(output,0, sizeof(output));
	_x5b_app_add_card_json(output, sizeof(output), card);
	if(strlen(output))
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_MAKE_CARD), strlen(output), output);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Add Card CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}

int x5b_app_del_card_json(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	char output[1024];
	uci_face_card_t *card = (uci_face_card_t *)info;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(card != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Del Card CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	memset(output, 0, sizeof(output));
	_x5b_app_delete_card_json(output, sizeof(output), card);
	if(strlen(output))
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_DELETE_CARD), strlen(output), output);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Del Card CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}

/****************************************************************/
/****************************************************************/
static int voip_card_web_handle_tran(uci_face_card_t * fcard, face_card_t *dbase)
{
	zassert(fcard != NULL);
	zassert(dbase != NULL);
	memset(dbase, 0, sizeof(face_card_t));
	if(strlen(fcard->username))
	{
		memset(dbase->username, 0, sizeof(dbase->username));
		strncpy(dbase->username, fcard->username, MIN(sizeof(dbase->username), strlen(fcard->username)));
	}
	if(strlen(fcard->user_id))
	{
		memset(dbase->user_id, 0, sizeof(dbase->user_id));
		strncpy(dbase->user_id, fcard->user_id, MIN(sizeof(dbase->user_id), strlen(fcard->user_id)));
		//strcpy(dbase->user_id, fcard->user_id);
	}
	if(strlen(fcard->cardid))
	{
		memset(dbase->card_id, 0, sizeof(dbase->card_id));
		strncpy(dbase->card_id, fcard->cardid, MIN(sizeof(dbase->card_id), strlen(fcard->cardid)));
		strcpy(dbase->card_id, fcard->cardid);
	}
	if(strlen(fcard->face_img))
	{
		memset(dbase->img_id, 0, sizeof(dbase->img_id));
		strncpy(dbase->img_id, fcard->face_img, MIN(sizeof(dbase->img_id), strlen(fcard->face_img)));
		//strcpy(dbase->img_id, fcard->face_img);
	}
/*
	u_int32 		start_date;
	u_int32 		stop_date;
	char 			cardtype[APP_ID_MAX];
	u_int8 			make_edit;
	u_int8 			make_card;
	u_int8 			make_face;
*/
	dbase->start_time = fcard->start_date;
	dbase->stop_time = fcard->stop_date;
	//dbase->card_type = fcard->typeid;
	dbase->make_card = fcard->make_card;
	dbase->make_face = fcard->make_face;
	dbase->face_id = fcard->face_id;

	if(strlen(fcard->cardtype))
	{
		if(strstr(fcard->cardtype,"Blacklist"))
			dbase->card_type = 1;
		else if(strstr(fcard->cardtype, "Whitelist"))
			dbase->card_type = 2;
		else
			dbase->card_type = 0;

		zlog_debug(ZLOG_APP, "===================%s ->%s type=%d", __func__,
				   fcard->cardtype, dbase->card_type);
	}
	return OK;
}
/****************************************************************/
/****************************************************************/
static int voip_card_web_handle(int add, uci_face_card_t * fcard)
{
	face_card_t info;
	zassert(fcard != NULL);
	voip_card_web_handle_tran(fcard, &info);
	if(add == 1)
	{
		if(!voip_card_node_lookup_by_username(NULL, info.user_id))
		{
			zlog_debug(ZLOG_APP, "----------------------ADD CARD");
			return voip_card_add_cardid(&info);
			//return voip_card_update_cardid(info.card_id, &info);
		}
		else
		{
			zlog_debug(ZLOG_APP, "----------------------UPDATE CARD");
			return voip_card_update_cardid(info.user_id, &info);
		}
	}
	else if(add == 2)
	{
		if(!voip_card_node_lookup_by_username(NULL, info.user_id))
		{
			zlog_debug(ZLOG_APP, "----------------------ADD CARD");
			return voip_card_add_cardid(&info);
			//return voip_card_update_cardid(info.card_id, &info);
		}
		else
		{
			zlog_debug(ZLOG_APP, "----------------------UPDATE CARD");
			return voip_card_update_cardid_by_userid(info.user_id, &info);
		}
	}
	else if(add == 3)
	{
		if(!voip_card_node_lookup_by_username(NULL, info.user_id))
		{
			zlog_debug(ZLOG_APP, "----------------------ADD FACE");
			return voip_card_add_cardid(&info);
			//return voip_card_update_cardid(info.card_id, &info);
		}
		else
		{
			zlog_debug(ZLOG_APP, "----------------------UPDATE FACE");
			return voip_card_update_face_by_userid(info.user_id, &info);
		}
	}
	else
	{
		zlog_debug(ZLOG_APP, "----------------------DEL CARD");
		return voip_card_del_cardid(NULL/*info.username*/, info.user_id, NULL);
	}
	return OK;
}
/****************************************************************/
/****************************************************************/
int x5b_app_make_card(uci_face_card_t *info)
{
	int cmd = 0, ret = 0;
	face_card_t *card = NULL;
	zassert(info != NULL);
	if(voip_card_lookup_by_cardid_userid(info->cardid, info->user_id) == OK)
	{
		zlog_debug(ZLOG_APP, "this Card is already binding another User.");
		return ERROR;
	}
	card = voip_card_node_lookup_by_username(NULL, info->user_id);
	if(card)
	{
		if(info->make_card && info->make_face)//制卡，发脸
		{
			cmd = 1;
			uci_face_card_t decard;
			memset(&decard, 0, sizeof(decard));
			if(card->make_card)
			{
				decard.make_card = 1;
				memcpy(decard.cardid, card->card_id, sizeof(decard.cardid));
				zlog_debug(ZLOG_APP, "delete old card to A");
				x5b_app_delete_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
			}
			if(card->make_face)
			{
				zlog_debug(ZLOG_APP, "delete old face and card to C");
				memcpy(decard.username, card->username, sizeof(decard.username));
				memcpy(decard.user_id, card->user_id, sizeof(decard.user_id));
				decard.make_face = 1;
				decard.face_id = card->face_id;
				x5b_app_del_card_json(x5b_app_mgt, &decard, E_CMD_TO_C);
			}
		}
		else if(info->make_card)
		{
			cmd = 2;
			uci_face_card_t decard;
			memset(&decard, 0, sizeof(decard));
			if(card->make_card)
			{
				decard.make_card = 1;
				memcpy(decard.cardid, card->card_id, sizeof(decard.cardid));
				zlog_debug(ZLOG_APP, "delete old card to A");
				x5b_app_delete_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
			}
			if(card->make_face)
			{
				zlog_debug(ZLOG_APP, "delete old card to C");
				memcpy(decard.username, card->username, sizeof(decard.username));
				memcpy(decard.user_id, card->user_id, sizeof(decard.user_id));
	/*			decard.make_face = 1;
				decard.face_id = card->face_id;*/
				x5b_app_del_card_json(x5b_app_mgt, &decard, E_CMD_TO_C);
			}
		}
		else if(info->make_face)
		{
			cmd = 3;
			uci_face_card_t decard;
			memset(&decard, 0, sizeof(decard));
/*			if(card->make_card)
			{
				decard.make_card = 1;
				memcpy(decard.cardid, card->card_id, sizeof(decard.cardid));
				zlog_debug(ZLOG_APP, "delete old card to A");
				x5b_app_delete_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
			}*/
			if(card->make_face)
			{
				zlog_debug(ZLOG_APP, "delete old card to C");
				memcpy(decard.username, card->username, sizeof(decard.username));
				memcpy(decard.user_id, card->user_id, sizeof(decard.user_id));
				decard.make_face = 1;
				decard.face_id = card->face_id;
				x5b_app_del_card_json(x5b_app_mgt, &decard, E_CMD_TO_C);
			}
		}
	}
	else
		cmd = 1;
	ret = 0;
	if(x5b_app_mgt->X5CM)
		ret |= x5b_app_add_card_json(x5b_app_mgt, info, E_CMD_TO_C);

	if(info->make_card)
	{
		permiListType pacard;
		memset(&pacard, 0, sizeof(permiListType));
		if(strlen(info->cardid))
		{
			card_id_string_to_hex(info->cardid, strlen(info->cardid), pacard.ID);
		}
		pacard.start_time = htonl(info->start_date);
		pacard.stop_time = htonl(info->stop_date);

		if(strlen(info->cardtype))
		{
			if(strstr(info->cardtype,"Blacklist"))
				pacard.status = 1;
			else if(strstr(info->cardtype,"Whitelist"))
				pacard.status = 2;
		}
		ret |= x5b_app_add_card(x5b_app_mgt, &pacard, E_CMD_TO_A);
	}
	if(ret != OK)
		return ERROR;
	return voip_card_web_handle(cmd, info);
}

/****************************************************************/
int x5b_app_del_card(uci_face_card_t *info)
{
	int cmd = 0, ret = 0;
	face_card_t *card = NULL;
	zassert(info != NULL);
	card = voip_card_node_lookup_by_username(NULL, info->user_id);
	if(card)
	{
		if(info->make_card && info->make_face)//制卡，发脸
		{
			cmd = 1;
			uci_face_card_t decard;
			memset(&decard, 0, sizeof(decard));
			if(card->make_card)
			{
				decard.make_card = 1;
				memcpy(decard.cardid, card->card_id, sizeof(decard.cardid));
				zlog_debug(ZLOG_APP, "delete card to A");
				ret |= x5b_app_delete_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
			}
			if(card->make_face)
			{
				zlog_debug(ZLOG_APP, "delete face and card to C");
				memcpy(decard.username, card->username, sizeof(decard.username));
				memcpy(decard.user_id, card->user_id, sizeof(decard.user_id));
				decard.make_face = 1;
				decard.face_id = card->face_id;
				ret |= x5b_app_del_card_json(x5b_app_mgt, &decard, E_CMD_TO_C);
			}
		}
		else if(info->make_card)
		{
			cmd = 2;
			uci_face_card_t decard;
			memset(&decard, 0, sizeof(decard));
			if(card->make_card)
			{
				decard.make_card = 1;
				memcpy(decard.cardid, card->card_id, sizeof(decard.cardid));
				zlog_debug(ZLOG_APP, "delete card to A");
				ret |= x5b_app_delete_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
			}
			if(card->make_face)
			{
				zlog_debug(ZLOG_APP, "delete card to C");
				memcpy(decard.username, card->username, sizeof(decard.username));
				memcpy(decard.user_id, card->user_id, sizeof(decard.user_id));
	/*			decard.make_face = 1;
				decard.face_id = card->face_id;*/
				ret |= x5b_app_del_card_json(x5b_app_mgt, &decard, E_CMD_TO_C);
			}
		}
		else if(info->make_face)
		{
			cmd = 3;
			uci_face_card_t decard;
			memset(&decard, 0, sizeof(decard));
/*			if(card->make_card)
			{
				decard.make_card = 1;
				memcpy(decard.cardid, card->card_id, sizeof(decard.cardid));
				zlog_debug(ZLOG_APP, "delete old card to A");
				x5b_app_delete_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
			}*/
			if(card->make_face)
			{
				zlog_debug(ZLOG_APP, "delete old card to C");
				memcpy(decard.username, card->username, sizeof(decard.username));
				memcpy(decard.user_id, card->user_id, sizeof(decard.user_id));
				decard.make_face = 1;
				decard.face_id = card->face_id;
				ret |= x5b_app_del_card_json(x5b_app_mgt, &decard, E_CMD_TO_C);
			}
		}
	}
	else
	{
		return OK;
	}
	if(ret != OK)
		return ERROR;
	return voip_card_web_handle(0, info);
}









#if 0

static int x5b_app_make_card_json(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	char *output = NULL;
	make_face_card_t *card = (make_face_card_t *)info;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Add Card CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}

	_x5b_app_make_card_cjson(&output, card, 1);
	if(output)
	{
		x5b_app_hdr_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
		{
			zlog_debug(ZLOG_APP, "=====================================");
			zlog_debug(ZLOG_APP, "Make Card Json:%s", output);
			zlog_debug(ZLOG_APP, "=====================================");
		}
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_MAKE_CARD), strlen(output), output);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Add Card CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		free(output);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}



static int x5b_app_delete_card_json(x5b_app_mgt_t *app, void *info, int to, int jst)
{
	int len = 0;
	char *output;
	//make_face_card_t card;
	face_card_t *cardnode;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Delete Card CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(jst == 0)
	{
		//memset(&card, 0, sizeof(card));
		//strcpy(card.cardid, info);
		//cardnode = voip_card_node_lookup_by_cardid(card.cardid);
/*		if(cardnode == NULL)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "Can not lookup by cardid %s", card.cardid);
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return ERROR;
		}*/
		//memcpy(&card, cardnode, sizeof(card));

/*		strcpy(card.user_id, cardnode->user_id);
		strcpy(card.user_id, cardnode->user_id);
		strcpy(card.user_id, cardnode->user_id);
		strcpy(card.user_id, cardnode->user_id);
		NODE			node;
		char 			username[APP_USERNAME_MAX];
		char 			user_id[APP_ID_MAX];
		char 			card_id[APP_CARD_ID_MAX];
		char 			img_id[APP_IMG_ID_MAX];
		u_int32			face_id;
		u_int32			start_time;
		u_int32			stop_time;
		u_int8			card_type:4;//(1:2)
		u_int8			make_card:2;
		u_int8			make_face:2;
		*/
		cardnode = (face_card_t *)info;
/*
		zlog_debug(ZLOG_APP, "  username %s", strlen(cardnode->username)?cardnode->username:" ");
		zlog_debug(ZLOG_APP, "  user_id %s", strlen(cardnode->user_id)?cardnode->user_id:" ");
		zlog_debug(ZLOG_APP, "  card_id %s", strlen(cardnode->card_id)?cardnode->card_id:" ");
		zlog_debug(ZLOG_APP, "  card_type %d", cardnode->card_type);
		zlog_debug(ZLOG_APP, "  make_card %d", cardnode->make_card);
		zlog_debug(ZLOG_APP, "  make_face %d", cardnode->make_face);
*/
		_x5b_app_delete_card_cjson(&output, cardnode, 0);
	}
	else
	{
		cardnode = (face_card_t *)info;
		_x5b_app_delete_card_cjson(&output, cardnode, 0);
	}
	if(output)
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_DELETE_CARD), strlen(output), output);

		if(X5_B_ESP32_DEBUG(EVENT))
		{
			zlog_debug(ZLOG_APP, "=====================================");
			zlog_debug(ZLOG_APP, " Delate Card Json:%s", output);
			zlog_debug(ZLOG_APP, "=====================================");
		}

		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Delete Card CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}


int x5b_app_add_face_card(x5b_app_mgt_t *app, void *info, int to)
{
	if(to == E_CMD_TO_A)
	{
		 return x5b_app_make_card_pack_to_a(app, info,  to);
	}
	else
	{
		zlog_debug(ZLOG_APP, "===================%s -> C", __func__);
		return x5b_app_make_card_json(app, info, to);
	}
	return ERROR;
}

int x5b_app_delete_face_card(x5b_app_mgt_t *app, void *info, int to)
{
	if(to == E_CMD_TO_A)
	{
		 return x5b_app_delete_card_pack_to_a(app, info,  to);
	}
	else
	{
		zlog_debug(ZLOG_APP, "===================%s -> C", __func__);
		return x5b_app_delete_card_json(app, info, to, 0);
	}
	return ERROR;
}

int x5b_app_delete_face_card_hw(x5b_app_mgt_t *app, void *info, int to)
{
	if(to == E_CMD_TO_A)
	{
		 return x5b_app_delete_card_pack_to_a(app, info,  to);
	}
	else
	{
		zlog_debug(ZLOG_APP, "===================%s -> C", __func__);
		return x5b_app_delete_card_json(app, info, to, 1);
	}
	return ERROR;
}

#endif
/*
int x5b_app_req_card_tlv_json(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	make_face_card_t card;
	memset(&card, 0, sizeof(card));
	if(tlv->len)
	{
		{
			int iCount = 0, i = 0;
			cJSON* pRoot = cJSON_Parse(tlv->val.pval);
			if(pRoot == NULL)
			{
				return -1;
			}
			cJSON* pArray = cJSON_GetObjectItem(pRoot, "make_card");
			if (NULL == pArray)
			{
				return -1;
			}
			iCount = cJSON_GetArraySize(pArray);
			for (; i < iCount; ++i)
			{
				cJSON* pItem = cJSON_GetArrayItem(pArray, i);
				if (NULL == pItem)
				{
					continue;
				}

				if(cJSON_GetObjectItem(pItem, "username")->valuestring)
					strcpy(card.username, cJSON_GetObjectItem(pItem, "username")->valuestring);

				if(cJSON_GetObjectItem(pItem, "userid")->valuestring)
					strcpy(card.user_id, cJSON_GetObjectItem(pItem, "userid")->valuestring);

				if(cJSON_GetObjectItem(pItem, "cardid")->valuestring)
					strcpy(card.cardid, cJSON_GetObjectItem(pItem, "cardid")->valuestring);

				if(cJSON_GetObjectItem(pItem, "type")->valuestring)
					strcpy(card.cardtype, cJSON_GetObjectItem(pItem, "type")->valuestring);

				if(cJSON_GetObjectItem(pItem, "time1")->valueint)
					card.start_date = cJSON_GetObjectItem(pItem, "time1")->valueint;

				if(cJSON_GetObjectItem(pItem, "time2")->valueint)
					card.stop_date = cJSON_GetObjectItem(pItem, "time2")->valueint;


				x5b_app_req_card_tlv_json_detial(
						&card,
						cJSON_GetObjectItem(pItem, "img")->valuestring);

			}
			cJSON_Delete(pRoot);
			if(E_CMD_FROM_C(tlv->tag))
				x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_C);

			x5b_app_make_card_test();
			return OK;
		}
	}
	return ERROR;
}*/



/****************************************************************************/
/* C 模块人脸参数 */
static int _x5b_app_face_config_cjson(char **output, make_face_config_t *card)
{
	char* szJSON = NULL;
	cJSON* pRoot = cJSON_CreateObject();
	cJSON* pArray = cJSON_CreateArray();
	cJSON* pItem = cJSON_CreateObject();
	if(pRoot && pArray && pItem && card)
	{
		cJSON_AddItemToObject(pRoot, "face-config", pArray);
		cJSON_AddNumberToObject(pItem, "faceYawLeft", card->faceYawLeft);

		cJSON_AddNumberToObject(pItem, "faceYawRight", card->faceYawRight);

		cJSON_AddNumberToObject(pItem, "facePitchUp", card->facePitchUp);

		cJSON_AddNumberToObject(pItem, "facePitchDown", card->facePitchDown);


		cJSON_AddNumberToObject(pItem, "faceRecordWidth", card->faceRecordWidth);

		cJSON_AddNumberToObject(pItem, "faceRecordHeight", card->faceRecordHeight);

		cJSON_AddNumberToObject(pItem, "faceRecognizeWidth", card->faceRecognizeWidth);

		cJSON_AddNumberToObject(pItem, "faceRecognizeHeight", card->faceRecognizeHeight);


		cJSON_AddNumberToObject(pItem, "similarRecord", card->similarRecord);

		cJSON_AddNumberToObject(pItem, "similarRecognize", card->similarRecognize);

		cJSON_AddNumberToObject(pItem, "similarSecondRecognize", card->similarSecondRecognize);

		if(card->livenessSwitch)
			cJSON_AddTrueToObject(pItem, "livenessSwitch");
		else
			cJSON_AddFalseToObject(pItem, "livenessSwitch");
		cJSON_AddItemToArray(pArray, pItem);

		szJSON = cJSON_Print(pRoot);
		if(szJSON)
		{
			if(output)
				*output = strdup(szJSON);
			cjson_free(szJSON);
		}
		cJSON_Delete(pRoot);
		return OK;
	}
	return ERROR;
}

int x5b_app_face_config_json(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	char *output = NULL;
	make_face_config_t *card = (make_face_config_t *)info;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(info != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	_x5b_app_face_config_cjson(&output, card);
	if(output)
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_FACE_CONFIG), strlen(output), output);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "FACE CONFIG CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		free(output);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}



int x5b_app_make_card_test(void)
{
	//x5b_app_make_card_json(x5b_app_mgt, "admin", "123456", NULL, "1314245245353", "2019/04/05 15:66", "blacklist", E_CMD_TO_C);
	return OK;
}

/**************************************************************************/
/**************************************************************************/
static int x5b_get_load_respone_string(char *input, char *userid, int *faceid, int *res)
{
	char *us = NULL;
	cJSON* pItem = cJSON_Parse (input);
	cJSON* pj_tmp = NULL;
	if (pItem == NULL)
	{
		return ERROR;
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "userId");

	if (pj_tmp == NULL)
	{
		return ERROR;
	}
	us = pj_tmp->valuestring;
	if(us)
	{
		if(userid)
			strcpy(userid, us);

		if (faceid)
		{
			pj_tmp = cJSON_GetObjectItem (pItem, "faceId");
			if (pj_tmp == NULL)
			{
				return ERROR;
			}
			*faceid = pj_tmp->valueint;
			//*faceid = cJSON_GetObjectItem (pItem, "faceId")->valueint;
		}
		if (res)
		{
			pj_tmp = cJSON_GetObjectItem (pItem, "res");
			if (pj_tmp == NULL)
			{
				return ERROR;
			}
			*res = pj_tmp->valueint;
			//*res = cJSON_GetObjectItem (pItem, "res")->valueint;
		}
		return OK;
	}
/*
		if (cJSON_GetObjectItem (pItem, "userId"))
			printf ("userId:%s\r\n",
					cJSON_GetObjectItem (pItem, "userId")->valuestring);

		if (cJSON_GetObjectItem (pItem, "faceId"))
			printf ("faceId:%d\r\n",
					cJSON_GetObjectItem (pItem, "faceId")->valueint);

		if (cJSON_GetObjectItem (pItem, "res"))
			printf ("res:%d\r\n",
					cJSON_GetObjectItem (pItem, "res")->valueint);
	}
	return OK;
*/

	return ERROR;
}

int x5b_app_face_load_respone(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	char userid[64];
	char *temp = NULL;
	int faceid = 0, res = 0;
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	if(E_CMD_FROM_C(tlv->tag) && tlv->val.pval)
	{
		//return OK;
		memset(userid, 0, sizeof(userid));
		if(tlv->len >= 1024)
			return ERROR;
		temp = malloc(tlv->len);
		if(temp == NULL)
			return ERROR;
		memset(temp, 0, tlv->len);
		memcpy(temp, tlv->val.pval, tlv->len);
		//if(X5_B_ESP32_DEBUG(MSG) && X5_B_ESP32_DEBUG(WEB))
			zlog_debug(ZLOG_APP,"Recv make Face IMG respone len=%d val=%s\r\n", tlv->len, temp);
		//zlog_debug(ZLOG_APP,"tlv->len=%d ->%s", tlv->len, temp);
		if(x5b_get_load_respone_string(temp, userid, &faceid, &res) == OK)
		{
			if(res == 1)
			{
				x5b_user_del(NULL, userid);
			}
		}
		free(temp);
	}
	return OK;
}


/*int asdadasdsas()
{
	char *dsadasd = "{\"userid\":\"1166\",\"faceid\":1,\"res\":0}";//
	x5b_get_load_respone_string(dsadasd, NULL, NULL, NULL);
	return OK;
}*/
/*
 * [{"faceId":1,"picUrl":"/storage/emulated/0/x5-cm/webFace/faces/234234-8/234234-1.jpg","us
    erId":"234234"}]
 */
int x5b_app_json_test(void)
{
	char *aaa = "[{\"faceId\":1,\"picUrl\":\"/storage/emulated/0/x5-cm/webFace/faces/234234-8/234234-1.jpg\",\"userId\":\"234234\"}]";
	{
		{
			int iCount = 0, i = 0;
			cJSON* pArray = cJSON_Parse(aaa);
			if(pArray == NULL)
			{
				return -1;
			}
/*			cJSON* pArray = cJSON_GetObjectItem(pRoot, NULL);
			if (NULL == pArray)
			{
				return -1;
			}*/
			iCount = cJSON_GetArraySize(pArray);
			for (; i < iCount; ++i)
			{
				cJSON* pItem = cJSON_GetArrayItem(pArray, i);
				if (NULL == pItem)
				{
					continue;
				}

				//if(cJSON_GetObjectItem(pItem, "faceId")->valueint)
					printf("faceId:%d\r\n", cJSON_GetObjectItem(pItem, "faceId")->valueint);

				if(cJSON_GetObjectItem(pItem, "picUrl")->valuestring)
					printf("picUrl:%s\r\n", cJSON_GetObjectItem(pItem, "picUrl")->valuestring);

				if(cJSON_GetObjectItem(pItem, "userId")->valuestring)
					printf("userId:%s\r\n", cJSON_GetObjectItem(pItem, "userId")->valuestring);
			}
			return OK;
		}
	}
	return ERROR;
}
/**************************************************************************/
/********************************* Huifu **********************************/
int x5b_app_device_json(x5b_app_mgt_t *app, char *device,
						char *address, char *dire, int to)
{
	int len = 0;
	char output[512];
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);

	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	memset(output, 0, sizeof(output));
	if(device || address ||dire)
	{
		char *szJSON = NULL;
		cJSON* pRoot = cJSON_CreateObject();
		if(pRoot)
		{
			if(device && strlen(device))
				cJSON_AddStringToObject(pRoot, "name", device);
			if(address && strlen(address))
				cJSON_AddStringToObject(pRoot, "address", address);
			if(dire && strlen(dire))
				cJSON_AddStringToObject(pRoot, "dir", dire);

			szJSON = cJSON_Print(pRoot);
			if(szJSON)
			{
				strcpy(output, szJSON);
				//strcpy(output, strdup(szJSON));
				cjson_free(szJSON);
				cJSON_Delete(pRoot);
			}
			else
			{
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				cJSON_Delete(pRoot);
				return ERROR;
			}
		}
		else
		{
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return ERROR;
		}
	}
	if(strlen(output))
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_DEVICE_OPT), strlen(output), output);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "DEVICE CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}
#ifdef PL_VOIP_MODULE
#ifdef PL_OSIP_MODULE
static int x5b_sip_load_from_json(char *input)
{
	char *us = NULL;
	cJSON* pItem = cJSON_Parse (input);
	cJSON* pj_tmp = NULL;
	if (pItem == NULL)
	{
		return ERROR;
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "phone");
	if (pj_tmp == NULL)
	{
		return ERROR;
	}
	us = pj_tmp->valuestring;
	if(us)
	{
		voip_sip_local_number_set_api(us, FALSE);
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "username");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			voip_sip_user_set_api(us, FALSE);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "password");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			voip_sip_password_set_api(us, FALSE);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "sip-address");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			u_int16 port = 0;
			u_int32 ip = ntohl(inet_addr(us));
			voip_sip_server_get_api(NULL, &port, FALSE);
			voip_sip_server_set_api(ip, port, FALSE);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "sip-port");
	if (pj_tmp)
	{
		if(pj_tmp->valueint)
		{
			u_int32 ip = ntohl(inet_addr(us));
			voip_sip_server_get_api(&ip, NULL, FALSE);
			voip_sip_server_set_api(ip, pj_tmp->valueint, FALSE);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "codec");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			voip_sip_payload_name_set_api(us);
/*			if(voip_sip_payload_index(us) >= 0)
			{
				extern int voip_sip_payload_name_set_api(char * value);
				extern int voip_sip_get_payload_index(void);
				extern int voip_sip_payload_index(char *name);
				extern char * voip_sip_payload_name(int index);
				extern char * voip_sip_payload_rtpmap(int index);
				extern int voip_sip_payload_ptime(int index);
				//voip_stream_payload_api(tmp, -1);
				sip->payload = voip_sip_payload_index(us);
				memset(sip->payload_name, 0, sizeof(sip->payload_name));
				strcpy(sip->payload_name, strlwr(tmp));
			}*/
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "dtmf");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			if(strstr(us, "2833"))
				voip_sip_dtmf_set_api(VOIP_SIP_RFC2833);
			else if(strstr(us, "info") || strstr(us, "INFO"))
				voip_sip_dtmf_set_api(VOIP_SIP_INFO);
			else if(strstr(us, "INBAND") || strstr(us, "inband"))
				voip_sip_dtmf_set_api(VOIP_SIP_INBAND);
		}
	}
	voip_osip_restart();
	vty_execute_shell("write memory");
	return OK;
}
#endif
#ifdef PL_PJSIP_MODULE
static int x5b_sip_load_from_json(char *input)
{
	char *us = NULL;
	s_int8 user[128];
	s_int8 pass[128];
	cJSON* pItem = cJSON_Parse (input);
	cJSON* pj_tmp = NULL;
	if (pItem == NULL)
	{
		return ERROR;
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "phone");
	if (pj_tmp == NULL)
	{
		return ERROR;
	}
	us = pj_tmp->valuestring;
/*	if(us)
	{
		pl_pjsip_local_number_set_api(us, FALSE);
	}*/
	pj_tmp = cJSON_GetObjectItem (pItem, "username");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			memset(user, 0, sizeof(user));
			memset(pass, 0, sizeof(pass));
			pl_pjsip_username_get_api(user, pass, FALSE);
			pl_pjsip_username_set_api(us, pass, FALSE);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "password");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			memset(user, 0, sizeof(user));
			memset(pass, 0, sizeof(pass));
			pl_pjsip_username_get_api(user, pass, FALSE);
			pl_pjsip_username_set_api(user, us, FALSE);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "sip-address");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			u_int16 port = 0;
			//u_int32 ip = ntohl(inet_addr(us));
			memset(user, 0, sizeof(user));
			pl_pjsip_server_get_api(user, &port, FALSE);
			pl_pjsip_server_set_api(us, port, FALSE);

/*			voip_sip_server_get_api(NULL, &port, FALSE);
			voip_sip_server_set_api(ip, port, FALSE);*/
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "sip-port");
	if (pj_tmp)
	{
		if(pj_tmp->valueint)
		{
			u_int16 port = 0;
			memset(user, 0, sizeof(user));
			pl_pjsip_server_get_api(user, &port, FALSE);
			pl_pjsip_server_set_api(user, pj_tmp->valueint, FALSE);

/*			u_int32 ip = ntohl(inet_addr(us));
			voip_sip_server_get_api(&ip, NULL, FALSE);
			voip_sip_server_set_api(ip, pj_tmp->valueint, FALSE);*/
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "codec");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			pl_pjsip_codec_default_set_api(us, 0);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "dtmf");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			if(strstr(us, "2833"))
				pl_pjsip_dtmf_set_api(PJSIP_DTMF_RFC2833);
			else if(strstr(us, "info") || strstr(us, "INFO"))
				pl_pjsip_dtmf_set_api(PJSIP_DTMF_INFO);
			else if(strstr(us, "INBAND") || strstr(us, "inband"))
				pl_pjsip_dtmf_set_api(PJSIP_DTMF_INBAND);
		}
	}
	//voip_osip_restart();
	vty_execute_shell("write memory");
	return OK;
}
#endif
#endif

int x5b_app_sip_load_respone(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	char *temp = NULL;
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	if(E_CMD_FROM_C(tlv->tag) && tlv->val.pval)
	{
		if(tlv->len >= 1024)
			return ERROR;
		temp = malloc(tlv->len);
		if(temp == NULL)
			return ERROR;
		memset(temp, 0, tlv->len);
		memcpy(temp, tlv->val.pval, tlv->len);
		//if(X5_B_ESP32_DEBUG(MSG) && X5_B_ESP32_DEBUG(WEB))
			zlog_debug(ZLOG_APP,"Recv make Face IMG respone len=%d val=%s\r\n", tlv->len, temp);
		//zlog_debug(ZLOG_APP,"tlv->len=%d ->%s", tlv->len, temp);
#ifdef PL_VOIP_MODULE
		if(x5b_sip_load_from_json(temp) == OK)
		{
		}
#endif
		free(temp);
	}
	return OK;
}


int x5b_app_call_respone(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	char *temp = NULL;
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	if(E_CMD_FROM_C(tlv->tag) && tlv->val.pval)
	{
		if(tlv->len >= 1024)
			return ERROR;
		temp = malloc(tlv->len + 1);
		if(temp == NULL)
			return ERROR;
		memset(temp, 0, tlv->len + 1);
		memcpy(temp, tlv->val.pval, tlv->len);
		temp[tlv->len] = '\0';
		zlog_debug(ZLOG_APP,"Recv Phone Call IMG respone len=%d val=%s\r\n", tlv->len, temp);
		x5b_app_start_call_phone(TRUE, temp);
		free(temp);
	}
	return OK;
}
