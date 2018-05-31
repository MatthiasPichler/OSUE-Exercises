#include <stdlib.h>
#include <stdio.h>

#include "../include/graph.h"


graph_t* new_graph(const edge_t* edges, size_t size)
{
	debug_print("%s\n", "called");
	graph_t* graph = (graph_t*)malloc(sizeof(graph_t));
	if (graph != NULL) {
		graph->vertices = new_tree();
	}

	for (int i = 0; i < size; i++) {
		if (add_edge(graph, edges[i]) < 0) {
			debug_print("%s\n", "add_edge failed");
			return NULL;
		}
	}

	return graph;
}

void graph_print(const graph_t* graph)
{
	iterator_t* iter = tree_min(graph->vertices);
	if (iter == NULL) {
		return;
	}

	fprintf(stdout, "[\n");

	vertex_t* v;
	char* c;
	do {
		v = iter->value;

		switch (v->color) {
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

		fprintf(stdout, "\t(%d:%s)->", v->id, c);
		tree_print(v->edges);
	} while (next(iter) != -1);

	fprintf(stdout, "]\n");

	free(iter);
}

int add_edge(graph_t* graph, const edge_t edge)
{
	debug_print("%s\n", "called");
	iterator_t* iter;
	if ((iter = tree_search(graph->vertices, edge.begin)) == NULL) {
		debug_print("%s\n", "invalid iterator");
		return -1;
	}
	vertex_t* begin = iter->value;
	if (begin == NULL) {
		begin = new_vertex(edge.begin);
		if (tree_add(graph->vertices, begin) < 0) {
			debug_print("%s\n", "tree_add failed");
			free(iter);
			free(begin);
			return -1;
		}
	}

	if ((iter = tree_search(graph->vertices, edge.end)) == NULL) {
		debug_print("%s\n", "invalid iterator");
		return -1;
	}
	vertex_t* end = iter->value;
	if (end == NULL) {
		end = new_vertex(edge.end);
		if (tree_add(graph->vertices, end) < 0) {
			debug_print("%s\n", "tree_add failed");
			free(iter);
			free(end);
			return -1;
		}
	}
	free(iter);


	if (tree_add(begin->edges, end) < 0) {
		return -1;
	}

	return tree_add(end->edges, begin);
}

int delete_edge(graph_t* graph, const edge_t edge)
{
	debug_print("%s\n", "called");
	iterator_t* iter;
	vertex_t* begin;
	vertex_t* end;
	if ((iter = tree_search(graph->vertices, edge.begin)) == NULL) {
		debug_print("%s\n", "invalid iterator");
		return -1;
	}
	begin = iter->value;
	if (begin == NULL) {
		debug_print("%s\n", "invalid iterator value");
		free(iter);
		return -1;
	}
	free(iter);

	if ((iter = tree_search(graph->vertices, edge.end)) == NULL) {
		debug_print("%s\n", "invalid iterator");
		return -1;
	}
	end = iter->value;
	if (end == NULL) {
		debug_print("%s\n", "invalid iterator value");
		free(iter);
		return -1;
	}
	free(iter);

	if (tree_remove(begin->edges, edge.end) == NULL) {
		debug_print("%s\n", "tree_remove failed");
		return -1;
	}
	if (tree_remove(end->edges, edge.begin) == NULL) {
		debug_print("%s\n", "tree_remove failed");
		return -1;
	}

	return 0;
}

bool graph_colored(const graph_t* graph)
{
	debug_print("%s\n", "called");
	iterator_t* iter = tree_min(graph->vertices);
	if (iter == NULL) {
		debug_print("%s\n", "invalid iterator");
		return false;
	}

	iterator_t* edge_iter;
	do {
		vertex_t* vertex = iter->value;
		color_t color = vertex->color;
		debug_print("%s: %d\n", "vertex", vertex->id);

		if (vertex == NULL) {
			continue;
		}

		edge_iter = tree_min(vertex->edges);
		if (edge_iter == NULL) {
			debug_print("%s\n", "invalid iterator");
			free(iter);
			return false;
		}

		do {
			vertex_t* neighbor = edge_iter->value;
			if (neighbor == NULL) {
				debug_print("%d: %s\n", vertex->id, "No edges");
				continue;
			}
			debug_print("%s: (%d,%d)\n", "edge", vertex->id, neighbor->id);
			if (color == neighbor->color) {
				debug_print(
					"%s: (%d,%d)\n",
					"Same color found",
					vertex->id,
					neighbor->id);
				free(iter);
				free(edge_iter);
				return false;
			}
		} while (next(edge_iter) != -1);
	} while (next(iter) != -1);

	free(iter);
	free(edge_iter);
	return true;
}
