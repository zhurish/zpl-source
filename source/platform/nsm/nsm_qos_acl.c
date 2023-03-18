/*
 * nsm_qos.c
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"

#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "algorithm.h"
#include "nsm_qos.h"
#include "nsm_interface.h"
#include "nsm_qos_acl.h"

//static qos_access_list_t _qos_alc_t;
static global_qos_access_list_t _global_qos_access_list;


static int _qos_access_filter_free_layer(qos_access_filter_t *node)
{
    XFREE(MTYPE_QOS_ACL_NODE, node);
    return OK;
}

static int _qos_access_filter_list_destroy_layer(qos_access_filter_list_t *node)
{
    qos_access_filter_t *pstNode = NULL;
    NODE index;
    if (node->mutex)
        os_mutex_lock(node->mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_access_filter_t *)lstFirst(&node->list);
         pstNode != NULL; pstNode = (qos_access_filter_t *)lstNext((NODE *)&index))
    {
        index = pstNode->node;
        if (pstNode)
        {
            lstDelete(&_global_qos_access_list._qos_alc_t.list, (NODE *)pstNode);
            XFREE(MTYPE_QOS_ACL_NODE, pstNode);
        }
    }
    if (node->mutex)
    {
        os_mutex_unlock(node->mutex);
        os_mutex_destroy(node->mutex);
    }
    return OK;
}

static qos_access_filter_t *_qos_access_filter_alloc_layer(void)
{
    qos_access_filter_t *node = XMALLOC(MTYPE_QOS_ACL_NODE, sizeof(qos_access_filter_t));
    if (node)
    {
        memset(node, 0, sizeof(qos_access_filter_t));
    }
    return node;
}

static int _qos_access_filter_list_add_layer(qos_access_filter_list_t *acllist, qos_access_filter_t *node)
{
    if (acllist->mutex)
        os_mutex_lock(acllist->mutex, OS_WAIT_FOREVER);
    lstAdd(&acllist->list, (NODE *)node);
    if (acllist->mutex)
        os_mutex_unlock(acllist->mutex);
    return OK;
}

static int _qos_access_filter_list_del_layer(qos_access_filter_list_t *acllist, qos_access_filter_t *node)
{
    if (acllist->mutex)
        os_mutex_lock(acllist->mutex, OS_WAIT_FOREVER);
    lstDelete(&acllist->list, (NODE *)node);
    XFREE(MTYPE_QOS_ACL_NODE, node);
    if (acllist->mutex)
        os_mutex_unlock(acllist->mutex);
    return OK;
}

static qos_access_filter_t *_qos_access_filter_list_lookup_layer(qos_access_filter_list_t *acllist, zpl_int32 seqnum, qos_access_filter_t *node)
{
    qos_access_filter_t *pstNode = NULL;
    NODE index;
    if (acllist->mutex)
        os_mutex_lock(acllist->mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_access_filter_t *)lstFirst(&acllist->list);
         pstNode != NULL; pstNode = (qos_access_filter_t *)lstNext((NODE *)&index))
    {
        index = pstNode->node;
        if (seqnum >= 0 && pstNode->seqnum == seqnum)
        {
            if (acllist->mutex)
                os_mutex_unlock(acllist->mutex);
            return pstNode;
        }
        if (node && node->nodetype == pstNode->nodetype && node->type == pstNode->type)
        {
            if(node->nodetype == FILTER_COMMON)
            {
                if(filter_compare_cisco(&pstNode->u.cfilter, &node->u.cfilter))
                {
                    if (acllist->mutex)
                        os_mutex_unlock(acllist->mutex);
                    return pstNode;
                }
            }
            else if(node->nodetype == FILTER_ZEBOS)
            {
                if(filter_compare_zebra(&pstNode->u.zfilter, &node->u.zfilter))
                {
                    if (acllist->mutex)
                        os_mutex_unlock(acllist->mutex);
                    return pstNode;
                }
            }
#ifdef ZPL_FILTER_NORMAL_EXT
            else if(node->nodetype == FILTER_ZEBOS_EXT)
            {
                if(filter_compare_zebos_extended(&pstNode->u.zextfilter, &node->u.zextfilter))
                {
                    if (acllist->mutex)
                        os_mutex_unlock(acllist->mutex);
                    return pstNode;
                }
            }
#endif
#ifdef ZPL_FILTER_MAC
            else if(node->nodetype == FILTER_MAC)
            {
                if(filter_compare_l2mac(&pstNode->u.mac_filter, &node->u.mac_filter))
                {
                    if (acllist->mutex)
                        os_mutex_unlock(acllist->mutex);
                    return pstNode;
                }
            }
#endif
        }
    }
    if (acllist->mutex)
        os_mutex_unlock(acllist->mutex);
    return NULL;
}

static int _qos_access_filter_list_foreach_layer(qos_access_filter_list_t *acllist, int (*cb)(void *, qos_access_filter_t *), void *pVoid)
{
    qos_access_filter_t *pstNode = NULL;
    NODE index;
    if (acllist->mutex)
        os_mutex_lock(acllist->mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_access_filter_t *)lstFirst(&acllist->list);
         pstNode != NULL; pstNode = (qos_access_filter_t *)lstNext((NODE *)&index))
    {
        if (pstNode)
        {
            index = pstNode->node;
            if (cb)
                (cb)(pVoid, pstNode);
        }
    }
    if (acllist->mutex)
        os_mutex_unlock(acllist->mutex);
    return OK;
}


qos_access_filter_t *qos_access_filter_alloc(void)
{
    return _qos_access_filter_alloc_layer();
}

int qos_access_filter_free(qos_access_filter_t *node)
{
    if(node)
    {
        XFREE(MTYPE_QOS_ACL_NODE, node);
    }
    return OK;
}

enum filter_type qos_access_filter_list_apply(qos_access_filter_list_t *acllist, void *object)
{
    struct prefix *p;
    p = (struct prefix *)object;
#ifdef ZPL_FILTER_NORMAL_EXT
    struct filter_zebos_ext *exp = (struct filter_zebos_ext *)object;
#endif
#ifdef ZPL_FILTER_MAC
    struct filter_l2 *l2mac = (struct filter_l2 *)object;
#endif
    qos_access_filter_t *pstNode = NULL;
    NODE index;
    if (acllist == NULL)
        return FILTER_DENY;
    if (acllist->mutex)
        os_mutex_lock(acllist->mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_access_filter_t *)lstFirst(&acllist->list);
         pstNode != NULL; pstNode = (qos_access_filter_t *)lstNext((NODE *)&index))
    {
        if (pstNode)
        {
            index = pstNode->node;
            if (pstNode->nodetype == FILTER_COMMON)
            {
                if (filter_match_cisco(&pstNode->u.cfilter, p))
                    return pstNode->type;
            }
            else if (pstNode->nodetype == FILTER_ZEBOS)
            {
                if (filter_match_zebra(&pstNode->u.zfilter, p))
                    return pstNode->type;
            }
#ifdef ZPL_FILTER_NORMAL_EXT
            else if (pstNode->nodetype == FILTER_ZEBOS_EXT)
            {
                if (filter_match_zebos_ext(&pstNode->u.zextfilter, exp))
                    return pstNode->type;
            }
#endif
#ifdef ZPL_FILTER_MAC
            else if (pstNode->nodetype == FILTER_MAC)
            {
                if (filter_match_mac(&pstNode->u.mac_filter, l2mac))
                    return pstNode->type;
            }
#endif
        }
    }
    if (acllist->mutex)
        os_mutex_unlock(acllist->mutex);
    return FILTER_DENY;
}

int qos_access_filter_list_add(qos_access_filter_list_t *acllist, qos_access_filter_t *node)
{
    return _qos_access_filter_list_add_layer(acllist, node);
}

int qos_access_filter_list_del(qos_access_filter_list_t *acllist, qos_access_filter_t *node)
{
    return _qos_access_filter_list_del_layer(acllist, node);
}

qos_access_filter_t *qos_access_filter_list_lookup(qos_access_filter_list_t *acllist, zpl_int32 seqnum, qos_access_filter_t *node)
{
    return _qos_access_filter_list_lookup_layer(acllist, seqnum, node);
}

int qos_access_filter_list_foreach(qos_access_filter_list_t *acllist, int (*cb)(void *, qos_access_filter_t *), void *pVoid)
{
    return _qos_access_filter_list_foreach_layer(acllist, cb, pVoid);
}



/* qos acl list */

