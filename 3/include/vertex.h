/**
 * @file vertex.h
 * @author Matthias Pichler, 01634256
 * @date 2018-05-29
 * @brief OSUE Exercise 3
 */

#ifndef VERTEX_H
#define VERTEX_H

#include <stdint.h>

typedef enum
{
	undef = -1,
	red = 0,
	green = 1,
	blue = 2
} color_t;

typedef struct vertex vertex_t;
typedef struct vertex_tree vertex_tree_t;
typedef struct node node_t;

struct vertex
{
	uint8_t id;
	color_t color;
	vertex_tree_t *edges;
};

struct node
{
	vertex_t *value;
	struct node *left, *right;
};

struct vertex_tree
{
	node_t *root;
};

/**
 * @brief creates a new empty tree
 * @return a pointer to a new tree or NULL on failure
 */
vertex_tree_t *new_tree();

/**
 * @brief add the given vertex to the given tree
 * @return 0 on success, -1 on failure
 */
int8_t tree_add(vertex_tree_t *tree, vertex_t vertex);

/**
 * @brief removes the vertex with the given id from the tree
 * @returns the removed vertex or NULL on failure e.g. node not found
 */
vertex_t *tree_remove(vertex_tree_t *tree, uint8_t id);

/**
 * @brief finds the vertex with the given id from the tree
 * @returns the corresponding vertex or NULL on failure e.g. node not found
 */
vertex_t *tree_search(vertex_tree_t *tree, uint8_t id);

/**
 * @brief update the color of the vertex with the given id in the given tree
 * @return 0 on success, -1 on failure e.g. node not found
 */
int8_t tree_update_color(vertex_tree_t *tree, uint8_t id, color_t color);

/**
 * @brief print the values of all nodes in-order
 */
void tree_print(vertex_tree_t *tree);

#endif  // VERTEX_H