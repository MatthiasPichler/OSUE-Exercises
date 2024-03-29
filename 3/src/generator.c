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

static char *p_name = NULL;
static edge_t *edges = NULL;
static graph_t *graph = NULL;
static ring_buffer_t *buffer;

static void sig_handler(int signo);
static void exit_handler(void);

static int parse_args(int argc, char *argv[]);
static int parse_edge(char *str, vid_t *begin, vid_t *end);
static void usage(void);

static int find_edge(graph_t *graph, edge_t *edge);

int main(int argc, char *argv[])
{

	if (atexit(exit_handler) < 0) {
		fprintf(stderr, "Could not set exit function\n");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGINT signal handler\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	if (parse_args(argc, argv) < 0) {
		usage();
		exit(EXIT_FAILURE);
	}

	if ((buffer = new_ring_buffer()) == NULL) {
		fprintf(stderr, "Could not open ring buffer\n");
		exit(EXIT_FAILURE);
	}

	while (true) {
		free_graph(graph);
		graph = new_graph(edges, argc - 1);
		solution_t solution = {.size = 0ul};
		while (!graph_colored(graph)) {
			edge_t edge;
			if (find_edge(graph, &edge) < 0) {
				break;
			}
			if (delete_edge(graph, edge) < 0) {
				break;
			}
			solution.edges[solution.size] = edge;
			solution.size++;

			if (solution.size > MAX_SOLUTION_SIZE) {
				break;
			}
		}


		if (solution.size > MAX_SOLUTION_SIZE) {
			continue;
		}

		if (block_write(buffer, solution) < 0) {
			fprintf(stderr, "%s: writing failed.\n", p_name);
			exit(EXIT_FAILURE);
		}

		if (solution.size == 0) {
			fprintf(stdout, "%s: Graph already 3colorable\n", p_name);
			exit(EXIT_SUCCESS);
		}
	}

	return EXIT_SUCCESS;
}

/**
 * @brief handler for calls to exit
 */
static void exit_handler(void)
{
	free_graph(graph);
	clean_buffer(buffer);
	fclose(stdout);
}

/**
 * @brief the signal handler for SIGINT to kill all child processes
 * @param signo the identifier of the signal
 */
static void sig_handler(int signo)
{
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
	p_name = argv[0];

	if (argc < 2) {
		return -1;
	}

	edges = (edge_t *)malloc(sizeof(edge_t) * (argc - 1));
	if (edges == NULL) {
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		vid_t begin;
		vid_t end;
		if (parse_edge(argv[i], &begin, &end) < 0) {
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

	if (begin == NULL || end == NULL) {
		return -1;
	}

	bool del_f = false;
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == '-') {
			if (del_f) {
				return -1;
			}
			del_f = true;
			if (i == 0 || i == strlen(str) - 1) {
				return -1;
			}
			continue;
		}
		if (str[i] < '0' || str[i] > '9') {
			return -1;
		}
	}
	if (!del_f) {
		return -1;
	}

	char *del;
	if ((del = strchr(str, '-')) == NULL) {
		return -1;
	}

	int n = strlen(str) - strlen(del);
	char *begin_str;
	if ((begin_str = strndup(str, n)) == NULL) {
		return -1;
	}

	*begin = strtol(begin_str, NULL, 10);
	free(begin_str);


	char *end_str;
	if ((end_str = strdup(del + 1)) == NULL) {
		return -1;
	}

	*end = strtol(end_str, NULL, 10);
	free(end_str);
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
 * @brief finds an edge that violates the 3coloring of the graph
 * @param graph the graph to search
 * @param edge the edge to hold the found values
 * @return 0 on success, -1 on failure e.g if no edge was found
 */
static int find_edge(graph_t *graph, edge_t *edge)
{
	for (size_t i = 0; i < graph->edge_size; i++) {
		edge_t e = graph->edges[i];
		if (graph->vertices[e.begin] == graph->vertices[e.end]) {
			memcpy(edge, &e, sizeof(edge_t));
			return 0;
		}
	}
	return -1;
}
