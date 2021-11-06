#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct __tree_Node {
	uintptr_t           key;          // node key
	struct __tree_Node *left, *right; // left and right sons
	struct __tree_Node *father;       // father node
	uintptr_t           internal;     // internal data for balanced trees
	char                data[1];      // placeholder for object data
} tree_Node;

// Tree is a basic tree-based associative container.
//
// Right now it's a Treap.
typedef struct {
	uintptr_t  objectSize; // size in bytes of the object
	tree_Node *root;       // root of the tree, NULL if empty
	uintptr_t  size;       // number of objects in the tree
} tree_Tree;

// Create allocates and creates a new default Tree object.
tree_Tree *tree_Create(uintptr_t objectSize);

// Destroy properly frees all data related to the structure, and itself.
void tree_Destroy(tree_Tree *tree);

// Insert inserts a new object (or locates an existing one).
//
// If *added is not NULL, it is set to true if the key does
// not exist and is actually added.
//
// Newly allocated data is not zeroed, nor initialized in any way.
//
// Returns the pointer to the newly allocated (or existing) data.
void *tree_Insert(tree_Tree *tree, uintptr_t key, bool *added);

// InsertNode does the same as Insert, but returns Node* instead of data.
tree_Node *tree_InsertNode(tree_Tree *tree, uintptr_t key, bool *added);

// Find locates an existing object by its key.
//
// Returns NULL if the object does not exist.
void *tree_Find(tree_Tree *tree, uintptr_t key);

// FindNode returns an existing tree node by its key.
//
// Used for iterating the tree objects.
tree_Node *tree_FindNode(tree_Tree *tree, uintptr_t key);

// FirstNode returns the first node in increasing order.
tree_Node *tree_FirstNode(tree_Tree *tree);

// LastNode returns the last node in increasing order.
tree_Node *tree_LastNode(tree_Tree *tree);

// Delete deletes an existing node from the tree.
void tree_Delete(tree_Tree *tree, tree_Node *node);

// Node_Next returns the next node. Returns NULL if the node is the last.
tree_Node *tree_Node_Next(tree_Node *node);

// Node_Previous returns the previous node. Returns NULL if first.
tree_Node *tree_Node_Previous(tree_Node *node);


#ifdef __cplusplus
}
#endif
