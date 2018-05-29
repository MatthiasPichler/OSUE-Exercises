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

graph_t* new_graph();

#endif  // GRAPH_H