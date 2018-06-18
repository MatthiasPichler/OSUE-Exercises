#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#include "../include/secvault.h"
#include "../include/debug.h"

static dev_t ctl_devno;
static struct cdev ctl_cdev;

static bool flag_init = false;

static long ctl_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
	vault_params_t params;

	switch (cmd) {
		case CMD_CREATE:
			if (copy_from_user(
					&params, (vault_params_t*)arg, sizeof(vault_params_t))
				< 0) {
				debug_print("%s\n", "Copy from user failed");
				return -EFAULT;
			}
			return vault_create(&params);
			break;
		case CMD_DELETE:
			return vault_delete((vid_t)arg);
			break;
		case CMD_ERASE:
			return vault_erase((vid_t)arg);
			break;
		default:
			printk(KERN_WARNING "Invalid ioctl command %d", cmd);
			return -EINVAL;
	}
	return -EINVAL;
}

static struct file_operations ctl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = ctl_ioctl,
};

int ctl_setup(void)
{
	int err;

	debug_print("%s\n", "Called ctl setup");
	ctl_devno = MKDEV(MAJ_DEV_NUM, MIN_CTL_DEV_NUM);

	if ((err = register_chrdev_region(ctl_devno, 1, CTL_DEV_NAME)) < 0) {
		debug_print("%s:%d\n", "Failed to register devices", err);
		return err;
	}

	cdev_init(&ctl_cdev, &ctl_fops);
	ctl_cdev.owner = THIS_MODULE;
	ctl_cdev.ops = &ctl_fops;
	if ((err = cdev_add(&ctl_cdev, ctl_devno, 1)) < 0) {
		debug_print("%s:%d\n", "Failed to add devices", err);
		return err;
	}
	flag_init = true;
	return 0;
}

void ctl_cleanup(void)
{
	debug_print("%s\n", "Called ctl cleanup");
	if (flag_init) {
		cdev_del(&ctl_cdev);
	}
	unregister_chrdev_region(ctl_devno, 1);
}