/**
 * @file common.h
 * @author Matthias Pichler, 01634256
 * @date 2018-04-10
 *
 * @brief Common definitions for OSUE exercise 1B `Battleship'.
 */

// guard block:
#ifndef COMMON_H
#define COMMON_H

#define DEBUG 1

#include <stdint.h>
#include <stdbool.h>

// default hostname and port:
#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "1280"

// Length of each side of the map:
#define MAP_SIZE 10

// Minimum and maximum length of the ships:
#define MIN_SHIP_LEN 2
#define MAX_SHIP_LEN 4

// Number of ships of each length:
#define SHIP_CNT_LEN2 2  // 2 ships of length 2
#define SHIP_CNT_LEN3 3  // 3 ships of length 3
#define SHIP_CNT_LEN4 1  // 1 ship of length 4
// Number of ships in total
#define SHIP_CNT_TOTAL SHIP_CNT_LEN2 + SHIP_CNT_LEN3 + SHIP_CNT_LEN4

// error exit codes
#define EXIT_PARITY_ERR 2
#define EXIT_COORDINATE_ERR 3

// coordinate format paramters
#define COORDINATE_LEN 4
#define COORDINATE_BITS 0x3F
#define X_COORDINATE_OFFSET 6

// Maximum number of rounds after which the client loses the game:
#define MAX_ROUNDS 80

// Suggested values to save information about the squares of the map:
typedef enum
{
	unknown = 0,  // the square has not been targeted yet
	hit = 1,	  // a shot at the square hit a ship
	miss = 2	  // a shot at the square was a miss (thus it is empty)
} hit_t;

// Hit status codes
typedef enum
{
	report_no_hit = 0,	// no hit was made
	report_hit = 1,		  // a ship was hit, but not sunk
	report_sunk = 2,	  // a ship was hit and sunk
	report_last_sunk = 3  // the last ship was hit and sunk
} hit_report_t;

// status codes
typedef enum
{
	game_ongoing = 0x0,  // game is still in progress
	game_over = 0x4,  // the game finished, either by win or by number of rounds

	err_parity = 0x8,	 // the last message contained an invalid parity
	err_coordinate = 0xc  // the last message contained an invalid coordinate
} status_t;

typedef struct
{
	uint8_t row;
	uint8_t col;
} coordinate_t;

extern const coordinate_t invalid_coordinate;

/**
 * @brief checks whether or not the given coordinates lay within the map
 * @param c the coordinate to be checked
 * @return true if c is a valid coordinate, false otherwise
 */
bool check_coordinate(coordinate_t c);

#define debug_print(fmt, ...)       \
	do {                            \
		if (DEBUG)                  \
			fprintf(                \
				stderr,             \
				"%s:%d:%s(): " fmt, \
				__FILE__,           \
				__LINE__,           \
				__func__,           \
				__VA_ARGS__);       \
	} while (0)


#endif  // COMMON_H
