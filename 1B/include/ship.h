/**
 * @file ship.h
 * @author Matthias Pichler, 01634256
 * @date 2018-04-10
 *
 * @brief Ship type for OSUE exercise 1B `Battleship'.
 */
#ifndef SHIP_H
#define SHIP_H

#include "common.h"

typedef enum
{
	horizontal = 0,
	vertical = 1,
} alignment_t;

typedef struct
{
	const coordinate_t begin;
	const coordinate_t end;
	const uint8_t length;
	const alignment_t alignment;
} ship_t;

ship_t* parse_coordinates(const char* coordinate_str);

#endif  // SHIP_H