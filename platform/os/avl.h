/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef	_AVL_H
#define	_AVL_H


#ifdef	__cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "os_include.h"
#include "zpl_include.h"
typedef zpl_ulong	avl_uintptr_t;


#define COMPILE_32
/*
 * generic AVL tree implementation for kernel use
 *
 * There are 5 pieces of information stored for each node in an AVL tree
 *
 * 	pointer to less than child
 * 	pointer to greater than child
 * 	a pointer to the parent of this node
 *	an indication  [0/1]  of which child I am of my parent
 * 	a "balance" (-1, 0, +1)  indicating which child tree is taller
 *
 * Since they only need 3 bits, the last two fields are packed into the
 * bottom bits of the parent pointer on 64 bit machines to save on space.
 */

#ifdef COMPILE_32
#undef _LP64
#endif

#ifndef _LP64
struct avl_node {
	struct avl_node *avl_child[2];	/* left/right children */
	struct avl_node *avl_parent;	/* this node's parent */
	zpl_uint16 avl_child_index;	/* my index in parent's avl_child[] */
	zpl_int16 avl_balance;		/* balance value: -1, 0, +1 */
};

#define	AVL_XPARENT(n)		((n)->avl_parent)
#define	AVL_SETPARENT(n, p)	((n)->avl_parent = (p))

#define	AVL_XCHILD(n)		((n)->avl_child_index)
#define	AVL_SETCHILD(n, c)	((n)->avl_child_index = (zpl_uint16)(c))

#define	AVL_XBALANCE(n)		((n)->avl_balance)
#define	AVL_SETBALANCE(n, b)	((n)->avl_balance = (zpl_int16)(b))

#else /* _LP64 */

/*
 * for 64 bit machines, avl_pcb contains parent pointer, balance and child_index
 * values packed in the following manner:
 *
 * |63                                  3|        2        |1          0 |
 * |-------------------------------------|-----------------|-------------|
 * |      avl_parent hi order bits       | avl_child_index | avl_balance |
 * |                                     |                 |     + 1     |
 * |-------------------------------------|-----------------|-------------|
 *
 */
struct avl_node {
	struct avl_node *avl_child[2];	/* left/right children nodes */
	avl_uintptr_t avl_pcb;		/* parent, child_index, balance */
};

/*
 * macros to extract/set fields in avl_pcb
 *
 * pointer to the parent of the current node is the high order bits
 */
#define	AVL_XPARENT(n)		((struct avl_node *)((n)->avl_pcb & ~7))
#define	AVL_SETPARENT(n, p)						\
	((n)->avl_pcb = (((n)->avl_pcb & 7) | (avl_uintptr_t)(p)))

/*
 * index of this node in its parent's avl_child[]: bit #2
 */
#define	AVL_XCHILD(n)		(((n)->avl_pcb >> 2) & 1)
#define	AVL_SETCHILD(n, c)						\
	((n)->avl_pcb = (avl_uintptr_t)(((n)->avl_pcb & ~4) | ((c) << 2)))

/*
 * balance indication for a node, lowest 2 bits. A valid balance is
 * -1, 0, or +1, and is encoded by adding 1 to the value to get the
 * unsigned values of 0, 1, 2.
 */
#define	AVL_XBALANCE(n)		((int)(((n)->avl_pcb & 3) - 1))
#define	AVL_SETBALANCE(n, b)						\
	((n)->avl_pcb = (avl_uintptr_t)((((n)->avl_pcb & ~3) | ((b) + 1))))

#endif /* _LP64 */


/*
 * switch between a node and data pointer for a given tree
 * the value of "o" is tree->avl_offset
 */
#define	AVL_NODE2DATA(n, o)	((void *)((avl_uintptr_t)(n) - (o)))
#define	AVL_DATA2NODE(d, o)	((struct avl_node *)((avl_uintptr_t)(d) + (o)))



/*
 * macros used to create/access an avl_index_t
 */
#define	AVL_INDEX2NODE(x)	((avl_node_t *)((x) & ~1))
#define	AVL_INDEX2CHILD(x)	((x) & 1)
#define	AVL_MKINDEX(n, c)	((avl_index_t)(n) | (c))

