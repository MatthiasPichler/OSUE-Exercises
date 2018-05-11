/**
 * @file map.begin
 * @author Matthias Pichler, 01634256
 * @date 2018-04-10
 *
 * @brief Map type for OSUE exercise 1B `Battleship'.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "../include/map.h"


static inline void put_ship(map_t* map, uint8_t value, coordinate_t coordinate)
{
	map->field[coordinate.row * MAP_SIZE + coordinate.col] = value;
}

static inline int8_t get_ship(const map_t* map, coordinate_t coordinate)
{
	return map->field[coordinate.row * MAP_SIZE + coordinate.col];
}

hit_t get_hit(const map_t* map, coordinate_t coordinate)
{
	return map->hits[coordinate.row * MAP_SIZE + coordinate.col];
}

void put_hit(map_t* map, hit_t value, coordinate_t coordinate)
{
	map->hits[coordinate.row * MAP_SIZE + coordinate.col] = value;
}

void add_ship(map_t* map, const ship_t* ship)
{
	uint8_t value = map->ship_count;


	if (ship->alignment == horizontal) {

		for (int i = ship->begin.col; i <= ship->end.col; i++) {
			coordinate_t c = {.row = ship->begin.row, .col = i};
			put_ship(map, value, c);
		}

	} else {

		for (int i = ship->begin.row; i <= ship->end.row; i++) {
			coordinate_t c = {
				.row = i,
				.col = ship->begin.col,
			};
			put_ship(map, value, c);
		}
	}

	map->ships[value].ship = ship;
	map->ships[value].ship_remainder = ship->length;
	map->ship_count++;
}

bool check_ship_count(const map_t* map)
{
	int ship_cnt_2 = SHIP_CNT_LEN2;
	int ship_cnt_3 = SHIP_CNT_LEN3;
	int ship_cnt_4 = SHIP_CNT_LEN4;

	for (int i = 0; i < map->ship_count; i++) {
		switch (map->ships[i].ship->length) {
			case 2:
				ship_cnt_2--;
				break;
			case 3:
				ship_cnt_3--;
				break;
			case 4:
				ship_cnt_4--;
				break;
			default:
				return false;
		}
	}

	return ship_cnt_2 == 0 && ship_cnt_3 == 0 && ship_cnt_4 == 0;
}


bool check_ship_touch(const map_t* map)
{
	// for each ship
	for (int s = 0; s < map->ship_count; s++) {
		const ship_t* ship = map->ships[s].ship;

		int8_t neighbors[9];

		coordinate_t c = ship->begin;
		// for each coordinate
		for (int l = 0; l < 2; l++) {
			// for each row
			for (int i = -1; i <= 1; i++) {
				// for each column
				for (int j = -1; j <= 1; j++) {

					int8_t x = c.row + i;
					int8_t y = c.col + j;

					if (x < 0 || x > MAP_SIZE - 1) {
						x = c.row;
					}
					if (y < 0 || y > MAP_SIZE - 1) {
						y = c.col;
					}

					int idx = 3 * i + j + 4;
					coordinate_t c = {.row = x, .col = y};
					neighbors[idx] = get_ship(map, c);
				}
			}
			// for each neighbor
			for (int i = 0; i < 9; i++) {
				if (neighbors[i] != -1 && neighbors[i] != s) {
					return false;
				}
			}

			c = ship->end;
		}
	}

	return true;
}


/**
 * @brief Print a map showing the squares where ships have been hit.
 * @param map the map structure containing the targeted ships
 *
 */
void print_map(const map_t* map)
{
	int x, y;

	printf("  ");
	for (x = 0; x < MAP_SIZE; x++) {
		printf("%c ", 'A' + x);
	}

	printf("\n");

	for (y = 0; y < MAP_SIZE; y++) {
		printf("%c ", '0' + y);
		for (x = 0; x < MAP_SIZE; x++) {
			coordinate_t c = {.row = y, .col = x};
			hit_t val = get_hit(map, c);
			printf("%c ", val == unknown ? ' ' : ((val == hit) ? 'x' : 'o'));
		}

		printf("\n");
	}
}

hit_report_t shoot(map_t* map, coordinate_t c)
{
	if (get_hit(map, c) != unknown) {
		// field was already targeted
		debug_print("%s\n", "Field already targeted");
		return report_no_hit;
	}

	int8_t val = get_ship(map, c);
	if (val == -1) {
		// no ship at this position
		debug_print("%s\n", "Miss");
		put_hit(map, miss, c);
		return report_no_hit;
	}

	map->ships[val].ship_remainder--;
	put_hit(map, hit, c);

	if (map->ships[val].ship_remainder > 0) {
		debug_print(
			"Ship not sunk. Remainder: %d\n", map->ships[val].ship_remainder);
		return report_hit;
	} else {
		map->ship_count--;
		if (map->ship_count > 0) {
			debug_print("%s\n", "Ship sunk");
			return report_sunk;
		} else {
			debug_print("%s\n", "Last ship sunk");
			return report_last_sunk;
		}
	}
}

map_t* get_map(void)
{
	map_t* map = (map_t*)malloc(sizeof(map_t));
	if (map == NULL) {
		return NULL;
	}

	for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
		map->field[i] = -1;
		map->hits[i] = unknown;
	}
	map->ship_count = 0;
	return map;
}