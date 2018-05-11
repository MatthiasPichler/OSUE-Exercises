/**
 * @file server.c
 * @author Matthias Pichler, 01634256
 * @date 2018-04-10
 *
 * @brief Server for OSUE exercise 1B `Battleship'.
 */
// IO, C standard library, POSIX API, data types:
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

// Assertions, errors, signals:
#include <assert.h>
#include <errno.h>
#include <signal.h>

// Time:
#include <time.h>

// Sockets, TCP, ... :
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

// stuff shared by client and server:
#include "../include/common.h"
#include "../include/ship.h"
#include "../include/map.h"
#include "../include/msg.h"

// Static variables for things you might want to access from several functions:
static const char *port = DEFAULT_PORT;  // the port to bind to

// Static variables for resources that should be freed before exiting:
static struct addrinfo *ai = NULL;  // stores address information
static int sock_fd = -1;			// socket file descriptor
static int conn_fd = -1;			// connection file descriptor

static char *program_name;
static map_t *map = NULL;

static int parse_args(int argc, char *argv[]);
static void print_usage(void);

static int recv_msg(client_msg_t *msg);
static int send_msg(server_msg_t msg);

static void print_err(char *msg);
static void exit_cleanup();
static void signal_cleanup(int signo);


int main(int argc, char *argv[])
{
	debug_print("%s\n", "Register exit function");
	if (atexit(exit_cleanup) < 0) {
		fprintf(stderr, "Could not set exit function");
		return EXIT_FAILURE;
	}
	debug_print("%s\n", "Register SIGINT handler");
	if (signal(SIGINT, signal_cleanup) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGINT signal handler");
		return EXIT_FAILURE;
	}
	debug_print("%s\n", "Register SIGTERM handler");
	if (signal(SIGTERM, signal_cleanup) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGTERM signal handler");
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Creating map");
	map = get_map();

	debug_print("%s\n", "Parsing arguments");
	if (parse_args(argc, argv) < 0) {
		print_usage();
		return EXIT_FAILURE;
	}


	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	debug_print("%s\n", "Get address info");
	int res = getaddrinfo(NULL, port, &hints, &ai);
	if (res != 0) {  // no errno
		fprintf(
			stderr, "%s: Could not set parameters for socket", program_name);
		fprintf(stderr, "\t%s\n", gai_strerror(res));
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Creating socket");
	sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (sock_fd < 0) {
		print_err("Could not create socket");
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Setting socket options");
	int val = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val) < 0) {
		print_err("Could set options for socket");
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Binding socket");
	if (bind(sock_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
		print_err("Could bind socket");
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Listening on socket");
	if (listen(sock_fd, 1) < 0) {
		print_err("Could listen on socket");
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Accepting connections");
	conn_fd = accept(sock_fd, NULL, NULL);
	if (conn_fd < 0) {
		print_err("Could accept connections");
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Starting event loop");
	for (uint8_t round = 0; round < MAX_ROUNDS; round++) {
		client_msg_t request = 0;
		// receive
		debug_print("%s\n", "Receiving message");
		if (recv_msg(&request) < 0) {
			print_err("Could receive message on socket");
			return EXIT_FAILURE;
		}

		debug_print("%s\n", "Checking parity");
		if (!check_parity(request)) {
			if (send_msg(err_parity) < 0) {
				print_err("Could send error message on socket");
			}
			fprintf(stderr, "%s: Parity error\n", program_name);
			return EXIT_PARITY_ERR;
		}

		const coordinate_t coordinate = get_coordinates(request);


		debug_print("%s\n", "Checking coordinates");
		if (!check_coordinate(coordinate)) {
			if (send_msg(err_coordinate) < 0) {
				print_err("Could send error message on socket");
			}
			fprintf(stderr, "%s: Invalid coordinate\n", program_name);
			return EXIT_COORDINATE_ERR;
		}

		debug_print(
			"coordinates: row=%d col=%d\n", coordinate.row, coordinate.col);

		debug_print("%s\n", "Shooting at coordinates");
		hit_report_t report = shoot(map, coordinate);

		if (DEBUG) {
			print_map(map);
		}

		switch (report) {
			case report_no_hit:
				// fallthrough
			case report_hit:
				// fallthrough
			case report_sunk:
				debug_print("%s\n", "Sending shot report");
				if (send_msg(game_ongoing | report) < 0) {
					print_err("Could send message on socket");
					return EXIT_FAILURE;
				}
				break;
			case report_last_sunk:
				debug_print("%s\n", "Last ship sunk");
				if (send_msg(game_over | report) < 0) {
					print_err("Could send message on socket");
					return EXIT_FAILURE;
				}

				printf("%s: Rounds: %d\n", program_name, round);
				return EXIT_SUCCESS;
				break;
		}
	}
	// loop exit
	debug_print("%s\n", "Loop exited");
	if (send_msg(game_over) < 0) {
		print_err("Could send error message on socket");
		return EXIT_FAILURE;
	}
	printf("%s: Game lost\n", program_name);
	return EXIT_SUCCESS;
}

/**
 * @brief Print the usage message to stdout
 */
static void print_usage(void)
{

	printf("\nUsage:\n");
	printf("\tserver [-p PORT] SHIPS...\n");
	printf("\n\t-p\tthe port to listen on. Defaults to %s\n", DEFAULT_PORT);
	printf(
		"\n\tships\ta list of 6 coordinate pairs, each denoting the begin and "
		"end of a ship. None of the ships are allowed to touch each other.\n");
	printf("\nexample:\n");
	printf("\tserver -p 1280 C2E2 F0H0 B6A6 E8E6 I2I5 H8I8\n");
}

/**
 * @brief Parses the program command line options
 * @details Returns 0 if all parameters were parsed correctly.
 * -1 is returned when one of the following conditions is violated:
 * 	- Any two ships touch each other
 * 	- Any coordinate is not within the map
 * 	- The amount of ships deviates from the expected number
 * 	- The amount for any ship type deviate from the expected number
 * 	- wrong/unknown/unexpected arguments are specified
 * @param argc the argument counter, length of argv
 * @param argv an array of arguments
 * @return 0 on success, -1 on failure
 */
static int parse_args(int argc, char *argv[])
{
	program_name = argv[0];

	int arg_c;
	while ((arg_c = getopt(argc, argv, "p:")) != EOF) {
		switch (arg_c) {
			case 'p':
				port = optarg;
				break;
			default:
				return -1;
		}
	}

	// parse remaining arguments aka ships
	if ((argc - optind) != SHIP_CNT_TOTAL) {
		return -1;
	}

	for (int i = optind; i < argc; i++) {

		ship_t *new_ship = parse_coordinates(argv[i]);
		if (new_ship == NULL) {
			return -1;
		}
		add_ship(map, new_ship);
	}

	if (!check_ship_count(map)) {
		return -1;
	}

	if (!check_ship_touch(map)) {
		return -1;
	}

	return 0;
}

/**
 * @brief print to stderr, but prepand the program name
 * @param msg the text to print
 */
static void print_err(char *msg)
{
	fprintf(stderr, "%s: %s\n", program_name, msg);
	fprintf(stderr, "\t%s\n", strerror(errno));
}

/**
 * @brief receive a message on the socket
 * @details reads a message in little endian byte order from the socket.
 * @param msg the message is stored into this parameter
 * @return 0 if the read succeeded, -1 otherwise
 */
static int recv_msg(client_msg_t *msg)
{
	*msg = 0;
	uint8_t buf[sizeof(client_msg_t)];
	if (recv(conn_fd, buf, sizeof(client_msg_t), MSG_WAITALL) < 0) {
		return -1;
	}

	for (int i = 0; i < sizeof(client_msg_t); i++) {
		*msg |= (client_msg_t)buf[i] << 8 * i;
	}

	debug_print("Received message %04x\n", *msg);
	return 0;
}

/**
 * @brief send a message on the socket
 * @details sends a message in little endian byte order over the socket.
 * @param msg the message to be send
 * @return 0 if sending succeeded, -1 otherwise
 */
static int send_msg(server_msg_t msg)
{
	debug_print("Sending message %04x\n", msg);
	uint8_t buf[sizeof(server_msg_t)];
	for (int i = 0; i < sizeof(server_msg_t); i++) {
		buf[i] = msg >> 8 * i;
	}
	if (send(conn_fd, buf, sizeof(server_msg_t), 0) < 0) {
		return -1;
	}
	return 0;
}

/**
 * @brief free all dependencies and exit with the specified exit code
 * @param exit_code the exit code to exit with
 */
static void exit_cleanup()
{
	if (ai != NULL) {
		freeaddrinfo(ai);
	}

	if (sock_fd != -1) {
		close(sock_fd);
	}

	if (conn_fd != -1) {
		close(conn_fd);
	}

	if (map != NULL) {
		free(map);
	}
}

/**
 * @brief cleanup all dependencies and exit with 0
 * @param signo the signal number
 */
static void signal_cleanup(int signo)
{
	exit_cleanup();
	exit(EXIT_SUCCESS);
}