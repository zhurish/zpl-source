/*
 * rbtree.h
 *
 *  Created on: Mar 15, 2018
 *      Author: Administrator
 *     Version: V1.0
 */

#ifndef __RBTREE_H_
#define __RBTREE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"

#if defined(container_of)
  #undef container_of
  #define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (zpl_char *)__mptr - offsetof(type,member) );})
#else
  #define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (zpl_char *)__mptr - offsetof(type,member) );})
#endif

#if defined(offsetof)
  #undef offsetof
  #define offsetof(TYPE, MEMBER) ((zpl_size_t) &((TYPE *)0)->MEMBER)
#else
  #define offsetof(TYPE, MEMBER) ((zpl_size_t) &((TYPE *)0)->MEMBER)
#endif
/*
#undef NULL
#if defined(__cplusplus)
  #define NULL 0
#else
  #define NULL ((void *)0)
#endif
*/

#define	RB_RED		0
#define	RB_BLACK	1


struct rb_node {
	zpl_ulong  __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));
    /* The alignment might seem pointless, but allegedly CRIS needs it */

struct rb_root {
	struct rb_node *rb_node;
	int	count;
	int	(*cmp)(void *, void *);
	int	(*del)(void *);
};


#define rb_parent(r)   ((struct rb_node *)((r)->__rb_parent_color & ~3))
#define rb_color(r)   ((r)->__rb_parent_color & 1)
#define rb_is_red(r)   (!rb_color(r))
#define rb_is_black(r) rb_color(r)
#define rb_set_red(r)  do { (r)->__rb_parent_color &= ~1; } while (0)
#define rb_set_black(r)  do { (r)->__rb_parent_color |= 1; } while (0)

static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->__rb_parent_color = (rb->__rb_parent_color & 3) | (zpl_ulong)p;
}
static inline void rb_set_color(struct rb_node *rb, int color)
{
	rb->__rb_parent_color = (rb->__rb_parent_color & ~1) | color;
}



#define RB_ROOT	(struct rb_root) { NULL, }
#define	rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root)  ((root)->rb_node == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbree */
#define RB_EMPTY_NODE(node)  \
	((node)->__rb_parent_color == (zpl_ulong)(node))
#define RB_CLEAR_NODE(node)  \
	((node)->__rb_parent_color = (zpl_ulong)(node))


static inline void rb_init_node(struct rb_node *rb)
{
	rb->__rb_parent_color = 0;
	rb->rb_right = NULL;
	rb->rb_left = NULL;
	RB_CLEAR_NODE(rb);
}


extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);

/* Find logical next and previous nodes in a tree */
extern struct rb_node *rb_next(const struct rb_node *);
extern struct rb_node *rb_prev(const struct rb_node *);
extern struct rb_node *rb_first(const struct rb_root *);
extern struct rb_node *rb_last(const struct rb_root *);


/* Fast replacement of a single node without remove/rebalance/add/rebalance */
extern void rb_replace_node(struct rb_node *, struct rb_node *,
			    struct rb_root *);

static inline void rb_link_node(struct rb_node * node, struct rb_node * parent,
				struct rb_node ** rb_link)
{
	node->__rb_parent_color = (zpl_ulong )parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}


typedef void (*rb_augment_f)(struct rb_node *node, void *data);

extern void rb_augment_insert(struct rb_node *node,
			      rb_augment_f func, void *data);
extern struct rb_node *rb_augment_erase_begin(struct rb_node *node);
extern void rb_augment_erase_end(struct rb_node *node,
				 rb_augment_f func, void *data);


/******************************************************************/
//API
typedef struct rb_node RBNODE;
    /* The alignment might seem pointless, but allegedly CRIS needs it */

typedef struct rb_root RBLIST;



extern RBNODE *rblstFirst (RBLIST *pList);
extern RBNODE *rblstLast (RBLIST *pList);
extern RBNODE *rblstNext (RBNODE *pNode);
extern RBNODE *rblstPrevious (RBNODE *pNode);
extern RBNODE *rblstLookup (RBLIST *pList, RBNODE *pNode);
extern int 	rblstCount (RBLIST *pList);
//extern int 	rblstFind (RBLIST *pList, RBNODE *pNode);
extern void rblstAdd (RBLIST *pList, RBNODE *pNode);
extern void rblstDelete (RBLIST *pList, RBNODE *pNode);
extern void rblstFree (RBLIST *pList);
extern int rblstInit (RBLIST *pList, void *cmp, void *del);
//extern void rblstInsert (RBLIST *pList, RBNODE *pPrev, RBNODE *pNode);


 
#ifdef __cplusplus
}
#endif

#endif /* __RBTREE_H_ */
