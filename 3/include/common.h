/**
 * @file common.h
 * @author Matthias Pichler, 01634256
 * @date 2018-05-29
 * @brief OSUE Exercise 3
 */

#ifndef COMMON_H
#define COMMON_H

#define DEBUG 1

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