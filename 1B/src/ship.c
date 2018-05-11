/**
 * @file ship.c
 * @author Matthias Pichler, 01634256
 * @date 2018-04-10
 *
 * @brief Ship type for OSUE exercise 1B `Battleship'.
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../include/ship.h"

static int min(int lh, int rh)
{
	return lh < rh ? lh : rh;
}

static int max(int lh, int rh)
{
	return lh > rh ? lh : rh;
}

static int char_to_int(char c)
{
	if ('A' <= c && c <= 'z') {
		return (int)(c - 'A');
	} else if ('0' <= c && c <= '9') {
		return (int)(c - '0');
	} else {
		return -1;
	}
}

ship_t* parse_coordinates(const char* coordinate_str)
{
	if (strlen(coordinate_str) != COORDINATE_LEN) {
		return NULL;
	}

	// ensure that coordinates always go from small to large
	int coordinates[COORDINATE_LEN];
	for (int i = 0; i < COORDINATE_LEN; i++) {
		coordinates[i] = char_to_int(coordinate_str[i]);

		int x = coordinates[i];
		if (x < 0 || x > MAP_SIZE - 1) {
			return NULL;
		}
	}

	coordinate_t begin;

	begin.col = min(coordinates[0], coordinates[2]);
	begin.row = min(coordinates[1], coordinates[3]);

	coordinate_t end;

	end.col = max(coordinates[0], coordinates[2]);
	end.row = max(coordinates[1], coordinates[3]);


	if (begin.row != end.row && begin.col != end.col) {
		// the ship is not contained within 1 row/column
		return NULL;
	}

	int length;
	alignment_t alignment;
	if (begin.col != end.col) {
		length = end.col - begin.col + 1;
		alignment = horizontal;
	} else if (begin.row != end.row) {
		length = end.row - begin.row + 1;
		alignment = vertical;
	} else {
		// ship has no length
		return NULL;
	}

	if (length < MIN_SHIP_LEN || length > MAX_SHIP_LEN) {
		// ship's length is invalid
		return NULL;
	}


	ship_t init = {
		.begin = begin, .end = end, .length = length, .alignment = alignment};

	ship_t* ship = (ship_t*)malloc(sizeof(ship_t));
	if (ship == NULL) {
		// allocation failed
		return NULL;
	}

	memcpy(ship, &init, sizeof(ship_t));
	return ship;
}
