#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../include/ring_buffer.h"


ring_buffer_t* new_ring_buffer(void)
{

	ring_buffer_t* buffer = (ring_buffer_t*)malloc(sizeof(ring_buffer_t));
	if (buffer == NULL) {
		return NULL;
	}

	int fd;
	if ((fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, PERMISSION)) < 0) {
		fprintf(stderr, "could not open shared memory.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		free(buffer);
		return NULL;
	}
	if (ftruncate(fd, sizeof(ring_buffer_mem_t)) < 0) {
		fprintf(stderr, "could not set size for shared memory.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		free(buffer);
		close(fd);
		return NULL;
	}

	ring_buffer_mem_t* mem;
	if ((mem = (ring_buffer_mem_t*)mmap(
			 NULL,
			 sizeof(ring_buffer_mem_t),
			 PROT_READ | PROT_WRITE,
			 MAP_SHARED,
			 fd,
			 0))
		== MAP_FAILED) {
		fprintf(stderr, "could not map shared memory.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		clean_buffer(buffer);
		close(fd);
		return NULL;
	}

	if (close(fd) < 0) {
		fprintf(stderr, "could not close file descriptor.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		clean_buffer(buffer);
		return NULL;
	}


	mem->open = true;
	mem->w_pos = 0;
	mem->r_pos = 0;
	buffer->memory = mem;

	if ((buffer->free_sem = sem_open(FREE_SEM, O_CREAT, PERMISSION, BUF_SIZE))
		== SEM_FAILED) {
		fprintf(stderr, "could not open semaphore.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		clean_buffer(buffer);
		return NULL;
	}

	if ((buffer->used_sem = sem_open(USED_SEM, O_CREAT, PERMISSION, 0))
		== SEM_FAILED) {
		fprintf(stderr, "could not open semaphore.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		clean_buffer(buffer);
		return NULL;
	}
	if ((buffer->w_sem = sem_open(RW_SEM, O_CREAT, PERMISSION, 1))
		== SEM_FAILED) {
		fprintf(stderr, "could not open semaphore.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		clean_buffer(buffer);
		return NULL;
	}

	return buffer;
}

int clean_buffer(ring_buffer_t* buffer)
{
	if (buffer == NULL) {
		return -1;
	}

	int res = 0;
	if (buffer != NULL) {
		if (munmap(buffer->memory, sizeof(ring_buffer_mem_t)) < 0) {
			fprintf(stderr, "could not unmap shared memory.\n");
			fprintf(stderr, "\t%s\n", strerror(errno));
			res = -1;
		}
	}

	if (shm_unlink(SHM_NAME) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr, "could not unlink shared memory.\n");
			fprintf(stderr, "\t%s\n", strerror(errno));
			res = -1;
		}
	}

	if (sem_close(buffer->free_sem) < 0) {
		fprintf(stderr, "could not close semaphore.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		res = -1;
	}

	if (sem_close(buffer->used_sem) < 0) {
		fprintf(stderr, "could not close semaphore.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		res = -1;
	}

	if (sem_close(buffer->w_sem) < 0) {
		fprintf(stderr, "could not close semaphore.\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		res = -1;
	}

	if (sem_unlink(FREE_SEM) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr, "could not unlink semaphore.\n");
			fprintf(stderr, "\t%s\n", strerror(errno));
			res = -1;
		}
	}

	if (sem_unlink(USED_SEM) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr, "could not unlink semaphore.\n");
			fprintf(stderr, "\t%s\n", strerror(errno));
			res = -1;
		}
	}

	if (sem_unlink(RW_SEM) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr, "could not unlink semaphore.\n");
			fprintf(stderr, "\t%s\n", strerror(errno));
			res = -1;
		}
	}

	free(buffer);

	return res;
}

int close_buffer(ring_buffer_t* buffer)
{
	buffer->memory->open = false;
	return 0;
}

static int try_sem_wait(ring_buffer_t* buffer, sem_t* sem)
{
	while (buffer->memory->open) {
		if (sem_wait(sem) < 0) {
			if (errno == EINTR) {
				continue;
			}
			fprintf(stderr, "could not wait on semaphore\n");
			fprintf(stderr, "\t%s\n", strerror(errno));
			return -1;
		}
		return 0;
	}
	fprintf(stderr, "Buffer closed\n");
	exit(EXIT_SUCCESS);
	return -1;
}

static int try_sem_post(ring_buffer_t* buffer, sem_t* sem)
{
	while (sem_post(sem) < 0) {
		if (errno == EINTR) {
			continue;
		}
		fprintf(stderr, "could not post semaphore\n");
		fprintf(stderr, "\t%s\n", strerror(errno));
		return -1;
	}
	return 0;
}

static void append(ring_buffer_t* buffer, solution_t solution)
{
	buffer->memory->solutions[buffer->memory->w_pos] = solution;

	buffer->memory->w_pos = (buffer->memory->w_pos + 1) % BUF_SIZE;
}

static solution_t take(ring_buffer_t* buffer)
{
	solution_t solution = buffer->memory->solutions[buffer->memory->r_pos];

	buffer->memory->r_pos = (buffer->memory->r_pos + 1) % BUF_SIZE;

	return solution;
}

int block_write(ring_buffer_t* buffer, solution_t solution)
{

	if (try_sem_wait(buffer, buffer->free_sem) < 0) {
		return -1;
	}

	if (try_sem_wait(buffer, buffer->w_sem) < 0) {
		sem_post(buffer->free_sem);
		return -1;
	}

	append(buffer, solution);

	if (try_sem_post(buffer, buffer->w_sem) < 0) {
		sem_post(buffer->used_sem);
		return -1;
	}
	if (try_sem_post(buffer, buffer->used_sem) < 0) {
		return -1;
	}

	return 0;
}

int block_read(ring_buffer_t* buffer, solution_t* solution)
{
	if (try_sem_wait(buffer, buffer->used_sem) < 0) {
		return -1;
	}

	*solution = take(buffer);

	if (try_sem_post(buffer, buffer->free_sem) < 0) {
		return -1;
	}

	return 0;
}