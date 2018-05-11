/**
 * @file solver.h
 * @author Matthias Pichler, 01634256
 * @date 2018-04-13
 *
 * @brief handling game tracking and calculation of next move based on current
 * state and reported hits.
 */

#ifndef SOLVER_H
#define SOLVER_H

#include "../include/map.h"
#include "../include/ship.h"

/**
 * @brief a type to describe a direction on the map, by steps in row and column
 * directions
 */
typedef struct
{
	const int8_t d_row;
	const int8_t d_col;
} direction_t;

// constant direction on the map
extern const direction_t up;
extern const direction_t down;
extern const direction_t left;
extern const direction_t right;

/**
 * @brief initialize the solver with an empty map and set all internal state
 * ready for use
 * @details sets the current hit count to 0, initializes the random number
 * generator, creates an empty map and stack
 */
void init_solver(void);

/**
 * @brief free all resources of the solver
 */
void free_solver(void);

/**
 * @brief based on the internal state and the provided information calculate the
 * next move to take
 * @details first record the given hit information on the internal map and then
 * go either in target(trying to sink a ship) or scan mode(firing randomly with
 * a checkerboard pattern).
 * @param coordinate the coordinate of the last shot taken
 * @param hit_report the server feedback of the last shot
 */
coordinate_t next_move(coordinate_t coordinate, hit_report_t hit_report);


#endif