static qos_access_filter_list_t *_qos_access_filter_list_create_layer(char *name)
{
    qos_access_filter_list_t *node = XMALLOC(MTYPE_QOS_ACL, sizeof(qos_access_filter_list_t));
    if (node)
    {
        os_memset(node, 0, sizeof(qos_access_filter_list_t));
        lstInitFree(&node->list, _qos_access_filter_free_layer);
        node->mutex = os_mutex_name_create(os_name_format("%s-mutex",name));
        node->seqnum_step = SEQNUM_STEP;
        strcpy(node->name, name);
    }
    return node;
}

static qos_access_filter_list_t *_qos_access_list_list_lookup_layer(char *name)
{
    qos_access_filter_list_t *pstNode = NULL;
    NODE index;
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_lock(_global_qos_access_list._qos_alc_t.mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_access_filter_list_t *)lstFirst(&_global_qos_access_list._qos_alc_t.list);
         pstNode != NULL; pstNode = (qos_access_filter_list_t *)lstNext((NODE *)&index))
    {
        index = pstNode->node;
        if (strcmp(pstNode->name, name) == 0)
        {
            if (_global_qos_access_list._qos_alc_t.mutex)
                os_mutex_unlock(_global_qos_access_list._qos_alc_t.mutex);
            return pstNode;
        }
    }
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_unlock(_global_qos_access_list._qos_alc_t.mutex);
    return NULL;
}

