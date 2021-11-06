
#include "tree.h"
#include "tree_internal.h"

#include "../main.h"
#include "../memory/memory.h"
#include "../driver/random/random.h"
#include "string.h"


// Create allocates and creates a new default Tree object.
tree_Tree *tree_Create(uintptr_t objectSize) {
	tree_Tree *t  = kMalloc(sizeof(tree_Tree));
	t->objectSize = objectSize;
	t->size       = 0;
	t->root       = 0;
	return t;
}


static inline tree_Node *__tree_NewNode(tree_Tree *t, uintptr_t key, tree_Node *father, uintptr_t internal) {
	t->size++;

	tree_Node *node = kMalloc(sizeof(tree_Node) - 1 + t->objectSize);
	node->left = node->right = 0;
	node->father             = father;
	node->key                = key;
	node->internal           = internal;
	return node;
}

static void __tree_DestroyNodes(tree_Node *node) {
	if (node->left)
		__tree_DestroyNodes(node->left);
	if (node->right)
		__tree_DestroyNodes(node->right);
	kFree(node);
}

void tree_Destroy(tree_Tree *tree) {
	if (tree->root)
		__tree_DestroyNodes(tree->root);
	kFree(tree);
}


// Will not return NULL
// SysV ABI because of the 6 arguments
SYSV_ABI static tree_Node *__tree_InsertNodes(tree_Tree *t, tree_Node *node, tree_Node *father, uintptr_t key, tree_Node **result, bool *added) {
	if (!node) {
		if (added)
			*added = true;
		return *result = __tree_NewNode(t, key, father, random_Rand() ^ key);
	} else if (key < node->key) {
		node->left = __tree_InsertNodes(t, node->left, node, key, result, added);
		return node;
	} else if (key > node->key) {
		node->right = __tree_InsertNodes(t, node->right, node, key, result, added);
		return node;
	} else {
		if (added)
			*added = false;
		*result = node;
		return node;
	}
}

tree_Node *tree_InsertNode(tree_Tree *t, uintptr_t key, bool *added) {
	tree_Node *result;
	t->root = __tree_InsertNodes(t, t->root, 0, key, &result, added);

	if (*added)
		__tree_treap_Adjust(result, &t->root);

	return result;
}

void *tree_Insert(tree_Tree *t, uintptr_t key, bool *added) {
	return tree_InsertNode(t, key, added)->data;
}


static tree_Node *__tree_FindNode(tree_Node *node, uintptr_t key) {
	if (!node)
		return NULL;
	else if (key < node->key)
		return __tree_FindNode(node->left, key);
	else if (key > node->key)
		return __tree_FindNode(node->right, key);
	else
		return node;
}

tree_Node *tree_FindNode(tree_Tree *t, uintptr_t key) {
	return __tree_FindNode(t->root, key);
}

void *tree_Find(tree_Tree *t, uintptr_t key) {
	tree_Node *node = tree_FindNode(t, key);
	if (!node)
		return NULL;
	else
		return node->data;
}


void tree_Delete(tree_Tree *t, tree_Node *node) {
	while (node->left && node->right)
		if (node->left->internal < node->right->internal)
			__tree_Rotate(node->left, &t->root);
		else
			__tree_Rotate(node->right, &t->root);

	if (node == t->root)
		t->root = (node->left ? node->left : node->right);
	__tree_Connect(node->father, (node->left ? node->left : node->right), __tree_Tell(node));

	kFree(node);
}


tree_Node *tree_FirstNode(tree_Tree *tree) {
	tree_Node *result = tree->root;
	while (result->left)
		result = result->left;
	return result;
}

tree_Node *tree_LastNode(tree_Tree *tree) {
	tree_Node *result = tree->root;
	while (result->right)
		result = result->right;
	return result;
}


tree_Node *tree_Node_Next(tree_Node *node) {
	if (node->right) {
		tree_Node *result = node->right;
		while (result->left)
			result = result->left;
		return result;
	} else {
		tree_Node *result = node;
		while (result->father && __tree_Tell(result) == __tree_Right)
			result = result->father;
		return result->father;
	}
}

// Node_Previous returns the previous node. Returns NULL if first.
tree_Node *tree_Node_Previous(tree_Node *node) {
	if (node->left) {
		tree_Node *result = node->left;
		while (result->right)
			result = result->right;
		return result;
	} else {
		tree_Node *result = node;
		while (result->father && __tree_Tell(result) == __tree_Left)
			result = result->father;
		return result->father;
	}
}
