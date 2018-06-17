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
#define debug_print(fmt, ...)                  \
	do {                                       \
		if (debug) {                           \
			printk(                            \
				KERN_DEBUG "%s:%d:%s(): " fmt, \
				__FILE__,                      \
				__LINE__,                      \
				__func__,                      \
				__VA_ARGS__);                  \
		}                                      \
	} while (0)

#endif  // DEBUG_H