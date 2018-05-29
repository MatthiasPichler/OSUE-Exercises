/**
 * @file dsort.c
 * @author Matthias Pichler, 01634256
 * @date 2018-05-12
 * @brief OSUE Exercise 2
 */

#define SAFE_DELETE(p) \
	if (p != NULL) {   \
		free(p);       \
	}                  \
	p = NULL;

// standard library, i/o, etc
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
// fork, exec, wait
#include <unistd.h>
#include <sys/wait.h>

// constants
#define MAX_CHARS 1024
#define SHELL "/bin/bash"
#define SHELL_OPT "-c"

static char *p_name = NULL;
static char *cmd1 = NULL, *cmd2 = NULL;
static int pipe_fd[2];
static pid_t c1_pid;
static pid_t c2_pid;

static void sigint_handler(int signo);
static void exit_handler(void);

static int parse_args(int argc, char *argv[]);
static void usage(void);

static int setup_pipe_write(void);
static FILE *setup_pipe_read(void);

static int exec_cmd(char *args[]);
static int wait_children();
static int exec_uniq();

static size_t read_lines(FILE *stream, char **p_lines[]);
static int str_cmp(const void *s1, const void *s2);

int main(int argc, char *argv[])
{
	// parse args
	if (parse_args(argc, argv) < 0) {
		usage();
		exit(EXIT_FAILURE);
	}

	// create pipe
	if (pipe(pipe_fd) < 0) {
		fprintf(stderr, "%s: Pipe creation failed\n", p_name);
		exit(EXIT_FAILURE);
	}

	// execute child 1
	char *args1[] = {SHELL, SHELL_OPT, cmd1, NULL};
	if ((c1_pid = exec_cmd(args1)) < 0) {
		fprintf(stderr, "%s: Child process failed\n", p_name);
		exit(EXIT_FAILURE);
	}

	// execute child 2
	char *args2[] = {SHELL, SHELL_OPT, cmd2, NULL};
	if ((c2_pid = exec_cmd(args2)) < 0) {
		fprintf(stderr, "%s: Child process failed\n", p_name);
		exit(EXIT_FAILURE);
	}

	// register parent exit function
	if (atexit(exit_handler) < 0) {
		fprintf(stderr, "Could not set exit function\n");
		exit(EXIT_FAILURE);
	}

	// register signal handlers for parent
	if (signal(SIGINT, sigint_handler) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGINT signal handler\n");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGTERM, sigint_handler) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGINT signal handler\n");
		exit(EXIT_FAILURE);
	}

	if (wait_children() < 0) {
		fprintf(stderr, "%s: Waiting failed\n", p_name);
		exit(EXIT_FAILURE);
	}

	FILE *stream;
	if ((stream = setup_pipe_read()) == NULL) {
		fprintf(stderr, "%s: Could not creat stream to pipe", p_name);
		exit(EXIT_FAILURE);
	}
	char **lines = NULL;
	size_t size;
	if ((size = read_lines(stream, &lines)) < 0) {
		fprintf(stderr, "%s: Could not read lines\n", p_name);
	}

	qsort(lines, size, sizeof(char *), str_cmp);

	// create pipe
	if (pipe(pipe_fd) < 0) {
		fprintf(stderr, "%s: Pipe creation failed\n", p_name);
		exit(EXIT_FAILURE);
	}

	pid_t pid;
	if ((pid = exec_uniq()) < 0) {
		fprintf(stderr, "%s: Child process failed\n", p_name);
		exit(EXIT_FAILURE);
	}

	if (setup_pipe_write() < 0) {
		fprintf(stderr, "%s: Could not setup pipe", p_name);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < size; i++) {
		fprintf(stdout, "%s", lines[i]);
		fflush(stdout);
		free(lines[i]);
	}
	free(lines);

	if (fclose(stdout) < 0) {
		fprintf(stderr, "%s: Could not close stdout\n", p_name);
		exit(EXIT_FAILURE);
	}

	if (wait_children() < 0) {
		fprintf(stderr, "%s: Waiting failed\n", p_name);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

/**
 * @brief handler for calls to exit
 */
static void exit_handler(void)
{
	fclose(stdout);
}

/**
 * @brief the signal handler for SIGINT to kill all child processes
 * @param signo the identifier of the signal
 */
static void sigint_handler(int signo)
{
	if ((kill(c1_pid, SIGTERM)) < 0) {
		fprintf(
			stderr,
			"%s: Could not kill process %d: %s\n",
			p_name,
			c1_pid,
			strerror(errno));
	}
	if ((kill(c2_pid, SIGTERM)) < 0) {
		fprintf(
			stderr,
			"%s: Could not kill process %d: %s\n",
			p_name,
			c2_pid,
			strerror(errno));
	}
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

	if (argc != 3) {
		return -1;
	}

	cmd1 = argv[1];
	cmd2 = argv[2];

	return 0;
}

/**
 * @brief Print the usage message to stderr
 */
static void usage(void)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "dsort cmd_1 cmd_2\n");
}

/**
 * @brief redirect stdout of the current process to the given pipe's write end
 * @return 0 if setup was successful, -1 otherwise
 */
