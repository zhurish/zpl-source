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


voip_sip_t voip_sip_config;
static voip_sip_ctl_t voip_sip_ctl;

static int voip_sip_socket_init(voip_sip_ctl_t *sipctl);
static int voip_sip_socket_exit(voip_sip_ctl_t *sipctl);
static int voip_sip_read_handle(voip_sip_ctl_t *sipctl, char *buf, int len);


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
			fprintf(fp, "realm = %s\n", SIP_REALM_DEFAULT);

		fprintf(fp, "local_ip = %s\n", inet_address(0));
		fprintf(fp, "local_port = %d\n", sip->sip_local_port);
		fprintf(fp, "dtmf = %d\n", "rfc2833");
		fflush(fp);
		fclose(fp);
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
		vty_out(vty, " sip loal-port        :%d%s", (sip->sip_local_port), VTY_NEWLINE);

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
 * event socket (SIP)
 */

int voip_sip_ctl_module_init()
{
	os_memset(&voip_sip_ctl, 0, sizeof(voip_sip_ctl));
	voip_sip_ctl.tcpMode = TRUE;
	if(master_eloop[MODULE_VOIP] == NULL)
		master_eloop[MODULE_VOIP] = eloop_master_module_create(MODULE_VOIP);
	voip_sip_ctl.master = master_eloop[MODULE_VOIP];
	return voip_sip_socket_init(&voip_sip_ctl);
}

int voip_sip_ctl_module_exit()
{
	voip_sip_socket_exit(&voip_sip_ctl);
	os_memset(&voip_sip_ctl, 0, sizeof(voip_sip_ctl));
	return OK;
}


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
			sipctl->sock = fd;
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

static int voip_sip_socket_exit(voip_sip_ctl_t *sipctl)
{
	if(sipctl && sipctl->t_accept)
	{
		eloop_cancel(sipctl->t_accept);
		sipctl->t_accept = NULL;
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
			close(sipctl->sock);
		memset(sipctl->buf, 0, sizeof(sipctl->buf));
		return OK;
	}
	return ERROR;
}


