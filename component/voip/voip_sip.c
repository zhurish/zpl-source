/*
 * voip_sip.c
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */


#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "network.h"
#include "vty.h"

#include "os_util.h"
#include "os_socket.h"

#include "voip_def.h"
#include "voip_event.h"
#include "voip_socket.h"
#include "voip_sip.h"
#include "voip_stream.h"

#ifdef SIP_CTL_MSGQ
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

voip_sip_t voip_sip_config;
static voip_sip_ctl_t voip_sip_ctl;
#ifdef SIP_CTL_SOCKET
static int voip_sip_socket_init(voip_sip_ctl_t *sipctl);
static int voip_sip_socket_exit(voip_sip_ctl_t *sipctl);
static int voip_sip_read_handle(voip_sip_ctl_t *sipctl, char *buf, int len);
#endif

#ifdef SIP_CTL_MSGQ
static int sip_msgq_create_read(int msgKey);
static int sip_msgq_create_write(int msgKey);
/*static int sip_msgq_fflush(int msgId);
static int sip_msgq_buffsize(int msgId, int qBytes);*/
static int sip_msgq_delete(int msgId);
static int sip_msgq_recv(int msgId, void* pMsgHdr, int uiMaxBytes, int msgType, int wait);
static int sip_msgq_send(int msgQId, char* pMsg, int len);
static int voip_sip_msgq_init(voip_sip_ctl_t *sipctl);
static int voip_sip_msgq_exit(voip_sip_ctl_t *sipctl);
static int sip_msgq_task_init(voip_sip_ctl_t *sipctl);
static int sip_msgq_task_exit(voip_sip_ctl_t *sipctl);
#endif

static int voip_sip_config_default(voip_sip_t *sip)
{
	sip->sip_enable		= SIP_ENABLE_DEFAULT;
	//sip->sip_server;					//SIP��������ַ
	//sip->sip_server_sec;				//SIP���÷�������ַ
	//sip->sip_proxy_server;			//�����������ַ
	//sip->sip_proxy_server_sec;		//��ѡ�����������ַ

	sip->sip_port		= SIP_PORT_DEFAULT;					//SIP�������˿ں�
	sip->sip_port_sec	= SIP_PORT_SEC_DEFAULT;				//SIP���÷������˿ں�
	sip->sip_proxy_port	= SIP_PROXY_PORT_DEFAULT;				//����������˿ں�
	sip->sip_proxy_port_sec = SIP_PROXY_PORT_SEC_DEFAULT;			//��ѡ����������˿ں�

	sip->sip_local_port = SIP_PORT_DEFAULT;				//���ñ���SIP�������˿ں�

	sip->sip_time_sync = SIP_TIME_DEFAULT;				//ʱ���Ƿ�ͬ��

	sip->sip_ring	= SIP_RING_DEFAULT;					//��������

	sip->sip_register_interval = SIP_REGINTER_DEFAULT;		//����ע������

	strcpy(sip->sip_hostpart, SIP_HOSTPART_DEFAULT);				//����hostpart
	sip->sip_interval = SIP_INTERVAL_DEFAULT;				//��������
	sip->sip_100_rel = SIP_100_REL_DEFAULT;				//����100rel�Ƿ�ǿ��ʹ��
	sip->sip_dis_name = SIP_DIS_NAME_DEFAULT;				//����display name
	//sip->sip_local_number[SIP_NUMBER_MAX];	//���ñ��غ���
	//sip->sip_user[SIP_DATA_MAX];
	//sip->sip_password[SIP_DATA_MAX];
	strcpy(sip->sip_local_number, SIP_PHONE_DEFAULT);
	strcpy(sip->sip_user, SIP_USERNAME_DEFAULT);
	strcpy(sip->sip_password, SIP_PASSWORD_DEFAULT);
	strcpy(sip->sip_realm, SIP_HOSTPART_DEFAULT);					//����realm
	strcpy(sip->sip_dialplan, SIP_DIALPLAN_DEFAULT);				//����dialplan
	sip->sip_encrypt = SIP_ENCRYPT_DEFAULT;				//����ע������

	return OK;
}

int voip_sip_module_init()
{
	os_memset(&voip_sip_config, 0, sizeof(voip_sip_config));
	voip_sip_config_default(&voip_sip_config);
	return OK;
}

int voip_sip_module_exit()
{
	os_memset(&voip_sip_config, 0, sizeof(voip_sip_config));
	voip_sip_config_default(&voip_sip_config);
	return OK;
}

int voip_sip_enable(BOOL enable, u_int16 port)
{
	voip_sip_config.sip_enable = enable;
	voip_sip_config.sip_local_port = port ? port:SIP_PORT_DEFAULT;
	return OK;
}



int voip_sip_server_set_api(u_int32 ip, u_int16 port, BOOL sec)
{
	if(sec)
	{
		voip_sip_config.sip_server_sec = ip;
		voip_sip_config.sip_port_sec = port ? port:SIP_PORT_SEC_DEFAULT;
	}
	else
	{
		voip_sip_config.sip_server = ip;
		voip_sip_config.sip_port = port ? port:SIP_PORT_DEFAULT;
		voip_sip_config_update_api(&voip_sip_config);
	}
	return OK;
}

int voip_sip_proxy_server_set_api(u_int32 ip, u_int16 port, BOOL sec)
{
	if(sec)
	{
		voip_sip_config.sip_proxy_server_sec = ip;
		voip_sip_config.sip_proxy_port_sec = port ? port:SIP_PROXY_PORT_SEC_DEFAULT;
	}
	else
	{
		voip_sip_config.sip_proxy_server = ip;
		voip_sip_config.sip_proxy_port = port ? port:SIP_PROXY_PORT_DEFAULT;
	}
	return OK;
}

int voip_sip_time_syne_set_api(BOOL enable)
{
	voip_sip_config.sip_time_sync = enable;
	return OK;
}

int voip_sip_ring_set_api(u_int16 value)
{
	voip_sip_config.sip_ring = value ? value:SIP_RING_DEFAULT;
	return OK;
}

int voip_sip_register_interval_set_api(u_int16 value)
{
	voip_sip_config.sip_register_interval = value ? value:SIP_REGINTER_DEFAULT;
	return OK;
}

int voip_sip_hostpart_set_api(u_int8 * value)
{
	memset(voip_sip_config.sip_hostpart, 0, sizeof(voip_sip_config.sip_hostpart));
	if(value)
		strcpy(voip_sip_config.sip_hostpart, value);
	return OK;
}

int voip_sip_interval_set_api(u_int16 value)
{
	voip_sip_config.sip_interval = value ? value:SIP_INTERVAL_DEFAULT;
	return OK;
}