static int _qos_access_list_list_add_layer(qos_access_filter_list_t *node)
{
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_lock(_global_qos_access_list._qos_alc_t.mutex, OS_WAIT_FOREVER);
    lstAdd(&_global_qos_access_list._qos_alc_t.list, (NODE *)node);
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_unlock(_global_qos_access_list._qos_alc_t.mutex);
    return OK;
}

static int _qos_access_list_list_del_layer(qos_access_filter_list_t *node)
{
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_lock(_global_qos_access_list._qos_alc_t.mutex, OS_WAIT_FOREVER);
    lstDelete(&_global_qos_access_list._qos_alc_t.list, (NODE *)node);
    _qos_access_filter_list_destroy_layer(node);
    XFREE(MTYPE_QOS_ACL, node);
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_unlock(_global_qos_access_list._qos_alc_t.mutex);
    return OK;
}

static int _qos_access_list_list_foreach_layer(int (*cb)(void *, qos_access_filter_list_t *), void *pVoid)
{
    qos_access_filter_list_t *pstNode = NULL;
    NODE index;
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_lock(_global_qos_access_list._qos_alc_t.mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_access_filter_list_t *)lstFirst(&_global_qos_access_list._qos_alc_t.list);
         pstNode != NULL; pstNode = (qos_access_filter_list_t *)lstNext((NODE *)&index))
    {
        if (pstNode)
        {
            index = pstNode->node;
            if (cb)
                (cb)(pVoid, pstNode);
        }
    }
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_unlock(_global_qos_access_list._qos_alc_t.mutex);
    return OK;
}

qos_access_filter_list_t *qos_access_list_create(char *name)
{
    if (name)
    {
        qos_access_filter_list_t *node = _qos_access_filter_list_create_layer(name);
        if(node)
            qos_access_list_add(node);
        return node;
    }
    return NULL;
}

int qos_access_list_add(qos_access_filter_list_t *node)
{
    return _qos_access_list_list_add_layer(node);
}

qos_access_filter_list_t *qos_access_list_lookup(char *name)
{
    return _qos_access_list_list_lookup_layer(name);
}

int qos_access_list_reference(char *name, zpl_bool enable)
{
    qos_access_filter_list_t *node = _qos_access_list_list_lookup_layer(name);
    if(node)
    {
        if(enable)
            node->ref_cnt += 1;
        else if(node->ref_cnt)
        {
            node->ref_cnt -= 1;
        }    
        return OK;
    }
    return ERROR;
}

