/*
 * x5_b_test.c
 *
 *  Created on: 2019年7月23日
 *      Author: DELL
 */

#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "os_util.h"
#include "os_socket.h"
#include "eloop.h"
#include "ubus_sync.h"

#include "x5_b_cmd.h"
#include "x5_b_app.h"
#include "x5_b_util.h"
#include "x5_b_test.h"

static x5b_app_mgt_t *mgt_test = NULL;
static int x5b_app_test_read_eloop(struct eloop *eloop);

static int x5b_app_test_send_msg_without_ack(x5b_app_mgt_t *mgt)
{
	ospl_uint32 len = 0;
	zassert(mgt != NULL);
	zassert(mgt->app != NULL);
	zassert(mgt->app->address != 0);
	{
		//if(X5_B_ESP32_DEBUG(SEND))
/*		{
			zlog_debug(MODULE_APP, "MSG to %s:%d %d byte (seqnum=%d)", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen, mgt->app->seqnum);
			//if(X5_B_ESP32_DEBUG(HEX))
			//	x5b_app_hex_debug(mgt, "SEND", 0);
		}*/
		if(mgt->w_fd)
			len = sock_client_write(mgt->w_fd, inet_address(mgt->app->address), mgt->app->remote_port,
					mgt->app->sbuf, mgt->app->slen);
	}
	//x5b_app_statistics(mgt, 1, mgt->app->id);
	mgt->app->seqnum++;
	return len ? OK : ERROR;
}


static int x5b_app_test_send_msg(x5b_app_mgt_t *mgt)
{
	ospl_uint32 len = 0;
	zassert(mgt != NULL);
	zassert(mgt->app != NULL);
	zassert(mgt->app->address != 0);
	{
		if(mgt->w_fd)
			len = sock_client_write(mgt->w_fd, inet_address(mgt->app->address), mgt->app->remote_port,
					mgt->app->sbuf, mgt->app->slen);
	}
	if(mgt->sync_ack)
	{
		{
			int ret = 0;
			if(mgt->r_thread)
			{
				eloop_cancel(mgt->r_thread);
				mgt->r_thread = NULL;
			}
			//zlog_debug(MODULE_APP, "----wait ack 1" );
			ret = x5b_app_read_msg_timeout(mgt, X5B_APP_WAITING_TIMEOUT, NULL, 0);
			if(!mgt->upgrade && mgt->r_thread == NULL)
			{
				mgt->r_thread = eloop_add_read(mgt->master, x5b_app_test_read_eloop, mgt, mgt->r_fd);
			}
			mgt->app->seqnum++;
			return ret;
		}
		mgt->app->seqnum++;
		return OK;
	}
	mgt->app->seqnum++;
	return len ? OK : ERROR;
}


static int x5b_app_test_ack_api(x5b_app_mgt_t *mgt, ospl_uint8 seqnum, int to)
{
	int ret = 0;
	ospl_uint8 val = (ospl_uint8)seqnum;
	ospl_uint32 len = 0;
	zassert(mgt != NULL);
	mgt->app = &mgt->app_c;
	if(!mgt->app->reg_state)
	{
		//if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(MODULE_APP, "Remote is Not Register");
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		//if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(MODULE_APP, "ACK MSG Can not send, Unknown Remote IP Address");
		return ERROR;
	}
	x5b_app_hdr_make(mgt);
	//x5b_app_ack_make(mgt, seqnum);

	len = os_tlv_set_integer(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_C, E_CMD_BASE, E_CMD_ACK), E_CMD_ACK_LEN, &val);
	mgt->app->offset += len;

	x5b_app_crc_make(mgt);
	//if(X5_B_ESP32_DEBUG(EVENT))
	//zlog_debug(MODULE_APP, "ACK MSG to %s:%d %d byte(seqnum=%d)", inet_address(mgt->app->address),
	//			mgt->app->remote_port, mgt->app->slen, seqnum);
	ret = x5b_app_test_send_msg_without_ack(mgt);
	return ret;
}