int voip_sip_100_rel_set_api(BOOL value)
{
	voip_sip_config.sip_100_rel = value;
	return OK;
}

int voip_sip_display_name_set_api(BOOL value)
{
	voip_sip_config.sip_dis_name = value;
	return OK;
}

int voip_sip_local_number_set_api(char * value)
{
	memset(voip_sip_config.sip_local_number, 0, sizeof(voip_sip_config.sip_local_number));
	if(value)
		strcpy(voip_sip_config.sip_local_number, value);
	return OK;
}
int voip_sip_password_set_api(char * value)
{
	memset(voip_sip_config.sip_password, 0, sizeof(voip_sip_config.sip_password));
	if(value)
		strcpy(voip_sip_config.sip_password, value);
	voip_sip_config_update_api(&voip_sip_config);
	return OK;
}

int voip_sip_user_set_api(char * value)
{
	memset(voip_sip_config.sip_user, 0, sizeof(voip_sip_config.sip_user));
	if(value)
		strcpy(voip_sip_config.sip_user, value);
	voip_sip_config_update_api(&voip_sip_config);
	return OK;
}

int voip_sip_realm_set_api(char * value)
{
	memset(voip_sip_config.sip_realm, 0, sizeof(voip_sip_config.sip_realm));
	if(value)
		strcpy(voip_sip_config.sip_realm, value);
	voip_sip_config_update_api(&voip_sip_config);
	return OK;
}

int voip_sip_dialplan_set_api(u_int8  *value)
{
	memset(voip_sip_config.sip_dialplan, 0, sizeof(voip_sip_config.sip_dialplan));
	if(value)
		strcpy(voip_sip_config.sip_dialplan, value);
	//voip_sip_config.sip_dialplan = value;
	return OK;
}

int voip_sip_encrypt_set_api(BOOL value)
{
	voip_sip_config.sip_encrypt = value;
	return OK;
}

/*
 *
[sip_config]
server_ip = 192.168.1.23
server_port = 5060
user_name = 333
passwd = 123456
realm = test
local_ip = 0.0.0.0
local_port = 5060
dtmf = rfc2833
*/
static int voip_sip_config_update_thread(struct eloop *eloop)
{
	voip_sip_t *sip = ELOOP_ARG(eloop);
	FILE *fp = fopen(SIP_CONFIG_FILE, "w+");
	if(fp)
	{
		super_system("killall -9 VmrMgr");
		fprintf(fp, "[sip_config]\n");
		fprintf(fp, "server_ip = %s\n", inet_address(sip->sip_server));
		fprintf(fp, "server_port = %d\n", sip->sip_port);
		if(strlen(sip->sip_user))
			fprintf(fp, "user_name = %s\n", sip->sip_user);
		else
			fprintf(fp, "user_name = %s\n", SIP_USERNAME_DEFAULT);

		if(strlen(sip->sip_password))
			fprintf(fp, "passwd = %s\n", sip->sip_password);
		else
			fprintf(fp, "passwd = %s\n", SIP_PASSWORD_DEFAULT);

		if(strlen(sip->sip_realm))
			fprintf(fp, "realm = %s\n", sip->sip_realm);
		else
			fprintf(fp, "realm = \n");
			//fprintf(fp, "realm = %s\n", SIP_REALM_DEFAULT);

		fprintf(fp, "local_ip = %s\n", ("192.168.2.100"));
		fprintf(fp, "local_port = %d\n", sip->sip_local_port);
		fprintf(fp, "dtmf = %d\n", "rfc2833");

		fprintf(fp, "reg_expire = %d\n", sip->sip_register_interval);
		fprintf(fp, "rtp_port = %d\n", voip_stream->l_rtp_port);

		fflush(fp);
		fclose(fp);

		//super_system("killall -9 VmrMgr");
		if(child_process_create() == 0)
		{
			chdir("/app");
			super_system_execvp("./VmrMgr", NULL);
		}
		return OK;
	}
	return ERROR;
}


int voip_sip_config_update_api(voip_sip_t *sip)
{
	//struct eloop eloop;
	//eloop.arg = sip;
	//voip_sip_config_update_thread(&eloop);
	if(voip_socket.master)
	{
		eloop_cancel(sip->t_event);
		sip->t_event = eloop_add_timer(voip_socket.master, voip_sip_config_update_thread, sip, 1);
	}
	return OK;
}






int voip_sip_write_config(struct vty *vty)
{
	voip_sip_t *sip = &voip_sip_config;
	if(sip->sip_enable)
	{
		vty_out(vty, "service sip%s", VTY_NEWLINE);

		if(strlen(sip->sip_local_number))
			vty_out(vty, " ip sip local-phone %s%s", sip->sip_local_number, VTY_NEWLINE);

		if(strlen(sip->sip_user))
			vty_out(vty, " ip sip username %s%s", sip->sip_user, VTY_NEWLINE);

		if(strlen(sip->sip_password))
			vty_out(vty, " ip sip password %s%s", sip->sip_password, VTY_NEWLINE);

		if(sip->sip_local_port != SIP_PORT_DEFAULT)
			vty_out(vty, " ip sip local-port %d%s", (sip->sip_local_port), VTY_NEWLINE);

		if(sip->sip_server)
			vty_out(vty, " ip sip-server %s%s", inet_address(sip->sip_server), VTY_NEWLINE);
		if(sip->sip_server_sec)
			vty_out(vty, " ip sip-server %s secondary%s", inet_address(sip->sip_server_sec), VTY_NEWLINE);
		if(sip->sip_proxy_server)
			vty_out(vty, " ip sip-proxy-server %s%s", inet_address(sip->sip_proxy_server), VTY_NEWLINE);
		if(sip->sip_proxy_server_sec)
			vty_out(vty, " ip sip-proxy-server %s secondary%s", inet_address(sip->sip_proxy_server_sec), VTY_NEWLINE);
		if(sip->sip_port != SIP_PORT_DEFAULT)
			vty_out(vty, " ip sip-server port %d%s", (sip->sip_port), VTY_NEWLINE);
		if(sip->sip_port_sec != SIP_PORT_SEC_DEFAULT)
			vty_out(vty, " ip sip-server port %d secondary%s", (sip->sip_port_sec), VTY_NEWLINE);
		if(sip->sip_proxy_port != SIP_PROXY_PORT_DEFAULT)
			vty_out(vty, " ip sip-proxy-server port %d%s", (sip->sip_proxy_port), VTY_NEWLINE);
		if(sip->sip_proxy_port_sec != SIP_PROXY_PORT_SEC_DEFAULT)
			vty_out(vty, " ip sip-proxy-server port %d secondary%s", (sip->sip_proxy_port_sec), VTY_NEWLINE);

		if(sip->sip_time_sync != SIP_TIME_DEFAULT)
			vty_out(vty, " ip sip time-sync%s", VTY_NEWLINE);

		if(sip->sip_ring != SIP_RING_DEFAULT)
			vty_out(vty, " ip sip ring %d%s", sip->sip_ring, VTY_NEWLINE);

		if(sip->sip_register_interval != SIP_REGINTER_DEFAULT)
			vty_out(vty, " ip sip register-interval %d%s", sip->sip_register_interval, VTY_NEWLINE);

		//if(sip->sip_hostpart != SIP_HOSTPART_DEFAULT)
		if(strlen(sip->sip_hostpart))
			vty_out(vty, " ip sip hostpart %s%s", sip->sip_hostpart, VTY_NEWLINE);

		if(sip->sip_interval != SIP_INTERVAL_DEFAULT)
			vty_out(vty, " ip sip keep-interval %d%s", sip->sip_interval, VTY_NEWLINE);


		if(sip->sip_100_rel != SIP_100_REL_DEFAULT)
			vty_out(vty, " ip sip rel-100%s", VTY_NEWLINE);

		if(sip->sip_dis_name != SIP_DIS_NAME_DEFAULT)
			vty_out(vty, " ip sip display-name %s", VTY_NEWLINE);

		if(sip->sip_encrypt != SIP_ENCRYPT_DEFAULT)
			vty_out(vty, " ip sip encrypt %s", VTY_NEWLINE);

		//if(sip->sip_dialplan != SIP_DIALPLAN_DEFAULT)
		if(strlen(sip->sip_dialplan))
			vty_out(vty, " ip sip dialplan %s%s", sip->sip_dialplan, VTY_NEWLINE);

		if(strlen(sip->sip_realm))
			vty_out(vty, " ip sip realm %s%s", sip->sip_realm, VTY_NEWLINE);

		vty_out(vty, "!%s",VTY_NEWLINE);
	}
	return OK;
}

