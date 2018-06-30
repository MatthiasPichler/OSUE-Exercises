/**
 * @file common.h
 * @author Matthias Pichler, 01634256
 * @date 2018-06-21
 * @brief OSUE bonus exercise
 */

#ifndef COMMON_H
#define COMMON_H


#include <linux/ioctl.h>

#ifdef MODULE
#include <linux/types.h>
#else
#include <stdlib.h>
#include <stdint.h>
#endif

#define MAJ_DEV_NUM 231
#define MIN_CTL_DEV_NUM 0
#define CTL_DEV_NAME "sv_ctl"
#define MIN_VAULT_DEV_NUM 1
#define VAULT_DEV_NAME "sv_data"
#define MAX_NUM_VAULTS 4
#define MAX_VAULT_SIZE (1024 * 1024)

#define VAULT_KEY_SIZE 10

/**
 * @brief struct that holds any parameters for vault creation, these are also
 * kept within the device structure
 */
struct vault_params {
	uint8_t id;
	char key[VAULT_KEY_SIZE + 1];
	size_t max_size;
};

#define CMD_CREATE _IOR(MAJ_DEV_NUM, 0, struct vault_params *)
#define CMD_DELETE _IOW(MAJ_DEV_NUM, 1, uint8_t)
#define CMD_ERASE _IOW(MAJ_DEV_NUM, 2, uint8_t)
#define CMD_SIZE _IOR(MAJ_DEV_NUM, 3, uint8_t)
#define CMD_CHANGE_KEY _IOW(MAJ_DEV_NUM, 4, struct vault_params *)

#endif /* COMMON_H */
