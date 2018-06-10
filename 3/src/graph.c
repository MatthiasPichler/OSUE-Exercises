#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/graph.h"
#include "../include/common.h"

static vid_t max_node(const edge_t* edges, size_t size)
{
	vid_t max = 0;
	for (int i = 0; i < size; i++) {
		if (edges[i].begin > max) {
			max = edges[i].begin;
		}
		if (edges[i].end > max) {
			max = edges[i].end;
		}
	}
	return max;
}

graph_t* new_graph(const edge_t* edges, size_t size)
{
	graph_t* graph = (graph_t*)malloc(sizeof(graph_t));
	if (graph == NULL) {
		return NULL;
	}

	graph->edge_size = size;
	graph->edges = (edge_t*)calloc(size, sizeof(edge_t));
	if (graph->edges == NULL) {
		free(graph);
		return NULL;
	}
	memcpy(graph->edges, edges, sizeof(edge_t) * size);

	graph->vertex_size = max_node(edges, size) + 1;
	graph->vertices = (color_t*)calloc(graph->vertex_size, sizeof(color_t));
	if (graph->vertices == NULL) {
		// TODO manage cleanup
		SAFE_DELETE(graph->edges)
		// free(graph->edges);
		SAFE_DELETE(graph)
		// free(graph);
		return NULL;
	}

	return graph;
}

void free_graph(graph_t* graph)
{

	if (graph == NULL) {
		return;
	}
	// TODO decide cleanup
	SAFE_DELETE(graph->edges)
	// free(graph->edges);
	SAFE_DELETE(graph->vertices)
	// free(graph->vertices);
	SAFE_DELETE(graph)
	// free(graph);
}

void graph_print(const graph_t* graph)
{
	fprintf(stdout, "[\n");
	for (int i = 0; i < graph->vertex_size; i++) {
		char c;
		switch (graph->vertices[i]) {
			case red:
				c = 'r';
				break;
			case green:
				c = 'g';
				break;
			case blue:
				c = 'b';
				break;
			default:
				c = '-';
		}
		fprintf(stdout, "(%d:%c): [", i, c);
		for (int j = 0; j < graph->edge_size; j++) {
			if (graph->edges[j].begin == i) {
				fprintf(stdout, " %d ", graph->edges[j].end);
			}
			if (graph->edges[j].end == i) {
				fprintf(stdout, " %d ", graph->edges[j].begin);
			}
		}
		fprintf(stdout, "]\n");
	}
	fprintf(stdout, "]\n");
}

int delete_edge(graph_t* graph, const edge_t edge)
{

	vid_t i;
	bool found = false;
	for (i = 0; i < graph->edge_size; i++) {
		edge_t e = graph->edges[i];
		if (e.begin == edge.begin && e.end == edge.end) {
			found = true;
			break;
		}
	}

	if (!found) {
		return -1;
	}

	edge_t tmp = graph->edges[i];
	graph->edges[i] = graph->edges[graph->edge_size - 1];
	graph->edges[graph->edge_size - 1] = tmp;

	graph->edge_size--;
	graph->edges =
		(edge_t*)realloc(graph->edges, sizeof(edge_t) * graph->edge_size);
	if (graph->edges == NULL) {
		exit(EXIT_FAILURE);
	}
	return 0;
}

bool graph_colored(const graph_t* graph)
{
	for (size_t i = 0; i < graph->edge_size; i++) {
		edge_t e = graph->edges[i];
		if (graph->vertices[e.begin] == graph->vertices[e.end]) {
			return false;
		}
	}

	return true;
}
