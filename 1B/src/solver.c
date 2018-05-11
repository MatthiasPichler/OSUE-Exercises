#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../include/solver.h"
#include "../include/map.h"
#include "../include/deque.h"

const direction_t up = {.d_row = -1, .d_col = 0};
const direction_t down = {.d_row = 1, .d_col = 0};
const direction_t left = {.d_row = 0, .d_col = -1};
const direction_t right = {.d_row = 0, .d_col = 1};

static map_t* map;
static deque_t* target_queue;
static deque_t* hit_queue;
static uint8_t ship_counts[] = {0,
								0,
								SHIP_CNT_LEN2,
								SHIP_CNT_LEN3,
								SHIP_CNT_LEN4};

static bool scan_mode = true;

static int8_t add_targets(coordinate_t coordinate);

static coordinate_t get_random_coordinate(void);
static coordinate_t get_sink_coordinate(
	coordinate_t coordinate,
	hit_report_t hit_report);

static void mark_surroundings(ship_t ship);
static ship_t get_ship_at(coordinate_t coordinate);

static coordinate_t add_direction(coordinate_t c, direction_t d);
static alignment_t get_alignment(coordinate_t c1, coordinate_t c2);

static uint8_t get_max_size(void);
static uint8_t get_min_size(void);

void init_solver(void)
{
	srand(time(NULL));
	map = get_map();
	target_queue = get_deque();
	hit_queue = get_deque();
}

void free_solver(void)
{
	if (map != NULL) {
		free(map);
	}

	if (target_queue != NULL) {
		free(target_queue);
	}

	if (hit_queue != NULL) {
		free(hit_queue);
	}
}

coordinate_t next_move(coordinate_t coordinate, hit_report_t hit_report)
{

	if (!check_coordinate(coordinate)) {
		return get_random_coordinate();
	}

	switch (hit_report) {
		case report_hit:
			put_hit(map, hit, coordinate);
			scan_mode = false;
			if (add_targets(coordinate) < 0) {
				debug_print("%s\n", "Could not add targets");
				exit(EXIT_FAILURE);
			}
			if (push_front(hit_queue, coordinate) < 0) {
				debug_print("%s\n", "Could not add hit");
				exit(EXIT_FAILURE);
			}
			break;
		case report_no_hit:
			put_hit(map, miss, coordinate);
			break;
		case report_sunk:
			// fallthrough
		case report_last_sunk:
			put_hit(map, hit, coordinate);
			ship_t ship = get_ship_at(coordinate);
			ship_counts[ship.length]--;
			mark_surroundings(ship);
			clear(target_queue);
			clear(hit_queue);
			scan_mode = true;
			break;
	}

	if (scan_mode) {
		return get_random_coordinate();
	} else {
		return get_sink_coordinate(coordinate, hit_report);
	}
}

static int8_t add_targets(coordinate_t coordinate)
{
	coordinate_t c;
	c = add_direction(coordinate, up);
	if (check_coordinate(c) && get_hit(map, c) == unknown) {
		if (push_front(target_queue, c) < 0) {
			return -1;
		}
	}
	c = add_direction(coordinate, down);
	if (check_coordinate(c) && get_hit(map, c) == unknown) {
		if (push_front(target_queue, c) < 0) {
			return -1;
		}
	}
	c = add_direction(coordinate, left);
	if (check_coordinate(c) && get_hit(map, c) == unknown) {
		if (push_front(target_queue, c) < 0) {
			return -1;
		}
	}
	c = add_direction(coordinate, right);
	if (check_coordinate(c) && get_hit(map, c) == unknown) {
		if (push_front(target_queue, c) < 0) {
			return -1;
		}
	}
	return 0;
}

static ship_t get_ship_at(coordinate_t coordinate)
{

	uint8_t size = 0;
	alignment_t alignment;
	coordinate_t begin = coordinate;
	coordinate_t end = coordinate;

	begin.row--;
	end.row++;

	if (!check_coordinate(begin)) {
		begin = end;
	} else if (!check_coordinate(end)) {
		begin = end;
	}

	if (get_hit(map, begin) == hit || get_hit(map, end) == hit) {
		alignment = vertical;
	} else {
		alignment = horizontal;
	}

	if (alignment == vertical) {
		while (true) {
			if (check_coordinate(begin)) {
				if (get_hit(map, begin) == hit) {
					begin.row--;
				} else {
					begin.row++;
					break;
				}
			} else {
				begin.row++;
				break;
			}
		}

		end = begin;

		do {
			size++;
			end.row++;
		} while (get_hit(map, end) == hit);
		ship_t ship = {
			.begin = begin, .end = end, .length = size, .alignment = alignment};
		return ship;
	} else {
		while (true) {
			if (check_coordinate(begin)) {
				if (get_hit(map, begin) == hit) {
					begin.col--;
				} else {
					begin.col++;
					break;
				}
			} else {
				begin.col++;
				break;
			}
		}

		end = begin;

		do {
			size++;
			end.col++;
		} while (get_hit(map, end) == hit);
		ship_t ship = {
			.begin = begin, .end = end, .length = size, .alignment = alignment};
		return ship;
	}
}

