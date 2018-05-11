/**
 * @file palindrom.c
 * @author Matthias Pichler, 01634256
 * @date 2018-03-10
 *
 * @brief Palindrom checker
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>

/**
 * @brief is_palindrom checks whether or not a given string is the same to its
 * reverse.
 * @details The method does neither check any length constaints nor manipulate
 * the string in any way. It is case and whitespace sensitive.
 * @param str the string to check
 * @return true if the string is a palindrom and false otherwise
 */
static bool is_palindrom(const char *str)
{
	const size_t length = strlen(str);

	size_t i = 0, j = length - 1;

	while (i < j) {
		if (str[i] != str[j]) {
			return false;
		}
		i++;
		j--;
	}

	return true;
}

/**
 * @brief to_lower casts every character in str to lower case
 * @param str the string to cast
 */
static void to_lower(char *str)
{
	const size_t length = strlen(str);

	for (size_t i = 0; i < length; i++) {
		str[i] = tolower(str[i]);
	}
}

/**
 * @brief is_whitespace checks whether a given character is whitespace (space,
 * tab, newline)
 * @param c the character to check
 * @return true if the character is whitespace, false otherwise
 */
static bool is_whitespace(const char c)
{
	return (c == ' ' || c == '\t' || c == '\n');
}

/**
 * @brief remove_whitespace removes every whitespace characterfrom the string
 * @details Every space, tab and newline is removed from the string, and the end
 * is padded with null characters
 * @param str the string to scan
 */
static void remove_whitespace(char *str)
{
	const size_t length = strlen(str);

	size_t writer = 0;

	for (size_t reader = 0; reader < length; reader++) {
		if (!is_whitespace(str[reader])) {
			str[writer] = str[reader];
			writer++;
		}
	}
	for (; writer < length; writer++) {
		str[writer] = '\0';
	}
}

/**
 * @brief trim_newline removes all trailing newline characters
 * @details It stops when any non newline charater is encountered. It replaces
 * the newline character with a null  character
 * @param str the string to trim
 */
static void trim_newline(char *str)
{
	for (size_t i = strlen(str) - 1; i >= 0; i--) {
		if (str[i] == '\n') {
			str[i] = '\0';
		} else {
			break;
		}
	}
}

/**
 * @brief print_usage outputs a help screen explaining the parameters
 */
static void print_usage(void)
{

	printf(
		"\nUsage:"
		"\tispalindrom [-s] [-i] [-o outfile] [infile]\n\n");
	printf("\tIf no infile is specified STDIN is used instead.\n\n");
	printf("\ts\tIgnore whitespace\n");
	printf("\ti\tIgnore case\n");
	printf(
		"\to\tWrite to the specified file, and create it if necessary. If this "
		"option is omitted STDOUT is used instead.\n");
}

int main(int argc, char *argv[])
{
	const size_t CHUNK_SIZE = 42;

	bool flag_case = false, flag_white = false;

	FILE *infile = stdin;
	FILE *outfile = stdout;

	int c;
	while ((c = getopt(argc, argv, "sio:")) != -1) {
		switch (c) {
			case 's':
				flag_white = true;
				break;
			case 'i':
				flag_case = true;
				break;
			case 'o':
				outfile = fopen(optarg, "w");
				if (infile == NULL) {
					fprintf(
						stderr, "[%s] Could not open output file.\n", argv[0]);
					return EXIT_FAILURE;
				}
				break;
			default:
				print_usage();
				return EXIT_FAILURE;
				break;
		}
	}

	if (optind < argc) {
		infile = fopen(argv[optind], "r");
		if (infile == NULL) {
			fprintf(stderr, "[%s] Could not open input file.\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (argc - optind > 1) {
		fprintf(stderr, "[%s] Too many arguments\n", argv[0]);
		print_usage();
		exit(EXIT_FAILURE);
	}

	char buff[CHUNK_SIZE];
	char original[CHUNK_SIZE];

	while (fgets(buff, CHUNK_SIZE, infile) != NULL) {

		if (buff[strlen(buff) - 1] != '\n') {
			fprintf(
				stderr,
				"[%s] Eingabe zu lang, max %lu Zeichen!\n",
				argv[0],
				CHUNK_SIZE - 2);
			return EXIT_FAILURE;
		}

		trim_newline(buff);
		strcpy(original, buff);

		if (flag_white) {
			remove_whitespace(buff);
		}

		if (flag_case) {
			to_lower(buff);
		}

		if (is_palindrom(buff)) {
			fprintf(outfile, "%s ist ein Palindrom\n", original);
		} else {
			fprintf(outfile, "%s ist kein Palindrom\n", original);
		}
	}

	fclose(infile);
	fclose(outfile);

	return EXIT_SUCCESS;
}