int voip_sip_show_config(struct vty *vty, BOOL detail)
{
	voip_sip_t *sip = &voip_sip_config;
	if(sip->sip_enable)
	{
		vty_out(vty, "SIP Service :%s", VTY_NEWLINE);
		vty_out(vty, " local-phone          : %s%s",
				strlen(sip->sip_local_number)? sip->sip_local_number:" ", VTY_NEWLINE);

		vty_out(vty, " username             : %s%s",
				strlen(sip->sip_user)? sip->sip_user:" ", VTY_NEWLINE);

		vty_out(vty, " password             : %s%s",
				strlen(sip->sip_password)? sip->sip_password:" ", VTY_NEWLINE);
		vty_out(vty, " sip loal-port        : %d%s", (sip->sip_local_port), VTY_NEWLINE);

		vty_out(vty, " sip-server           : %s%s", inet_address(sip->sip_server), VTY_NEWLINE);
		vty_out(vty, " sip-server           : %s secondary%s", inet_address(sip->sip_server_sec), VTY_NEWLINE);
		vty_out(vty, " sip-proxy-server     : %s%s", inet_address(sip->sip_proxy_server), VTY_NEWLINE);
		vty_out(vty, " sip-proxy-server     : %s secondary%s", inet_address(sip->sip_proxy_server_sec), VTY_NEWLINE);
		vty_out(vty, " sip-server port      : %d%s", (sip->sip_port), VTY_NEWLINE);
		vty_out(vty, " sip-server port      : %d secondary%s", (sip->sip_port_sec), VTY_NEWLINE);

		vty_out(vty, " sip-proxy-server port: %d%s", (sip->sip_proxy_port), VTY_NEWLINE);
		vty_out(vty, " sip-proxy-server port: %d secondary%s", (sip->sip_proxy_port_sec), VTY_NEWLINE);
		vty_out(vty, " sip time-sync        : %s%s", sip->sip_time_sync ? "TRUE":"FALSE",VTY_NEWLINE);
		vty_out(vty, " sip ring             : %d%s", sip->sip_ring ,VTY_NEWLINE);

		vty_out(vty, " sip register-interval: %d%s", sip->sip_register_interval, VTY_NEWLINE);
		vty_out(vty, " sip hostpart         : %s%s", strlen(sip->sip_hostpart)? sip->sip_hostpart:" ",VTY_NEWLINE);
		vty_out(vty, " sip keep-interval    : %d%s", sip->sip_interval, VTY_NEWLINE);
		vty_out(vty, " sip rel-100          : %s%s", sip->sip_100_rel ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " sip display-name     : %s%s", sip->sip_dis_name ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " sip encrypt          : %s%s", sip->sip_encrypt ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " sip dialplan         : %s%s", strlen(sip->sip_dialplan)? sip->sip_dialplan:" ", VTY_NEWLINE);
		vty_out(vty, " sip realm            : %s%s", strlen(sip->sip_realm)? sip->sip_realm:" ", VTY_NEWLINE);
	}
	else
		vty_out(vty, "SIP Service is not enable%s", VTY_NEWLINE);
	return OK;
}


/*
100试呼叫（Trying）
180振铃（Ringing）
181呼叫正在前转（Call is Being Forwarded）
200成功响应（OK）
302临时迁移（Moved Temporarily）
400错误请求（Bad Request）
401未授权（Unauthorized）
403禁止（Forbidden）
404用户不存在（Not Found）
408请求超时（Request Timeout）
480暂时无人接听（Temporarily Unavailable）
486线路忙（Busy Here）
504服务器超时（Server Time-out）
600全忙（Busy Everywhere）
 */
/*
 * event socket (SIP)
 */

int voip_sip_ctl_module_init()
{
	os_memset(&voip_sip_ctl, 0, sizeof(voip_sip_ctl));
#ifdef SIP_CTL_SOCKET
	voip_sip_ctl.tcpMode = TRUE;
#endif

	if(master_eloop[MODULE_VOIP] == NULL)
		master_eloop[MODULE_VOIP] = eloop_master_module_create(MODULE_VOIP);
	voip_sip_ctl.master = master_eloop[MODULE_VOIP];

#ifdef SIP_CTL_MSGQ
	voip_sip_msgq_init(&voip_sip_ctl);
#endif
	voip_sip_ctl.debug = 0xffff;
#ifdef SIP_CTL_SOCKET
	return voip_sip_socket_init(&voip_sip_ctl);
#endif
#ifdef SIP_CTL_MSGQ
	return sip_msgq_task_init(&voip_sip_ctl);
#endif
	return OK;
}

