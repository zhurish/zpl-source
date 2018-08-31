/* ospf_table.h */

#if !defined (_OSPF_TABLE_H_)
#define _OSPF_TABLE_H_

#include "avl.h"

#ifdef __cplusplus
extern "C" {
#endif 

struct ospf_lstnode{
    struct avl_node avl_nd;
};

struct ospf_lst{
    /*use avl as internal struct*/
    struct avl_tree avl;
  
    /*first user node in avl, it's result is same as avl_first*/
    void *p_first;
    
  #ifndef OSPF_LIST_FUNCTION
    /*flag used to check if some node not init*/
    u_int cookie;      
  #endif
}; 

#if 1//ndef OSPF_LIST_FUNCTION
#define OSPF_TABLE_COOKIE 0xacbd
#define TABLE_BUG(p_table)    do{ if (p_table->cookie != OSPF_TABLE_COOKIE)ospf_lstwarning(p_table);}while(0)

/*scan first node, returned value is user node*/
#define ospf_lstfirst(lst) ({\
    void *n = NULL;\
    TABLE_BUG((lst));\
    ospf.stat.list_first++;\
    if (NULL == ((lst)->p_first)) \
        (lst)->p_first = avl_first(&((lst)->avl));\
    n = (lst)->p_first;\
    n = n;\
})

/*get last node*/
#define ospf_lstlast(lst) ({\
    void *n = NULL;\
    TABLE_BUG((lst));\
    n = avl_last(&((lst)->avl)) ;\
    n = n;\
})

/*get first node > input node*/
#define ospf_lstgreater(lst, info) ({\
    avl_index_t where;\
    void *p_node = NULL;\
    TABLE_BUG((lst));\
    p_node = avl_find((&((lst)->avl)), info, &where);\
    if (p_node != NULL)\
        p_node = AVL_NEXT((&((lst)->avl)), p_node) ;\
    else\
        p_node = avl_nearest((&((lst)->avl)), where, AVL_AFTER);\
    p_node = p_node;\
})

/*get node not less than input node,may same or larger*/
#define ospf_lstnoless(lst, info) ({\
    avl_index_t where;\
    void *p_node = NULL;\
    TABLE_BUG((lst));\
    p_node = avl_find((&((lst)->avl)), info, &where);\
    if (p_node == NULL)\
        p_node = avl_nearest((&((lst)->avl)), where, AVL_AFTER);\
    p_node = p_node;\
})

/*get next node,input node is user node and must be valid node in table, we will translate it*/
#define ospf_lstnext(lst, info) ({\
    void *x = NULL;\
   TABLE_BUG((lst));\
    if (info) x = AVL_NEXT((&((lst)->avl)), info) ;\
    x = x;\
})

/*lookup,input is user node*/
#define ospf_lstlookup(lst, info) ({\
    avl_index_t where;\
    void *x = NULL;\
    TABLE_BUG((lst));\
    ospf.stat.list_lookup++;\
    x = avl_find((&((lst)->avl)), info, &where);\
    x = x;\
})

/*add to table,input is user node,must not in any list before add*/
#define ospf_lstadd(lst, info) ({\
    STATUS x = OK;\
    TABLE_BUG((lst));\
    ospf.stat.list_add++;\
    if (ospf_lstlookup(lst, info)!=NULL)\
    {\
    }\
    avl_add((&((lst)->avl)), info);\
    (lst)->p_first = NULL;\
    x = x;\
})

/*remove from table, input is user node,must in table*/
#define ospf_lstdel(lst, info) ({\
    STATUS x = ERR;\
    if (info)\
    {\
        TABLE_BUG((lst));\
        ospf.stat.list_delete++;\
        if ((lst)->p_first == info)\
            (lst)->p_first = NULL;\
        if(&((lst)->avl))\
        {\
	        avl_remove((&((lst)->avl)), (info));\
        }\
        x = OK;\
    }\
    x = x;\
})

#define ospf_lstcnt(lst) ({\
    u_int x = 0 ;\
    TABLE_BUG((lst));\
    x = avl_numnodes((&((lst)->avl)));\
    x = x;\
})

#else
 INLINE void *ospf_lstfirst(struct ospf_lst *lst)
{ 
    if (NULL == (lst->p_first))  
        lst->p_first = avl_first(&(lst->avl)); 
    return lst->p_first; 
}

