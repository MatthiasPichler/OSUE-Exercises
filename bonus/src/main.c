// module stuff
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "../include/debug.h"
#include "../include/secvault.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Matthias Pichler <e1634256@student.tuwien.ac.at>");
MODULE_DESCRIPTION("Secure Vault");
MODULE_VERSION("1.1");

static dev_t device;

module_param(debug, bool, S_IRUGO);
MODULE_PARM_DESC(debug, "Enable debugging");

static void __exit sv_exit(void);
static int __init sv_init(void);


static int __init sv_init(void)
{
	debug_print("%s\n", "Init function called");
	debug_print("%s\n", "Registering devices...");

	device = MKDEV(MAJ_DEV_NUM, 0);
	int err;
	if ((err = register_chrdev_region(device, MAX_NUM_VAULTS, "sv_data")) < 0) {
		debug_print("%s: %d\n", "Failed to register devices", err);
		sv_exit();  // FAILS because of __exit macro
		return err;
	}

	debug_print("%s\n", "Devices registered");

	return 0;
}

static void __exit sv_exit(void)
{
	debug_print("%s\n", "Unregistering devices...");
	unregister_chrdev_region(device, MAX_NUM_VAULTS);
	debug_print("%s\n", "Devices unregistered");
}


module_init(sv_init);
module_exit(sv_exit);