#ifndef ulong_t
#define		ulong_t		zpl_ulong
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/*
 * The tree structure. The fields avl_root, avl_compar, and avl_offset come
 * first since they are needed for avl_find().  We want them to fit into
 * a single 64 byte cache line to make avl_find() as fast as possible.
 */
struct avl_tree {
	struct avl_node *avl_root;	/* root node in tree */
	int (*avl_compar)(const void *, const void *);
	zpl_ulong avl_offset;		/* offsetof(type, avl_link_t field) */
	zpl_ulong avl_numnodes;		/* number of nodes in the tree */
	zpl_ulong avl_size;		/* sizeof user type struct */
};



/*
 * This is a generic implemenatation of AVL trees for use in the Solaris kernel.
 * The interfaces provide an efficient way of implementing an ordered set of
 * data structures.
 *
 * AVL trees provide an alternative to using an ordered linked list. Using AVL
 * trees will usually be faster, however they requires more storage. An ordered
 * linked list in general requires 2 pointers in each data structure. The
 * AVL tree implementation uses 3 pointers. The following chart gives the
 * approximate performance of operations with the different approaches:
 *
 *	Operation	 Link List	AVL tree
 *	---------	 --------	--------
 *	lookup		   O(n)		O(log(n))
 *
 *	insert 1 node	 constant	constant
 *
 *	delete 1 node	 constant	between constant and O(log(n))
 *
 *	delete all nodes   O(n)		O(n)
 *
 *	visit the next
 *	or prev node	 constant	between constant and O(log(n))
 *
 *
 * The data structure nodes are anchored at an "avl_tree_t" (the equivalent
 * of a list header) and the individual nodes will have a field of
 * type "avl_node_t" (corresponding to list pointers).
 *
 * The type "avl_index_t" is used to indicate a position in the list for
 * certain calls.
 *
 * The usage scenario is generally:
 *
 * 1. Create the list/tree with: avl_create()
 *
 * followed by any mixture of:
 *
 * 2a. Insert nodes with: avl_add(), or avl_find() and avl_insert()
 *
 * 2b. Visited elements with:
 *	 avl_first() - returns the lowest valued node
 *	 avl_last() - returns the highest valued node
 *	 AVL_NEXT() - given a node go to next higher one
 *	 AVL_PREV() - given a node go to previous lower one
 *
 * 2c.  Find the node with the closest value either less than or greater
 *	than a given value with avl_nearest().
 *
 * 2d. Remove individual nodes from the list/tree with avl_remove().
 *
 * and finally when the list is being destroyed
 *
 * 3. Use avl_destroy_nodes() to quickly process/free up any remaining nodes.
 *    Note that once you use avl_destroy_nodes(), you can no longer
 *    use any routine except avl_destroy_nodes() and avl_destoy().
 *
 * 4. Use avl_destroy() to destroy the AVL tree itself.
 *
 * Any locking for multiple thread access is up to the user to provide, just
 * as is needed for any linked list implementation.
 */


/*
 * Type used for the root of the AVL tree.
 */
typedef struct avl_tree avl_tree_t;

/*
 * The data nodes in the AVL tree must have a field of this type.
 */
typedef struct avl_node avl_node_t;

/*
 * An opaque type used to locate a position in the tree where a node
 * would be inserted.
 */
typedef avl_uintptr_t avl_index_t;



/*
 * Direction constants used for avl_nearest().
 */
#define	AVL_BEFORE	(0)
#define	AVL_AFTER	(1)


/*
 * Prototypes
 *
 * Where not otherwise mentioned, "void *" arguments are a pointer to the
 * user data structure which must contain a field of type avl_node_t.
 *
 * Also assume the user data structures looks like:
 *	stuct my_type {
 *		...
 *		avl_node_t	my_link;
 *		...
 *	};
 */

/*
 * Initialize an AVL tree. Arguments are:
 *
 * tree   - the tree to be initialized
 * compar - function to compare two nodes, it must return exactly: -1, 0, or +1
 *          -1 for <, 0 for ==, and +1 for >
 * size   - the value of sizeof(struct my_type)
 * offset - the value of OFFSETOF(struct my_type, my_link)
 */
extern void avl_create(avl_tree_t *tree,
	int (*compar) (const void *, const void *), size_t size, size_t offset);


