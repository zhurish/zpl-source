/*
 * list_tree.h
 *
 *  Created on: Feb 11, 2018
 *      Author: zhurish
 */

/*
 * Doubly-linked list
 * Copyright (c) 2009, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef LIST_TREE_H
#define LIST_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * struct dl_list - Doubly-linked list
 */
struct tree_node {
	void *data;
	struct tree_node *left;
	struct tree_node *right;
	//int	(*cmp)(void *, void *);
};

struct list_tree{
	struct tree_node *root;
	ospl_int	(*cmp)(void *, void *);
	ospl_int	(*del)(void *);
	ospl_uint32 count;
};


static inline ospl_bool list_tree_empty(struct list_tree *tree)
{
	return (tree->root == NULL)? 1:0;
}

static inline ospl_uint32 list_tree_count(struct list_tree *tree)
{
	return tree->count;
}

static inline void * list_tree_getroot(struct list_tree *tree)
{
	return tree->root->data;
}

static inline void * list_tree_lookup(struct list_tree *tree, struct tree_node *root, void *data)
{
	int ret = 0;
	if(root == NULL)
		return NULL;
	if(tree->cmp)
	{
		ret = (tree->cmp)(root->data, data);
		if(ret == 0)
			return root->data;
		else if(ret < 0)
			return list_tree_lookup(tree, root->left, data);
		else //if(node->data == data)
			return list_tree_lookup(tree, root->right, data);
	}
	else
	{
		if(root->data == data)
			return root->data;
		else if(root->data < data)
			return list_tree_lookup(tree, root->left, data);
		else //if(node->data == data)
			return list_tree_lookup(tree, root->right, data);
	}
	return NULL;
}



static inline struct tree_node * list_tree_create(void *data)
{
	struct tree_node* pn = (struct tree_node*)malloc(sizeof(struct tree_node));
    pn->data = data;
    pn->left = NULL;
    pn->right = NULL;
    return pn;
}


static inline int list_tree_insert(struct list_tree *tree, struct tree_node *root, struct tree_node *node)
{
	int ret = 0;
	if(root == NULL)
	{
		root = node;
		tree->count++;
		return 0;
	}
	if(tree->cmp)
	{
		ret = (tree->cmp)(root->data, node->data);
		if(ret < 0)
			return list_tree_insert(tree, root->left, node);
		else //if(node->data == data)
			return list_tree_insert(tree, root->right, node);
	}
	else
	{
		if(root < node)
			return list_tree_insert(tree, root->left, node);
		else //if(node->data == data)
			return list_tree_insert(tree, root->right, node);
	}
	return -1;
}

static inline int list_tree_clear(struct list_tree *tree, struct tree_node *root)
{
	if(root)
	{
		list_tree_clear(tree, root->left);
		list_tree_clear(tree, root->right);
		if(tree->del)
			(tree->del)(root->data);
		free(root);
		root = NULL;
	}
	return 0;
}

static inline int list_tree_del(struct list_tree *tree, struct tree_node *root, void *data)
{
	struct tree_node *node1 = NULL;
	struct tree_node *node = list_tree_lookup(tree, root, data);
	if(!node)
	{
		return -1;
	}
	if(node->left)
		list_tree_insert(tree, node->right, node->left);
	node1 = node;
	node = node->right;
	if(tree->del)
		(tree->del)(node1->data);
	free(node1);
	node1 = NULL;
	tree->count--;
	return 0;
}

 
#ifdef __cplusplus
}
#endif

#endif /* LIST_TREE_H */
