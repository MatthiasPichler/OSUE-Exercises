/**
 * @file deque.h
 * @author Matthias Pichler, 01634256
 * @date 2018-04-13
 *
 * @brief A simple deque data structure for coordinates based on a singly linked
 * list.
 */
#ifndef DEQUE_H
#define DEQUE_H


#include <stdlib.h>
#include "../include/common.h"

/**
 * @brief an entry within the deque, keeping track of the data and the next
 * element
 */
typedef struct node
{
	struct node *next;
	coordinate_t data;
} node_t;

/**
 * @brief type for a simple deque
 */
typedef struct
{
	size_t size;
	node_t *head;
	node_t *tail;
} deque_t;

/**
 * @brief create a new deque, with size 0
 * @return a pointer to a new deque
 */
deque_t *get_deque(void);

/**
 * @brief push the provided coordinate on the given deque, incrementing the size
 * @return 0 if the push was successful, -1 otherwise
 */
int8_t push_front(deque_t *deque, coordinate_t c);

/**
 * @brief pop the top element of the deque, removing it from there, and
 * decrementing the size
 * @return the coordinate of the top element
 */
coordinate_t pop_front(deque_t *deque);

/**
 * @brief get top element of the deque, without removing it
 * @return the coordinate of the top element
 */
coordinate_t peek_front(deque_t *deque);

/**
 * @brief push the provided coordinate on the back of the given deque,
 * incrementing the size
 * @return 0 if the push was successful, -1 otherwise
 */
int8_t push_back(deque_t *deque, coordinate_t c);

/**
 * @brief pop the last element of the deque, removing it from there, and
 * decrementing the size
 * @return the coordinate of the top element
 */
coordinate_t pop_back(deque_t *deque);

/**
 * @brief get the lasts element of the deque, without removing it
 * @return the coordinate of the top element
 */
coordinate_t peek_back(deque_t *deque);

/**
 * @brief checks if the given element is present in the deque
 * @param deque the deque to search
 * @param element the element to search for
 * @return true if such an element is present, else otherwise
 */
bool contains(const deque_t *deque, coordinate_t element);

/**
 * @brief remove every item from the given deque, setting its size to 0
 */
void clear(deque_t *deque);

#endif  // DEQUE_H
