
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "common.h"
#include "vertex.h"

#define SHM_NAME "/1634256_ring_buf_shm"
#define BUF_SIZE 32
#define MAX_SOLUTION_SIZE 8

#define FREE_SEM "/1634256_free_sem"
#define USED_SEM "/1634256_used_sem"
#define RW_SEM "/1634256_rw_sem"

#define PERMISSION 0600

typedef struct solution
{
	size_t size;
	edge_t edges[MAX_SOLUTION_SIZE];

} solution_t;

typedef struct ring_buffer_mem
{
	volatile sig_atomic_t open;
	int fd;
	size_t w_pos, r_pos;
	solution_t solutions[BUF_SIZE];
} ring_buffer_mem_t;

typedef struct ring_buffer
{
	sem_t *free_sem, *used_sem, *w_sem;
	ring_buffer_mem_t* memory;
} ring_buffer_t;

/**
 * @brief creates a new ring_buffer in shared memory
 * @return a pointer to the new buffer, NULL on failure
 */
ring_buffer_t* new_ring_buffer(void);

/**
 * @brief cleans up all resources used by the buffer
 * @param buffer the buffer to be removed, is unusable after call
 * @return 0 on success. -1 on failure
 */
int clean_buffer(ring_buffer_t* buffer);

/**
 * @brief marks the buffer as closed, does not perform cleanup
 * @param buffer the buffer to close
 * @return 0 on success, -1 on failure
 */
int close_buffer(ring_buffer_t* buffer);

/**
 * @brief perform a write on the given buffer, but block until space is
 * available
 * @param buffer the buffer to write to
 * @param solution  the value to write
 * @return 0 on success, -1 on failure
 */
int block_write(ring_buffer_t* buffer, solution_t solution);

/**
 * @brief perform a read on the given buffer but block until a value was read
 * @param buffer the buffer to read from
 * @param the pointer to hold the read value
 * @return 0 on success, -1 otherwise
 */
int block_read(ring_buffer_t* buffer, solution_t* solution);

#endif  // RING_BUFFER_H