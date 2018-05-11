/**
 * @file client.c
 * @author Matthias Pichler, 01634256
 * @date 2018-04-10
 *
 * @brief Client for OSUE exercise 1B `Battleship'.
 */

// IO, C standard library, POSIX API, data types:
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

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
#include "../include/msg.h"
#include "../include/solver.h"

// Static variables for things you might want to access from several functions:
static const char *port = DEFAULT_PORT;  // the port to connect to
static const char *host = DEFAULT_HOST;  // the port to connect to

// Static variables for resources that should be freed before exiting:
static struct addrinfo *ai = NULL;  // stores address information
static int sock_fd = -1;			// socket file descriptor
static char *program_name;

static int parse_args(int argc, char *argv[]);
static void print_usage(void);

static int send_msg(client_msg_t msg);
static int recv_msg(server_msg_t *msg);

static void print_err(char *msg);
static void exit_cleanup();

static coordinate_t last_shot;
static hit_report_t last_report = report_no_hit;

int main(int argc, char *argv[])
{
	debug_print("%s\n", "Register exit function");
	if (atexit(exit_cleanup) < 0) {
		fprintf(stderr, "Could not set exit function");
		return EXIT_FAILURE;
	}

	memcpy(&last_shot, &invalid_coordinate, sizeof(coordinate_t));

	debug_print("%s\n", "Parsing arguments");
	if (parse_args(argc, argv) < 0) {
		print_usage();
		return EXIT_FAILURE;
	}

	init_solver();

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	debug_print("%s\n", "Get address info");
	int res = getaddrinfo(NULL, port, &hints, &ai);
	if (res != 0) {  // no errno
		fprintf(
			stderr, "%s: Could not set parameters for socket:", program_name);
		fprintf(stderr, "\t%s\n", gai_strerror(res));
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Creating socket");
	sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (sock_fd < 0) {
		print_err("Could not create socket:");
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Connecting");
	if (connect(sock_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
		print_err("Could connect on socket:");
		return EXIT_FAILURE;
	}

	debug_print("%s\n", "Starting event loop");
	while (true) {


		coordinate_t c = next_move(last_shot, last_report);
		last_shot = c;

		debug_print("row=%d col=%d\n", c.row, c.col);

		client_msg_t request = (c.col << X_COORDINATE_OFFSET) | c.row;

		debug_print("Request: %04x\n", request);

		uint8_t p = calc_parity_bit(request);

		debug_print("%s\n", "Sending message");
		if (send_msg(set_parity_bit(request, p)) < 0) {
			print_err("Could not send message over socket:");
			return EXIT_FAILURE;
		}

		debug_print("%s\n", "Receiving message");
		server_msg_t response;
		if (recv_msg(&response) < 0) {
			print_err("Could not receive message over socket:");
			return EXIT_FAILURE;
		}

		last_report = get_hit_report(response);
		debug_print("Hit: %d\n", last_report);

		status_t status = get_status(response);
		debug_print("Status: %d\n", status);

		switch (status) {
			case game_over:
				if (get_hit_report(response) == report_last_sunk) {
					printf("%s: Game won\n", program_name);
					return EXIT_SUCCESS;
				} else {
					printf("%s: Game lost\n", program_name);
					return EXIT_SUCCESS;
				}
				break;
			case err_coordinate:
				printf("%s: Invalid coordinate\n", program_name);
				return EXIT_COORDINATE_ERR;
				break;
			case err_parity:
				printf("%s: Parity error\n", program_name);
				return EXIT_PARITY_ERR;
				break;
			default:
				continue;
		}
	}


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

	program_name = argv[0];

	int arg_c;
	while ((arg_c = getopt(argc, argv, "h:p:")) != EOF) {
		switch (arg_c) {
			case 'p':
				port = optarg;
				break;
			case 'h':
				host = optarg;
				break;
			default:
				return -1;
		}
	}

	if (argc - optind >= 1) {
		return -1;
	}

	return 0;
}

/**
 * @brief Print the usage message to stdout
 */
static void print_usage(void)
{

	printf("\nUsage:\n");
	printf("\tclient [-h HOST] [-p PORT]\n");
	printf("\n\t-p\tthe port to connect on. Defaults to %s\n", DEFAULT_PORT);
	printf("\n\t-p\tthe addres to connect to. Defaults to %s\n", DEFAULT_HOST);
	printf("\tclient -h localhost -p 1280\n");
}

/**
 * @brief receive a message on the socket
 * @details reads a message in little endian byte order from the socket.
 * @param msg the message is stored into this parameter
 * @return 0 if the read succeeded, -1 otherwise
 */
static int recv_msg(server_msg_t *msg)
{
	*msg = 0;
	uint8_t buf[sizeof(server_msg_t)];
	if (recv(sock_fd, buf, sizeof(server_msg_t), MSG_WAITALL) < 0) {
		return -1;
	}

	for (int i = 0; i < sizeof(server_msg_t); i++) {
		*msg |= (server_msg_t)buf[i] << 8 * i;
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
static int send_msg(client_msg_t msg)
{
	debug_print("Sending message %04x\n", msg);
	uint8_t buf[sizeof(client_msg_t)];
	for (int i = 0; i < sizeof(client_msg_t); i++) {
		buf[i] = msg >> 8 * i;
	}
	return send(sock_fd, buf, sizeof(client_msg_t), 0);
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

	free_solver();
}