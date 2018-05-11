/**
 * @file map.h
 * @author Matthias Pichler, 01634256
 * @date 2018-04-10
 *
 * @brief Map type for OSUE exercise 1B `Battleship'.
 */
#ifndef MAP_H
#define MAP_H

#include "ship.h"
#include "common.h"

// a struct of a ship together with its remaining parts
typedef struct
{
	const ship_t* ship;
	uint8_t ship_remainder;
} ship_entry_t;

// a struct representing the map.
typedef struct
{
	uint8_t ship_count;					 // count of intact ships on the map
	int8_t field[MAP_SIZE * MAP_SIZE];   // an array of all positions of ships
	hit_t hits[MAP_SIZE * MAP_SIZE];	 // an array of all hit states
	ship_entry_t ships[SHIP_CNT_TOTAL];  // an array of all ships

} map_t;

/**
 * @brief add a ship to the given map
 * @details adds a ship to the map, filling the corresponding fields with its
 * index, and incrementing the ship count
 * @param map the map to add the ship to
 * @param ship the ship to add
 */
void add_ship(map_t* map, const ship_t* ship);

/**
 * @brief check if the number of ships per type, corresponds to the rules
 * @param map the map to check
 * @return true if all ships are present according to the rules, false otherwise
 */
bool check_ship_count(const map_t* map);

/**
 * @brief check if any two ships on the map touch each other
 * @param map the map to check
 * @retrun true if no two ships touch, false otherwise
 */
bool check_ship_touch(const map_t* map);

/**
 * @brief print the contents of the map to stdout
 * @param map the map to be printed
 */
void print_map(const map_t* map);

/**
 * @brief shoot at the specified coordinate in the given map
 * @return report_no_hit if the coordinate was already targeted in the past
 * report_hit if there was a ship and it was hit but not sunk
 * report_sunk if the ship at the coordinate was sunk but other ships remain.
 * report_last-sunk it it was the last ship and it was sunk
 */
hit_report_t shoot(map_t* map, coordinate_t c);

/**
 * @brief get the hit status of the map at the given coordinate
 * @param map the map to query
 * @param coordinate the coordinate to look at, has to be valid
 * @return the state of the field, if it was hit and if it was missed
 */
hit_t get_hit(const map_t* map, coordinate_t coordinate);

/**
 * @brief sets the hit status of the map at the given coordinate to the given
 * value
 * @param map the map to update
 * @param value the value to write to the map
 * @param coordinate the coordinate to update at, must be valid
 */
void put_hit(map_t* map, hit_t value, coordinate_t coordinate);

/**
 * @brief create a new map with all arrays initialized with their default values
 * @return a fully initialized map
 */
map_t* get_map(void);

#endif  // MAP_H