/*
 * os_list_t.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_LIST_H__
#define __OS_LIST_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "zpl_type.h"

    /* type definitions */

    typedef struct node /* Node of a linked list. */
    {
        struct node *next;     /* Points at the next node in the list */
        struct node *previous; /* Points at the previous node in the list */
        void    *data;
    } NODE, os_listnode;

    /* HIDDEN */

    typedef struct /* Header for a linked list. */
    {
        NODE node;        /* Header list node */
        zpl_uint32 count; /* Number of nodes in list */
        void (*free)(void *);
        int (*cmp)(void *, void *);
    } LIST, os_list_t;

    /* END_HIDDEN */

    /* function declarations */

    extern void lstLibInit(void);
    extern NODE *lstFirst(LIST *pList);
    extern NODE *lstGet(LIST *pList);
    extern NODE *lstLast(LIST *pList);
    extern NODE *lstNStep(NODE *pNode, zpl_int32 nStep);
    extern NODE *lstNext(NODE *pNode);
    extern NODE *lstNth(LIST *pList, zpl_int32 nodenum);
    extern NODE *lstPrevious(NODE *pNode);
    extern zpl_uint32 lstCount(LIST *pList);
    extern int lstFind(LIST *pList, NODE *pNode);
    extern void lstAdd(LIST *pList, NODE *pNode);
    extern void lstConcat(LIST *pDstList, LIST *pAddList);
    extern void lstDelete(LIST *pList, NODE *pNode);
    extern void lstExtract(LIST *pSrcList, NODE *pStartNode, NODE *pEndNode,
                           LIST *pDstList);
    extern void lstFree(LIST *pList);
    extern void lstInit(LIST *pList);
    extern void lstInsert(LIST *pList, NODE *pPrev, NODE *pNode);
    extern void lstInsertBefore(LIST *pList, NODE *pPrev, NODE *pNode);
    extern void lstSortInit(LIST *pList, int (*cmp)(void *, void *));
    extern void lstInitFree(LIST *pList, void (*freecb)(void *));
    extern void lstAddSort(LIST *pList, NODE *pNode);
    extern NODE *lstLookup(LIST *pList, NODE *p);
    extern void lstQsort(LIST *pList, int (*cmp)(void *, void *));


#define os_listprevnode(X) ((X) ? ((X)->previous) : NULL)
#define os_listnextnode(X) ((X) ? ((X)->next) : NULL)
#define os_listhead(X) ((X) ? ((X)->node.next) : NULL)
#define os_listtail(X) ((X) ? ((X)->node.previous) : NULL)
#define os_listcount(X) ((X)->count)
#define os_listisempty(X) ((X)->node.next == NULL && (X)->node.previous == NULL)
#define os_listgetdata(X) (assert((X)->data != NULL), (X)->data)

/* Prototypes. */
/* Prototypes. */
extern os_list_t *os_list_new(void); /* encouraged: set list.del callback on new lists */
extern void os_list_free (os_list_t *);
extern void os_list_init(os_list_t *pList, void (*freecb)(void *));

extern void os_listnode_add (os_list_t *, void *);
extern void os_listnode_add_sort (os_list_t *, void *);
extern void os_listnode_add_after (os_list_t *, os_listnode *, void *);
extern os_listnode *os_listnode_add_before (os_list_t *, os_listnode *, void *);
extern void os_listnode_move_to_tail (os_list_t *, os_listnode *);
extern void os_listnode_delete (os_list_t *, void *);
extern os_listnode *os_listnode_lookup (os_list_t *, void *);
extern void *os_listnode_head (os_list_t *);
extern os_listnode *os_listnode_getprev (os_list_t *, void *);
extern os_listnode *os_listnode_getnext (os_list_t *, void *);
extern void os_list_delete (os_list_t *);
extern void os_list_delete_withdata (os_list_t *);
extern void os_list_delete_all_node_withdata (os_list_t *);
extern void os_list_delete_all_node (os_list_t *);
/* For ospfd and ospf6d. */
extern void os_list_delete_node (os_list_t *, os_listnode *);
extern void os_list_add_node (os_list_t *list, os_listnode *node);
/* For ospf_spf.c */
extern void os_list_add_node_prev (os_list_t *, os_listnode *, void *);
extern void os_list_add_node_next (os_list_t *, os_listnode *, void *);
extern void os_list_add_list (os_list_t *, os_list_t *);
extern void os_list_concat(os_list_t* list, os_list_t* second);
extern void os_list_unconcat(os_list_t* list, os_list_t* second);
extern void os_list_qsort(os_list_t *list, int (*cmp)(void *, void *));
extern void os_list_for_each(const os_list_t* list, int (*func)(void *));
extern void os_list_for_each2(const os_list_t* list, int (*func)(void *, void *), void *user_data);

/* List iteration macro. 
 * Usage: for (ALL_LIST_ELEMENTS (...) { ... }
 * It is safe to delete the lstnode using this macro.
 */
#define OS_ALL_LIST_ELEMENTS(list,node,nextnode,data) \
  (node) = os_listhead(list), ((data) = NULL); \
  (node) != NULL && \
    ((data) = os_listgetdata(node),(nextnode) = node->next, 1); \
  (node) = (nextnode), ((data) = NULL)

/* read-only lst iteration macro.
 * Usage: as per ALL_LIST_ELEMENTS, but not safe to delete the lstnode Only
 * use this macro when it is *immediately obvious* the lstnode is not
 * deleted in the body of the loop. Does not have forward-reference overhead
 * of previous macro.
 */
#define OS_ALL_LIST_ELEMENTS_RO(list,node,data) \
  (node) = os_listhead(list), ((data) = NULL);\
  (node) != NULL && ((data) = os_listgetdata(node), 1); \
  (node) = os_listnextnode(node), ((data) = NULL)

/* these *do not* cleanup list nodes and referenced data, as the functions
 * do - these macros simply {de,at}tach a listnode from/to a list.
 */

/* List node attach macro.  */
#define OS_LISTNODE_ATTACH(L,N) \
  do { \
    (N)->previous = (L)->node.previous; \
    (N)->next = NULL; \
    if ((L)->node.next == NULL) \
      (L)->node.next = (N); \
    else \
      (L)->node.previous->next = (N); \
    (L)->node.previous = (N); \
    (L)->count++; \
  } while (0)

/* List node detach macro.  */
#define OS_LISTNODE_DETACH(L,N) \
  do { \
    if ((N)->previous) \
      (N)->previous->next = (N)->next; \
    else \
      (L)->node.next = (N)->next; \
    if ((N)->next) \
      (N)->next->previous = (N)->previous; \
    else \
      (L)->node.previous = (N)->previous; \
    (L)->count--; \
  } while (0)



#ifdef __cplusplus
}
#endif

#endif /* __OS_LIST_H__ */
