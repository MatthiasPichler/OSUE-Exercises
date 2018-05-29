
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

#include "vertex.h"

typedef struct solution
{
	uint8_t size;
	edge_t* edges;

} solution_t;

typedef struct ring_buffer
{
	solution_t* solutions;
} ring_buffer_t;

#endif  // RING_BUFFER_H