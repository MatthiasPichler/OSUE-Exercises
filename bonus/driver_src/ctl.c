/**
 * @file ctl.c
 * @author Matthias Pichler, 01634256
 * @date 2018-06-19
 * @brief OSUE bonus exercise
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#include "../include/secvault.h"
#include "../include/debug.h"

static dev_t ctl_devno;
static struct cdev ctl_cdev;

static bool flag_init;

static long ctl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct vault_params params;

	flag_init = false;

	switch (cmd) {
	case CMD_CREATE:
		debug_print("%s\n", "Create case");
		if (copy_from_user(&params, (struct vault_params *)arg,
				   sizeof(struct vault_params))
		    > 0) {
			debug_print("%s\n", "Copy from user failed");
			return -EFAULT;
		}
		debug_print("Params: id: %d, size: %lu, key: %s\n", params.id,
			    params.max_size, params.key);
		return vault_create(&params);
	case CMD_DELETE:
		debug_print("%s\n", "Delete case");
		return vault_delete((uint8_t)arg);
	case CMD_ERASE:
		debug_print("%s\n", "Erase case");
		return vault_erase((uint8_t)arg);
	case CMD_CHANGE_KEY:
		debug_print("%s\n", "Change case");
		if (copy_from_user(&params, (struct vault_params *)arg,
				   sizeof(struct vault_params))
		    > 0) {
			debug_print("%s\n", "Copy from user failed");
			return -EFAULT;
		}
		debug_print("Params: id: %d, key: %s\n", params.id, params.key);
		return vault_change_key(&params);
	case CMD_SIZE:
		debug_print("%s\n", "Size case");
		return vault_size((uint8_t)arg);
	default:
		pr_warn("Invalid ioctl command %d", cmd);
		return -EINVAL;
	}
	return -EINVAL;
}

static const struct file_operations ctl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = ctl_ioctl,
};

int ctl_setup(void)
{
	int err;

	debug_print("%s\n", "Called ctl setup");
	ctl_devno = MKDEV(MAJ_DEV_NUM, MIN_CTL_DEV_NUM);

	err = register_chrdev_region(ctl_devno, 1, CTL_DEV_NAME);
	if (err < 0) {
		debug_print("%s:%d\n", "Failed to register devices", err);
		return err;
	}

	cdev_init(&ctl_cdev, &ctl_fops);
	ctl_cdev.owner = THIS_MODULE;
	ctl_cdev.ops = &ctl_fops;
	err = cdev_add(&ctl_cdev, ctl_devno, 1);
	if (err < 0) {
		debug_print("%s:%d\n", "Failed to add devices", err);
		return err;
	}
	flag_init = true;
	return 0;
}

void ctl_cleanup(void)
{
	debug_print("%s\n", "Called ctl cleanup");

	if (flag_init)
		cdev_del(&ctl_cdev);


	unregister_chrdev_region(ctl_devno, 1);
}