/*
 * Find a node with a matching value in the tree. Returns the matching node
 * found. If not found, it returns NULL and then if "where" is not NULL it sets
 * "where" for use with avl_insert() or avl_nearest().
 *
 * node   - node that has the value being looked for
 * where  - position for use with avl_nearest() or avl_insert(), may be NULL
 */
extern void *avl_find(avl_tree_t *tree, const void *node, avl_index_t *where);

/*
 * Insert a node into the tree.
 *
 * node   - the node to insert
 * where  - position as returned from avl_find()
 */
extern void avl_insert(avl_tree_t *tree, void *node, avl_index_t where);

/*
 * Insert "new_data" in "tree" in the given "direction" either after
 * or before the data "here".
 *
 * This might be usefull for avl clients caching recently accessed
 * data to avoid doing avl_find() again for insertion.
 *
 * new_data	- new data to insert
 * here		- existing node in "tree"
 * direction	- either AVL_AFTER or AVL_BEFORE the data "here".
 */
extern void avl_insert_here(avl_tree_t *tree, void *new_data, void *here,
    int direction);


/*
 * Return the first or last valued node in the tree. Will return NULL
 * if the tree is empty.
 *
 */
extern void *avl_first(avl_tree_t *tree);
extern void *avl_next(avl_tree_t *tree, void *node);
extern void *avl_last(avl_tree_t *tree);
extern void *avl_prev(avl_tree_t *tree, void *node);

/*
 * This will only by used via AVL_NEXT() or AVL_PREV()
 */
extern void *avl_walk(struct avl_tree *, void *, int);

/*
 * Return the next or previous valued node in the tree.
 * AVL_NEXT() will return NULL if at the last node.
 * AVL_PREV() will return NULL if at the first node.
 *
 * node   - the node from which the next or previous node is found
 */
#define	AVL_NEXT(tree, node)	avl_walk(tree, node, AVL_AFTER)
#define	AVL_PREV(tree, node)	avl_walk(tree, node, AVL_BEFORE)


/*
 * Find the node with the nearest value either greater or less than
 * the value from a previous avl_find(). Returns the node or NULL if
 * there isn't a matching one.
 *
 * where     - position as returned from avl_find()
 * direction - either AVL_BEFORE or AVL_AFTER
 *
 * EXAMPLE get the greatest node that is less than a given value:
 *
 *	avl_tree_t *tree;
 *	struct my_data look_for_value = {....};
 *	struct my_data *node;
 *	struct my_data *less;
 *	avl_index_t where;
 *
 *	node = avl_find(tree, &look_for_value, &where);
 *	if (node != NULL)
 *		less = AVL_PREV(tree, node);
 *	else
 *		less = avl_nearest(tree, where, AVL_BEFORE);
 */
extern void *avl_nearest(avl_tree_t *tree, avl_index_t where, int direction);


/*
 * Add a single node to the tree.
 * The node must not be in the tree, and it must not
 * compare equal to any other node already in the tree.
 *
 * node   - the node to add
 */
extern void avl_add(avl_tree_t *tree, void *node);


/*
 * Remove a single node from the tree.  The node must be in the tree.
 *
 * node   - the node to remove
 */
extern void avl_remove(avl_tree_t *tree, void *node);

/*
 * Reinsert a node only if its order has changed relative to its nearest
 * neighbors. To optimize performance avl_update_lt() checks only the previous
 * node and avl_update_gt() checks only the next node. Use avl_update_lt() and
 * avl_update_gt() only if you know the direction in which the order of the
 * node may change.
 */
extern int avl_update(avl_tree_t *, void *);
extern int avl_update_lt(avl_tree_t *, void *);
extern int avl_update_gt(avl_tree_t *, void *);

/*
 * Return the number of nodes in the tree
 */
extern ulong_t avl_numnodes(avl_tree_t *tree);

/*
 * Return B_TRUE if there are zero nodes in the tree, B_FALSE otherwise.
 */
extern int avl_is_empty(avl_tree_t *tree);

