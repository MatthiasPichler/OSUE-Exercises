#ifndef SECVAULT_H
#define SECVAULT_H

#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/fs.h>

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
 * @brief struct that holds any parameters for vault creation, these are also
 * kept within the device structure
 */
typedef struct vault_params
{
	vid_t id;  // the id of the vault, from 0 to MAX_NUM_VAULTS
	char key[VAULT_KEY_SIZE + 1];  // the encryption key of the vault
	size_t max_size;			   // the size that this vault was created with
} vault_params_t;

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
 * @return 0 on success, -1 otherwise
 */
int ctl_setup(void);

/**
 * @brief cleanup any resources used by the control device, called on exit
 */
void ctl_cleanup(void);

/**
 * @brief setup all resources needed for vault creation
 * @return 0 on success, -1 on failure
 */
int vault_setup(void);

/**
 * @brief cleanup any resources used by the vaults, called on exit
 */
void vault_cleanup(void);

/**
 * @brief create a new vault with the given paramters
 * @param params the creation paramters for this vault
 * @return 0 on success, -1 on failure
 */
int vault_create(const vault_params_t* params);

/**
 * @brief delete the vault with the given id
 * @param id the id of the vault to delete
 * @return 0 on success, -1 on failure
 */
int vault_delete(vid_t id);

/**
 * @brief erase all data of the vault with the given id
 * @param id the id of the vault to erase
 * @return 0 on success, -1 on failure
 */
int vault_erase(vid_t id);

// register unique IOCTL commands
#define CMD_CREATE _IOR(MAJ_DEV_NUM, 0, vault_params_t)
#define CMD_DELETE _IOR(MAJ_DEV_NUM, 1, vid_t)
#define CMD_ERASE _IOR(MAJ_DEV_NUM, 2, vid_t)

#endif /* SECVAULT_H */