int voip_sip_ctl_module_exit()
{
#ifdef SIP_CTL_MSGQ
	sip_msgq_task_exit(&voip_sip_ctl);
#endif
#ifdef SIP_CTL_SOCKET
	voip_sip_socket_exit(&voip_sip_ctl);
#endif
#ifdef SIP_CTL_MSGQ
	voip_sip_msgq_exit(&voip_sip_ctl);
#endif
	os_memset(&voip_sip_ctl, 0, sizeof(voip_sip_ctl));
	return OK;
}

#ifdef SIP_CTL_SOCKET
static int voip_sip_socket_read_eloop(struct eloop *eloop)
{
	voip_sip_ctl_t *sipctl = ELOOP_ARG(eloop);
	int sock = ELOOP_FD(eloop);
	//ELOOP_VAL(X)
	sipctl->t_read = NULL;
	memset(sipctl->buf, 0, sizeof(sipctl->buf));

	int len = read(sock, sipctl->buf, sizeof(sipctl->buf));
	if (len <= 0)
	{
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				//return 0;
				//mgt->reset_thread = eloop_add_timer_msec(mgt->master, x5_b_a_reset_eloop, mgt, 100);
				return OK;
			}
			else
			{

			}
		}
	}
	else
	{
		if(SIP_CTL_DEBUG(RECV))
		{
			zlog_debug(ZLOG_VOIP, "RECV MSG %d byte", len);
		}
		sipctl->len = len;
		voip_sip_read_handle(sipctl, sipctl->buf, len);
	}
	sipctl->t_read = eloop_add_read(sipctl->master, voip_sip_socket_read_eloop, sipctl, sock);
	return OK;
}

static int voip_sip_socket_accept_eloop(struct eloop *eloop)
{
	voip_sip_ctl_t *sipctl = ELOOP_ARG(eloop);
	int sock = ELOOP_FD(eloop);
	int client = 0;
	client = unix_sock_accept (sock, NULL);
	sipctl->t_accept = eloop_add_read(sipctl->master,
		voip_sip_socket_accept_eloop, sipctl, sock);

	if(client)
	{
		os_set_nonblocking(client);
		if(sipctl->sock)
		{
			close(sipctl->sock);
		}
		sipctl->sock = client;
		sipctl->wfd = client;
		sipctl->t_read = eloop_add_read(sipctl->master, voip_sip_socket_read_eloop, sipctl, client);
	}
	else
		return ERROR;
	return OK;
}

static int voip_sip_socket_init(voip_sip_ctl_t *sipctl)
{
	int fd = unix_sock_server_create(sipctl->tcpMode, "voipSip");
	if(fd)
	{
		if(sipctl->tcpMode)
			sipctl->accept = fd;
		else
		{
			sipctl->sock = fd;
			sipctl->wfd = unix_sock_client_create(sipctl->tcpMode, "sip");
		}
		os_set_nonblocking(fd);
		if(sipctl->master)
		{
			if(sipctl->tcpMode)
				sipctl->t_accept = eloop_add_read(sipctl->master,
					voip_sip_socket_accept_eloop, sipctl, fd);
			else
				sipctl->t_read = eloop_add_read(sipctl->master, voip_sip_socket_read_eloop, sipctl, fd);
		}
		//sipctl->t_thread = eloop_add_timer(sipctl->master, x5_b_a_timer_eloop, sipctl, sipctl->interval);
		return OK;
	}
	return ERROR;
}
#endif

#ifdef SIP_CTL_MSGQ
static int voip_sip_msgq_init(voip_sip_ctl_t *sipctl)
{
	sipctl->rq = -1;
	sipctl->wq = -1;
	//sipctl->rq = sip_msgq_create(SIP_CTL_LMSGQ_KEY, FALSE);
/*	if(sipctl->rq < 0)
		return ERROR;*/
/*	if(sipctl->rq < 0)
		sip_msgq_fflush(sipctl->rq);
	*/
	//sipctl->wq = sip_msgq_create(SIP_CTL_MSGQ_KEY, TRUE);
/*
	if(sipctl->wq < 0)
		return ERROR;
*/
	//zlog_debug(ZLOG_VOIP, "Create MSGQ (rq key=%d:%d wq key=%d:%d)", SIP_CTL_LMSGQ_KEY, sipctl->rq, SIP_CTL_MSGQ_KEY, sipctl->wq);
	//printf("Create MSGQ (rq key=%d:%d wq key=%d:%d)", SIP_CTL_LMSGQ_KEY, sipctl->rq, SIP_CTL_MSGQ_KEY, sipctl->wq);
//	sip_msgq_buffsize(sipctl->rq, 4096);
	return ERROR;
}
#endif

#ifdef SIP_CTL_SOCKET
static int voip_sip_socket_exit(voip_sip_ctl_t *sipctl)
{
	if(sipctl && sipctl->t_accept)
	{
		eloop_cancel(sipctl->t_accept);
		sipctl->t_accept = NULL;
		if(sipctl->accept)
		{
			close(sipctl->accept);
			sipctl->accept = 0;
		}
	}
	if(sipctl && sipctl->t_read)
	{
		eloop_cancel(sipctl->t_read);
		sipctl->t_read = NULL;
	}
	if(sipctl && sipctl->t_write)
	{
		eloop_cancel(sipctl->t_write);
		sipctl->t_write = NULL;
	}
	if(sipctl && sipctl->t_event)
	{
		eloop_cancel(sipctl->t_event);
		sipctl->t_event = NULL;
	}
	if(sipctl && sipctl->t_time)
	{
		eloop_cancel(sipctl->t_time);
		sipctl->t_time = NULL;
	}
	if(sipctl)
	{
		if(sipctl->sock)
		{
			close(sipctl->sock);
			sipctl->sock = 0;
		}
		if(!sipctl->tcpMode)
		{
			if(sipctl->wfd)
				close(sipctl->wfd);
			sipctl->wfd = 0;
		}
		else
			sipctl->wfd = 0;
		memset(sipctl->buf, 0, sizeof(sipctl->buf));
		return OK;
	}
	return ERROR;
}
#endif

#ifdef SIP_CTL_MSGQ
static int voip_sip_msgq_exit(voip_sip_ctl_t *sipctl)
{
	if(sipctl && sipctl->t_event)
	{
		eloop_cancel(sipctl->t_event);
		sipctl->t_event = NULL;
	}
	if(sipctl && sipctl->t_time)
	{
		eloop_cancel(sipctl->t_time);
		sipctl->t_time = NULL;
	}

	sip_msgq_delete(sipctl->wq);
	sip_msgq_delete(sipctl->rq);
	sipctl->rq = -1;
	sipctl->wq = -1;

	if(sipctl)
	{
		memset(sipctl->buf, 0, sizeof(sipctl->buf));
		return OK;
	}
	return ERROR;
}
#endif


