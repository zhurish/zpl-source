/*
 * voip_sip_ctl_msgq.c
 *
 *  Created on: Jan 18, 2019
 *      Author: zhurish
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


static int sip_ctl_msgq_fflush(int msgId);
static int sip_ctl_msgq_create_read(int msgKey);
static int sip_ctl_msgq_get_write(int msgKey);

static int sip_ctl_msgq_delete(int msgId);



int sip_ctl_msgq_init(voip_sip_ctl_t *sipctl)
{
	sipctl->sip_rqueue = -1;
	sipctl->sip_wqueue = -1;
	sipctl->sip_rqueue = sip_ctl_msgq_create_read(SIP_CTL_LMSGQ_KEY);
	if(SIP_CTL_DEBUG(MSGQ))
		zlog_debug(ZLOG_VOIP, "Create SIP Recv MSGQ msgq@%d on %d)", sipctl->sip_rqueue, SIP_CTL_LMSGQ_KEY);
	return OK;
}

int sip_ctl_msgq_exit(voip_sip_ctl_t *sipctl)
{
	//sip_ctl_msgq_delete(sipctl->sip_wqueue);
	sip_ctl_msgq_delete(sipctl->sip_rqueue);
	sipctl->sip_rqueue = -1;
	sipctl->sip_wqueue = -1;
	return OK;
}

static int sip_ctl_msgq_create_read(int msgKey)
{
    int msgId = -1;
    // make applId as msg key
    if ((msgId = msgget(msgKey, (S_IRUSR|S_IWUSR|IPC_CREAT/*|IPC_EXCL*/))) == -1)
    {
        if ((msgId = msgget(msgKey, S_IRUSR|S_IWUSR)) == -1)
        {
            zlog_err(ZLOG_VOIP,"sip msgq create (key=0x%x)Error:%s ", msgKey, strerror(errno));
            return ERROR;
        }
    }
    sip_ctl_msgq_fflush(msgId);
	//zlog_debug(ZLOG_VOIP, "sip_ctl_msgq_create_read  msgq@%d on :%d", msgId, msgKey);
    return msgId;
}


static int sip_ctl_msgq_get_write(int msgKey)
{
    int msgId = -1;
    // make applId as msg key
    if ((msgId = msgget(msgKey, S_IRUSR|S_IWUSR)) == -1)
    {
        zlog_err(ZLOG_VOIP,"sip msgq get (key=0x%x) Error:%s ", msgKey, strerror(errno));
        return ERROR;
    }
    //zlog_debug(ZLOG_VOIP, "sip_ctl_msgq_get_write  msgq@%d on :%d", msgId, msgKey);
    return msgId;
}


static int sip_ctl_msgq_delete(int msgId)
{
    if( 0 != msgctl(msgId, IPC_RMID, NULL))
    {
        zlog_err(ZLOG_VOIP,"sip msgq delete msgid=%d Error:%s", msgId, strerror(errno));
        return ERROR;
    }
	if(SIP_CTL_DEBUG(MSGQ))
	{
		zlog_debug(ZLOG_VOIP, "sip msgq delete msgq@%d",msgId);
	}
    return OK;
}

static int sip_ctl_msgq_fflush(int msgId)
{
    char buf[1024];
    while(1)
    {
    	if(msgrcv(msgId, buf, sizeof(buf), 0, IPC_NOWAIT) == -1)
    		break;
    }
    return OK;
}

