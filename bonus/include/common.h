/**
 * @file secvault.h
 * @author Matthias Pichler, 01634256
 * @date 2018-06-21
 * @brief OSUE bonus exercise
 */

#ifndef COMMON_H
#define COMMON_H


#include <linux/ioctl.h>

#ifdef KERNEL
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
#define MAX_VAULT_SIZE 1048576

#define VAULT_KEY_SIZE 10

typedef uint8_t vid_t;

/**
 * @brief describes the different actions to be performed on a vault
 */
typedef enum vault_action
{
	create,
	erase,
	delete,
	print_size,
	change_key
} vault_action_t;

/**
 * @brief struct that holds any parameters for vault creation, these are also
 * kept within the device structure
 */
typedef struct vault_params
{
	vid_t id;  // the id of the vault, from 0 to MAX_NUM_VAULTS
	char key[VAULT_KEY_SIZE + 1];  // the encryption key of the vault
	size_t max_size;			   // the size that this vault was created with
} vault_params_t;

// register unique IOCTL commands
#define CMD_CREATE _IOR(MAJ_DEV_NUM, 0, vault_params_t*)
#define CMD_DELETE _IOR(MAJ_DEV_NUM, 1, vid_t)
#define CMD_ERASE _IOR(MAJ_DEV_NUM, 2, vid_t)
#define CMD_SIZE _IOR(MAJ_DEV_NUM, 3, vid_t)
#define CMD_CHANGE_KEY _IOR(MAJ_DEV_NUM, 4, vault_params_t*)

#endif /* COMMON_H */