static int x5b_app_test_keepalive_send(x5b_app_mgt_t *mgt)
{
	ospl_uint32 len = 0;
	zassert(mgt != NULL);
	mgt->app = &mgt->app_c;
	if(!mgt->app->reg_state)
	{
		//if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(MODULE_APP, "Remote is Not Register");
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		//if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(MODULE_APP, "Keepalive MSG Can not send, Unknown Remote IP Address");
		return ERROR;
	}

	x5b_app_hdr_make(mgt);

	len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_C, E_CMD_BASE, E_CMD_KEEPALIVE), 0);

	mgt->app->offset += len;
	x5b_app_crc_make(mgt);

	len = x5b_app_test_send_msg_without_ack(mgt);

	return len;
}

static int x5b_app_test_register_send(x5b_app_mgt_t *mgt)
{
	ospl_uint32 len = 0, id = X5B_APP_MODULE_ID_C;
	zassert(mgt != NULL);
	mgt->app = &mgt->app_c;

	if(mgt->app->address == 0)
	{
		//if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(MODULE_APP, "Keepalive MSG Can not send, Unknown Remote IP Address");
		return ERROR;
	}

	x5b_app_hdr_make(mgt);

	len = os_tlv_set_integer(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_C, E_CMD_BASE, E_CMD_REGISTER), 4, &id);

	mgt->app->offset += len;
	x5b_app_crc_make(mgt);

	len = x5b_app_test_send_msg_without_ack(mgt);

	return len;
}

static int x5b_app_test_version_send(x5b_app_mgt_t *mgt)
{
	ospl_uint32 len = 0;
	zassert(mgt != NULL);
	mgt->app = &mgt->app_c;
	if(!mgt->app->reg_state)
	{
		//if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(MODULE_APP, "Remote is Not Register");
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		//if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(MODULE_APP, "Keepalive MSG Can not send, Unknown Remote IP Address");
		return ERROR;
	}

	x5b_app_hdr_make(mgt);

	len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_C, E_CMD_STATUS, E_CMD_ACK_VERSION), strlen("X5CM-C-MODULE-UT-V0.1"), "X5CM-C-MODULE-UT-V0.1");

	mgt->app->offset += len;
	x5b_app_crc_make(mgt);

	len = x5b_app_test_send_msg_without_ack(mgt);

	return len;
}


