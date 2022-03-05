#include <netinet/ip_icmp.h>
#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "bsp_netpkt.h"

struct netpkt_filter
{
    struct osker_list_head node;
    struct netpkt_field netpkt_field;
    netpkt_rx_cb rx_cb;
};

static osker_list_head_t netpkt_filter_lst;
static int netpkt_filter_destroy(struct netpkt_filter *session);

int netpkt_filter_init(void)
{
    INIT_OSKER_LIST_HEAD(&netpkt_filter_lst);
    return OK;
}

int netpkt_filter_exit(void)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    struct netpkt_filter *p = NULL;
    osker_list_for_each_safe(pos,n, &netpkt_filter_lst)
    {
        p = (struct netpkt_filter*)osker_list_entry(pos, struct netpkt_filter, node);
        if(p)
        {
            osker_list_del(pos);
            netpkt_filter_destroy(p);
            p = NULL;
        }
    }
    return 0;
}


static int netpkt_filter_destroy(struct netpkt_filter *session)
{
    if(session)
    {
        free(session);
    }
    return 0;
}

static struct netpkt_filter * netpkt_filter_lookup(struct osker_list_head * list, struct netpkt_field *netpkt_field)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    struct netpkt_filter *p = NULL;
    osker_list_for_each_safe(pos,n,list)
    {
        p = (struct netpkt_filter*)osker_list_entry(pos, struct netpkt_filter, node);
        if(p && memcpy(&p->netpkt_field, netpkt_field, sizeof(struct netpkt_field))== 0)
        {
            return p;
        }
    }
    return NULL;
}

static struct netpkt_filter * netpkt_filter_add(struct osker_list_head * list, struct netpkt_field *netpkt_field, netpkt_rx_cb rx_cb)
{
    struct netpkt_filter * newNode = NULL; //每次申请链表结点时所使用的指针
    newNode = (struct netpkt_filter *)malloc(sizeof(struct netpkt_filter));
    if(newNode)
    {
        memset(&newNode->netpkt_field, 0, sizeof(struct netpkt_field));
        newNode->rx_cb = rx_cb;
        osker_list_add_tail(&newNode->node, list); //调用list.h中的添加节点的函数osker_list_add_tail
        return newNode;
    }
    return newNode;
}

static int netpkt_filter_del(struct osker_list_head * list, struct netpkt_field *netpkt_field)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    struct netpkt_filter *p = NULL;
    osker_list_for_each_safe(pos,n,list)
    {
        p = (struct netpkt_filter*)osker_list_entry(pos, struct netpkt_filter, node);
        if(p && memcpy(&p->netpkt_field, netpkt_field, sizeof(struct netpkt_field))== 0)
        {
            osker_list_del(pos);
            netpkt_filter_destroy(p);
            p = NULL;
            return OK;
        }
    }
    return ERROR;
}

int netpkt_filter_register(struct netpkt_field *netpkt_field, netpkt_rx_cb rx_cb)
{
    if(netpkt_filter_lookup(&netpkt_filter_lst, netpkt_field))
        return ERROR;
    if(netpkt_filter_add(&netpkt_filter_lst, netpkt_field, rx_cb))
        return OK;
    return ERROR;    
}


int netpkt_filter_unregister(struct netpkt_field *netpkt_field)
{
    return netpkt_filter_del(&netpkt_filter_lst, netpkt_field);
}