int qos_access_list_destroy(char *name)
{
    if (name)
    {
        qos_access_filter_list_t *node = _qos_access_list_list_lookup_layer(name);
        if (node)
        {
            if(node->ref_cnt == 0)
                return _qos_access_list_list_del_layer(node);
            else if(node->ref_cnt > 0)
                return IPSTACK_ERRNO_EBUSY;    
        }
    }
    return ERROR;
}

int qos_access_list_foreach(int (*cb)(void *, qos_access_filter_list_t *), void *pVoid)
{
    return _qos_access_list_list_foreach_layer(cb, pVoid);
}

/* class-map */

qos_class_map_t *qos_class_map_create(char *name)
{
    qos_class_map_t *node = XMALLOC(MTYPE_QOS_CLASS_MAP, sizeof(qos_class_map_t));
    if (node)
    {
        os_memset(node, 0, sizeof(qos_class_map_t));
        strcpy(node->name, name);
        qos_class_map_add(node);
    }
    return node;
}

qos_class_map_t *qos_class_map_lookup(char *name)
{
    qos_class_map_t *pstNode = NULL;
    NODE index;
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_lock(_global_qos_access_list.class_map_mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_class_map_t *)lstFirst(&_global_qos_access_list.class_map_list);
         pstNode != NULL; pstNode = (qos_class_map_t *)lstNext((NODE *)&index))
    {
        index = pstNode->node;
        if (strcmp(pstNode->name, name) == 0)
        {
            if (_global_qos_access_list.class_map_mutex)
                os_mutex_unlock(_global_qos_access_list.class_map_mutex);
            return pstNode;
        }
    }
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_unlock(_global_qos_access_list.class_map_mutex);
    return NULL;
}

int qos_class_map_reference(char *name, zpl_bool enable)
{
    qos_class_map_t *node = qos_class_map_lookup(name);
    if(node)
    {
        if(enable)
            node->ref_cnt += 1;
        else if(node->ref_cnt)
        {
            node->ref_cnt -= 1;
        }    
        return OK;
    }
    return ERROR;
}

int qos_class_map_add(qos_class_map_t *node)
{
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_lock(_global_qos_access_list.class_map_mutex, OS_WAIT_FOREVER);
    lstAdd(&_global_qos_access_list.class_map_list, (NODE *)node);
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_unlock(_global_qos_access_list.class_map_mutex);
    return OK;
}

int qos_class_map_del(qos_class_map_t *node)
{
    if(node->ref_cnt > 0)
        return IPSTACK_ERRNO_EBUSY; 
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_lock(_global_qos_access_list.class_map_mutex, OS_WAIT_FOREVER);
    lstDelete(&_global_qos_access_list.class_map_list, (NODE *)node);
    XFREE(MTYPE_QOS_CLASS_MAP, node);
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_unlock(_global_qos_access_list.class_map_mutex);
    return OK;
}

int qos_class_map_bind_access_list_set(qos_class_map_t *node, char *name, zpl_bool enable)
{
    int ret = ERROR;
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_lock(_global_qos_access_list.class_map_mutex, OS_WAIT_FOREVER);
    if(enable && name)
    {
        if(qos_access_list_reference(name, zpl_true) == OK)
        {
            memset(node->acl_name, 0, sizeof(node->acl_name));
            strncpy(node->acl_name, name, min(sizeof(node->acl_name),strlen(name)));
            ret = OK;
        }
    }
    if(!enable)
    {
        if(qos_access_list_reference(node->acl_name, zpl_false) == OK)
        {
            memset(node->acl_name, 0, sizeof(node->acl_name));
            ret = OK;
        }
    }
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_unlock(_global_qos_access_list.class_map_mutex);
    return ret;
}

char * qos_class_map_bind_access_list_get(qos_class_map_t *node)
{
    if(strlen(node->acl_name))
        return node->acl_name;
    return NULL;
}

int qos_class_map_limit_set(qos_class_map_t *node, nsm_qos_limit_t *rate)
{
    int ret = OK;
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_lock(_global_qos_access_list.class_map_mutex, OS_WAIT_FOREVER);
    os_memcpy(&node->stream_limit, rate, sizeof(nsm_qos_limit_t));
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_unlock(_global_qos_access_list.class_map_mutex);
    return ret;
}