static int voip_sip_send_msg(voip_sip_ctl_t *sipctl)
{
#ifdef SIP_CTL_MSGQ
	if(sipctl && (sipctl->wq < 0))
		sipctl->wq = sip_msgq_create_write(SIP_CTL_MSGQ_KEY);
	if(sipctl && (sipctl->wq >= 0))
	{
		printf("%s : key=%d id=%d\n", __func__, SIP_CTL_MSGQ_KEY, sipctl->wq);
		sip_msgq_send(sipctl->wq, sipctl->sbuf, sipctl->slen);
	}
	return OK;
#else
	if(SIP_CTL_DEBUG(SEND))
	{
		zlog_debug(ZLOG_VOIP, "SEND MSG %d byte", sipctl->slen);
	}
	if(sipctl && sipctl->wfd)
		write(sipctl->wfd, sipctl->sbuf, sipctl->slen);
#endif
	return ERROR;
}

#ifdef SIP_CTL_SOCKET
static int voip_sip_wait_read_msg(voip_sip_ctl_t *sipctl, int timeoutms)
{
	int len = 0, maxfd = 0, num = 0;
	fd_set rfdset;
	FD_ZERO(&rfdset);

	FD_SET(sipctl->sock, &rfdset);
	maxfd = sipctl->sock;
	memset(sipctl->buf, 0, sizeof(sipctl->buf));
r_again:
	FD_SET(sipctl->sock, &rfdset);
	num = os_select_wait(maxfd + 1, &rfdset, NULL, timeoutms);
	if(num > 0)
	{
		//process_log_debug("start unix_sock_accept %s", progname);
		if(FD_ISSET(sipctl->sock, &rfdset))
		{
			FD_CLR(sipctl->sock, &rfdset);
			len = read(sipctl->sock, sipctl->buf, sizeof(sipctl->buf));
			if(len)
			{
				if(SIP_CTL_DEBUG(RECV))
				{
					zlog_debug(ZLOG_VOIP, "RECV MSG %d byte", sipctl->slen);
				}
				sipctl->len = len;
				voip_sip_read_handle(sipctl, sipctl->buf, len);
			}
			else
			{
				if (ERRNO_IO_RETRY(errno))
				{
					if(SIP_CTL_DEBUG(EVENT))
					{
						zlog_debug(ZLOG_VOIP, "RECV MSG AGAIN %s", strerror(errno));
					}
					//return 0;
					//mgt->t_reset = eloop_add_timer_msec(mgt->master, x5_b_a_reset_eloop, mgt, 100);
					goto r_again;
				}
				else
				{
					if(SIP_CTL_DEBUG(EVENT))
					{
						zlog_debug(ZLOG_VOIP, "RECV MSG ERROR %s", strerror(errno));
					}
					return ERROR;
				}
			}
		}
	}
	else if(num < 0)
	{
		//continue;
		return ERROR;
	}
	else
	{
		//timeout
		return -1;
	}
	return OK;
}
#ifdef SIP_CTL_SYNC
static int voip_sip_write_and_wait_respone(voip_sip_ctl_t *sipctl, int timeoutms)
{
	int rep = 0,ret = 0;
	if(sipctl->t_read)
	{
		eloop_cancel(sipctl->t_read);
		sipctl->t_read = NULL;
		rep = 1;
	}
	ret = voip_sip_send_msg(sipctl);
	if(ret > 0)
		ret = voip_sip_wait_read_msg(sipctl, timeoutms);
	if(rep)
	{
		sipctl->t_read = eloop_add_read(sipctl->master,
				voip_sip_socket_read_eloop, sipctl, sipctl->sock);
	}
	return ret;
}
#else
static int voip_sip_write_msg(voip_sip_ctl_t *sipctl, int timeoutms)
{
	int ret = 0;
	ret = voip_sip_send_msg(sipctl);
	return ret;
}
#endif
#else
static int voip_sip_write_msg(voip_sip_ctl_t *sipctl, int timeoutms)
{
	int ret = 0;
	ret = voip_sip_send_msg(sipctl);
	return ret;
}
#endif



sip_register_state_t voip_sip_register_state_get_api()
{
	return voip_sip_ctl.reg_state;
}

sip_call_error_t voip_sip_call_error_get_api()
{
	return voip_sip_ctl.call_error;
}

sip_call_state_t voip_sip_call_state_get_api()
{
	return voip_sip_ctl.call_state;
}

sip_stop_state_t voip_sip_stop_state_get_api()
{
	return voip_sip_ctl.stop_state;
}

/*
 * SIP ---> CTL
 */
static int voip_sip_register_ack(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_REG_RLT *ack;
	ack = (MSG_REG_ACT *)SIP_MSG_OFFSET(buf);

	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "RECV MSG TYPE=%x register ack state=%x ---> VOIP_SIP_REGISTER_SUCCESS", hdr->type, ack->rlt);
	}
	if(hdr->type == SIP_REGISTER_ACK_MSG)
	{
		if(ack->rlt == SIP_REGISTER_OK)
		{
			sipctl->reg_state = (VOIP_SIP_REGISTER_SUCCESS);
			return OK;
		}
		sipctl->reg_state = (VOIP_SIP_REGISTER_FAILED);
	}
	return ERROR;
}

//被叫振铃
static int voip_sip_call_ring(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_REMOT_ALERT *ack = (MSG_REMOT_ALERT *)(buf + sizeof(MSG_HDR_T));
	//ack = (MSG_REMOT_ALERT *)SIP_MSG_OFFSET(buf);
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "RECV MSG TYPE=%x ringing ack %s:%d codes:%d ", hdr->type, ack->rtp_addr, ack->rtp_port, ack->codec);
	}
	if(hdr->type == SIP_REMOTE_RING_MSG)
	{
		//if(ack->rlt == SIP_REGISTER_OK)
		if(ack->rtp_port)
		{
			voip_stream->r_rtp_port = ack->rtp_port;
			if(strlen(ack->rtp_addr))
				os_strcpy(voip_stream->r_rtp_address, ack->rtp_addr);
			voip_stream->payload = ack->codec;
			//voip_stream_remote_address_port_api(voip_stream, ack->rtp_addr, ack->rtp_port);
			//voip_stream_payload_type_api(voip_stream, NULL, ack->codec);
		}
		//voip_call_ring_start_api();
		sipctl->call_state = (VOIP_SIP_CALL_RINGING);
		return OK;
	}
	return ERROR;
}

