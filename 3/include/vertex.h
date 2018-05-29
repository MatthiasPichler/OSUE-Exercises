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

typedef uint8_t vid_t;
typedef struct vertex vertex_t;
typedef struct edge edge_t;
typedef struct vertex_tree vertex_tree_t;
typedef struct node node_t;


struct vertex
{
	vid_t id;
	color_t color;
	vertex_tree_t *edges;
};

struct edge
{
	vid_t begin;
	vid_t end;
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
 * @brief creates a new vertex
 * @param id the id of the new vertex
 * @return a pointer to a new vertex or NULL on failure
 */
vertex_t *new_vertex(const vid_t id);

/**
 * @brief creates a new empty tree
 * @return a pointer to a new tree or NULL on failure
 */
vertex_tree_t *new_tree(void);

/**
 * @brief add the given vertex to the given tree
 * @param tree the tree to modify
 * @param vertex the vertex to add
 * @return 0 on success, -1 on failure
 */
int8_t tree_add(vertex_tree_t *tree, vertex_t *vertex);

/**
 * @brief removes the vertex with the given id from the tree
 * @param tree the tree to modify
 * @param id the id to search for
 * @returns the removed vertex or NULL on failure e.g. node not found
 */
vertex_t *tree_remove(vertex_tree_t *tree, const vid_t id);

/**
 * @brief finds the vertex with the given id from the tree
 * @param tree the tree to search
 * @param id the id to search for
 * @returns the corresponding vertex or NULL on failure e.g. node not found
 */
vertex_t *tree_search(const vertex_tree_t *tree, const vid_t id);

/**
 * @brief update the color of the vertex with the given id in the given tree
 * @param tree the tree to modify
 * @param id the id to search for
 * @param color the new color value
 * @return 0 on success, -1 on failure e.g. node not found
 */
int8_t
tree_update_color(vertex_tree_t *tree, const vid_t id, const color_t color);

/**
 * @brief print the values of all nodes in-order
 * @param tree the tree to print
 */
void tree_print(const vertex_tree_t *tree);

#endif  // VERTEX_H