#include <linux/types.h>
#include <linux/cdev.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/thread_info.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include "../include/secvault.h"

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

static int sv_ioctl(
	struct inode* inode,
	struct file* filp,
	unsigned int cmd,
	unsigned long arg)
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