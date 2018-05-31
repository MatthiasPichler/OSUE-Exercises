/**
 * @file generator.c
 * @author Matthias Pichler, 01634256
 * @date 2018-05-29
 * @brief OSUE Exercise 3
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "../include/graph.h"
#include "../include/ring_buffer.h"

static edge_t *edges = NULL;
static graph_t *graph = NULL;
static char *p_name = NULL;

static void sigint_handler(int signo);
static void exit_handler(void);

static int parse_args(int argc, char *argv[]);
static int parse_edge(char *str, vid_t *begin, vid_t *end);
static void usage(void);

static void random_color(graph_t *graph);
static int find_edge(graph_t *graph, edge_t *edge);

int main(int argc, char *argv[])
{

	if (atexit(exit_handler) < 0) {
		fprintf(stderr, "Could not set exit function\n");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGINT, sigint_handler) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGINT signal handler\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	if (parse_args(argc, argv) < 0) {
		usage();
		exit(EXIT_FAILURE);
	}


	while (true) {

		graph = new_graph(edges, argc - 1);
		random_color(graph);
		fprintf(stdout, "\n\nStart:\n");
		graph_print(graph);

		while (!graph_colored(graph)) {

			edge_t edge;
			if (find_edge(graph, &edge) < 0) {
				break;
			}
			fprintf(stdout, "Removing: (%d,%d)\n", edge.begin, edge.end);
			delete_edge(graph, edge);
			fprintf(stdout, "Removed: (%d,%d)\n", edge.begin, edge.end);
			graph_print(graph);
		}
		fprintf(stdout, "Solution found:\n");
		graph_print(graph);

		sleep(1);
	}


	return EXIT_SUCCESS;
}

/**
 * @brief handler for calls to exit
 */
static void exit_handler(void)
{
	free(graph);
	fclose(stdout);
}

/**
 * @brief the signal handler for SIGINT to kill all child processes
 * @param signo the identifier of the signal
 */
static void sigint_handler(int signo)
{
	exit_handler();
	exit(EXIT_FAILURE);
}

/**
 * @brief Parses the program command line options
 * @details Returns 0 if all parameters were parsed correctly, -1 otherwise
 * @param argc the argument counter, length of argv
 * @param argv an array of arguments
 * @return 0 on success, -1 on failure
 */
static int parse_args(int argc, char *argv[])
{
	debug_print("%s\n", "called");
	p_name = argv[0];

	if (argc < 2) {
		debug_print("%s\n", "to few arguments");
		return -1;
	}

	edges = (edge_t *)malloc(sizeof(edge_t) * (argc - 1));
	if (edges == NULL) {
		debug_print("%s\n", "malloc failed");
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		vid_t begin;
		vid_t end;
		if (parse_edge(argv[i], &begin, &end) < 0) {
			debug_print("%s: %s\n", "parse_edge failed", argv[i]);
			return -1;
		}

		edge_t edge = {.begin = begin, .end = end};
		edges[i - 1] = edge;
	}

	return 0;
}

/**
 * @brief parses and validates the strings passed as arguments to the generator
 * @param str the string to be parsed
 * @param begin the pointer which holds the beginning vertex of the edge after
 * parsing
 * @param end the pointer which holds the ending vertex of the edge after
 * parsing
 * @return 0 on success, -1 on failure
 *
 */
static int parse_edge(char *str, vid_t *begin, vid_t *end)
{
	debug_print("%s\n", "called");
	if (begin == NULL || end == NULL) {
		return -1;
	}

	bool del_f = false;
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == '-') {
			if (del_f) {
				debug_print("%s\n", "too many delimiters");
				return -1;
			}
			del_f = true;
			continue;
		}
		if (str[i] < '0' || str[i] > '9') {
			debug_print("%s: %c\n", "not a digit", str[i]);
			return -1;
		}
	}
	if (!del_f) {
		debug_print("%s\n", "no delimiter");
		return -1;
	}

	char *endptr;

	char *del;
	if ((del = strchr(str, '-')) == NULL) {
		debug_print("%s\n", "No delimiter found");
		return -1;
	}

	int n = strlen(str) - strlen(del);
	char *begin_str;
	if ((begin_str = strndup(str, n)) == NULL) {
		return -1;
	}

	*begin = strtol(begin_str, &endptr, 10);
	free(begin_str);
	if (strcmp(endptr, "\0") != 0) {
		debug_print("%s\n", "could not parse begin to number");
		return -1;
	}

	char *end_str;
	if ((end_str = strdup(del + 1)) == NULL) {
		return -1;
	}

	*end = strtol(end_str, &endptr, 10);
	free(end_str);
	if (strcmp(endptr, "\0") != 0) {
		debug_print("%s\n", "could not parse end to number");
		return -1;
	}
	return 0;
}

/**
 * @brief Print the usage message to stderr
 */
static void usage(void)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "generator EDGES...\n");
	fprintf(stderr, "where each edge is of format 1-2\n");
}

/**
 * @brief randomly color the given graph
 * @param graph the graph to be colored
 */
static void random_color(graph_t *graph)
{

	debug_print("%s\n", "called");
	iterator_t *iter = tree_min(graph->vertices);
	if (iter == NULL) {
		debug_print("%s\n", "invalid iterator");
		return;
	}

	vertex_t *v;
	do {
		v = iter->value;
		color_t c = (color_t)(rand() % 3);
		if (tree_update_color(graph->vertices, v->id, c) < 0) {
			debug_print("%s\n", "update color failed");
			return;
		}
	} while (next(iter) != -1);

	free(iter);
}

/**
 * @brief finds an edge that violates the 3coloring of the graph
 * @param graph the graph to search
 * @param edge the edge to hold the found values
 * @return 0 on success, -1 on failure e.g if no edge was found
 */
static int find_edge(graph_t *graph, edge_t *edge)
{
	debug_print("%s\n", "called");
	iterator_t *iter = tree_min(graph->vertices);
	if (iter == NULL) {
		debug_print("%s\n", "invalid iterator");
		return -1;
	}

	iterator_t *edges;
	do {
		vertex_t *vertex = iter->value;
		color_t color = vertex->color;

		edges = tree_min(vertex->edges);
		if (edges == NULL) {
			debug_print("%s\n", "invalid iterator");
			free(iter);
			return -1;
		}

		do {
			if (edges->value == NULL) {
				debug_print("%d: %s\n", vertex->id, "No edges");
				continue;
			}
			if (color == edges->value->color) {
				edge->begin = vertex->id;
				edge->end = edges->value->id;
				free(iter);
				free(edges);
				return 0;
			}
		} while (next(edges) != -1);
	} while (next(iter) != -1);

	free(iter);
	free(edges);
	return -1;
}
