#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "auto_include.h"
#include "zplos_include.h"

#include "cJSON.h"
#include "os_netservice.h"

struct zpl_osnet_service _os_netservice;

static void cjson_free(void *ptr)
{
    free(ptr);
}

static int os_netservice_json_write_obj(cJSON *obj, struct zpl_osnet_service *ua)
{
    if (obj && ua)
    {
        cJSON_AddItemToObject(obj, "telnet_port", cJSON_CreateNumber(ua->telnet_port));
        cJSON_AddItemToObject(obj, "tftpd_port", cJSON_CreateNumber(ua->tftpd_port));
        cJSON_AddItemToObject(obj, "ftpd_port", cJSON_CreateNumber(ua->ftpd_port));
        cJSON_AddItemToObject(obj, "sntpd_port", cJSON_CreateNumber(ua->sntpd_port));
        cJSON_AddItemToObject(obj, "zclient_port", cJSON_CreateNumber(ua->zclient_port));
        cJSON_AddItemToObject(obj, "standbyd_port", cJSON_CreateNumber(ua->standbyd_port));
        cJSON_AddItemToObject(obj, "ipbc_port", cJSON_CreateNumber(ua->ipbc_port));
        cJSON_AddItemToObject(obj, "hal_port", cJSON_CreateNumber(ua->hal_port));
        cJSON_AddItemToObject(obj, "pal_port", cJSON_CreateNumber(ua->pal_port));
        cJSON_AddItemToObject(obj, "rtsp_port", cJSON_CreateNumber(ua->rtsp_port));
        cJSON_AddItemToObject(obj, "web_port", cJSON_CreateNumber(ua->web_port));
        cJSON_AddItemToObject(obj, "sshd_port", cJSON_CreateNumber(ua->sshd_port));
        cJSON_AddItemToObject(obj, "dhcpd_port", cJSON_CreateNumber(ua->dhcpd_port));
        cJSON_AddItemToObject(obj, "modbus_port", cJSON_CreateNumber(ua->modbus_port));
        cJSON_AddItemToObject(obj, "mqtt_port", cJSON_CreateNumber(ua->mqtt_port));
        return OK;
    }
    return ERROR;
}

