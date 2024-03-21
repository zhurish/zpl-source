/*
 * hal_igmp.c
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zpl_type.h"
#include "osker_list.h"
#include "os_ipstack.h"
#include "if.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_txrx.h"
#include "hal_netpkt.h"


struct hal_netpkt_filter
{
    struct osker_list_head node;
    struct hal_netpkt_field _netpkt_field;
    hal_netpkt_rx_cb rx_cb;
	zpl_socket_t	sock;	
};

static osker_list_head_t hal_netpkt_filter_lst;

static int hal_netpkt_filter_destroy(struct hal_netpkt_filter *session);


int hal_netpkt_filter_init(void)
{
    INIT_OSKER_LIST_HEAD(&hal_netpkt_filter_lst);
    return OK;
}

int hal_netpkt_filter_exit(void)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    struct hal_netpkt_filter *p = NULL;
    osker_list_for_each_safe(pos,n, &hal_netpkt_filter_lst)
    {
        p = (struct hal_netpkt_filter*)osker_list_entry(pos, struct hal_netpkt_filter, node);
        if(p)
        {
            osker_list_del(pos);
            hal_netpkt_filter_destroy(p);
            p = NULL;
        }
    }
    return 0;
}


static int hal_netpkt_filter_destroy(struct hal_netpkt_filter *session)
{
    if(session)
    {
		ipstack_drstroy(session->sock);
        free(session);
    }
    return 0;
}

static struct hal_netpkt_filter * hal_netpkt_filter_lookup(struct osker_list_head * list, struct hal_netpkt_field *session)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    struct hal_netpkt_filter *p = NULL;
    osker_list_for_each_safe(pos,n,list)
    {
        p = (struct hal_netpkt_filter*)osker_list_entry(pos, struct hal_netpkt_filter, node);
        if(p && memcpy(&p->_netpkt_field, session, sizeof(struct hal_netpkt_field))== 0)
        {
            return p;
        }
    }
    return NULL;
}

static struct hal_netpkt_filter * hal_netpkt_filter_add(struct osker_list_head * list, struct hal_netpkt_field *session, hal_netpkt_rx_cb rx_cb, zpl_socket_t sock)
{
    struct hal_netpkt_filter * newNode = NULL; //每次申请链表结点时所使用的指针
    newNode = (struct hal_netpkt_filter *)malloc(sizeof(struct hal_netpkt_filter));
	if(newNode)
	newNode->sock = ipstack_create(IPSTACK_OS);
    if(newNode && !ipstack_invalid(newNode->sock))
    {
        memset(&newNode->_netpkt_field, 0, sizeof(struct hal_netpkt_field));
		memcpy(&newNode->_netpkt_field, session, sizeof(struct hal_netpkt_field));
        newNode->rx_cb = rx_cb;
		ipstack_copy(sock, newNode->sock);
        osker_list_add_tail(&newNode->node, list); //调用list.h中的添加节点的函数osker_list_add_tail
        return newNode;
    }
    return newNode;
}

static int hal_netpkt_filter_del(struct osker_list_head * list, struct hal_netpkt_field *session)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    struct hal_netpkt_filter *p = NULL;
    osker_list_for_each_safe(pos,n,list)
    {
        p = (struct hal_netpkt_filter*)osker_list_entry(pos, struct hal_netpkt_filter, node);
        if(p && memcpy(&p->_netpkt_field, session, sizeof(struct hal_netpkt_field))== 0)
        {
            osker_list_del(pos);
            hal_netpkt_filter_destroy(p);
            p = NULL;
            return OK;
        }
    }
    return ERROR;
}

int hal_netpkt_filter_register(struct hal_netpkt_field *hal_netpkt_field, hal_netpkt_rx_cb rx_cb, zpl_socket_t sock)
{
    if(hal_netpkt_filter_lookup(&hal_netpkt_filter_lst, hal_netpkt_field))
        return ERROR;
    if(hal_netpkt_filter_add(&hal_netpkt_filter_lst, hal_netpkt_field, rx_cb, sock))
        return OK;
    return ERROR;    
}


int hal_netpkt_filter_unregister(struct hal_netpkt_field *hal_netpkt_field)
{
    return hal_netpkt_filter_del(&hal_netpkt_filter_lst, hal_netpkt_field);
}

int hal_netpkt_filter_distribute(char *data, int len)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    struct hal_netpkt_filter *p = NULL;
	struct hal_netpkt_field session;
	struct osker_list_head * list = &hal_netpkt_filter_lst;

    osker_list_for_each_safe(pos,n,list)
    {
        p = (struct hal_netpkt_filter*)osker_list_entry(pos, struct hal_netpkt_filter, node);
        if(p && memcpy(&p->_netpkt_field, &session, sizeof(struct hal_netpkt_field))== 0)
        {
			if(!ipstack_invalid(p->sock))
				ipstack_write(p->sock, data, len);
            return 0;
        }
    }
    return 1;
}



int hal_netpkt_send(ifindex_t ifindex, zpl_vlan_t vlanid, 
	zpl_uint8 pri, zpl_uchar *data, zpl_uint32 len)
{
	#if 1
	return 0;//hal_txrx_sendto_port((char *)data,  len, 0, 0, IF_IFINDEX_PHYID_GET(ifindex), vlanid, 0, 1);
	//hal_txrx_vlan_flood((char *)data,  len, 0, 0, vlanid, 0, 1);	
	#else
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	command = IPCCMD_SET(HAL_MODULE_CPU, HAL_MODULE_CMD_DATA, 0);
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//zpl_skb_data_t
	hal_ipcmsg_create_header(&ipcmsg, command);
	//hal_ipcmsg_data_set(&ipcmsg, ifindex, vlanid, pri);
	return hal_ipcmsg_send(IF_UNIT_ALL, &ipcmsg);
	#endif
}

