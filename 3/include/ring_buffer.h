
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "vertex.h"

typedef struct solution
{
	size_t size;
	edge_t* edges;

} solution_t;

typedef struct ring_buffer
{
	size_t size;
	solution_t* solutions;
} ring_buffer_t;

/**
 * @brief creates a new ring_buffer of the given size
 * @param size the size of the new buffer
 * @return a pointer to the
 */
ring_buffer_t* new_ring_buffer(const size_t size);

#endif  // RING_BUFFER_H