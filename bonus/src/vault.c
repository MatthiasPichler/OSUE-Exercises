#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/cred.h>

#include "../include/secvault.h"
#include "../include/debug.h"

static dev_t first_vault;
static vault_dev_t vaults[MAX_NUM_VAULTS];

/**
 * @brief check if the calling process has permission to access the given vault
 * @param dev the vault device to check
 * @return true if access is allowed, false otherwise
 */
static bool has_permission(const vault_dev_t* dev)
{
	return (dev->creator == current_uid().val);
}

static loff_t sv_llseek(struct file* filp, loff_t off, int whence)
{
	return 0;
}

static ssize_t
sv_read(struct file* filp, char __user* buf, size_t count, loff_t* f_pos)
{
	return 0;
}

static ssize_t
sv_write(struct file* filp, const char __user* buf, size_t count, loff_t* f_pos)
{
	return 0;
}

static long sv_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static int sv_open(struct inode* inode, struct file* filp)
{
	return 0;
}

static int sv_release(struct inode* inode, struct file* filp)
{
	return 0;
}

static struct file_operations sv_fops = {
	.owner = THIS_MODULE,
	.llseek = sv_llseek,
	.read = sv_read,
	.write = sv_write,
	.unlocked_ioctl = sv_ioctl,
	.open = sv_open,
	.release = sv_release,
};


int vault_setup(void)
{
	debug_print("%s\n", "Called vault setup");
	first_vault = MKDEV(MAJ_DEV_NUM, MIN_VAULT_DEV_NUM);
	int err;
	if ((err = register_chrdev_region(
			 first_vault, MAX_NUM_VAULTS, VAULT_DEV_NAME))
		< 0) {
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
	for (int i = 0; i < MAX_NUM_VAULTS; i++) {
		vault_delete(i);
	}

	/* unregister char devices */
	unregister_chrdev_region(first_vault, MAX_NUM_VAULTS);
}

int vault_create(const vault_params_t* params)
{
	debug_print("%s\n", "Called vault create");

	if (params == NULL) {
		debug_print("%s\n", "Params are NULL");
		return -EINVAL;
	}

	if (params->id < 0 || params->id >= MAX_NUM_VAULTS) {
		debug_print("%s: %d\n", "Invalid vault id", params->id);
		return -ENODEV;
	}

	vault_dev_t* vault = &(vaults[params->id]);
	if (vault == NULL) {
		debug_print("%s\n", "Setup error: Vault is NULL");
		return -ENODEV;
	}
	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		return -ERESTART;
	}

	if (vault->data != NULL) {
		debug_print("%s: %d\n", "Vault already contains data", params->id);
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
	vault->creator = current_uid().val;


	int i;
	for (i = 0; i < VAULT_KEY_SIZE; i++) {
		vault->params.key[i] = params->key[i];
	}
	vault->params.key[i] = '\0';

	cdev_init(&vault->cdev, &sv_fops);
	vault->cdev.owner = THIS_MODULE;
	vault->cdev.ops = &sv_fops;
	int err;
	if ((err = cdev_add(&(vault->cdev), first_vault + params->id, 1)) < 0) {
		debug_print("%s\n", "Cdev add failed");
		up(&vault->sem);
		return err;
	}

	up(&vault->sem);
	return 0;
}

int vault_delete(vid_t id)
{
	debug_print("%s\n", "Called vault delete");

	if (id < 0 || id >= MAX_NUM_VAULTS) {
		debug_print("%s: %d\n", "Invalid vault id", id);
		return -ENODEV;
	}

	vault_dev_t* vault = &(vaults[id]);
	if (vault == NULL) {
		debug_print("%s\n", "Setup error: Vault is NULL");
		return -ENODEV;
	}

	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		return -ERESTART;
	}

	if (vault->data == NULL) {
		debug_print("%s: %d\n", "Vault contains no data", id);
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
	vault->creator = -1;

	return 0;
}

int vault_erase(vid_t id)
{
	debug_print("%s\n", "Called vault erase");

	if (id < 0 || id >= MAX_NUM_VAULTS) {
		debug_print("%s: %d\n", "Invalid vault id", id);
		return -ENODEV;
	}

	vault_dev_t* vault = &(vaults[id]);
	if (vault == NULL) {
		debug_print("%s\n", "Setup error: Vault is NULL");
		return -ENODEV;
	}

	if (down_interruptible(&vault->sem) < 0) {
		debug_print("%s\n", "Could not lock semaphore");
		return -ERESTART;
	}

	if (vault->data == NULL) {
		debug_print("%s: %d\n", "Vault contains no data", id);
		up(&vault->sem);
		return -ENODEV;
	}

	if (!has_permission(vault)) {
		debug_print("%s\n", "Permission denied");
		up(&vault->sem);
		return -EACCES;
	}

	if (memset(vault->data, 0, vault->params.max_size) == NULL) {
		up(&vault->sem);
		return -EIO;
	}

	vault->size = 0;
	up(&vault->sem);
	return 0;
}