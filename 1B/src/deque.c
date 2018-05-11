#include <stdlib.h>
#include <stdio.h>
#include "../include/common.h"
#include "../include/deque.h"


deque_t *get_deque(void)
{
	deque_t *deque = (deque_t *)malloc(sizeof(deque_t));
	if (deque == NULL) {
		return NULL;
	}
	deque->size = 0;
	deque->head = NULL;
	deque->tail = NULL;

	return deque;
}

int8_t push_front(deque_t *deque, coordinate_t c)
{
	node_t *node = (node_t *)malloc(sizeof(node_t));
	if (node == NULL) {
		debug_print("%s\n", "Could not push front");
		return -1;
	}

	node->data = c;

	if (deque->size == 0) {
		deque->head = node;
		deque->tail = node;
		deque->size = 1;
	} else {
		node->next = deque->head;
		deque->head = node;
		deque->size++;
	}


	return 0;
}

coordinate_t pop_front(deque_t *deque)
{

	if (deque->size == 0) {
		return invalid_coordinate;
	}

	node_t *old = deque->head;
	coordinate_t c = old->data;

	deque->head = old->next;
	free(old);
	deque->size--;
	return c;
}

coordinate_t peek_front(deque_t *deque)
{
	if (deque->size == 0) {
		return invalid_coordinate;
	} else {
		return deque->head->data;
	}
}

int8_t push_back(deque_t *deque, coordinate_t c)
{
	node_t *node = (node_t *)malloc(sizeof(node_t));
	if (node == NULL) {
		return -1;
	}
	node->data = c;

	if (deque->size == 0) {
		deque->head = node;
		deque->tail = node;
		deque->size = 1;
	} else {
		deque->tail->next = node;
		deque->tail = node;
		deque->size++;
	}


	return 0;
}

coordinate_t pop_back(deque_t *deque)
{

	if (deque->size == 0) {
		return invalid_coordinate;
	}

	node_t *old = deque->tail;
	coordinate_t c = old->data;

	node_t *n;
	for (n = deque->head; n->next != deque->tail; n = n->next) {
	}
	n->next = NULL;
	deque->tail = n;

	free(old);
	deque->size--;

	return c;
}

coordinate_t peek_back(deque_t *deque)
{
	if (deque->size == 0) {
		return invalid_coordinate;
	} else {
		return deque->tail->data;
	}
}

bool contains(const deque_t *deque, coordinate_t element)
{
	for (node_t *n = deque->head; n->next != NULL; n = n->next) {
		if (n->data.col == element.col && n->data.row == element.row) {
			return true;
		}
	}

	return false;
}

void clear(deque_t *deque)
{
	while (deque->size > 0) {
		pop_front(deque);
	}
}