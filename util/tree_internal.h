#pragma once

#include "tree.h"


typedef enum {
	__tree_Left,
	__tree_Right,
} __tree_ConnectType;

static inline __tree_ConnectType __tree_ConnectType_Invert(__tree_ConnectType type) {
	return (__tree_ConnectType)(!type);
}

static inline __tree_ConnectType __tree_Tell(tree_Node *son) {
	if (!son->father)
		return __tree_Left;
	if (son->father->left == son)
		return __tree_Left;
	else
		return __tree_Right;
}

static inline tree_Node *__tree_Get(tree_Node *father, __tree_ConnectType type) {
	return (type == __tree_Left) ? father->left : father->right;
}

static inline void __tree_Connect(tree_Node *father, tree_Node *son, __tree_ConnectType type) {
	if (son)
		son->father = father;
	if (father) {
		if (type == __tree_Left)
			father->left = son;
		else
			father->right = son;
	}
}

// Rotates the node up.
static inline void __tree_Rotate(tree_Node *node, tree_Node **root) {
	if (!node->father)
		return;

	__tree_ConnectType type = __tree_Tell(node);

	tree_Node *f = node->father,
			  *b = __tree_Get(node, __tree_ConnectType_Invert(type));
	__tree_Connect(f->father, node, __tree_Tell(f));
	__tree_Connect(node, f, __tree_ConnectType_Invert(type));
	__tree_Connect(f, b, type);

	if (!node->father)
		*root = node;
}

// Adjust the tree as a Treap
static inline void __tree_treap_Adjust(tree_Node *node, tree_Node **root) {
	while (node->father && node->father->internal > node->internal)
		__tree_Rotate(node, root);
}
