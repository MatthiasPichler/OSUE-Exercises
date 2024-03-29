/**
 * @file debug.h
 * @author Matthias Pichler, 01634256
 * @date 2018-06-19
 * @brief OSUE bonus exercise
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <linux/types.h>
#include <linux/kernel.h>

extern bool debug;

/**
 * @brief print debug messages to KERN_DEBUG,when debug module paramters was set
 * to true
 * @param fmt the format string to print
 * @param varargs the values for the format string
 */
#define debug_print(fmt, ...)                                                  \
	do {                                                                   \
		if (debug) {                                                   \
			pr_debug("%s:%d:%s(): " fmt, __FILE__, __LINE__,       \
				 __func__, __VA_ARGS__);                       \
		}                                                              \
	} while (0)

#endif /*DEBUG_H*/
