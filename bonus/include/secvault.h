/**
 * @file secvault.h
 * @author Matthias Pichler, 01634256
 * @date 2018-06-19
 * @brief OSUE bonus exercise
 */

#ifndef SECVAULT_H
#define SECVAULT_H

#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/fs.h>

#include "common.h"

/**
 * @brief struct to define a secvault device
 */
typedef struct vault_dev
{
	char* data;				// the data stored in the vault at any given time
	vault_params_t params;  // the paramters this vault was created with
	size_t size;			// the current size of the vault
	struct semaphore sem;  // a semaphore to ensure exclusive access to the data
	struct cdev cdev;	  // the actual charater device
	uid_t creator;		   // the user id of the creator of the vault
} vault_dev_t;

/**
 * @brief setup the control device of the secvaults
 * @return 0 on success, a negative error code on failure
 */
int ctl_setup(void);

/**
 * @brief cleanup any resources used by the control device, called on exit
 */
void ctl_cleanup(void);

/**
 * @brief setup all resources needed for vault creation
 * @return 0 on success, a negative error code on failure
 */
int vault_setup(void);

/**
 * @brief cleanup any resources used by the vaults, called on exit
 */
void vault_cleanup(void);

/**
 * @brief create a new vault with the given paramters
 * @param params the creation paramters for this vault
 * @return 0 on success, a negative error code on failure
 */
int vault_create(const vault_params_t* params);

/**
 * @brief delete the vault with the given id
 * @param id the id of the vault to delete
 * @return 0 on success, a negative error code on failure
 */
int vault_delete(vid_t id);

/**
 * @brief erase all data of the vault with the given id
 * @param id the id of the vault to erase
 * @return 0 on success, a negative error code on failure
 */
int vault_erase(vid_t id);

/**
 * @brief get the current size of a vault
 * @param id the id of the vault
 * @return the size on success, a negative error code on failure
 */
long vault_size(vid_t id);

/**
 * @brief change the key of a vault and re-encrypt the data
 * @param params the change paramters for this vault
 * @return 0 on success, a negative error code on failure
 */
int vault_change_key(const vault_params_t* params);


#endif /* SECVAULT_H */