static int x5b_app_test_read_handle(x5b_app_mgt_t *mgt)
{
	ospl_uint32 len = 0, offset = 0, ack = 0;
	os_tlv_t tlv;
	zassert(mgt != NULL);
	{
		offset += sizeof(x5b_app_hdr_t);
	}
	zassert(mgt != NULL);
	len = x5b_app_read_chk_handle(mgt);
	while(len > 0)
	{
		memset(&tlv, 0, sizeof(os_tlv_t));
			offset += os_tlv_get(mgt->buf + offset, &tlv);
		if(tlv.len >= X5B_APP_TLV_DEFAULT)
			continue;

		if(/*ack == 0 &&*/
				(E_CMD_GET(tlv.tag) != E_CMD_REGISTER) &&
				(E_CMD_GET(tlv.tag) != E_CMD_ACK) &&
				(E_CMD_GET(tlv.tag) != E_CMD_KEEPALIVE) )
		{
			if(E_CMD_FROM_B(tlv.tag))
			{
				if((E_CMD_GET(tlv.tag) != E_CMD_CALL_RESULT))
					x5b_app_test_ack_api(mgt, mgt->seqnum, E_CMD_TO_B);
			}
			ack = 1;
		}

		switch(E_CMD_TYPE_GET(tlv.tag))
		{
		case E_CMD_BASE:
			{
				switch(E_CMD_GET(tlv.tag))
				{
					case E_CMD_REGISTER_OK:
						mgt->app = &mgt->app_c;
						mgt->app->reg_state = ospl_true;
						zlog_debug(MODULE_APP, "C Module Register OK and start up");
						break;
					case E_CMD_OPEN:
						zlog_debug(MODULE_APP, "C Module Recv OPEN Door CMD");
						break;
					case E_CMD_KEEPALIVE:
						break;
					default:
						break;
				}
			}
			break;
		case E_CMD_SET:
		{
			switch(E_CMD_GET(tlv.tag))
			{
			case E_CMD_FACE_CONFIG:
				zlog_debug(MODULE_APP, "C Module Recv FACE CONFIG CMD");
				break;
			case E_CMD_DEVICE_OPT:
				zlog_debug(MODULE_APP, "C Module Recv DEVICE OPT CMD");
				break;
			case E_CMD_SYSTEM_CONFIG:
				zlog_debug(MODULE_APP, "C Module Recv SYSTEM CONFIG CMD");
				break;
			default:
				break;
			}
		}
			break;
		case E_CMD_CALL:
		{
			switch(E_CMD_GET(tlv.tag))
			{
			case E_CMD_CALL_RESULT:
				zlog_debug(MODULE_APP, "C Module Recv CALL STATUS CMD");
				break;
			default:
				break;
			}
		}
			break;
		case E_CMD_STATUS:
		{
			switch(E_CMD_GET(tlv.tag))
			{
			case E_CMD_NETSTATUS:
				zlog_debug(MODULE_APP, "C Module Recv NET STATUS CMD");
				break;

			case E_CMD_ACK_STATUS:
				zlog_debug(MODULE_APP, "C Module Recv REQ NET STATUS CMD");
				break;
			case E_CMD_REG_STATUS:
				zlog_debug(MODULE_APP, "C Module Recv SIP REGISTER STATUS");
				break;
			case E_CMD_ACK_REGISTER:
				zlog_debug(MODULE_APP, "C Module Recv SIP REGISTER CMD");
				break;

			case E_CMD_REQ_VERSION:
				zlog_debug(MODULE_APP, "C Module Recv REQ Version CMD");
				x5b_app_test_version_send(mgt);
				break;
			default:
				break;
			}
		}
			break;
		default:
			//if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(MODULE_APP, "TAG HDR = 0x%x(0x%x) (seqnum=%d)", tlv.tag, E_CMD_TYPE_GET(tlv.tag), mgt->seqnum);
			break;
		}
		len -= offset;
	}
	return OK;
}










static int x5b_app_test_timer_eloop(struct eloop *eloop)
{
	zassert(eloop != NULL);
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);

	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	mgt_test->app_c.t_thread = NULL;
	//zlog_debug(MODULE_APP, "x5b_app_test_timer_eloop X5CM:%s reg-state:%d", x5b_app_mode_X5CM() ? "ospl_true":"ospl_false", mgt->app_c.reg_state);
	if(x5b_app_mode_X5CM())
	{
		if(!mgt->app_c.reg_state)
		{
			x5b_app_test_register_send(mgt_test);
		}
		else
		{
			x5b_app_test_keepalive_send(mgt_test);
		}
		mgt_test->app_c.t_thread = eloop_add_timer(mgt_test->master, x5b_app_test_timer_eloop, mgt_test, mgt_test->app_c.interval + 5);
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}

static int x5b_app_test_read_eloop(struct eloop *eloop)
{
	int sock_len, len = 0;

	zassert(eloop != NULL);
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);
	int sock = ELOOP_FD(eloop);

	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	mgt->r_thread = NULL;
	memset(mgt->buf, 0, sizeof(mgt->buf));

	sock_len = sizeof(struct sockaddr_in);
	len = recvfrom(sock, mgt->buf, sizeof(mgt->buf), 0, &mgt->from, &sock_len);
	if (len <= 0)
	{
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				//return 0;
				zlog_err(MODULE_APP, "RECV mgt on socket (%s)", strerror(errno));
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				return OK;
			}
		}
	}
	else
	{
		if(len > X5B_APP_BUF_DEFAULT)
		{
			zlog_err(MODULE_APP, "Recv buf size is too big on socket (%d byte)", len);
			if(mgt->r_thread == NULL)
				mgt->r_thread = eloop_add_read(mgt->master, x5b_app_test_read_eloop, mgt, sock);
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return OK;
		}
		mgt->len = len;
		//if(X5_B_ESP32_DEBUG(RECV))
/*		{
			zlog_debug(MODULE_APP, "MSG from %s:%d %d byte", inet_address(ntohl(mgt->from.sin_addr.s_addr)),
					ntohs(mgt->from.sin_port), mgt->len);
		}*/

		x5b_app_test_read_handle(mgt);

	}
	if(mgt->r_thread == NULL)
		mgt->r_thread = eloop_add_read(mgt->master, x5b_app_test_read_eloop, mgt, sock);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}