static int sip_ctl_msgq_hex_debug(char *hdr, char *aa, int len, int rx)
{
	char buf[1560];
	char tmp[16];
	u_int8 *p = aa;
	int i = 0;
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


static int sip_ctl_msgq_send_hw(voip_sip_ctl_t *sipctl, char* pMsg, int len)
{
	int tryCounter = 0;
	if(sipctl && (sipctl->sip_wqueue < 0))
		sipctl->sip_wqueue = sip_ctl_msgq_get_write(SIP_CTL_MSGQ_KEY);

	while (sipctl->sip_wqueue >= 0)
	{
/*
		if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "SIP Module send msgq@%d on 0x%x",sipctl->sip_wqueue, SIP_CTL_MSGQ_KEY);
*/

		if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(SEND))
		{
			zlog_debug(ZLOG_VOIP, "SIP Module send %d byte on msgq@%d", len - 4, sipctl->sip_wqueue);
			if(SIP_CTL_DEBUG(DETAIL))
				sip_ctl_msgq_hex_debug("SEND", pMsg, len - 4, 0);
		}

		if(msgsnd(sipctl->sip_wqueue, pMsg, len - 4, IPC_NOWAIT) < 0)
		{
			if (((EINTR == errno)||(EAGAIN == errno)) && (tryCounter < 5))
			{
				tryCounter++;
				os_msleep(10+10*tryCounter);
				continue;
			}
			if(SIP_CTL_DEBUG(MSGQ))
				zlog_err(ZLOG_VOIP, "SIP Module send msg to msgq@%d on 0x%x (%s)", sipctl->sip_wqueue, SIP_CTL_MSGQ_KEY, strerror(errno));
			return ERROR;
		}
		else
		{
			return OK;
		}
	}
    return ERROR;
}

int sip_ctl_msgq_send(voip_sip_ctl_t *sipctl, char* pMsg, int len)
{
	return sip_ctl_msgq_send_hw(sipctl, pMsg, len);
}


static int sip_ctl_msgq_task(void *p)
{
	int ret = 0;
	voip_sip_ctl_t *sipctl = (voip_sip_ctl_t *)p;

	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	if(sipctl->sip_rqueue < 0)
		sipctl->sip_rqueue = sip_ctl_msgq_create_read(SIP_CTL_LMSGQ_KEY);
#ifdef DOUBLE_PROCESS
	voip_sip_process_init(sipctl);
	//os_time_create_once(voip_sip_process_init, sipctl, 1000);
#endif
	while(1)
	{
/*
		if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "SIP Module recv msgq@%d on 0x%x",sipctl->sip_rqueue, SIP_CTL_LMSGQ_KEY);
*/
		ret = msgrcv(sipctl->sip_rqueue, sipctl->buf, sizeof(sipctl->buf), 0, 0);
		//ret = sip_ctl_msgq_recv(sipctl->sip_rqueue, buf, sizeof(buf), -2, 1);
		if(ret > 0)
		{
			sipctl->len = ret;
			if(SIP_CTL_DEBUG(MSGQ) && SIP_CTL_DEBUG(RECV))
			{
				zlog_debug(ZLOG_VOIP, "SIP Module recv %d byte on msgq@%d", ret, sipctl->sip_rqueue);
				if(SIP_CTL_DEBUG(DETAIL))
					sip_ctl_msgq_hex_debug("RECV", sipctl->buf, sipctl->len, 0);
			}
			voip_sip_respone_handle(sipctl, sipctl->buf, sipctl->len);
		}
		else
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				os_msleep(100);
				continue;
			}
			//sleep(1);
			//printf("sip_ctl_msgq_task\n");
		}
	}
	return ERROR;
}


int sip_ctl_msgq_task_init(voip_sip_ctl_t *sipctl)
{
	if(sipctl->taskid)
		return OK;
	sipctl->taskid = os_task_create("sipMsgQ", OS_TASK_DEFAULT_PRIORITY,
	               0, sip_ctl_msgq_task, sipctl, OS_TASK_DEFAULT_STACK);
	if(sipctl->taskid)
		return OK;
	return ERROR;
}


int sip_ctl_msgq_task_exit(voip_sip_ctl_t *sipctl)
{
	if(sipctl->taskid)
	{
		if(os_task_destroy(sipctl->taskid)==OK)
			sipctl->taskid = 0;
	}
	return OK;
}
#endif
