/**
 * @file main.c
 * @author Matthias Pichler, 01634256
 * @date 2018-06-19
 * @brief OSUE bonus exercise
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "../include/common.h"

static int file_desc;
static vault_action_t action;
static char* p_name = NULL;
static vault_params_t params;

/**
 * @brief print the usage message of the program to STDOUT
 */
static void usage(void)
{
	fprintf(stdout, "svctl [-c <size>|-e|-d] <secvault id>\n");
	fprintf(
		stdout,
		"-c\t create a new secvault with the specified size and given id\n");
	fprintf(stdout, "-e\t erase the vault with the given id\n");
	fprintf(stdout, "-e\t delete the vault with the given id\n");
}

/**
 * @brief convert a string to an integer
 * @param str the source string
 * @param val the pointer that will hold the integer value after conversion
 * @return 0 on success, -1 otherwise
 */
static int str2int(const char* str, long* val)
{
	char* endptr;
	errno = 0;
	*val = strtol(str, &endptr, 10);

	if (errno == ERANGE) {
		return -1;
	}
	if ((errno != 0 && *val == 0)) {
		return -1;
	}
	if (endptr == str) {
		return -1;
	}
	return 0;
}


/**
 * @brief parse the arguments given to the program
 * @param argc the number of arguments to parse
 * @param argv the argument strings passed to the program
 * @return 0 on success, -1 otherwise
 */
static int parse_args(int argc, char* argv[])
{
	p_name = argv[0];

	bool flag = false;
	size_t size = 0;
	int c, id;
	while ((c = getopt(argc, argv, "c:ked")) != -1) {
		switch (c) {
			case 'c':
				if (flag) {
					fprintf(stderr, "%s: Wrong number of arguments\n", p_name);
					return -1;
				}

				action = create;
				if (str2int(optarg, (long*)&size) < 0) {
					fprintf(stderr, "%s: Malformed size\n", p_name);
					return -1;
				}
				if (size < 1 || size > MAX_VAULT_SIZE) {
					fprintf(stderr, "%s: Invalid size\n", p_name);
					return -1;
				}
				break;
			case 'k':
				if (flag) {
					fprintf(stderr, "%s: Wrong number of arguments\n", p_name);
					return -1;
				}

				action = change_key;
				break;
			case 'e':
				if (flag) {
					fprintf(stderr, "%s: Wrong number of arguments\n", p_name);
					return -1;
				}

				action = erase;
				break;
			case 'd':
				if (flag) {
					fprintf(stderr, "%s: Wrong number of arguments\n", p_name);
					return -1;
				}

				action = delete;
				break;
			default:
				return -1;
				break;
		}
	}

	if (optind < argc) {
		if (str2int(argv[optind], (long*)&id) < 0) {
			fprintf(stderr, "%s: Malformed id\n", p_name);
			return -1;
		}
		if (id < 0 || id > MAX_NUM_VAULTS - 1) {
			fprintf(stderr, "%s: invalid id\n", p_name);
			return -1;
		}
	} else {
		fprintf(stderr, "%s: No id provided\n", p_name);
		return -1;
	}

	params.id = id;
	params.max_size = size;
	memset(params.key, 0, VAULT_KEY_SIZE + 1);

	if (!flag) {
		action = print_size;
	}

	if (argc - optind > 1) {
		fprintf(stderr, "%s: Too many arguments\n", p_name);
		return -1;
	}

	if (action == create || action == change_key) {
		fprintf(stdout, "Please enter key: ");
		for (int i = 0; i < VAULT_KEY_SIZE; i++) {
			if ((c = getchar()) == EOF) {
				break;
			}
			params.key[i] = c;
		}
	}

	return 0;
}

/**
 * @brief handles exit of the program by freeing all resources
 */
static void exit_handler(void)
{
	close(file_desc);
}

int main(int argc, char* argv[])
{
	if (atexit(exit_handler) < 0) {
		fprintf(stderr, "Could not set exit function\n");
		exit(EXIT_FAILURE);
	}

	if (parse_args(argc, argv) < 0) {
		usage();
		exit(EXIT_FAILURE);
	}

	file_desc = open("/dev/" CTL_DEV_NAME, 0);
	if (file_desc < 0) {
		fprintf(stderr, "%s: Could not open file\n", p_name);
		exit(EXIT_FAILURE);
	}

	long ret;
	switch (action) {
		case create:
			if (ioctl(file_desc, CMD_CREATE, &params) < 0) {
				fprintf(
					stderr,
					"%s: Creation failed: %s\n",
					p_name,
					strerror(errno));
				return EXIT_FAILURE;
			}
			break;
		case erase:
			if (ioctl(file_desc, CMD_ERASE, params.id) < 0) {
				fprintf(
					stderr, "%s: Erase failed: %s\n", p_name, strerror(errno));
				return EXIT_FAILURE;
			}
			break;
		case delete:
			if (ioctl(file_desc, CMD_DELETE, params.id) < 0) {
				fprintf(
					stderr, "%s: Delete failed: %s\n", p_name, strerror(errno));
				return EXIT_FAILURE;
			}
			break;
		case print_size:
			if ((ret = ioctl(file_desc, CMD_SIZE, params.id) < 0)) {
				fprintf(
					stderr,
					"%s: Query size failed: %s\n",
					p_name,
					strerror(errno));
				return EXIT_FAILURE;
			}
			fprintf(stdout, "%s: Size= %ld", p_name, ret);
			break;
		case change_key:
			if (ioctl(file_desc, CMD_CHANGE_KEY, &params) < 0) {
				fprintf(
					stderr,
					"%s: change key failed: %s\n",
					p_name,
					strerror(errno));
				return EXIT_FAILURE;
			}
			break;
	}

	return EXIT_SUCCESS;
}