static int setup_pipe_write(void)
{
	if ((dup2(pipe_fd[1], STDOUT_FILENO)) < 0) {
		fprintf(stderr, "%s: Could not duplicate stdout\n", p_name);
		return -1;
	}

	if (close(pipe_fd[0]) < 0) {  // close read end
		fprintf(stderr, "%s: Could not close read end of pipe\n", p_name);
		return -1;
	}

	if (close(pipe_fd[1]) < 0) {  // close write end
		fprintf(stderr, "%s: Could not close write end of pipe\n", p_name);
		return -1;
	}

	return 0;
}

/**
 * @brief redirect stdint of the current process to the given pipe's read end
 * @return a new stream to read from, or NULL on failure
 */
static FILE *setup_pipe_read(void)
{
	if (close(pipe_fd[1]) < 0) {  // close write end
		fprintf(stderr, "%s: Could not close write end of pipe\n", p_name);
		return NULL;
	}

	return fdopen(pipe_fd[0], "r");
}

/**
 * @brief executes the given command in a new process
 * @param args an array of arguments for the new command where args[0] has to be
 * the command to execute
 * @return the process id to the parent task and -1 on failure, the child
 * process executes the given task
 */
static int exec_cmd(char *args[])
{

	pid_t c_pid;
	// fork
	// switch between parent child
	switch (c_pid = fork()) {
		case -1:
			fprintf(stderr, "%s: Cannot fork\n", p_name);
			return -1;
			break;
		case 0: {
			// Child task
			if (setup_pipe_write() < 0) {
				fprintf(stderr, "%s: Could not setup pipe", p_name);
				_exit(EXIT_FAILURE);
			}
			execvp(args[0], args);
			// piping stdout
			fprintf(stderr, "%s: Child proccess execution failed\n", p_name);
			_exit(EXIT_FAILURE);
		}; break;
		default:
			// parent task
			return c_pid;
			break;
	}
}

/**
 * @brief executes uniq -d in a new process
 * @return the process id to the parent, -1 on failure and the child executes
 * the task
 */
static int exec_uniq()
{
	pid_t c_pid;
	// fork
	// switch between parent child
	switch (c_pid = fork()) {
		case -1:
			fprintf(stderr, "%s: Cannot fork\n", p_name);
			return -1;
			break;
		case 0: {
			// Child task
			if ((dup2(pipe_fd[0], STDIN_FILENO)) < 0) {
				fprintf(stderr, "%s: Could not duplicate stdin\n", p_name);
				_exit(EXIT_FAILURE);
			}

			if (close(pipe_fd[0]) < 0) {  // close read end
				fprintf(
					stderr, "%s: Could not close read end of pipe\n", p_name);
				_exit(EXIT_FAILURE);
			}

			if (close(pipe_fd[1]) < 0) {  // close write end
				fprintf(
					stderr, "%s: Could not close write end of pipe\n", p_name);
				_exit(EXIT_FAILURE);
			}

			char *args[] = {"uniq", "-d", NULL};
			execvp(args[0], args);
			fprintf(stderr, "%s: Child proccess execution failed\n", p_name);
			_exit(EXIT_FAILURE);
		}; break;
		default:
			// parent task
			return c_pid;
			break;
	}
}

/**
 * @brief wait for all child processes to finish
 * @return 0 if all child processes terminated correctly, -1 otherwise
 */
static int wait_children()
{
	// wait for children
	pid_t pid;
	int status;
	// block until a child finished
	while ((pid = wait(&status)) > 0) {
		// check if child finished correctly
		if (!WIFEXITED(status)) {
			fprintf(
				stderr,
				"%s: Child process %d did not exit correctly\n",
				p_name,
				pid);
			return -1;
		}
		if (WEXITSTATUS(status) != EXIT_SUCCESS) {
			fprintf(
				stderr,
				"%s: Child process %d exited with error code\n",
				p_name,
				pid);
			return -1;
		}
	}
	// an error occurred
	if (errno != ECHILD) {
		// no children to wait for was not the error
		fprintf(
			stderr,
			"%s: Error occurred while waiting: %s\n",
			p_name,
			strerror(errno));
		return -1;
	}

	return 0;
}

/**
 * @brief read all lines from the given stream and store them in an array
 * @param stream the stream to read from
 * @param lines the pointer to store the lines in
 * @return the size of the array of lines or -1 if an error occurred
 */
static size_t read_lines(FILE *stream, char **p_lines[])
{
	char **lines = (char **)malloc(sizeof(char *));
	if (lines == NULL) {
		return -1;
	}
	size_t i = 0;
	size_t nread, n = 0;
	char *buf = NULL;
	while ((nread = getline(&buf, &n, stream)) != -1) {

		lines[i] = buf;
		i++;
		lines = (char **)realloc(lines, (i + 1) * sizeof(char *));

		buf = NULL;
		n = 0;
	}
	*p_lines = lines;
	return i;
}

/**
 * @brief compare two string ignoring case
 * @return an integer less than, equal to, or greater than zero if s1 is, after
 ignoring case, found to be less than to match,  or  be greater than s2,
 respectively
 */
static int str_cmp(const void *s1, const void *s2)
{
	const char **ia = (const char **)s1;
	const char **ib = (const char **)s2;
	return strcasecmp(*ia, *ib);
}