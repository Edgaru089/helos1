//$CC -g test_tree.c tree.c ../driver/random/random.c

#include "stdlib.h"
#include "stdio.h"

#include "tree.h"


tree_Tree *tree;

void insert(int a) {
	bool new;
	int *d = tree_Insert(tree, a, &new);
	if (new)
		*d = 1;
	else
		(*d)++;
}

int count(int a) {
	int *d = tree_Find(tree, a);
	if (!d)
		return 0;
	else
		return *d;
}

void delete (int a) {
	tree_Node *node = tree_FindNode(tree, a);
	if (!node)
		return;

	(*(int *)node->data)--;
	if ((*(int *)node->data) == 0)
		tree_Delete(tree, node);
}


int main() {
	tree = tree_Create(sizeof(int));

	insert(1);
	insert(2);
	insert(3);
	insert(6);
	insert(1);

	printf("%d\n", count(2));
	printf("%d\n", count(4));
	printf("%d\n", count(1));
	delete (1);
	printf("%d\n", count(1));
	delete (1);
	printf("%d\n", count(1));
	delete (1);
	printf("%d\n", count(1));
	delete (2);
	printf("%d\n", count(1));

	insert(7);
	insert(10);
	insert(54);
	insert(18);
	insert(63);
	insert(39);

	tree_Node *i = tree_FirstNode(tree);
	while (i) {
		printf(" %d", (int)i->key);
		i = tree_Node_Next(i);
	}
	printf("\n");

	i = tree_LastNode(tree);
	while (i) {
		printf(" %d", (int)i->key);
		i = tree_Node_Previous(i);
	}
	printf("\n");
}
