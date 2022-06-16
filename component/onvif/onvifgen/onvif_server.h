/*
	onvif_server.h
*/
#ifndef __ONVIF_SERVER_H__
#define __ONVIF_SERVER_H__

#define ONVIF_MULTICAST_GROUP   "239.255.255.250" /* use a group IP such as "225.0.0.37" */
#define ONVIF_BIND_PORT         3702

#include "auto_include.h"
#include <zplos_include.h>
#include "lib_include.h"
#include "soapH.h"

#define ONVIF_ADDRESS_MAX       32
#define ONVIF_CLIENT_MAX        32

struct onvif_soapsrv_node
{
    struct soap soap;
    zpl_uint16  bindport;
    zpl_char    bindaddress[ONVIF_ADDRESS_MAX];
    void        *t_read;
    int         flag;
};

struct onvif_soapsrv
{
    struct soap dis_soap;
    zpl_char    dis_multicast[ONVIF_ADDRESS_MAX];
    zpl_uint16  dis_bindport;
    zpl_char    dis_bindaddress[ONVIF_ADDRESS_MAX];
    void        *t_read;
    struct onvif_soapsrv_node onvif_client[ONVIF_CLIENT_MAX];

    void        *master;
    zpl_taskid_t  taskid;
};


int onvif_soapsrv_client_add(struct onvif_soapsrv *srv, zpl_char *bindaddress, zpl_uint16 bindport);
int onvif_soapsrv_client_del(struct onvif_soapsrv *srv, zpl_char *bindaddress, zpl_uint16 bindport);
struct onvif_soapsrv_node *onvif_soapsrv_client_lookup(struct onvif_soapsrv *srv, zpl_char *bindaddress, zpl_uint16 bindport);

int onvif_soapsrv_cancel(struct onvif_soapsrv_node *node);
int onvif_soapsrv_dis_cancel(struct onvif_soapsrv *srv);

int onvif_soapsrv_init(struct onvif_soapsrv *srv);
int onvif_soapsrv_exit(struct onvif_soapsrv *srv);
int onvif_soapsrv_task_init(struct onvif_soapsrv *srv);
int onvif_soapsrv_task_exit(struct onvif_soapsrv *srv);



#endif /* __ONVIF_SERVER_H__ */