//被叫摘机
static int voip_sip_call_picking(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	//MSG_REMOT_ANSWER *ack;
	MSG_REMOT_ANSWER *ack = (MSG_REMOT_ANSWER *)(buf + sizeof(MSG_HDR_T));
	//ack = (MSG_REMOT_ANSWER *)SIP_MSG_OFFSET(buf);
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "RECV MSG TYPE=%x picking ack %s:%d codes:%d ", hdr->type, ack->rtp_addr, ack->rtp_port, ack->codec);
	}
	if(hdr->type == SIP_REMOTE_PICKING_MSG)
	{
		if(ack->rtp_port && strlen(ack->rtp_addr))
		{
			//voip_call_ring_stop_api();
			voip_stream->r_rtp_port = ack->rtp_port;
			if(strlen(ack->rtp_addr))
				memcpy(voip_stream->r_rtp_address, ack->rtp_addr, 64);
			voip_stream->payload = ack->codec;

			voip_stream_remote_t remote;
			memset(&remote, 0, sizeof(remote));
			remote.r_rtp_port = ack->rtp_port;
			strcpy(remote.r_rtp_address, voip_stream->r_rtp_address);
			//strcpy(remote.r_rtp_address, "192.168.2.202");
			voip_event_node_register(voip_app_ev_start_stream, NULL, &remote, sizeof(voip_stream_remote_t));
		}
		//if(ack->rlt == SIP_REGISTER_OK)
		sipctl->call_state = (VOIP_SIP_CALL_PICKUP);
		return OK;
	}
	return ERROR;
}

static int voip_sip_call_ack(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_REMOT_ACK *ack;
	ack = (MSG_REMOT_ACK *)(buf + sizeof(MSG_HDR_T));
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "RECV MSG TYPE=%x ack %s:%d codes:%d ", hdr->type, ack->rtp_addr, ack->rtp_port, ack->codec);
	}
	if(hdr->type == SIP_REMOTE_ACK_MSG)
	{
		if(ack->rtp_port && strlen(ack->rtp_addr))
		{
			//voip_call_ring_stop_api();
			voip_stream->r_rtp_port = ack->rtp_port;
			if(strlen(ack->rtp_addr))
				os_strcpy(voip_stream->r_rtp_address, ack->rtp_addr);
			voip_stream->payload = ack->codec;

			voip_stream_remote_t remote;
			memset(&remote, 0, sizeof(remote));
			remote.r_rtp_port = ack->rtp_port;
			strcpy(remote.r_rtp_address, ack->rtp_addr);
			//strcpy(remote.r_rtp_address, "192.168.2.202");
			voip_event_node_register(voip_app_ev_start_stream, NULL, &remote, sizeof(voip_stream_remote_t));
		}
		//if(ack->rlt == SIP_REGISTER_OK)
		sipctl->call_state = (VOIP_SIP_CALL_PICKUP);
		return OK;
	}
	return ERROR;
}

//呼叫出错
static int voip_sip_call_error(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_REMOT_ERROR *ack;
	ack = (MSG_REMOT_ERROR *)SIP_MSG_OFFSET(buf);
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "RECV MSG TYPE=%x call errno %x", hdr->type, ack->cause);
	}
	if(hdr->type == SIP_CALL_ERROR_MSG)
	{
		//if(ack->rlt == SIP_REGISTER_OK)
		sipctl->call_state = (VOIP_SIP_CALL_ERROR);
		sipctl->call_error = ack->cause;
		return OK;
	}
	return ERROR;
}

//远端挂机
static int voip_sip_call_stop_by_remote(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_REMOT_BYE *ack;
	ack = (MSG_REMOT_BYE *)SIP_MSG_OFFSET(buf);
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "RECV MSG TYPE=%x stop by remote", hdr->type);
	}
	if(hdr->type == SIP_REMOTE_STOP_MSG)
	{
		//if(ack->rlt == SIP_REGISTER_OK)
		sipctl->stop_state = (VOIP_SIP_REMOTE_STOP);
		//voip_event_node_register(voip_app_ev_stop_call, NULL, NULL, 0);
		voip_stream_stop_api();
		//sipctl->call_state = VOIP_SIP_CALL_IDLE;
		return OK;
	}
	return ERROR;
}

static int voip_sip_call_remote_info(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_SIP_INFO *ack;
	ack = (MSG_SIP_INFO *)SIP_MSG_OFFSET(buf);
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "RECV MSG TYPE=%x call num %d '%c' %s", hdr->type, (int)ack->content[0], ack->content[0], ack->content);
	}
	if(hdr->type == SIP_REMOTE_INFO_MSG)
	{
		return OK;
	}
	return ERROR;
}


int voip_sip_read_handle(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	int ret = ERROR;
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	switch(hdr->type)
	{
	case SIP_REGISTER_MSG:
		break;
	case SIP_REGISTER_ACK_MSG:
		ret = voip_sip_register_ack(sipctl, buf, len);
		break;

	case SIP_CALL_MSG:
		break;
	case SIP_REMOTE_RING_MSG:
		ret = voip_sip_call_ring(sipctl, buf, len);
		break;
	case SIP_REMOTE_PICKING_MSG:
		ret = voip_sip_call_picking(sipctl, buf, len);
		break;
	case SIP_REMOTE_ACK_MSG:
		ret = voip_sip_call_ack(sipctl, buf, len);
		break;
	case SIP_CALL_ERROR_MSG:
		ret = voip_sip_call_error(sipctl, buf, len);
		break;
	case SIP_REMOTE_STOP_MSG:
		ret = voip_sip_call_stop_by_remote(sipctl, buf, len);
		break;
	case SIP_LOCAL_STOP_MSG:
		break;
	case SIP_REMOTE_INFO_MSG:
		ret = voip_sip_call_remote_info(sipctl, buf, len);
		break;

	default:
		zlog_err(ZLOG_VOIP, "RECV MSG TYPE=%x ", hdr->type);
		break;
	}
	return ret;
}

/*
 * CTL ---> SIP
 */
static int voip_sip_hdr_make(voip_sip_ctl_t *sipctl, int type, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
#if 0
	hdr->srcApplId 	= VOIP_SIP_SRCID;  /*源进程号  VOIP--6，linectl---8*/
	hdr->dstAppId 	= VOIP_SIP_DSTID;   /*目的进程号*/
	hdr->type		= type;     /*消息类型 1--注册；2--注册返回，3--*/
	hdr->sync		= TRUE;      /* 是否是同步消息 */
#else
	hdr->magic		= 1;//SIP_MSG_HDR_MAGIC;
	hdr->priority	= 1;  /* Priority must be the first 4-byte */
	hdr->srcApplId	= VOIP_SIP_SRCID; /* VOS_APPL_ID */
	hdr->type		= (type);
	hdr->srcMsgQKey = SIP_CTL_LMSGQ_KEY;
	hdr->sync		= FALSE;      /* Sync or async message */
	hdr->len		= 0;       /* the length of the message, including the message hdr */
	hdr->tick		= 0;
	hdr->magic		= hdr->priority;
#endif
	hdr->len		= (SIP_MSG_LEN(len));       /* 消息长度，包含消息头 */
	return SIP_MSG_LEN(len);
}


