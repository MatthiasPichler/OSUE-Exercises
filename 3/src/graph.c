#include <stdlib.h>

#include "../include/graph.h"


graph_t* new_graph(void)
{
	graph_t* graph = (graph_t*)malloc(sizeof(graph_t));
	if (graph != NULL) {
		graph->vertices = new_tree();
	}
	return graph;
}

int add_edge(graph_t* graph, const edge_t edge)
{
	vertex_t* begin = tree_search(graph->vertices, edge.begin);
	if (begin == NULL) {
		begin = new_vertex(edge.begin);
		if (tree_add(graph->vertices, begin) < 0) {
			return -1;
		}
	}

	vertex_t* end = tree_search(graph->vertices, edge.end);
	if (end == NULL) {
		end = new_vertex(edge.end);
		if (tree_add(graph->vertices, end) < 0) {
			return -1;
		}
	}

	if (tree_add(begin->edges, end) < 0) {
		return -1;
	}
	return tree_add(end->edges, begin);
}