static int x5b_app_test_socket_init()
{
	zassert(mgt_test != NULL);
	if(mgt_test->r_fd > 0)
		return OK;
	int fd = sock_create(ospl_false);
	if(fd)
	{
		if(mgt_test->local_port == 0)
			mgt_test->local_port = X5B_APP_REMOTE_PORT_DEFAULT;
		if(sock_bind(fd, mgt_test->local_address, mgt_test->local_port) == OK)
		{
			mgt_test->r_fd = fd;
			mgt_test->w_fd = fd;

			setsockopt_so_recvbuf (fd, 8192);
			setsockopt_so_sendbuf (fd, 8192);

			mgt_test->r_thread = eloop_add_read(mgt_test->master, x5b_app_test_read_eloop, mgt_test, fd);
			mgt_test->app_c.t_thread = eloop_add_timer(mgt_test->master, x5b_app_test_timer_eloop, mgt_test, mgt_test->app_c.interval + 5);
			return OK;
		}
		else
		{
			zlog_err(MODULE_APP, "Can not bind UDP socket(:%s)", strerror(errno));
		}
	}
	zlog_err(MODULE_APP, "Can not Create UDP socket(:%s)", strerror(errno));
	return ERROR;
}

static int x5b_app_test_socket_exit(x5b_app_mgt_t *mgt)
{
	if(mgt && mgt->r_thread)
	{
		eloop_cancel(mgt->r_thread);
		mgt->r_thread = NULL;
	}
	if(mgt && mgt->reset_thread)
	{
		eloop_cancel(mgt->reset_thread);
		mgt->reset_thread = NULL;
	}
	if(mgt && mgt->app_c.t_thread)
	{
		eloop_cancel(mgt->app_c.t_thread);
		mgt->app_c.t_thread = NULL;
	}
	if(mgt)
	{
		if(mgt->r_fd)
			close(mgt->r_fd);
		mgt->r_fd = 0;
		memset(mgt->buf, 0, sizeof(mgt->buf));
		mgt->r_fd = 0;
		mgt->w_fd = 0;
		mgt->app_a.statistics.tx_packet = 0;
		mgt->app_a.statistics.rx_packet = 0;

		mgt->app_c.statistics.tx_packet = 0;
		mgt->app_c.statistics.rx_packet = 0;
		return OK;
	}
	return ERROR;
}



static int x5b_app_test_module_init(char *local, ospl_uint16 port)
{
	if(mgt_test == NULL)
	{
		mgt_test = malloc(sizeof(x5b_app_mgt_t));
		memset(mgt_test, 0, sizeof(x5b_app_mgt_t));

		if(master_eloop[MODULE_APP + 1] == NULL)
			master_eloop[MODULE_APP + 1] = eloop_master_module_create(MODULE_APP + 1);

		mgt_test->master = master_eloop[MODULE_APP + 1];

		mgt_test->mutex = os_mutex_init();

		if(local)
			mgt_test->local_address = strdup(local);
		else
			mgt_test->local_address = NULL;
		mgt_test->local_port = X5B_APP_REMOTE_PORT_DEFAULT + 1;
		mgt_test->app_c.interval = X5B_APP_INTERVAL_DEFAULT;
		mgt_test->app_c.keep_cnt = X5B_APP_INTERVAL_CNT_DEFAULT;
		mgt_test->app_c.remote_port = X5B_APP_LOCAL_PORT_DEFAULT;

		mgt_test->app_c.address = ntohl(inet_addr("127.0.0.1"));
		mgt_test->app_c.priv = mgt_test;
		if(x5b_app_mgt)
		{
			mgt_test->app_a.remote_port = x5b_app_mgt->app_c.remote_port;
			x5b_app_mgt->app_c.remote_port = X5B_APP_REMOTE_PORT_DEFAULT + 1;
			mgt_test->app_a.address = x5b_app_mgt->app_c.address;
			x5b_app_mgt->app_c.address = ntohl(inet_addr("127.0.0.1"));
		}
		mgt_test->sync_ack = ospl_true;
		x5b_app_test_socket_init();
		//mgt_test->debug = X5_B_ESP32_DEBUG_TIME | X5_B_ESP32_DEBUG_EVENT;
	}
	return OK;
}