#define ospf_lstlast(lst) avl_last(&((lst)->avl))

 INLINE void *ospf_lstgreater(struct ospf_lst *lst, void *info)
{ 
    avl_index_t where; 
    void *p_node = NULL; 
    p_node = avl_find((&((lst)->avl)), info, &where); 
    if (p_node != NULL)\
        p_node = AVL_NEXT((&((lst)->avl)), p_node) ; 
    else 
        p_node = avl_nearest((&((lst)->avl)), where, AVL_AFTER); 
    return p_node;
}

 INLINE void *ospf_lstnoless(struct ospf_lst *lst, void *info)
{ 
    avl_index_t where; 
    void *p_node = NULL; 
    p_node = avl_find((&((lst)->avl)), info, &where); 
    if (p_node == NULL) 
        p_node = avl_nearest((&((lst)->avl)), where, AVL_AFTER); 
   return p_node; 
}

#define ospf_lstnext(lst, info) ((info) ? (AVL_NEXT((&((lst)->avl)), info)) : NULL)

 INLINE void *ospf_lstlookup(struct ospf_lst *lst, void *info)
{ 
    avl_index_t where; 
    return avl_find((&((lst)->avl)), info, &where); 
}

 INLINE STATUS ospf_lstadd(struct ospf_lst *lst, void *info)
{
    avl_add((&((lst)->avl)), info); 
    (lst)->p_first = NULL; 
    return OK;
}

 INLINE STATUS ospf_lstdel(struct ospf_lst *lst, void *info)
{ 
    STATUS x = ERR; 
    if (info) 
    { 
        if ((lst)->p_first == info) 
            (lst)->p_first = NULL; 
        avl_remove((&((lst)->avl)), (info)); 
        x = OK; 
     } 
    return x;
}

#define ospf_lstcnt(lst) avl_numnodes((&((lst)->avl)))
#endif


#ifdef for_each_node
#undef for_each_node 
#endif
#define for_each_node(p_table, x, n) for ((x) = ospf_lstfirst(p_table), \
    (n) = ospf_lstnext(p_table, (x)); (x); (x) = (n), (n) = ospf_lstnext(p_table, (x)))

#define for_each_node_range(t, x, minnode, maxnode) for (\
              (x) = ospf_lstgreater((t), minnode) ; \
              (x) && ((t)->avl.avl_compar((x), maxnode) <= 0);\
              (x) = ospf_lstnext((t), (x)))

//#define OSPF_KEY_CMP(x, y, z) do{if (((x))->z != ((y))->z) return ((u_long)(((x))->z) > (u_long)(((y))->z)) ? 1 : -1;}while(0)
#define OSPF_KEY_CMP(x, y, z) do{if(!x || !y) return 1; if (((x))->z != ((y))->z) return ((u_long)(((x))->z) > (u_long)(((y))->z)) ? 1 : -1;}while(0)

//#define OSPF_KEY_HOST_CMP(x, y, z) do{if (((x))->z != ((y))->z) return (ntohl(((u_long)(((x))->z))) > ntohl(((u_long)(((y))->z)))) ? 1 : -1;}while(0)
#define OSPF_KEY_HOST_CMP(x, y, z) do{if(!x || !y) return 1;if (((x))->z != ((y))->z) return (ntohl(((u_long)(((x))->z))) > ntohl(((u_long)(((y))->z)))) ? 1 : -1;}while(0)

/*delete a node from list,and free it*/
#define ospf_lstdel_free(p_table, n, t) do\
{\
   ospf_lstdel(p_table, n);\
   ospf_mfree(n, t);\
}while(0)

/*just walkup a list, do not care about if node will be deleted*/
#define ospf_lstwalkup(p_table, oper) do\
   {\
    void *xxx = NULL;\
    void *yyy = NULL;\
    for_each_node(p_table, xxx, yyy)\
    {\
         oper(xxx);\
    }}while(0)

/*start scan from greater node*/    
#define for_each_node_greater(t, x, minnode) for (\
              (x) = ospf_lstgreater(t, minnode) ; \
              (x) ;\
              (x) = ospf_lstnext(t, x))

/*start scan from current or greater node*/
#define for_each_node_noless(t, x, minnode) for (\
              (x) = ospf_lstnoless(t, minnode) ; \
              (x) ;\
              (x) = ospf_lstnext(t, x))

/*init for a special list,no offset*/
#define ospf_lstinit(p_list, cmp)  ospf_lstinit2(p_list, cmp, 0)

void ospf_lstwarning(void *p_table);
void ospf_lstinit2(struct ospf_lst * p_table, void * cmp, u_int off);
void ospf_lstadd_unsort(struct ospf_lst * p_table, void * p_info);
void ospf_lstdel_unsort(struct ospf_lst * p_table, void * p_info);

#ifdef __cplusplus
}
#endif

#endif

