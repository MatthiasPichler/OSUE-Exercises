/**
 * @file graph.h
 * @author Matthias Pichler, 01634256
 * @date 2018-05-29
 * @brief OSUE Exercise 3
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

typedef enum
{
	undef = -1,
	red = 0,
	green = 1,
	blue = 2
} color_t;

typedef uint8_t vid_t;

typedef struct edge
{
	vid_t begin;
	vid_t end;
} edge_t;


typedef struct graph
{
	size_t vertex_size;
	color_t* vertices;
	size_t edge_size;
	edge_t* edges;
} graph_t;

/**
 * @brief delete the given edge from the graph
 * @param graph the graph to modify
 * @param edge the edge to add
 * @return 0 on success, -1 on failure
 */
int delete_edge(graph_t* graph, const edge_t edge);

/**
 * @brief create and initialize a new graph from the given edges
 * @param edges the array of edges
 * @param size the size of the arrray
 * @return the generated graph or NULL on failure
 */
graph_t* new_graph(const edge_t* edges, size_t size);

/**
 * @brief frees all resources used by the graph
 * @param graph the graph to be freed
 */
void free_graph(graph_t* graph);

/**
 * @brief prints the graph to stdout
 */
void graph_print(const graph_t* graph);

/**
 * @brief checks if the given tree is 3 colored
 * @return true if the given graph is 3 colored, false otherwise
 */
bool graph_colored(const graph_t* graph);


#endif  // GRAPH_H