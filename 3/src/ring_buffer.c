#include <stdio.h>

#include "../include/ring_buffer.h"

ring_buffer_t* new_ring_buffer(const size_t size)
{
	ring_buffer_t* buffer = (ring_buffer_t*)malloc(sizeof(ring_buffer_t));
	if (buffer != NULL) {
		buffer->size = size;
		buffer->solutions = (solution_t*)malloc(size * sizeof(solution_t));
		if (buffer->solutions == NULL) {
			debug_print("%s\n", "malloc failed");
			return NULL;
		}
	}

	return buffer;
}