static void mark_surroundings(ship_t ship)
{
	if (ship.alignment == horizontal) {
		coordinate_t left = ship.begin;
		left.col--;
		coordinate_t right = ship.end;
		right.col++;

		if (check_coordinate(left)) {
			put_hit(map, miss, left);
		}
		if (check_coordinate(right)) {
			put_hit(map, miss, right);
		}

		coordinate_t up = ship.begin;
		up.row--;
		coordinate_t down = ship.begin;
		down.row++;
		for (int i = 0; i < ship.length; i++) {
			if (check_coordinate(up)) {
				put_hit(map, miss, up);
			}
			if (check_coordinate(down)) {
				put_hit(map, miss, down);
			}
			up.col++;
			down.col++;
		}
	} else {
		coordinate_t up = ship.begin;
		up.row--;
		coordinate_t down = ship.end;
		down.row++;

		if (check_coordinate(up)) {
			put_hit(map, miss, up);
		}
		if (check_coordinate(down)) {
			put_hit(map, miss, down);
		}

		coordinate_t left = ship.begin;
		left.col--;
		coordinate_t right = ship.begin;
		right.col++;
		for (int i = 0; i < ship.length; i++) {
			if (check_coordinate(left)) {
				put_hit(map, miss, left);
			}
			if (check_coordinate(right)) {
				put_hit(map, miss, right);
			}
			left.row++;
			right.row++;
		}
	}
}

static coordinate_t get_random_coordinate(void)
{
	uint8_t parity = get_min_size();
	coordinate_t c;

	do {
		uint8_t limit = 10;
		do {
			c.row = rand() % 10;
			c.col = rand() % 10;
			limit--;
		} while ((c.col + c.row) % parity == 0 && limit > 0);

	} while (get_hit(map, c) != unknown);

	return c;
}

static coordinate_t get_sink_coordinate(
	coordinate_t coordinate,
	hit_report_t hit_report)
{

	if (hit_queue->size == get_max_size()) {
		debug_print("%s\n", "Max hit count reached");
		scan_mode = true;
		clear(target_queue);
		clear(hit_queue);
		return get_random_coordinate();
	}


	coordinate_t c, c1, c2;
	alignment_t alignment;

	if (hit_queue->size == 2) {
		c1 = peek_front(hit_queue);
		c2 = peek_back(hit_queue);


		alignment = get_alignment(c1, c2);
		debug_print("Alignment is %d\n", alignment);
		if (alignment == vertical) {
			debug_print("%s\n", "Alignment is vertical");
			push_front(target_queue, add_direction(c1, up));
			push_front(target_queue, add_direction(c1, down));
			push_front(target_queue, add_direction(c2, up));
			push_front(target_queue, add_direction(c2, down));
		} else {
			debug_print("%s\n", "Alignment is horizontal");
			push_front(target_queue, add_direction(c1, left));
			push_front(target_queue, add_direction(c1, right));
			push_front(target_queue, add_direction(c2, left));
			push_front(target_queue, add_direction(c2, right));
		}
	}


	do {
		c = pop_front(target_queue);
		debug_print("c={col=%d,=row%d}\n", c.col, c.row);
	} while (check_coordinate(c) && get_hit(map, c) != unknown);


	if (check_coordinate(c) && get_hit(map, c) == unknown) {
		return c;
	}

	return get_random_coordinate();
}

static coordinate_t add_direction(coordinate_t c, direction_t d)
{
	c.row += d.d_row;
	c.col += d.d_col;
	return c;
}

static alignment_t get_alignment(coordinate_t c1, coordinate_t c2)
{

	if (c1.col != c2.col) {
		return horizontal;
	} else {
		return vertical;
	}
}

static uint8_t get_max_size(void)
{
	for (int i = MAX_SHIP_LEN; i >= MIN_SHIP_LEN; i--) {
		if (ship_counts[i] != 0) {
			return i;
		}
	}
	return 0;
}


static uint8_t get_min_size(void)
{
	for (int i = MIN_SHIP_LEN; i < MAX_SHIP_LEN; i++) {
		if (ship_counts[i] != 0) {
			return i;
		}
	}
	return 0;
}
