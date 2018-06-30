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
struct vault_dev {
	char *data;
	struct vault_params params;
	size_t size;
	struct semaphore sem;
	struct cdev cdev;
	kuid_t creator;
};

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
int vault_create(const struct vault_params *params);

/**
 * @brief delete the vault with the given id
 * @param id the id of the vault to delete
 * @return 0 on success, a negative error code on failure
 */
int vault_delete(uint8_t id);

/**
 * @brief erase all data of the vault with the given id
 * @param id the id of the vault to erase
 * @return 0 on success, a negative error code on failure
 */
int vault_erase(uint8_t id);

/**
 * @brief get the current size of a vault
 * @param id the id of the vault
 * @return the size on success, a negative error code on failure
 */
long vault_size(uint8_t id);

/**
 * @brief change the key of a vault and re-encrypt the data
 * @param params the change paramters for this vault
 * @return 0 on success, a negative error code on failure
 */
int vault_change_key(const struct vault_params *params);


#endif /* SECVAULT_H */