static int voip_sip_send_msg(voip_sip_ctl_t *sipctl)
{
	if(sipctl && sipctl->sock)
		write(sipctl->sock, sipctl->sbuf, sipctl->slen);
	return OK;
}


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
				sipctl->len = len;
				voip_sip_read_handle(sipctl, sipctl->buf, len);
			}
			else
			{
				if (ERRNO_IO_RETRY(errno))
				{
					//return 0;
					//mgt->t_reset = eloop_add_timer_msec(mgt->master, x5_b_a_reset_eloop, mgt, 100);
					goto r_again;
				}
				else
				{
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




static int voip_sip_write_and_wait_respone(voip_sip_ctl_t *sipctl, int timeoutms)
{
	int rep = 0,ret = 0;
	if(sipctl->t_read)
	{
		eloop_cancel(sipctl->t_read);
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

/*
 * SIP ---> CTL
 */
static int voip_sip_register_ack(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_REG_RLT *ack;
	ack = (MSG_REG_ACT *)SIP_MSG_OFFSET(buf);
	if(hdr->type == SIP_REGISTER_ACK_MSG)
	{
		if(ack->rlt == SIP_REGISTER_OK)
		{
			voip_state_set(VOIP_STATE_REGISTER_SUCCESS);
			return OK;
		}
		voip_state_set(VOIP_STATE_REGISTER_FAILED);
	}
	return ERROR;
}

//被叫振铃
static int voip_sip_call_ring(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_REMOT_ALERT *ack;
	ack = (MSG_REMOT_ALERT *)SIP_MSG_OFFSET(buf);
	if(hdr->type == SIP_REMOTE_RING_MSG)
	{
		//if(ack->rlt == SIP_REGISTER_OK)
		if(ack->rtp_port)
		{
			voip_stream_remote_address_port_api(voip_stream, ack->rtp_addr, ack->rtp_port);
			voip_stream_payload_type_api(voip_stream, NULL, ack->codec);
		}
		voip_state_set(VOIP_STATE_CALLING);
			return OK;
	}
	return ERROR;
}

//被叫摘机
static int voip_sip_call_picking(voip_sip_ctl_t *sipctl, char *buf, int len)
{
	MSG_HDR_T *hdr = (	MSG_HDR_T *)buf;
	MSG_REMOT_ANSWER *ack;
	ack = (MSG_REMOT_ANSWER *)SIP_MSG_OFFSET(buf);
	if(hdr->type == SIP_REMOTE_PICKING_MSG)
	{
		if(ack->rtp_port && strlen(ack->rtp_addr))
		{
			voip_stream_remote_address_port_api(voip_stream, ack->rtp_addr, ack->rtp_port);
			voip_stream_payload_type_api(voip_stream, NULL, ack->codec);
		}
		//if(ack->rlt == SIP_REGISTER_OK)
		voip_state_set(VOIP_STATE_CALL_SUCCESS);
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
	if(hdr->type == SIP_CALL_ERROR_MSG)
	{
		//if(ack->rlt == SIP_REGISTER_OK)
		voip_state_set(VOIP_STATE_CALL_FAILED);
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
	if(hdr->type == SIP_REMOTE_STOP_MSG)
	{
		//if(ack->rlt == SIP_REGISTER_OK)
		voip_state_set(VOIP_STATE_BYE);

			return OK;
	}
	return ERROR;
}

static int voip_sip_read_handle(voip_sip_ctl_t *sipctl, char *buf, int len)
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
	case SIP_CALL_ERROR_MSG:
		ret = voip_sip_call_error(sipctl, buf, len);
		break;
	case SIP_REMOTE_STOP_MSG:
		ret = voip_sip_call_stop_by_remote(sipctl, buf, len);
		break;
	case SIP_LOCAL_STOP_MSG:
		break;
	default:
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
	hdr->srcApplId 	= VOIP_SIP_SRCID;  /*源进程号  VOIP--6，linectl---8*/
	hdr->dstAppId 	= VOIP_SIP_DSTID;   /*目的进程号*/
	hdr->type		= type;     /*消息类型 1--注册；2--注册返回，3--*/
	hdr->sync		= TRUE;      /* 是否是同步消息 */
	hdr->len		= htonl(SIP_MSG_LEN(len));       /* 消息长度，包含消息头 */
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
	return voip_sip_write_and_wait_respone(&voip_sip_ctl, SIP_CTL_TIMEOUT);
}


int voip_sip_call_start(char *phone)
{
	MSG_CAL_ACT *act;
	voip_sip_ctl.send_cmd	=	SIP_CALL_MSG;
	voip_sip_ctl.ack_cmd		=	SIP_REMOTE_RING_MSG | SIP_REMOTE_PICKING_MSG | SIP_CALL_ERROR_MSG;
	memset(voip_sip_ctl.sbuf, 0, sizeof(voip_sip_ctl.sbuf));
	act = (MSG_CAL_ACT *)SIP_MSG_OFFSET(voip_sip_ctl.sbuf);
	act->uid 		= 1;
	act->senid 		= 1;
	strcpy(act->digit, phone); /* 被叫号码*/
	act->digit_num = strlen(act->digit); /*被叫号码长度*/
	//act->name[48];  /*被叫名称*/
	//act->name_num;  /*被叫名称长度*/
	act->rtp_port 	= htonl(voip_stream->l_rtp_port);  /*本地rtp端口*/
	//act->rtp_addr	= 0;  /*本地RTP地址*/
	act->codec		= voip_stream->payload;     /*优先使用的codec*/
	voip_sip_ctl.slen = voip_sip_hdr_make(&voip_sip_ctl, SIP_CALL_MSG, voip_sip_ctl.sbuf, sizeof(MSG_CAL_ACT));
	return voip_sip_write_and_wait_respone(&voip_sip_ctl, SIP_CTL_TIMEOUT);
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
	voip_sip_ctl.slen = voip_sip_hdr_make(&voip_sip_ctl, SIP_LOCAL_STOP_MSG, voip_sip_ctl.sbuf, sizeof(MSG_LOCAL_BYE));
	return voip_sip_write_and_wait_respone(&voip_sip_ctl, SIP_CTL_TIMEOUT);
}











/*
 * SIP state Module
 */
voip_state_t voip_sip_state_get_api()
{
	return VOIP_STATE_CALL_SUCCESS;
}



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



