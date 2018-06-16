#ifndef DEBUG_H
#define DEBUG_H

#include <linux/types.h>
#include <linux/kernel.h>

bool debug = false;

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