int qos_class_map_limit_get(qos_class_map_t *node, nsm_qos_limit_t *rate)
{
    int ret = OK;
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_lock(_global_qos_access_list.class_map_mutex, OS_WAIT_FOREVER);
    os_memcpy(rate, &node->stream_limit, sizeof(nsm_qos_limit_t));
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_unlock(_global_qos_access_list.class_map_mutex);
    return ret;
}

int qos_class_map_foreach(int (*cb)(void *, qos_class_map_t *), void *pVoid)
{
    qos_class_map_t *pstNode = NULL;
    NODE index;
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_lock(_global_qos_access_list.class_map_mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_class_map_t *)lstFirst(&_global_qos_access_list.class_map_list);
         pstNode != NULL; pstNode = (qos_class_map_t *)lstNext((NODE *)&index))
    {
        if (pstNode)
        {
            index = pstNode->node;
            if (cb)
                (cb)(pVoid, pstNode);
        }
    }
    if (_global_qos_access_list.class_map_mutex)
        os_mutex_unlock(_global_qos_access_list.class_map_mutex);
    return OK;
}



/* service-policy */
qos_service_policy_t *qos_service_policy_create(char *name)
{
    qos_service_policy_t *node = XMALLOC(MTYPE_QOS_POLICY_MAP, sizeof(qos_service_policy_t));
    if (node)
    {
        os_memset(node, 0, sizeof(qos_service_policy_t));
        strcpy(node->name, name);
        qos_service_policy_add(node);
    }
    return node;
}

qos_service_policy_t *qos_service_policy_lookup(char *name)
{
    qos_service_policy_t *pstNode = NULL;
    NODE index;
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_lock(_global_qos_access_list.service_policy_mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_service_policy_t *)lstFirst(&_global_qos_access_list.service_policy_list);
         pstNode != NULL; pstNode = (qos_service_policy_t *)lstNext((NODE *)&index))
    {
        index = pstNode->node;
        if (strcmp(pstNode->name, name) == 0)
        {
            if (_global_qos_access_list.service_policy_mutex)
                os_mutex_unlock(_global_qos_access_list.service_policy_mutex);
            return pstNode;
        }
    }
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_unlock(_global_qos_access_list.service_policy_mutex);
    return NULL;
}

int qos_service_policy_reference(char *name, zpl_bool enable)
{
    qos_service_policy_t *node = qos_service_policy_lookup(name);
    if(node)
    {
        if(enable)
            node->ref_cnt += 1;
        else if(node->ref_cnt)
        {
            node->ref_cnt -= 1;
        }    
        return OK;
    }
    return ERROR;
}

int qos_service_policy_add(qos_service_policy_t *node)
{
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_lock(_global_qos_access_list.service_policy_mutex, OS_WAIT_FOREVER);
    lstAdd(&_global_qos_access_list.service_policy_list, (NODE *)node);
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_unlock(_global_qos_access_list.service_policy_mutex);
    return OK;
}

int qos_service_policy_del(qos_service_policy_t *node)
{
    if(node->ref_cnt > 0)
        return IPSTACK_ERRNO_EBUSY; 
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_lock(_global_qos_access_list.service_policy_mutex, OS_WAIT_FOREVER);
    if(node->ref_cnt == 0)
    {    
        lstDelete(&_global_qos_access_list.service_policy_list, (NODE *)node);
        XFREE(MTYPE_QOS_POLICY_MAP, node);
    }
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_unlock(_global_qos_access_list.service_policy_mutex);
    return OK;
}


int qos_service_policy_bind_class_map_set(qos_service_policy_t *node, char *name, zpl_bool enable)
{
    int ret = ERROR;
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_lock(_global_qos_access_list.service_policy_mutex, OS_WAIT_FOREVER);
    if(enable && name)
    {
        if(qos_class_map_reference(name, zpl_true) == OK)
        {
            memset(node->class_name, 0, sizeof(node->class_name));
            strncpy(node->class_name, name, min(sizeof(node->class_name),strlen(name)));
            ret = OK;
        }
    }
    if(!enable && name)
    {
        if(qos_class_map_reference(node->class_name, zpl_false) == OK)
        {
            memset(node->class_name, 0, sizeof(node->class_name));
            ret = OK;
        }
    }
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_unlock(_global_qos_access_list.service_policy_mutex);
    return ret;
}

