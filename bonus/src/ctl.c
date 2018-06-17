#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>

#include "../include/secvault.h"
#include "../include/debug.h"

static dev_t ctl_devno;
static struct cdev ctl_cdev;
static struct file_operations ctl_fops;

static bool cdev_init = false;

int ctl_setup(void)
{
	debug_print("%s\n", "Called ctl setup");
	ctl_devno = MKDEV(MAJ_DEV_NUM, MIN_CTL_DEV_NUM);
	ctl_fops = get_ctl_fops();
	int err;
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
	cdev_init = true;
	return 0;
}

void ctl_cleanup(void)
{
	debug_print("%s\n", "Called ctl cleanup");
	if (cdev_init) {
		cdev_del(&ctl_cdev);
	}
	unregister_chrdev_region(ctl_devno, 1);
}