int voip_sip_register_start(BOOL reg)
{
	MSG_REG_ACT *act;
	voip_sip_ctl.send_cmd	=	SIP_REGISTER_MSG;
	voip_sip_ctl.ack_cmd		=	SIP_REGISTER_ACK_MSG;
	memset(voip_sip_ctl.sbuf, 0, sizeof(voip_sip_ctl.sbuf));
	act = (MSG_REG_ACT *)SIP_MSG_OFFSET(voip_sip_ctl.sbuf);
	act->act = reg;
	voip_sip_ctl.slen = voip_sip_hdr_make(&voip_sip_ctl, SIP_REGISTER_MSG, voip_sip_ctl.sbuf, sizeof(MSG_REG_ACT));
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "SEND Register MSG %s (%d byte)", reg ? "Start":"Stop",voip_sip_ctl.slen);
	}
#ifdef SIP_CTL_SYNC
	return voip_sip_write_and_wait_respone(&voip_sip_ctl, SIP_CTL_TIMEOUT);
#else
	return voip_sip_write_msg(&voip_sip_ctl, SIP_CTL_TIMEOUT);
#endif
}


int voip_sip_call_start(char *phone)
{
	MSG_CAL_ACT *act = NULL;
	voip_sip_ctl.send_cmd	=	SIP_CALL_MSG;
	voip_sip_ctl.ack_cmd		=	SIP_REMOTE_RING_MSG | SIP_REMOTE_PICKING_MSG | SIP_CALL_ERROR_MSG;
	memset(voip_sip_ctl.sbuf, 0, sizeof(voip_sip_ctl.sbuf));
	//act = (MSG_CAL_ACT *)SIP_MSG_OFFSET(voip_sip_ctl.sbuf);
	act = (MSG_CAL_ACT *)(voip_sip_ctl.sbuf + sizeof(MSG_HDR_T));
	MSG_HDR_T *hdr = (	MSG_HDR_T *)voip_sip_ctl.sbuf;
	hdr->magic		= 1;//SIP_MSG_HDR_MAGIC;
	hdr->priority	= 1;  /* Priority must be the first 4-byte */
	hdr->srcApplId	= VOIP_SIP_SRCID; /* VOS_APPL_ID */
	hdr->type		= (SIP_CALL_MSG);
	hdr->srcMsgQKey = SIP_CTL_LMSGQ_KEY;
	hdr->sync		= FALSE;      /* Sync or async message */
	hdr->len		= 0;       /* the length of the message, including the message hdr */
	hdr->tick		= 0;
	hdr->magic		= hdr->priority;
	hdr->len		= (sizeof(MSG_HDR_T) + sizeof(MSG_CAL_ACT));       /* 消息长度，包含消息头 */

	act->uid 		= 1;
	act->senid 		= 1;
	strcpy(act->digit, "1001"); /* 被叫号码*/
	act->digit_num = strlen("1001"); /*被叫号码长度*/
	strcpy(act->name, "1001"); /* 被叫号码*/
	act->name_num = strlen("1001"); /*被叫号码长度*/

	strcpy(act->digit, phone); /* 被叫号码*/
	act->digit_num = strlen(phone); /*被叫号码长度*/
	strcpy(act->name, phone); /* 被叫号码*/
	act->name_num = strlen(phone); /*被叫号码长度*/

	//act->name[48];  /*被叫名称*/
	//act->name_num;  /*被叫名称长度*/
	act->rtp_port 	= 5555;//(voip_stream->l_rtp_port);  /*本地rtp端口*/
	//act->rtp_addr	= 0;  /*本地RTP地址*/
	act->codec		= 8;//voip_stream->payload;     /*优先使用的codec*/
	voip_sip_ctl.slen = hdr->len;
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "SEND Call MSG start @%s (%d byte)", phone,voip_sip_ctl.slen);
	}
#ifdef SIP_CTL_SYNC
	return voip_sip_write_and_wait_respone(&voip_sip_ctl, SIP_CTL_TIMEOUT);
#else
	return voip_sip_write_msg(&voip_sip_ctl, SIP_CTL_TIMEOUT);
#endif
}



int voip_sip_call_stop()
{
	MSG_LOCAL_BYE *act;
	voip_sip_ctl.send_cmd	=	SIP_LOCAL_STOP_MSG;
	//voip_sip_ctl.ack_cmd		=	SIP_REMOTE_RING_MSG | SIP_REMOTE_PICKING_MSG | SIP_CALL_ERROR_MSG;
	memset(voip_sip_ctl.sbuf, 0, sizeof(voip_sip_ctl.sbuf));
	act = (MSG_LOCAL_BYE *)SIP_MSG_OFFSET(voip_sip_ctl.sbuf);
	act->uid 		= 1;
	act->senid 		= 1;
	voip_sip_ctl.stop_state = (VOIP_SIP_LOCAL_STOP);
	voip_sip_ctl.call_state = VOIP_SIP_CALL_IDLE;
	voip_sip_ctl.slen = voip_sip_hdr_make(&voip_sip_ctl, SIP_LOCAL_STOP_MSG, voip_sip_ctl.sbuf, sizeof(MSG_LOCAL_BYE));
	if(SIP_CTL_DEBUG(RECV) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "SEND Call MSG stop (%d byte)",voip_sip_ctl.slen);
	}
	voip_event_node_register(voip_app_ev_stop_call, NULL, NULL, 0);
#ifdef SIP_CTL_SYNC
	return voip_sip_write_and_wait_respone(&voip_sip_ctl, SIP_CTL_TIMEOUT);
#else
	return voip_sip_write_msg(&voip_sip_ctl, SIP_CTL_TIMEOUT);
#endif
}











/*
 * SIP state Module
 */
/*
voip_state_t voip_sip_state_get_api()
{
	return VOIP_STATE_CALL_SUCCESS;
}
*/



/*
int voip_sip_register(char *phone, char *user, char *password, BOOL enable)
{
	return voip_sip_write_and_wait_respone(&voip_sip_ctl, SIP_CTL_TIMEOUT);
}


int voip_sip_call(char *phone, char *user, char *password, int timeoutms, BOOL start)
{
#ifdef VOIP_SIP_EVENT_LOOPBACK
	return OK;
#else
	return voip_sip_write_and_wait_respone(&voip_sip_ctl,  timeoutms);
#endif
}
*/