char * qos_service_policy_bind_class_map_get(qos_service_policy_t *node)
{
    if(strlen(node->class_name))
        return node->class_name;
    return NULL;
}



int qos_service_policy_foreach(int (*cb)(void *, qos_service_policy_t *), void *pVoid)
{
    qos_service_policy_t *pstNode = NULL;
    NODE index;
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_lock(_global_qos_access_list.service_policy_mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_service_policy_t *)lstFirst(&_global_qos_access_list.service_policy_list);
         pstNode != NULL; pstNode = (qos_service_policy_t *)lstNext((NODE *)&index))
    {
        if (pstNode)
        {
            index = pstNode->node;
            if (cb)
                (cb)(pVoid, pstNode);
        }
    }
    if (_global_qos_access_list.service_policy_mutex)
        os_mutex_unlock(_global_qos_access_list.service_policy_mutex);
    return OK;
}

/************************************************************************/
/************************************************************************/
#ifdef ZPL_SHELL_MODULE
static int _qos_access_filter_show_one(struct vty *vty, qos_access_filter_t *node)
{

    vty_out(vty, " %d %s", node->seqnum, filter_type_str(node->type));
    // vty_out(vty, " %d %s", node->seqnum, filter_type_str(node->type));
    if (node->nodetype == FILTER_COMMON)
    {
        access_list_write_config_cisco(vty, &node->u.cfilter);
    }
    else if (node->nodetype == FILTER_ZEBOS)
    {
        access_list_write_config_zebra(vty, &node->u.zfilter);
    }
#ifdef ZPL_FILTER_NORMAL_EXT
    else if (node->nodetype == FILTER_ZEBOS_EXT)
    {
        access_list_write_config_zebos_ext(vty, &node->u.zextfilter);
    }
#endif
#ifdef ZPL_FILTER_MAC
    else if (node->nodetype == FILTER_MAC)
    {
        access_list_write_config_mac(vty, &node->u.mac_filter);
    }
#endif
    return OK;
}

static int _qos_access_list_node_show_one(struct vty *vty, qos_access_filter_list_t *node)
{
    if (node && vty)
    {
        if(lstCount(&node->list))
        {
            vty_out(vty, "access-list %s%s", node->name, VTY_NEWLINE);
            _qos_access_filter_list_foreach_layer(node, _qos_access_filter_show_one, vty);
        }
    }
    return OK;
}

int qos_access_list_write_config(struct vty *vty, char *name)
{
    if (name)
    {
        qos_access_filter_list_t *node = _qos_access_list_list_lookup_layer(name);
        if (node)
            _qos_access_list_node_show_one(vty, node);
    }
    else
        _qos_access_list_list_foreach_layer(_qos_access_list_node_show_one, vty);
    return 0;
}


static int _qos_class_map_show_one(struct vty *vty, qos_class_map_t *node)
{
    if (node && vty)
    {
        const char *matchstr[] = {"", "all", "any"};
        vty_out(vty, "class-map match-%s%s%s", matchstr[node->match_type], node->name, VTY_NEWLINE);
        if(strlen(node->acl_name))
            vty_out(vty, " match access-group %s%s", node->acl_name, VTY_NEWLINE);

		if (node->stream_limit.qos_cir)
		{
            zpl_char tmpstr[512];
			memset(tmpstr, 0, sizeof(tmpstr));
			if(node->stream_limit.qos_pir)
			{
				strcat(tmpstr, "pir ");
				strcat(tmpstr, itoa(node->stream_limit.qos_pir, 10));
				strcat(tmpstr, " ");
			}
			if(node->stream_limit.qos_cbs)
			{
				strcat(tmpstr, "cbs ");
				strcat(tmpstr, itoa(node->stream_limit.qos_cbs, 10));
				strcat(tmpstr, " ");
			}
            //policer cir <1-10000000> pir <0-4000000> cbs <0-4000000>
			vty_out(vty, " policer cir %d %s%s", node->stream_limit.qos_cir, strlen(tmpstr)?tmpstr:" ", VTY_NEWLINE);
		}
        //vty_out(vty, " access-group %s%s", node->acl_name, VTY_NEWLINE);
        //vty_out(vty, " access-group %s%s", node->acl_name, VTY_NEWLINE);
    }
    return OK;
}

