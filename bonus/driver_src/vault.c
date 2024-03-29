/**
 * @file vault.c
 * @author Matthias Pichler, 01634256
 * @date 2018-06-19
 * @brief OSUE bonus exercise
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/cred.h>

#include "../include/secvault.h"
#include "../include/debug.h"

static dev_t first_vault;
static struct vault_dev vaults[MAX_NUM_VAULTS];

/**
 * @brief check if the calling process has permission to access the given vault
 * @param vault the vault device to check
 * @return true if access is allowed, false otherwise
 */
static bool has_permission(const struct vault_dev *vault)
{
	return uid_eq(vault->creator, current_uid());
}

/**
 * @brief zero out the data of the given vault, and set its size to 0
 * @detail this function does not perform any synchronization
 * @param vault the vault device trim
 * @return 0 on success, a negative error code on failure
 */
static int crit_sv_trim(struct vault_dev *vault)
{
	if (memset(vault->data, 0, vault->params.max_size) == NULL)
		return -EIO;

	vault->size = 0;
	return 0;
}

/**
 * @brief read the data from src and xor it with the key before copying it to
 * dest
 * @detail this function does not perform any synchronization
 * @param pos the start position to read from
 * @param src the source to read from
 * @param dest the destination to write to
 * @param key the key to use for de/encryption
 * @param count the number of bytes to copy
 */
static void crit_sv_sym_crypt(loff_t pos, char *src, char *dest, char *key,
			      size_t count)
{
	for (int i = 0; i < count; i++)
		dest[i] = src[i] ^ key[(pos + i) % VAULT_KEY_SIZE];
}

static loff_t sv_llseek(struct file *filp, loff_t off, int whence)
{
	struct vault_dev *vault = filp->private_data;
	loff_t new_pos;

	debug_print("Called seek: offset: %lld, whence: %d\n", off, whence);

	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		return -EACCES;
	}

	switch (whence) {
	case SEEK_SET:
		debug_print("%s\n", "SEEK_SET case");
		new_pos = off;
		break;

	case SEEK_CUR:
		debug_print("%s\n", "SEEK_CUR case");
		new_pos = filp->f_pos + off;
		break;

	case SEEK_END:
		debug_print("%s\n", "SEEK_END case");
		new_pos = vault->size + off;
		break;

	default: /* can't happen */
		return -EINVAL;
	}
	if (new_pos < 0) {
		debug_print("%s: %lld\n", "Negative new position", new_pos);
		return -EINVAL;
	}

	filp->f_pos = new_pos;
	return new_pos;
}

static ssize_t sv_read(struct file *filp, char __user *buf, size_t count,
		       loff_t *f_pos)
{

	struct vault_dev *vault = filp->private_data;
	loff_t curr_pos = *f_pos;
	char *decrypted = NULL;

	debug_print("Called read: count: %lu\n", count);

	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		kfree(decrypted);
		return -EACCES;
	}

	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		kfree(decrypted);
		return -ERESTART;
	}

	if (curr_pos > vault->size || curr_pos < 0) {
		debug_print("%s: %lld\n", "Invalid start position", curr_pos);
		kfree(decrypted);
		up(&vault->sem);
		return -EINVAL;
	}

	if (curr_pos + count > vault->size) {
		debug_print("%s\n",
			    "Count would exceed end of file... trimming");
		count = vault->size - curr_pos;
	}

	decrypted = kmalloc_array(count + 1, sizeof(char), GFP_KERNEL);
	if (decrypted == NULL) {
		debug_print("%s\n", "Memory allocation failed");
		kfree(decrypted);
		up(&vault->sem);
		return -ENOMEM;
	}

	crit_sv_sym_crypt(curr_pos, &(vault->data[curr_pos]), decrypted,
			  vault->params.key, count);

	decrypted[count] = '\0';

	if (copy_to_user(buf, decrypted, count) > 0) {
		debug_print("%s\n", "Copy to userspace failed");
		kfree(decrypted);
		up(&vault->sem);
		return -EFAULT;
	}

	*f_pos += count;

	kfree(decrypted);
	up(&vault->sem);
	return count;
}