#ifdef SIP_CTL_MSGQ

static int sip_msgq_create_read(int msgKey)
{
    int msgId;
    // make applId as msg key
    if ((msgId = msgget(msgKey, (S_IRUSR|S_IWUSR|IPC_CREAT/*|IPC_EXCL*/))) == -1)
    {
        if ((msgId = msgget(msgKey, S_IRUSR|S_IWUSR)) == -1)
        {
            zlog_err(ZLOG_VOIP,"sip_msgq_create_read Error:%s ", strerror(errno));
            return ERROR;
        }
        else
        {
        }
    }
	//if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "sip_msgq_create_read read msgq@%d on :%d", msgId, msgKey);
	}
    return msgId;
}

static int sip_msgq_create_write(int msgKey)
{
    int msgId;
    // make applId as msg key
    if ((msgId = msgget(msgKey, S_IRUSR|S_IWUSR)) == -1)
    {
        zlog_err(ZLOG_VOIP,"sip_msgq_create_write Error:%s ", strerror(errno));
        return ERROR;
    }
	//if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "sip_msgq_create_write write msgq@%d on :%d", msgId, msgKey);
	}
    return msgId;
}
/*static int sip_msgq_fflush(int msgId)
{
	char buf[1024];
	while(1)
	{
		if(msgrcv(msgId, buf, sizeof(buf), 0, IPC_NOWAIT) == -1)
		{
		//	if(errno == ENOMSG)
				return OK;
		}
	}
	return OK;
}*/

/*static int sip_msgq_buffsize(int msgId, int qBytes)
{
    struct msqid_ds msg_stat;
    qBytes = (qBytes&(~0x3ff)) + 0x400;
    if( 0 != msgctl(msgId, IPC_STAT, &msg_stat) )
    {
    	 zlog_err(ZLOG_VOIP,"sip_msgq_create IPC_STAT Error:%s", strerror(errno));
        return ERROR;
    }
    msg_stat.msg_qbytes = qBytes;
    if( 0 != msgctl(msgId, IPC_SET, &msg_stat) )
    {
         zlog_err(ZLOG_VOIP,"sip_msgq_create IPC_SET Error:%s", strerror(errno));
        return ERROR;
    }
    if( 0 != msgctl(msgId, IPC_STAT, &msg_stat) )
    {
         zlog_err(ZLOG_VOIP,"sip_msgq_create IPC_STAT Error:%s", strerror(errno));
        return ERROR;
    }
    if ( msg_stat.msg_qbytes == qBytes )
    {
         zlog_err(ZLOG_VOIP," msg queue size %d\r\n",qBytes);
    }
	if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "set msgq@%d buffer size %d", msgId, qBytes);
	}
    return OK;
}*/

static int sip_msgq_delete(int msgId)
{
    if( 0 != msgctl(msgId, IPC_RMID, NULL))
    {
        zlog_err(ZLOG_VOIP,"sip_msgq_delete Error:%s", strerror(errno));
        return ERROR;
    }
	if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "sip_msgq_delete msgq@%d",msgId);
	}
    return OK;
}

static int sip_msgq_recv(int msgId, void* pMsgHdr, int uiMaxBytes, int msgType, int wait)
{
    int size;
    if(ERROR == (size = msgrcv(msgId, pMsgHdr, uiMaxBytes, msgType, 0)))
    {
        return ERROR;
    }
    if (0 == size)
    {
        return ERROR;
    }
    return size;
}

static int x5_b_a_hex_debug(char *hdr, char *aa, int len, int rx)
{
	char buf[1560];
	char tmp[16];
	u_int8 *p = aa;
	int i = 0;
	return 0;
	//int len = len;
	p = (u_int8 *)aa;
	memset(buf, 0, sizeof(buf));
	for(i = 0; i < len; i++)
	{
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "0x%02x ", p[i]);
		if(i%6 == 0)
			strcat(buf, " ");
		if(i%12 == 0)
			strcat(buf, "\r\n");
		strcat(buf, tmp);
	}
	zlog_debug(ZLOG_VOIP, "%s : %s", hdr, buf);
	return OK;
}

static int sip_msgq_send(int msgQId, char* pMsg, int len)
{
	int tryCounter = 0;
	if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "send msg to msgq@%d buffer size %d", msgQId, len);
		//x5_b_a_hex_debug("SEND", pMsg, len, 0);
	}
	while (1)
	{
		if(msgsnd(msgQId, pMsg, len - 4, IPC_NOWAIT) < 0)
		{
			if (((EINTR == errno)||(EAGAIN == errno)) && (tryCounter < 5))
			{
				tryCounter++;
				os_msleep(10+10*tryCounter);
				continue;
			}
	     	zlog_debug(ZLOG_VOIP, "send msg to msgq : %s", strerror(errno));
			return ERROR;
		}
		else
		{
			return OK;
		}
	}
    return OK;
}

static int sip_msgq_task(void *p)
{
	char buf[1024];
	int ret = 0;
	voip_sip_ctl_t *sipctl = (voip_sip_ctl_t *)p;

	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	sipctl->rq = sip_msgq_create_read(SIP_CTL_LMSGQ_KEY);

	while(1)
	{
		sipctl->rq = 32769;
		printf( "RECV MSGQ ID %d\n", sipctl->rq);
		ret = sip_msgq_recv(sipctl->rq, buf, sizeof(buf), -2, 1);
		if(ret > 0)
		{
			if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(DETAIL))
			{
				printf("recv msg from msgq@%d buffer len %d\n", sipctl->rq, ret);
				x5_b_a_hex_debug("RECV", buf, ret, 0);
			}
			printf( "befor voip_sip_read_handle\n");
			voip_sip_read_handle(sipctl, buf, ret);
			printf("after voip_sip_read_handle\n");
		}
		else
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				os_msleep(100);
				continue;
			}
			sleep(1);
			printf("sip_msgq_task\n");
		}
	}
	return ERROR;
}


static int sip_msgq_task_init(voip_sip_ctl_t *sipctl)
{
	if(sipctl->taskid)
		return OK;
	sipctl->taskid = os_task_create("sipMsgQ", OS_TASK_DEFAULT_PRIORITY,
	               0, sip_msgq_task, &sipctl, OS_TASK_DEFAULT_STACK);
	if(sipctl->taskid)
		return OK;
	return ERROR;
}


static int sip_msgq_task_exit(voip_sip_ctl_t *sipctl)
{
	if(sipctl->taskid)
	{
		if(os_task_destroy(sipctl->taskid)==OK)
			sipctl->taskid = 0;
	}
	return OK;
}
#endif

