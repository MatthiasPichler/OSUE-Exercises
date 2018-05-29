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

#include "../include/graph.h"

static graph_t *graph;
static char *p_name = NULL;

static int parse_args(int argc, char *argv[]);
static int parse_edge(char *str, vid_t **begin, vid_t **end);
static void usage(void);

int main(int argc, char *argv[])
{
	graph = new_graph();

	if (parse_args(argc, argv) < 0) {
		usage();
		exit(EXIT_FAILURE);
	}

	tree_print(graph->vertices);

	return EXIT_SUCCESS;
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


	for (int i = 1; i < argc; i++) {
		vid_t *begin = NULL;
		vid_t *end = NULL;
		if (parse_edge(argv[i], &begin, &end) < 0) {
			return -1;
		}

		edge_t edge = {.begin = *begin, .end = *end};
		if (add_edge(graph, edge) < 0) {
			return -1;
		}
	}

	return 0;
}

/**
 * @brief parses and validates the strings passed as arguments to the generator
 * @param str the string to be parsed
 * @param begin the pointer which holds the beginning vertex of the edge after
 * parsing, if set to NULL it is initialized
 * @param end the pointer which holds the ending vertex of the edge after
 * parsing, if set to NULL it is initialized
 * @return 0 on success, -1 on failure
 *
 */
static int parse_edge(char *str, vid_t **begin, vid_t **end)
{
	bool del_f = false;
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == '-') {
			if (del_f) {
				return -1;
			}
			del_f = true;
			continue;
		}
		if (str[i] < '0' || str[i] > '9') {
			return -1;
		}
	}
	if (!del_f) {
		return -1;
	}

	// allocate return values if necessary
	if (begin == NULL) {
		begin = (vid_t **)malloc(sizeof(vid_t *));
		if (begin == NULL) {
			return -1;
		}
		*begin = NULL;
	}
	if (*begin == NULL) {
		*begin = (vid_t *)malloc(sizeof(vid_t));
		if (*begin == NULL) {
			return -1;
		}
	}
	if (end == NULL) {
		end = (vid_t **)malloc(sizeof(vid_t *));
		if (end == NULL) {
			return -1;
		}
		*end = NULL;
	}
	if (*end == NULL) {
		*end = (vid_t *)malloc(sizeof(vid_t));
		if (*end == NULL) {
			return -1;
		}
	}

	char *endptr;
	if ((endptr = (char *)malloc(sizeof(char))) == NULL) {
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

	**begin = strtol(begin_str, &endptr, 10);
	free(begin_str);
	if (strcmp(endptr, "\0") != 0) {
		return -1;
	}

	char *end_str;
	if ((end_str = strdup(del + 1)) == NULL) {
		return -1;
	}

	**end = strtol(end_str, &endptr, 10);
	free(end_str);
	if (strcmp(endptr, "\0") != 0) {
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