static ssize_t sv_write(struct file *filp, const char __user *buf, size_t count,
			loff_t *f_pos)
{

	struct vault_dev *vault = filp->private_data;
	loff_t curr_pos = *f_pos;
	ssize_t add_count = 0;

	debug_print("Called write: count: %lu\n", count);

	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		return -EACCES;
	}

	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		return -ERESTART;
	}

	/* in case of append seek to end of file*/
	if ((filp->f_flags & O_APPEND) != 0)
		curr_pos = sv_llseek(filp, 0, SEEK_END);


	if (curr_pos > vault->size || curr_pos < 0) {
		debug_print("%s: %lld\n", "Invalid start position", curr_pos);
		up(&vault->sem);
		return -EINVAL;
	}

	/* writing will expand current size... calculate addition to size*/
	if (curr_pos + count >= vault->size)
		add_count = (curr_pos + count - vault->size);


	/* writing would exceed end of file*/
	if (vault->size + add_count > vault->params.max_size) {
		debug_print("%s\n", "Writing would exceed end of file");
		up(&vault->sem);
		return -EFBIG;
	}

	if (copy_from_user(&vault->data[curr_pos], buf, count) > 0) {
		debug_print("%s\n", "Copy to user failed");
		up(&vault->sem);
		return -EFAULT;
	}

	crit_sv_sym_crypt(curr_pos, &vault->data[curr_pos],
			  &(vault->data[curr_pos]), vault->params.key, count);
	*f_pos += count;
	vault->size += add_count;

	up(&vault->sem);
	return count;
}

static int sv_open(struct inode *inode, struct file *filp)
{
	struct vault_dev *vault;
	int err;

	debug_print("%s\n", "Called open");
	vault = container_of(inode->i_cdev, struct vault_dev, cdev);
	filp->private_data = vault;

	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		return -EACCES;
	}


	if ((filp->f_flags & O_ACCMODE) == O_WRONLY
	    && (filp->f_flags & O_APPEND) == 0) {
		debug_print("%s\n", "Opened write only, no append");
		if (down_interruptible(&vault->sem) < 0) {
			debug_print("%s\n", "Could not lock semaphore");
			return -ERESTART;
		}

		err = crit_sv_trim(vault);
		if (err < 0) {
			up(&vault->sem);
			return err;
		}
		up(&vault->sem);
	}

	return 0;
}

static int sv_release(struct inode *inode, struct file *filp)
{
	debug_print("%s\n", "Called release");
	if (!has_permission(filp->private_data)) {
		debug_print("%s\n", "Permission denied");
		return -EACCES;
	}
	return 0;
}

static const struct file_operations sv_fops = {
	.owner = THIS_MODULE,
	.llseek = sv_llseek,
	.read = sv_read,
	.write = sv_write,
	.open = sv_open,
	.release = sv_release,
};


int vault_setup(void)
{
	int err;

	debug_print("%s\n", "Called vault setup");

	first_vault = MKDEV(MAJ_DEV_NUM, MIN_VAULT_DEV_NUM);

	err = register_chrdev_region(first_vault, MAX_NUM_VAULTS,
				     VAULT_DEV_NAME);
	if (err < 0) {
		debug_print("%s:%d\n", "Failed to register devices", err);
		return err;
	}

	for (int i = 0; i < MAX_NUM_VAULTS; i++) {
		vaults[i].data = NULL;
		sema_init(&(vaults[i].sem), 1);
	}

	return 0;
}

void vault_cleanup(void)
{
	debug_print("%s\n", "Called vault cleanup");
	for (int i = 0; i < MAX_NUM_VAULTS; i++)
		vault_delete(i);


	/* unregister char devices */
	unregister_chrdev_region(first_vault, MAX_NUM_VAULTS);
}

int vault_create(const struct vault_params *params)
{
	struct vault_dev *vault = NULL;
	int err;

	debug_print("%s\n", "Called vault create");

	if (params == NULL) {
		debug_print("%s\n", "Params are NULL");
		return -EINVAL;
	}

	debug_print("Params: id: %d, size: %lu, key: %s\n", params->id,
		    params->max_size, params->key);

	if (params->id < 0 || params->id >= MAX_NUM_VAULTS) {
		debug_print("%s: %d\n", "Invalid vault id", params->id);
		return -ENODEV;
	}

	vault = &(vaults[params->id]);
	if (vault == NULL) {
		debug_print("%s\n", "Setup error: Vault is NULL");
		return -ENODEV;
	}
	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		return -ERESTART;
	}

	if (vault->data != NULL) {
		debug_print("%s: %d\n", "Vault already contains data",
			    params->id);
		up(&vault->sem);
		return -EEXIST;
	}

	vault->data = kmalloc((params->max_size) * sizeof(char), GFP_KERNEL);
	if (vault->data == NULL) {
		debug_print("%s\n", "Memory allocation failed");
		up(&vault->sem);
		return -ENOMEM;
	}

	vault->size = 0;
	vault->params = *params;
	vault->creator = current_uid();

	for (int i = 0; i < VAULT_KEY_SIZE; i++)
		vault->params.key[i] = params->key[i];


	vault->params.key[VAULT_KEY_SIZE] = '\0';

	cdev_init(&vault->cdev, &sv_fops);
	vault->cdev.owner = THIS_MODULE;
	vault->cdev.ops = &sv_fops;

	err = cdev_add(&(vault->cdev), first_vault + params->id, 1);
	if (err < 0) {
		debug_print("%s\n", "Cdev add failed");
		up(&vault->sem);
		return err;
	}

	up(&vault->sem);
	return 0;
}

