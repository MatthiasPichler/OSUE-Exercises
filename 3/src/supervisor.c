/**
 * @file supervisor.c
 * @author Matthias Pichler, 01634256
 * @date 2018-05-29
 * @brief OSUE Exercise 3
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>

#include "../include/ring_buffer.h"
#include "../include/common.h"


static char *p_name;
static ring_buffer_t *buffer;
static solution_t record = {.size = SIZE_MAX};

static void sig_handler(int signo);
static void exit_handler(void);

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
	if (signal(SIGTERM, sig_handler) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGTERM signal handler\n");
		exit(EXIT_FAILURE);
	}


	p_name = argv[0];

	if (argc > 1) {
		fprintf(stderr, "%s: Too many arguments: expected=none\n", p_name);
		exit(EXIT_FAILURE);
	}


	if ((buffer = new_ring_buffer()) == NULL) {
		fprintf(stderr, "Could not open ring buffer\n");
		exit(EXIT_FAILURE);
	}

	while (true) {

		solution_t solution;
		if (block_read(buffer, &solution) < 0) {
			fprintf(stderr, "%s: read from buffer failed.\n", p_name);
			exit(EXIT_FAILURE);
		}

		if (solution.size < record.size) {
			record = solution;
			fprintf(
				stdout,
				"%s: New solution found: {size: %lu, edges: ",
				p_name,
				solution.size);

			for (size_t i = 0; i < solution.size; i++) {
				fprintf(
					stdout,
					"%u-%u ",
					solution.edges[i].begin,
					solution.edges[i].end);
			}

			fprintf(stdout, "}\n");
		}

		if (solution.size == 0) {
			fprintf(stdout, "%s: Graph already 3colorable.\n", p_name);
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
	close_buffer(buffer);
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
