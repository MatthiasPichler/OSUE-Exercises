/**
 * @file vertex.c
 * @author Matthias Pichler, 01634256
 * @date 2018-05-29
 * @brief OSUE Exercise 3
 */


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/vertex.h"

static int node_add(node_t **this_n, node_t *parent, vertex_t *vertex)
{
	if (*this_n == NULL) {
		node_t *node = (node_t *)malloc(sizeof(node_t));
		if (node == NULL) {

			return -1;
		}
		node->left = node->right = NULL;
		node->value = vertex;
		node->parent = parent;
		*this_n = node;
		return 0;
	}

	if (vertex->id < (*this_n)->value->id) {
		return node_add(&((*this_n)->left), *this_n, vertex);
	} else {
		return node_add(&((*this_n)->right), *this_n, vertex);
	}
}

static int8_t
node_update_color(node_t *this_n, const vid_t id, const color_t color)
{
	if (this_n == NULL) {
		return -1;
	}

	if (this_n->value->id == id) {
		this_n->value->color = color;
		return 0;
	}

	if (id < this_n->value->id) {
		return node_update_color(this_n->left, id, color);
	} else {
		return node_update_color(this_n->right, id, color);
	}
}

static node_t *node_search(node_t *this_n, const vid_t id)
{
	if (this_n == NULL) {
		return NULL;
	}

	if (this_n->value->id == id) {
		return this_n;
	}

	if (id < this_n->value->id) {
		return node_search(this_n->left, id);
	} else {
		return node_search(this_n->right, id);
	}
}

static node_t *node_min(node_t *this_n)
{
	if (this_n == NULL) {
		return NULL;
	}

	if (this_n->left == NULL) {
		return this_n;
	}

	return node_min(this_n->left);
}

static node_t *node_max(node_t *this_n)
{
	if (this_n == NULL) {
		return NULL;
	}

	if (this_n->right == NULL) {
		return this_n;
	}

	return node_max(this_n->right);
}

static node_t *node_next(node_t *this_n)
{
	if (this_n == NULL) {
		return NULL;
	}

	if (this_n->right != NULL) {
		return node_min(this_n->right);
	}

	node_t *next = this_n;
	while (next->parent != NULL && next == next->parent->right) {
		next = next->parent;
	}
	return next->parent;
}

static node_t *node_delete(node_t *this_n, const vid_t id)
{
	if (this_n == NULL) {
		return NULL;
	}

	if (id < this_n->value->id) {
		this_n->left = node_delete(this_n->left, id);
		if (this_n->left != NULL) {
			this_n->left->parent = this_n;
		}
	} else if (id > this_n->value->id) {
		this_n->right = node_delete(this_n->right, id);
		if (this_n->right != NULL) {
			this_n->right->parent = this_n;
		}
	} else {

		// 0 or 1 children
		if (this_n->left == NULL) {
			node_t *tmp = this_n->right;
			free(this_n);
			return tmp;
		} else if (this_n->right == NULL) {
			node_t *tmp = this_n->left;
			free(this_n);
			return tmp;
		}

		// 2 children
		// find smallest successor
		node_t *tmp = node_min(this_n->right);

		memcpy(this_n->value, tmp->value, sizeof(vertex_t));

		this_n->right = node_delete(this_n->right, tmp->value->id);
	}
	return this_n;
}

static size_t node_size(const node_t *this_n)
{
	if (this_n == NULL) {
		return 0;
	}

	return node_size(this_n->left) + 1 + node_size(this_n->right);
}

static void node_print(const node_t *this_n)
{
	if (this_n == NULL) {
		return;
	}
	node_print(this_n->left);
	char *c;
	switch (this_n->value->color) {
		case red:
			c = "r";
			break;
		case green:
			c = "g";
			break;
		case blue:
			c = "b";
			break;
		default:
			c = "-";
	}

	fprintf(stdout, " (%d:%s) ", this_n->value->id, c);
	node_print(this_n->right);
}

vertex_t *new_vertex(const vid_t id)
{
	vertex_t *vertex = (vertex_t *)malloc(sizeof(vertex_t));
	if (vertex != NULL) {
		vertex->id = id;
		vertex->color = undef;
		vertex->edges = new_tree();
	}

	return vertex;
}

vertex_tree_t *new_tree(void)
{
	vertex_tree_t *tree = (vertex_tree_t *)malloc(sizeof(vertex_tree_t));
	if (tree != NULL) {
		tree->root = NULL;
	}
	return tree;
}

size_t tree_size(const vertex_tree_t *tree)
{
	return node_size(tree->root);
}

int tree_add(vertex_tree_t *tree, vertex_t *vertex)
{
	return node_add(&(tree->root), NULL, vertex);
}

vertex_t *tree_remove(vertex_tree_t *tree, const vid_t id)
{
	iterator_t *iter = tree_search(tree, id);
	if (iter == NULL) {
		return NULL;
	}
	vertex_t *vertex = iter->value;
	if (vertex == NULL) {
		return NULL;
	}

	tree->root = node_delete(tree->root, id);
	return vertex;
}

iterator_t *tree_search(const vertex_tree_t *tree, const vid_t id)
{
	node_t *n = node_search(tree->root, id);

	iterator_t *iter = (iterator_t *)malloc(sizeof(iterator_t));
	if (iter == NULL) {

		return NULL;
	}
	iter->node = n;
	if (n == NULL) {
		iter->value = NULL;
		return iter;
	}
	iter->value = n->value;
	return iter;
}

iterator_t *tree_max(const vertex_tree_t *tree)
{
	node_t *n = node_max(tree->root);

	iterator_t *iter = (iterator_t *)malloc(sizeof(iterator_t));
	if (iter == NULL) {
		return NULL;
	}
	iter->node = n;
	if (n == NULL) {
		iter->value = NULL;
		return iter;
	}
	iter->value = n->value;
	return iter;
}

iterator_t *tree_min(const vertex_tree_t *tree)
{
	node_t *n = node_min(tree->root);

	iterator_t *iter = (iterator_t *)malloc(sizeof(iterator_t));
	if (iter == NULL) {
		return NULL;
	}
	iter->node = n;
	if (n == NULL) {
		iter->value = NULL;
		return iter;
	}
	iter->value = n->value;
	return iter;
}

int8_t
tree_update_color(vertex_tree_t *tree, const vid_t id, const color_t color)
{
	return node_update_color(tree->root, id, color);
}

void tree_print(const vertex_tree_t *tree)
{
	fprintf(stdout, "[");
	node_print(tree->root);
	fprintf(stdout, "]\n");
}

int next(iterator_t *iterator)
{
	iterator->node = node_next(iterator->node);
	if (iterator->node == NULL) {
		iterator->value = NULL;
		return -1;
	}
	iterator->value = iterator->node->value;
	return 0;
}