int vault_delete(uint8_t id)
{

	struct vault_dev *vault = NULL;

	debug_print("Called delete: %d\n", id);

	if (id < 0 || id >= MAX_NUM_VAULTS) {
		debug_print("%s: %d\n", "Invalid vault id", id);
		return -ENODEV;
	}

	vault = &(vaults[id]);
	if (vault == NULL) {
		debug_print("%s\n", "Setup error: Vault is NULL");
		return -ENODEV;
	}

	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		return -ERESTART;
	}
	if (vault->data == NULL) {
		debug_print("Vault %d contains no data\n", id);
		up(&vault->sem);
		return -ENODEV;
	}
	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		up(&vault->sem);
		return -EACCES;
	}

	cdev_del(&vault->cdev);
	kfree(vault->data);
	vault->data = NULL;
	vault->size = 0;
	vault->creator.val = -1;
	up(&vault->sem);
	return 0;
}

int vault_erase(uint8_t id)
{

	struct vault_dev *vault = NULL;
	int err;

	debug_print("Called erase: %d\n", id);

	if (id < 0 || id >= MAX_NUM_VAULTS) {
		debug_print("%s: %d\n", "Invalid vault id", id);
		return -ENODEV;
	}

	vault = &(vaults[id]);
	if (vault == NULL) {
		debug_print("%s\n", "Setup error: Vault is NULL");
		return -ENODEV;
	}

	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		return -ERESTART;
	}
	if (vault->data == NULL) {
		debug_print("Vault %d contains no data\n", id);
		up(&vault->sem);
		return -ENODEV;
	}
	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		up(&vault->sem);
		return -EACCES;
	}

	err = crit_sv_trim(vault);
	if (err < 0) {
		up(&vault->sem);
		return err;
	}
	up(&vault->sem);
	return 0;
}

long vault_size(uint8_t id)
{
	struct vault_dev *vault = NULL;

	debug_print("Called size: %d\n", id);

	if (id < 0 || id >= MAX_NUM_VAULTS) {
		debug_print("%s: %d\n", "Invalid vault id", id);
		return -ENODEV;
	}

	vault = &(vaults[id]);
	if (vault == NULL) {
		debug_print("%s\n", "Setup error: Vault is NULL");
		return -ENODEV;
	}
	if (vault->data == NULL) {
		debug_print("Vault %d contains no data\n", id);
		return -ENODEV;
	}
	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		return -EACCES;
	}


	return vault->size;
}

int vault_change_key(const struct vault_params *params)
{
	struct vault_dev *vault = NULL;

	debug_print("%s\n", "Called vault change key");

	if (params == NULL) {
		debug_print("%s\n", "Params are NULL");
		return -EINVAL;
	}

	debug_print("Params: id: %d, key: %s\n", params->id, params->key);

	if (params->id < 0 || params->id >= MAX_NUM_VAULTS) {
		debug_print("%s: %d\n", "Invalid vault id", params->id);
		return -ENODEV;
	}

	vault = &(vaults[params->id]);
	if (vault == NULL) {
		debug_print("%s\n", "Setup error: Vault is NULL");
		return -ENODEV;
	}
	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		return -ERESTART;
	}
	if (vault->data == NULL) {
		debug_print("Vault %d contains no data\n", params->id);
		up(&vault->sem);
		return -ENODEV;
	}
	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		up(&vault->sem);
		return -EACCES;
	}

	for (int i = 0; i < VAULT_KEY_SIZE; i++)
		vault->params.key[i] = params->key[i];


	vault->params.key[VAULT_KEY_SIZE] = '\0';


	up(&vault->sem);
	return 0;
}
