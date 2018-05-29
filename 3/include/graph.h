/**
 * @file graph.h
 * @author Matthias Pichler, 01634256
 * @date 2018-05-29
 * @brief OSUE Exercise 3
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "vertex.h"

typedef struct graph
{
	vertex_tree_t* vertices;
} graph_t;

/**
 * @brief add the given edge to the graph
 * @param graph the graph to modify
 * @param edge the edge to add
 * @return 0 on success, -1 on failure
 */
int add_edge(graph_t* graph, const edge_t edge);

/**
 * @brief create and initialize a new graph
 * @return the generated graph or NULL on failure
 */
graph_t* new_graph(void);

#endif  // GRAPH_H