int qos_class_map_write_config(struct vty *vty, char *name)
{
    if (name)
    {
        qos_class_map_t *node = qos_class_map_lookup(name);
        if (node)
            _qos_class_map_show_one(vty, node);
    }
    else
        qos_class_map_foreach(_qos_class_map_show_one, vty);
    return 0;
}

static int _qos_service_policy_show_one(struct vty *vty, qos_service_policy_t *node)
{
    if (node && vty)
    {
        vty_out(vty, "service-policy %s%s", node->name, VTY_NEWLINE);
        vty_out(vty, " class-match %s%s", node->class_name, VTY_NEWLINE);
    }
    return OK;
}

int qos_service_policy_write_config(struct vty *vty, char *name)
{
    if (name)
    {
        qos_service_policy_t *node = qos_service_policy_lookup(name);
        if (node)
            _qos_service_policy_show_one(vty, node);
    }
    else
        qos_service_policy_foreach(_qos_service_policy_show_one, vty);
    return 0;
}
#endif

/************************************************************************/
/************************************************************************/

static int _qos_policy_free_layer(qos_service_policy_t *node)
{
    XFREE(MTYPE_QOS_POLICY_MAP, node);
    return OK;
}

static int _qos_class_free_layer(qos_class_map_t *node)
{
    XFREE(MTYPE_QOS_CLASS_MAP, node);
    return OK;
}

static int _qos_access_list_list_init_layer(void)
{
    _global_qos_access_list.init = 1;
    os_memset(&_global_qos_access_list._qos_alc_t, 0, sizeof(qos_access_list_t));
    lstInitFree(&_global_qos_access_list._qos_alc_t.list, _qos_access_filter_list_destroy_layer);
    _global_qos_access_list._qos_alc_t.mutex = os_mutex_name_create("access-list-mutex");
    lstInitFree(&_global_qos_access_list.class_map_list, _qos_class_free_layer);
    lstInitFree(&_global_qos_access_list.service_policy_list, _qos_policy_free_layer);
    _global_qos_access_list.service_policy_mutex = os_mutex_name_create("service_policy_mutex");
    _global_qos_access_list.class_map_mutex = os_mutex_name_create("class_map_mutex");
    return OK;
}

static int _qos_access_list_list_cleanup_layer(int d)
{
    qos_access_filter_list_t *pstNode = NULL;
    NODE index;
    if (_global_qos_access_list._qos_alc_t.mutex)
        os_mutex_lock(_global_qos_access_list._qos_alc_t.mutex, OS_WAIT_FOREVER);
    for (pstNode = (qos_access_filter_list_t *)lstFirst(&_global_qos_access_list._qos_alc_t.list);
         pstNode != NULL; pstNode = (qos_access_filter_list_t *)lstNext((NODE *)&index))
    {
        index = pstNode->node;
        if (pstNode)
        {
            lstDelete(&_global_qos_access_list._qos_alc_t.list, (NODE *)pstNode);
            _qos_access_filter_list_destroy_layer(pstNode);
            XFREE(MTYPE_QOS_ACL, pstNode);
        }
    }
    if (_global_qos_access_list._qos_alc_t.mutex)
    {
        os_mutex_unlock(_global_qos_access_list._qos_alc_t.mutex);
        if (d)
            os_mutex_destroy(_global_qos_access_list._qos_alc_t.mutex);
    }
    return OK;
}

int qos_access_list_init(void)
{
    return _qos_access_list_list_init_layer();
}

int qos_access_list_exit(void)
{
    _global_qos_access_list.init = 0;
    return _qos_access_list_list_cleanup_layer(1);
}

int qos_access_list_clean(void)
{
    return _qos_access_list_list_cleanup_layer(0);
}

int qos_service_policy_clean(void)
{
    lstFree(&_global_qos_access_list.service_policy_list);
    return OK;
}

int qos_class_map_clean(void)
{
    lstFree(&_global_qos_access_list.class_map_list);
    return OK;
}