static int os_netservice_json_read_obj(cJSON *obj, struct zpl_osnet_service *ua)
{
    if (obj && ua)
    {
        if (cJSON_GetObjectItem(obj, "slot"))
            ua->slot = cJSON_GetObjectItemIntValue(obj, "slot");
        if (cJSON_GetObjectItem(obj, "vty_port"))
            ua->vty_port = cJSON_GetObjectItemIntValue(obj, "vty_port");
        if (cJSON_GetObjectItem(obj, "telnet_port"))
            ua->telnet_port = cJSON_GetObjectItemIntValue(obj, "telnet_port");
        if (cJSON_GetObjectItem(obj, "tftpd_port"))
            ua->tftpd_port = cJSON_GetObjectItemIntValue(obj, "tftpd_port");
        if (cJSON_GetObjectItem(obj, "ftpd_port"))
            ua->ftpd_port = cJSON_GetObjectItemIntValue(obj, "ftpd_port");
        if (cJSON_GetObjectItem(obj, "sntpd_port"))
            ua->sntpd_port = cJSON_GetObjectItemIntValue(obj, "sntpd_port");
        if (cJSON_GetObjectItem(obj, "zclient_port"))
            ua->zclient_port = cJSON_GetObjectItemIntValue(obj, "zclient_port");
        if (cJSON_GetObjectItem(obj, "standbyd_port"))
            ua->standbyd_port = cJSON_GetObjectItemIntValue(obj, "standbyd_port");
          if (cJSON_GetObjectItem(obj, "standbyc_port"))
            ua->standbyc_port = cJSON_GetObjectItemIntValue(obj, "standbyc_port");
        if (cJSON_GetObjectItem(obj, "ipbc_port"))
            ua->ipbc_port = cJSON_GetObjectItemIntValue(obj, "ipbc_port");
        if (cJSON_GetObjectItem(obj, "hal_port"))
            ua->hal_port = cJSON_GetObjectItemIntValue(obj, "hal_port");
        if (cJSON_GetObjectItem(obj, "pal_port"))
            ua->pal_port = cJSON_GetObjectItemIntValue(obj, "pal_port");
        if (cJSON_GetObjectItem(obj, "rtsp_port"))
            ua->rtsp_port = cJSON_GetObjectItemIntValue(obj, "rtsp_port");
        if (cJSON_GetObjectItem(obj, "web_port"))
            ua->web_port = cJSON_GetObjectItemIntValue(obj, "web_port");
        if (cJSON_GetObjectItem(obj, "web_sslport"))
            ua->web_sslport = cJSON_GetObjectItemIntValue(obj, "web_sslport");
        if (cJSON_GetObjectItem(obj, "sshd_port"))
            ua->sshd_port = cJSON_GetObjectItemIntValue(obj, "sshd_port");

        if (cJSON_GetObjectItem(obj, "dhcpd_port"))
            ua->dhcpd_port = cJSON_GetObjectItemIntValue(obj, "dhcpd_port");
        if (cJSON_GetObjectItem(obj, "dhcpc_port"))
            ua->dhcpc_port = cJSON_GetObjectItemIntValue(obj, "dhcpc_port");
        if (cJSON_GetObjectItem(obj, "dhcpd6_port"))
            ua->dhcpd6_port = cJSON_GetObjectItemIntValue(obj, "dhcpd6_port");
         if (cJSON_GetObjectItem(obj, "dhcpc6_port"))
            ua->dhcpc6_port = cJSON_GetObjectItemIntValue(obj, "dhcpc6_port");

        if (cJSON_GetObjectItem(obj, "modbus_port"))
            ua->modbus_port = cJSON_GetObjectItemIntValue(obj, "modbus_port");
        if (cJSON_GetObjectItem(obj, "mqtt_port"))
            ua->mqtt_port = cJSON_GetObjectItemIntValue(obj, "mqtt_port");
        return OK;
    }
    return ERROR;
}

static int os_netservice_default(struct zpl_osnet_service *ua)
{
    if (ua)
    {
        ua->vty_port = PLCLI_VTY_PORT;
        ua->telnet_port = 23;
        ua->tftpd_port = 69;
        ua->ftpd_port = 21;
        ua->sntpd_port = 580;
        ua->zclient_port = 65500;
        ua->standbyd_port = 65501;
        ua->standbyc_port = 65502;
        ua->ipbc_port = 65510;
        ua->hal_port = 65513;
        ua->pal_port = 65524;
        ua->rtsp_port = 554;
        ua->web_port = 8080;
        ua->web_sslport = 443;
        ua->sshd_port = 4022;
        ua->dhcpd_port = 67;
        ua->dhcpc_port = 547;
        ua->dhcpd6_port = 68;
        ua->dhcpc6_port = 546;
        ua->modbus_port = 5445;
        ua->mqtt_port = 1883;
        return OK;
    }
    return ERROR;
}


int os_netservice_port_get(char *name)
{
    if (strcasecmp(name, "vty_port") == 0)
        return _os_netservice.vty_port;
    if (strcasecmp(name, "slot") == 0)
        return _os_netservice.slot;
    if (strcasecmp(name, "telnet_port") == 0)
        return _os_netservice.telnet_port;
    if (strcasecmp(name, "tftpd_port") == 0)
        return _os_netservice.tftpd_port;
    if (strcasecmp(name, "ftpd_port") == 0)
        return _os_netservice.ftpd_port;
    if (strcasecmp(name, "sntpd_port") == 0)
        return _os_netservice.sntpd_port;
    if (strcasecmp(name, "zclient_port") == 0)
        return _os_netservice.zclient_port;
    if (strcasecmp(name, "standbyd_port") == 0)
        return _os_netservice.standbyd_port;
    if (strcasecmp(name, "standbyc_port") == 0)
        return _os_netservice.standbyc_port;
    if (strcasecmp(name, "ipbc_port") == 0)
        return _os_netservice.ipbc_port;
    if (strcasecmp(name, "hal_port") == 0)
        return _os_netservice.hal_port;
    if (strcasecmp(name, "pal_port") == 0)
        return _os_netservice.pal_port;
    if (strcasecmp(name, "rtsp_port") == 0)
        return _os_netservice.rtsp_port;
    if (strcasecmp(name, "web_port") == 0)
        return _os_netservice.web_port;
    if (strcasecmp(name, "sshd_port") == 0)
        return _os_netservice.sshd_port;

    if (strcasecmp(name, "dhcpd_port") == 0)
        return _os_netservice.dhcpd_port;
     if (strcasecmp(name, "dhcpc_port") == 0)
        return _os_netservice.dhcpc_port;
    if (strcasecmp(name, "dhcpd6_port") == 0)
        return _os_netservice.dhcpd6_port;
    if (strcasecmp(name, "dhcpc6_port") == 0)
        return _os_netservice.dhcpc6_port;

    if (strcasecmp(name, "modbus_port") == 0)
        return _os_netservice.modbus_port;
    if (strcasecmp(name, "mqtt_port") == 0)
        return _os_netservice.mqtt_port;
    return 0;
}

