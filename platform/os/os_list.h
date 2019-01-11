/*
 * os_list.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_LIST_H__
#define __OS_LIST_H__

/* type definitions */

typedef struct node		/* Node of a linked list. */
    {
    struct node *next;		/* Points at the next node in the list */
    struct node *previous;	/* Points at the previous node in the list */
    } NODE;


/* HIDDEN */

typedef struct			/* Header for a linked list. */
    {
    NODE node;			/* Header list node */
    int count;			/* Number of nodes in list */
    int (*free)(void *);
    int (*cmp)(void *, void *);
    } LIST;

/* END_HIDDEN */


/* function declarations */



extern void	lstLibInit (void);
extern NODE *	lstFirst (LIST *pList);
extern NODE *	lstGet (LIST *pList);
extern NODE *	lstLast (LIST *pList);
extern NODE *	lstNStep (NODE *pNode, int nStep);
extern NODE *	lstNext (NODE *pNode);
extern NODE *	lstNth (LIST *pList, int nodenum);
extern NODE *	lstPrevious (NODE *pNode);
extern int 	lstCount (LIST *pList);
extern int 	lstFind (LIST *pList, NODE *pNode);
extern void 	lstAdd (LIST *pList, NODE *pNode);
extern void 	lstConcat (LIST *pDstList, LIST *pAddList);
extern void 	lstDelete (LIST *pList, NODE *pNode);
extern void 	lstExtract (LIST *pSrcList, NODE *pStartNode, NODE *pEndNode,
	  		    LIST *pDstList);
extern void 	lstFree (LIST *pList);
extern void 	lstInit (LIST *pList);
extern void 	lstInsert (LIST *pList, NODE *pPrev, NODE *pNode);

extern void 	lstSortInit (LIST *pList, int(*cmp)(void *, void *));
extern void 	lstInitFree (LIST *pList, int(*free)(void *));
extern void 	lstAddSort (LIST *pList, NODE *pNode);

#endif /* __OS_LIST_H__ */
