

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

struct file_operations get_vault_fops(void)
{
	struct file_operations sv_fops = {
		.owner = THIS_MODULE,
		.llseek = sv_llseek,
		.read = sv_read,
		.write = sv_write,
		.unlocked_ioctl = sv_ioctl,
		.open = sv_open,
		.release = sv_release,
	};

	return sv_fops;
}

static long ctl_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
	return 0;
}

struct file_operations get_ctl_fops(void)
{

	struct file_operations ctl_fops = {
		.owner = THIS_MODULE,
		.unlocked_ioctl = ctl_ioctl,
	};

	return ctl_fops;
}