/*
 * Used to destroy any remaining nodes in a tree. The cookie argument should
 * be initialized to NULL before the first call. Returns a node that has been
 * removed from the tree and may be free()'d. Returns NULL when the tree is
 * empty.
 *
 * Once you call avl_destroy_nodes(), you can only continuing calling it and
 * finally avl_destroy(). No other AVL routines will be valid.
 *
 * cookie - a "void *" used to save state between calls to avl_destroy_nodes()
 *
 * EXAMPLE:
 *	avl_tree_t *tree;
 *	struct my_data *node;
 *	void *cookie;
 *
 *	cookie = NULL;
 *	while ((node = avl_destroy_nodes(tree, &cookie)) != NULL)
 *		free(node);
 *	avl_destroy(tree);
 */
extern void *avl_destroy_nodes(avl_tree_t *tree, void **cookie);


/*
 * Final destroy of an AVL tree. Arguments are:
 *
 * tree   - the empty tree to destroy
 */
extern void avl_destroy(avl_tree_t *tree);

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif


struct avl_list_node{
    struct avl_node stAvlNode;
};

struct avl_list{
    /*use avl as internal struct*/
    struct avl_tree stAvlTree;
  
    /*first user node in avl, it's result is same as avl_first*/
    void *pstFirst;
    
    /*flag used to check if some node not init*/
    u_int cookie;      
}; 


/*scan first node, returned value is user node*/
#define avl_list_first(list) ({\
    void *pstAvlNode = NULL;\
    if (NULL == ((list)->pstFirst)) \
        (list)->pstFirst = avl_first(&((list)->stAvlTree));\
    pstAvlNode = (list)->pstFirst;\
    pstAvlNode = pstAvlNode;\
})

/*get last node*/
#define avl_list_last(list) ({\
    void *pstAvlNode = NULL;\
    pstAvlNode = avl_last(&((list)->stAvlTree)) ;\
    pstAvlNode = pstAvlNode;\
})

/*get first node > input node*/
#define avl_list_greater(list, info) ({\
    avl_index_t where;\
    void *pstAvlNode = NULL;\
    pstAvlNode = avl_find((&((list)->stAvlTree)), info, &where);\
    if (pstAvlNode != NULL)\
        pstAvlNode = AVL_NEXT((&((list)->stAvlTree)), pstAvlNode) ;\
    else\
        pstAvlNode = avl_nearest((&((list)->stAvlTree)), where, AVL_AFTER);\
    pstAvlNode = pstAvlNode;\
})

/*get node not less than input node,may same or larger*/
#define avl_list_no_less(list, info) ({\
    avl_index_t where;\
    void *pstAvlNode = NULL;\
    pstAvlNode = avl_find((&((list)->stAvlTree)), info, &where);\
    if (pstAvlNode == NULL)\
        pstAvlNode = avl_nearest((&((list)->stAvlTree)), where, AVL_AFTER);\
    pstAvlNode = pstAvlNode;\
})

/*get next node,input node is user node and must be valid node in table, we will translate it*/
#define avl_list_next(list, info) ({\
    void *pstAvlNode = NULL;\
    if (info) pstAvlNode = AVL_NEXT((&((list)->stAvlTree)), info) ;\
    pstAvlNode = pstAvlNode;\
})

/*lookup,input is user node*/
#define avl_list_lookup(list, info) ({\
    avl_index_t where;\
    void *pstAvlNode = NULL;\
    pstAvlNode = avl_find((&((list)->stAvlTree)), info, &where);\
    pstAvlNode = pstAvlNode;\
})

/*add to table,input is user node,must not in any list before add*/
#define avl_list_add(list, info) ({\
    STATUS x = VOS_OK;\
    if (avl_list_lookup(list, info)!=NULL)\
    {\
    }\
    avl_add((&((list)->stAvlTree)), info);\
    (list)->pstFirst = NULL;\
    x = x;\
})

/*remove from table, input is user node,must in table*/
#define avl_list_del(list, info) ({\
    STATUS x = VOS_ERR;\
    if (info)\
    {\
        if ((list)->pstFirst == info)\
            (list)->pstFirst = NULL;\
        if(&((list)->stAvlTree))\
        {\
            avl_remove((&((list)->stAvlTree)), (info));\
        }\
        x = VOS_OK;\
    }\
    x = x;\
})

#define avl_list_count(list) ({\
    zpl_uint32 x = 0 ;\
    x = avl_numnodes((&((list)->stAvlTree)));\
    x = x;\
})

#ifdef	__cplusplus
}
#endif

#endif	/* _AVL_H */
