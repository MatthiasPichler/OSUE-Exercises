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