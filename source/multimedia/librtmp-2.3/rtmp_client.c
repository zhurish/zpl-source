/*  RTMPClient
 *  Copyright (C) 2009 Andrej Stepanchuk
 *  Copyright (C) 2009 Howard Chu
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RTMPDump; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "zplos_include.h"
#include "rtmp_sys.h"
#include "rtmp_log.h"
#include "rtmp_client.h"

/**
 * 本程序用于接收RTMP流媒体并在本地保存成FLV格式的文件。
 * This program can receive rtmp live stream and save it as local flv file.
 */

static int rtmp_client_restart(rtmp_client_t *client);
static int rtmp_client_task(rtmp_client_t *client);

static int rtmp_client_read_thread(void *p)
{
  rtmp_client_t *client = (rtmp_client_t *)p;
  if (client)
  {
    int nRead = RTMP_Read(client->rtmp, client->buf, client->maxlen);
    if(nRead)
    {
        //fwrite(client->buf, 1, nRead, fp);
        //RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB\n", nRead, countbufsize * 1.0 / 1024);
    }
    else
    {
      os_ansync_cancel(client->t_master, client->t_read);
      client->t_read = NULL;
      client->state = RTMP_CLIENT_CLOSE;
      RTMP_Close(client->rtmp);
      client->t_time = os_ansync_add(client->t_master, OS_ANSYNC_TIMER_ONCE, rtmp_client_restart, client, 1000);
    }
  }
  return 0;
}

static int rtmp_client_restart(rtmp_client_t *client)
{
    if (!RTMP_Connect(client->rtmp, NULL))
    {
        RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
        client->t_time = os_ansync_add(client->t_master, OS_ANSYNC_TIMER_ONCE, rtmp_client_restart, client, 1000);
        return -1;
    }
    client->state = RTMP_CLIENT_CONNECT;
    if (!RTMP_ConnectStream(client->rtmp, 0))
    {
        RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
        RTMP_Close(client->rtmp);
        client->t_time = os_ansync_add(client->t_master, OS_ANSYNC_TIMER_ONCE, rtmp_client_restart, client, 1000);
        return -1;
    }
    client->state = RTMP_CLIENT_READY;
    client->t_read = os_ansync_add(client->t_master, OS_ANSYNC_INPUT, rtmp_client_read_thread, client, client->rtmp->m_sb.sb_socket);    
    return 0;
}

int rtmp_client_close(rtmp_client_t *client)
{
    if(RTMP_IsConnected(client->rtmp))
        RTMP_Close(client->rtmp);
    RTMP_Free(client->rtmp);
    if(client->t_master)
    {
        os_ansync_lst_destroy(client->t_master);
    }
    if(client->taskid)
    {
        os_task_destroy(client->taskid);   
    }
    client->rtmp = NULL;
    return 0;
}

int rtmp_client_start(rtmp_client_t *client)
{
    if (!RTMP_Connect(client->rtmp, NULL))
    {
        RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
        RTMP_Free(client->rtmp);
        return -1;
    }
    client->state = RTMP_CLIENT_CONNECT;
    if (!RTMP_ConnectStream(client->rtmp, 0))
    {
        RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
        RTMP_Close(client->rtmp);
        RTMP_Free(client->rtmp);
        return -1;
    }
    client->state = RTMP_CLIENT_READY;
    client->t_read = os_ansync_add(client->t_master, OS_ANSYNC_INPUT, rtmp_client_read_thread, client, client->rtmp->m_sb.sb_socket);
    return 0;
}

int rtmp_client_stop(rtmp_client_t *client)
{
    os_ansync_cancel(client->t_master, client->t_read);
    client->t_read = NULL;
    client->state = RTMP_CLIENT_STOP;
    return 0;
}

int rtmp_client_open(rtmp_client_t *client, char *url, int alive)
{
    /* set log level */
    // RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
    // RTMP_LogSetLevel(loglvl);
    client->t_master = os_ansync_lst_create(0, 1024);
    client->rtmp = RTMP_Alloc();
    RTMP_Init(client->rtmp);
    // set connection timeout,default 30s
    client->rtmp->Link.timeout = 10;
    // HKS's live URL
    if (!RTMP_SetupURL(client->rtmp, url))
    {
        RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
        RTMP_Free(client->rtmp);
        return -1;
    }
    if (alive)
    {
        client->rtmp->Link.lFlags |= RTMP_LF_LIVE;
    }
    // 1hour
    RTMP_SetBufferMS(client->rtmp, 3600 * 1000);
    client->taskid = os_task_create("rtmpclient", OS_TASK_DEFAULT_PRIORITY, 0, rtmp_client_task, client, OS_TASK_DEFAULT_STACK);
    return 0;
}


static int rtmp_client_task(rtmp_client_t *client)
{
    os_ansync_main(client->t_master, OS_ANSYNC_EXECUTE_ARGV);
    return 0;
}


#if 0
int main(int argc, char *argv[])
{
    double duration = -1;
    int nRead;
    // is live stream ?
    int bLiveStream = 1;

    int bufsize = 1024 * 1024 * 10;
    char *buf = (char *)malloc(bufsize);
    memset(buf, 0, bufsize);
    long countbufsize = 0;

    FILE *fp = fopen("receive.flv", "wb");
    if (!fp)
    {
        RTMP_LogPrintf("Open File Error.\n");
        return -1;
    }

    /* set log level */
    // RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
    // RTMP_LogSetLevel(loglvl);

    RTMP *rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);
    // set connection timeout,default 30s
    rtmp->Link.timeout = 10;
    // HKS's live URL
    if (!RTMP_SetupURL(rtmp, "rtmp://live.hkstv.hk.lxdns.com/live/hks"))
    {
        RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
        RTMP_Free(rtmp);
        return -1;
    }
    if (bLiveStream)
    {
        rtmp->Link.lFlags |= RTMP_LF_LIVE;
    }

    // 1hour
    RTMP_SetBufferMS(rtmp, 3600 * 1000);

    if (!RTMP_Connect(rtmp, NULL))
    {
        RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
        RTMP_Free(rtmp);
        return -1;
    }

    if (!RTMP_ConnectStream(rtmp, 0))
    {
        RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        return -1;
    }

    while (nRead = RTMP_Read(rtmp, buf, bufsize))
    {
        fwrite(buf, 1, nRead, fp);

        countbufsize += nRead;
        RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB\n", nRead, countbufsize * 1.0 / 1024);
    }

    if (fp)
        fclose(fp);

    if (buf)
    {
        free(buf);
    }

    if (rtmp)
    {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp = NULL;
    }
    return 0;
}
#endif