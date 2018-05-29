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

static int8_t node_add(node_t **this_n, vertex_t *vertex)
{
	if (*this_n == NULL) {
		node_t *node = (node_t *)malloc(sizeof(node_t));
		if (node == NULL) {
			return -1;
		}
		node->left = node->right = NULL;
		node->value = vertex;
		*this_n = node;
		return 0;
	}

	if (vertex->id < (*this_n)->value->id) {
		return node_add(&((*this_n)->left), vertex);
	} else {
		return node_add(&((*this_n)->right), vertex);
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

static node_t *node_search_min(node_t *this_n)
{
	if (this_n->left == NULL) {
		return this_n;
	}

	return node_search_min(this_n->left);
}

static node_t *node_delete(node_t *this_n, const vid_t id)
{
	if (this_n == NULL) {
		return NULL;
	}

	if (id < this_n->value->id) {
		this_n->left = node_delete(this_n->left, id);
	} else if (id > this_n->value->id) {
		this_n->right = node_delete(this_n->right, id);
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
		node_t *tmp = node_search_min(this_n->right);

		memcpy(this_n->value, tmp->value, sizeof(vertex_t));

		this_n->right = node_delete(this_n->right, tmp->value->id);
	}
	return this_n;
}

static void node_print(const node_t *this_n)
{
	if (this_n == NULL) {
		return;
	}
	node_print(this_n->left);
	fprintf(stdout, " %d ", this_n->value->id);
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

int8_t tree_add(vertex_tree_t *tree, vertex_t *vertex)
{
	return node_add(&(tree->root), vertex);
}

vertex_t *tree_delete(vertex_tree_t *tree, const vid_t id)
{
	vertex_t *vertex = tree_search(tree, id);
	if (vertex == NULL) {
		return NULL;
	}

	tree->root = node_delete(tree->root, id);

	return vertex;
}

vertex_t *tree_search(const vertex_tree_t *tree, const vid_t id)
{
	node_t *node = node_search(tree->root, id);
	if (node == NULL) {
		return NULL;
	}

	vertex_t *vertex = (vertex_t *)malloc(sizeof(vertex_t));
	if (vertex == NULL) {
		return NULL;
	}
	memcpy(vertex, node->value, sizeof(node_t));

	return vertex;
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