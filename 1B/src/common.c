#include "../include/common.h"

bool check_coordinate(coordinate_t c)
{
	return c.row >= 0 && c.col >= 0 && c.row < MAP_SIZE && c.col < MAP_SIZE;
}

const coordinate_t invalid_coordinate = {.row = MAP_SIZE + 1,
										 .col = MAP_SIZE + 1};