static int x5b_app_test_module_exit()
{
	if(mgt_test)
	{
		x5b_app_test_socket_exit(mgt_test);
		if(mgt_test->local_address)
			free(mgt_test->local_address);
		if(mgt_test->mutex)
		{
			os_mutex_exit(mgt_test->mutex);
			mgt_test->mutex = NULL;
		}
		if(x5b_app_mgt)
		{
			x5b_app_mgt->app_c.remote_port = mgt_test->app_a.remote_port;
			x5b_app_mgt->app_c.address = mgt_test->app_a.address;
		}
		free(mgt_test);
		mgt_test = NULL;
	}
	return OK;
}

static int x5b_app_test_mgt_task(void *argv)
{
	zassert(argv != NULL);
	x5b_app_mgt_t *mgt = (x5b_app_mgt_t *)argv;
	zassert(mgt != NULL);
	module_setup_task(MODULE_APP + 1, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	if(!mgt->enable)
	{
		os_sleep(5);
	}
	//x5b_app_state_load(mgt);
	eloop_start_running(master_eloop[MODULE_APP + 1], MODULE_APP + 1);
	return OK;
}


static int x5b_app_test_task_init (x5b_app_mgt_t *mgt)
{
	zassert(mgt != NULL);
	if(master_eloop[MODULE_APP + 1] == NULL)
		master_eloop[MODULE_APP + 1] = eloop_master_module_create(MODULE_APP + 1);

	mgt->enable = ospl_true;
	mgt->task_id = os_task_create("appTest", OS_TASK_DEFAULT_PRIORITY,
	               0, x5b_app_test_mgt_task, mgt, OS_TASK_DEFAULT_STACK);
	if(mgt->task_id)
		return OK;
	return ERROR;
}


int x5b_app_test_start()
{
	if(mgt_test != NULL)
		return OK;
	if(mgt_test == NULL)
	{
		x5b_app_test_module_init(NULL, 0);
		//x5b_app_test_socket_init();
		x5b_app_test_task_init(mgt_test);
	}
	return OK;
}

int x5b_app_test_isstart()
{
	if(mgt_test != NULL && mgt_test->app_c.reg_state)
		return OK;
	return ERROR;
}

int x5b_app_test_stop()
{
	if(mgt_test && mgt_test->task_id)
		os_task_destroy(mgt_test->task_id);
	x5b_app_test_module_exit();
	return OK;
}

/********************************************************************************/
static int x5b_app_test_call_send(x5b_app_mgt_t *mgt, char *jsonstr, int jsonlen)
{
	ospl_uint32 len = 0;
	zassert(mgt != NULL);
	mgt->app = &mgt->app_c;

	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(MODULE_APP, "Keepalive MSG Can not send, Unknown Remote IP Address");
		return ERROR;
	}
	x5b_app_hdr_make(mgt);

	if(jsonstr && jsonlen)
	{
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_C, E_CMD_CALL, E_CMD_START_CALL_LIST), jsonlen, jsonstr);

		mgt->app->offset += len;
		x5b_app_crc_make(mgt);

		len = x5b_app_test_send_msg_without_ack(mgt);
	}
	else if(jsonstr && jsonlen == 0)
	{
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_C, E_CMD_CALL, E_CMD_START_CALL_PHONE), strlen(jsonstr), jsonstr);

		mgt->app->offset += len;
		x5b_app_crc_make(mgt);

		len = x5b_app_test_send_msg(mgt);
	}
	else
		len = -1;
	return len;
}
/********************************************************************************/
static int x5b_app_test_call_json_one(char *jsonstr, char *num)
{
	char *szJSON = NULL;
	cJSON* pRoot = cJSON_CreateObject();
	if(pRoot && num)
	{
		cJSON_AddStringToObject(pRoot, "use", "admin");
		cJSON_AddStringToObject(pRoot, "ID", "123456");

		cJSON_AddStringToObject(pRoot, "room", "2002");
		cJSON_AddStringToObject(pRoot, "phone", num);

		szJSON = cJSON_Print(pRoot);
		if(szJSON)
		{
			strcpy(jsonstr, szJSON);
			cjson_free(szJSON);
			szJSON = NULL;
			cJSON_Delete(pRoot);
			pRoot = NULL;
			return OK;
		}
		else
		{
			cJSON_Delete(pRoot);
			pRoot = NULL;
			return ERROR;
		}
	}
	if(pRoot)
	{
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	if(szJSON)
	{
		cjson_free(szJSON);
		szJSON = NULL;
	}
	return ERROR;
}
static int x5b_app_test_call_json_more(char *jsonstr, char *phone[], int num)
{
	ospl_uint32 i = 0;
	char *szJSON = NULL;
	cJSON* pRoot = cJSON_CreateArray();
	if(pRoot && num)
	{
		char usename[64], useid[64];
		for(i = 0; i < num; i++)
		{
			cJSON* pJsonsub = cJSON_CreateObject();
			if(phone[i] && pJsonsub)
			{
				cJSON_AddItemToArray(pRoot, pJsonsub);
				memset(usename, 0, sizeof(usename));
				memset(useid, 0, sizeof(useid));
				sprintf(usename, "admin-%d", i);
				sprintf(useid, "12345%d", i+1);

				cJSON_AddStringToObject(pJsonsub, "use", usename);
				cJSON_AddStringToObject(pJsonsub, "ID", useid);

				cJSON_AddStringToObject(pJsonsub, "room", "2002");
				cJSON_AddStringToObject(pJsonsub, "phone", phone[i]);
			}
		}
		szJSON = cJSON_Print(pRoot);
		if(szJSON)
		{
			strcpy(jsonstr, szJSON);
			cjson_free(szJSON);
			szJSON = NULL;
			cJSON_Delete(pRoot);
			pRoot = NULL;
			return OK;
		}
		else
		{
			cJSON_Delete(pRoot);
			pRoot = NULL;
			return ERROR;
		}
	}
	if(pRoot)
	{
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	if(szJSON)
	{
		cjson_free(szJSON);
		szJSON = NULL;
	}
	return ERROR;
}
/********************************************************************************/

int x5_b_app_test_call_phone(char *num)
{
	char output[512];
	if(x5b_app_test_isstart() == ERROR)
		return ERROR;
	memset(output, 0, sizeof(output));
	if(x5b_app_test_call_json_one(output, num) == OK)
	{
		x5b_app_test_call_send(mgt_test, output, strlen(output));
	}
	return OK;
}

int x5_b_app_test_call_phonenum(char *num)
{
	char output[1024];
	char *callnum[2] = {num, NULL};
	if(x5b_app_test_isstart() == ERROR)
		return ERROR;
	memset(output, 0, sizeof(output));
	if(x5b_app_test_call_json_more(output, callnum, 1) == OK)
	{
		x5b_app_test_call_send(mgt_test, output, strlen(output));
	}
	return OK;
}

int x5_b_app_test_call_list()
{
	char output[1024];
	//char *callnum[] = {"1003","1005","1004"};
	char *callnum[] = {"1002","1003","1005"};
	if(x5b_app_test_isstart() == ERROR)
		return ERROR;
	memset(output, 0, sizeof(output));
	if(x5b_app_test_call_json_more(output, callnum, 3) == OK)
	{
		x5b_app_test_call_send(mgt_test, output, strlen(output));
	}
	return OK;
}