char *os_netservice_sockpath_get(char *name)
{
    char *ret = NULL, *p = name;
    static char sockpathtmp[128];
    memset(sockpathtmp, 0, sizeof(sockpathtmp)); 
    ret = strrchr(p, '.');
    if(ret)
    {
        strncpy(sockpathtmp, p, ret - p);
        snprintf(sockpathtmp+strlen(sockpathtmp), sizeof(sockpathtmp)-strlen(sockpathtmp), "-%u%s", getpid(), ret);
    }
    else 
        snprintf(sockpathtmp, sizeof(sockpathtmp), "%s.%u", name, getpid());
    return sockpathtmp;
}

int os_netservice_config_write(char *filename)
{
    int wrsize = 0;
    os_netservice_default(&_os_netservice);
    cJSON *tmpobj = cJSON_CreateObject();
    cJSON *pRoot = cJSON_CreateObject();
    if (tmpobj && pRoot)
    {
        cJSON_AddItemToObject(pRoot, "netservice", tmpobj);

        if (os_netservice_json_write_obj(tmpobj, &_os_netservice) != OK)
        {
            cJSON_Delete(tmpobj);
            return ERROR;
        }
        char *szJSON = NULL;
        szJSON = cJSON_Print(pRoot);
        if (szJSON)
        {
            wrsize = strlen(szJSON);
            if (os_write_file(filename, szJSON, wrsize) != OK)
            {
                cJSON_Delete(pRoot);
                cjson_free(szJSON);
                remove(filename);
                return ERROR;
            }
            cjson_free(szJSON);
        }
        cJSON_Delete(pRoot);
        return OK;
    }
    return ERROR;
}

int os_netservice_config_load(char *filename)
{
    memset(&_os_netservice, 0, sizeof(_os_netservice));
    os_netservice_default(&_os_netservice);
    if (os_file_access(filename) != OK)
    {
        printf("os_netservice_config_load :%s is not exist.\r\n", filename);
        return ERROR;
    }
    int file_size = (int)os_file_size(filename);
    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        // printf("os_netservice_config_load : can not malloc buffer(%d byte)\r\n", file_size);
        return ERROR;
    }
    memset(buffer, 0, file_size + 1);
    if (os_read_file(filename, buffer, file_size) != OK)
    {
        // printf("os_netservice_config_load : can not read buffer(%d byte)\r\n", file_size);
        goto on_error;
    }
    cJSON *tmpobj = cJSON_Parse(buffer);
    if (tmpobj)
    {
        cJSON *pItem = cJSON_GetObjectItem(tmpobj, "netservice");
        if (pItem)
        {
            if (os_netservice_json_read_obj(pItem, &_os_netservice) != OK)
            {
                cJSON_Delete(tmpobj);
                if (buffer != NULL)
                    free(buffer);
                return ERROR;
            }
            cJSON_Delete(tmpobj);
        }
    }
    if (buffer != NULL)
        free(buffer);
    return OK;

on_error:
    if (buffer != NULL)
        free(buffer);
    return ERROR;
}